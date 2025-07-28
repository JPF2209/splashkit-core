// gpio_driver.cpp
// This file is part of the SplashKit Core Library.
// Copyright (©) 2024 Aditya Parmar. All Rights Reserved.

#include "network_driver.h"
#include "gpio_driver.h"
#include "easylogging++.h"

#include <string>
#include <iostream>
#include <cstdlib> // Add this line to include the necessary header for the exit() function

#include <unistd.h>
#include <cstring>
#ifdef RASPBERRY_PI
#include <wiringPi.h>
#include <unordered_map>
#include <wiringPiSPI.h>
#include <wiringPiI2C.h>

#define LOG(x) std::cerr

#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif



using namespace std;
// Use https://abyz.me.uk/rpi/pigpio/pdif2.html for local command reference
//   Archive Link: https://web.archive.org/web/20240423160241/https://abyz.me.uk/rpi/pigpio/pdif2.html
//
// Use https://abyz.me.uk/rpi/pigpio/sif.html for remote command reference
//   Archive Link: https://web.archive.org/web/20240423160319/https://abyz.me.uk/rpi/pigpio/sif.html
namespace splashkit_lib
{
    //Add map to track items for remote gpio
    std::unordered_map<int, int> r_pin_modes;
    std::unordered_map<int, int> r_pwm_range;
    std::string username;
    std::string ip;

    #ifdef RASPBERRY_PI
    int pi = -1;
    //Add map to track items for RPi GPIO
    std::unordered_map<int, int> pin_modes;
    std::unordered_map<int, int> pwm_range;
    std::unordered_map<int, int> handle_channel;

    // Check if wiringPiSetupGpio() has been called before any other GPIO functions
    bool check_pi()
    {
        if (pi < 0)
        {
            LOG(ERROR) << sk_gpio_error_message(pi);
            return false;
        }
        else return true;
    }

    // Initialize the GPIO library
    int sk_gpio_init()
    {
        if (wiringPiSetupGpio() == -1)
        {
            LOG(ERROR) << sk_gpio_error_message(pi);
            return 1;
        }
        pi = wiringPiSetupGpio();
        return pi;
    }

    // Read the value of a GPIO pin
    int sk_gpio_read(int pin)
    {
        if (check_pi())
        {
            //Checks whether the pins are in the correct range
            if (pin < 0 || pin > 40) 
            { 
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_GPIO);
                return -1;
            }
            int result = digitalRead(pin);
            //Verifies if a result is produced or not
            if (result < 0)
            {
                LOG(ERROR) << sk_gpio_error_message(result);
                return -1;
            }
            return result;
        }
        else
        {
            return GPIO_DEFAULT_VALUE;
        }
    }

    // Write a value to a GPIO pin
    void sk_gpio_write(int pin, int value)
    {
        if (check_pi())
        {
            //Checks whether the pins are in the correct range
            if (pin < 0 || pin > 40) 
            { 
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_GPIO);
                return;
            }
            //Checks if the value exists in the SplashKit library or not
            if (value < -1 || value > 2)
            {
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_GPIO);
                return;
            }
            digitalWrite(pin, value);
        }
    }

    // Set the mode of a GPIO pin
    void sk_gpio_set_mode(int pin, int mode)
    {
        if(check_pi())
        {
            //Checks whether the pins are in the correct range
            if (pin < 0 || pin > 40) 
            { 
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_GPIO);
                return;
            }
            //Checks if the value exists in the SplashKit library or not
            if (mode < 0 || mode > 7)
            {
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_MODE);
                return;
            }
            pinMode(pin, mode); 
            pin_modes[pin] = mode;
        }
    }

    // Get the mode of a GPIO pin
    int sk_gpio_get_mode(int pin)
    {
        if(check_pi())
        {
            //Checks whether the pins are in the correct range
            if (pin < 0 || pin > 40) 
            { 
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_GPIO);
                return -1;
            }
            int mode = pin_modes.count(pin) ? pin_modes[pin] : -1;
            return mode;
        }
        else
        {
            return PI_BAD_GPIO; 
        }
    }

    //Description
    void sk_gpio_set_pull_up_down(int pin, int pud)
    {
        //Checks whether the pins are in the correct range
        if(check_pi())
        {
            //Checks whether the pins are in the correct range
            if (pin < 0 || pin > 40) 
            { 
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_GPIO);
                return;
            }
            //Checks if the pud exists in the SplashKit library or not
            if (pud < 0 || pud > 2)
            {
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_PUD);
                return;
            }
            pinMode(pin, INPUT);
            pullUpDnControl(pin, pud);
        }
    }

    //Needs to be set before frequency and dutycycle
    void sk_set_pwm_range(int pin, int range)
    {
        if(check_pi())
        {
            //Checks whether the pins are in the correct range
            if (pin < 0 || pin > 40) 
            { 
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_GPIO);
                return;
            }
            //Checks whether newly set range is a reasonable value
            if (range <= 25 || range > 4096) 
            { 
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_DUTYRANGE);
                return;
            }
            //Save values to map to use for other functions (pigpio did this automatically)
            pinMode(pin, PWM_OUTPUT); 
            pin_modes[pin] = PWM_OUTPUT;
            pwmSetMode(PWM_MODE_MS);
            pwmSetRange(range);
            pwm_range[pin] = range;
        }
    }

    // Set frequency by setting both the range & clock
    void sk_set_pwm_frequency(int pin, int frequency)
    {
        if(check_pi())
        {
            //Checks whether the pins are in the correct range
            if (pin < 0 || pin > 40) 
            { 
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_GPIO);
                return;
            }
            int range = pwm_range[pin];
            //Checks if range exists in the map of know PWM ranges
            if (range < 25)
            {
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_DUTYRANGE);
                return;
            }
            // Find out what the clock divisor is using base clock, frequency and range
            double divisor = static_cast<double>(BASE_CLOCK) / (frequency * range);
            int clock_divisor = static_cast<int>(divisor + 0.5);
            //Checks if the new frequency is in a safe limit
            if ((range / clock_divisor) > 38400)
            {
                LOG(ERROR) << sk_gpio_error_message(-1);
                return;
            }
            pwmSetRange(range);
            pwmSetClock(clock_divisor);
        }
    }

    //Value must not be more than range (0% to 100%)
    void sk_set_pwm_dutycycle(int pin, int dutycycle)
    {
        if(check_pi())
        {
            //Checks whether the pins are in the correct range
            if (pin < 0 || pin > 40) 
            { 
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_GPIO);
                return;
            }
            int range = pwm_range[pin];
            //Checks if range exists in the map of know PWM ranges
            if (range < 25)
            {
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_DUTYRANGE);
                return;
            }
            //Check if dutycycle is less than range (percentage of cycle from 0 to 100% (range))
            else if (range < dutycycle)
            {
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_DUTYCYCLE);
                return;
            }
            pwmWrite(pin, dutycycle);
        }
    }

    void sk_gpio_clear_bank_1()
    {
        if(check_pi())
        {
            // Manually go through each pin and reset it to 0 (LOW)
            for (int pin = 0; pin <= PI_SIZE; ++pin)
            {
                if (PI4B_GPIO_BITMASK && (1 << pin))
                {
                    int currentPin = pin;
                    pinMode(pin, OUTPUT);
                    digitalWrite(pin, LOW);
                    pin_modes[pin] = LOW;
                }
            }
        }
    }

    //Delete function sk_gpio_cleanup since this cleans up everytime the RPi switches off (no predefined function for it)
    
    // WiringPi's version of spi_open doesn't need the variable flag so I removed it
    int sk_spi_open(int channel, int speed)
    {
        if(check_pi())
        {
            //Checks whether the channel is in the correct range
            if (channel < 0 || channel > 2) 
            { 
                LOG(ERROR) << sk_gpio_error_message(PI_BAD_GPIO);
                return -1;
            }
            int handle = wiringPiSPISetup(channel, speed);
            //Save handle to unordered map
            handle_channel[handle] = channel;
            return handle;
        }
        else
        {
            return -1;
        }
    }

    int sk_spi_close(int handle)
    {
        if(check_pi())
        {
            //Close SPI & reset handle value to 0
            close(handle); 
            handle_channel[handle] = 0;
            return 0;
        }
        else
        {
            return -1;
        }
    }

    // Open I2C device
    int sk_i2c_open(int bus, int address, int flags) {
        if (check_pi()) {
            int handle = wiringPiI2CSetup(address);
            if (handle < 0) {
                LOG(ERROR) << "Failed to open I2C device at address " << address << "\n";
            }
            return handle;
        }
        return -1;
    }

    int sk_i2c_read_byte(int handle)
    {
        // Assuming check_pi() ensures this is running on a Pi and initialized correctly
        if (check_pi())
        {
            int result = wiringPiI2CRead(handle);
            if (result < 0)
            {
                LOG(ERROR) << "I2C Read Error: " << result;  // Replace with your error handling
            }
            return result;
        }
        else
        {
            return -1;
        }
    }

    int sk_i2c_write_byte(int handle, int data)
    {
        if (check_pi())
        {
            int result = wiringPiI2CWrite(handle, data);
            if (result < 0)
            {
                LOG(ERROR) << "I2C Write Error: " << result;  // Replace with your error handling if needed
            }
            return result;
        }
        else
        {
            return -1;
        }
    }

    int sk_i2c_read_byte_data(int handle, int reg)
    {
        if (check_pi())
        {
            int result = wiringPiI2CReadReg8(handle, reg);
            if (result < 0)
            {
                LOG(ERROR) << "I2C ReadReg Error (reg " << reg << "): " << result;
            }
            return result;
        }
        else
        {
            return -1;
        }
    }

    void sk_i2c_write_byte_data(int handle, int reg, int data)
    {
        if (check_pi())
        {
            int result = wiringPiI2CWriteReg8(handle, reg, data);
            if (result < 0)
            {
                LOG(ERROR) << "I2C WriteReg Error (reg " << reg << ", data " << data << "): " << result;
            }
        }
    }

    int sk_i2c_read_word_data(int handle, int reg)
    {
        if (check_pi())
        {
            int result = wiringPiI2CReadReg16(handle, reg);
            if (result < 0)
            {
                LOG(ERROR) << "I2C ReadWord Error (reg " << reg << "): " << result;
            }
            return result;
        }
        else
        {
            return -1;
        }
    }

    void sk_i2c_write_word_data(int handle, int reg, int data)
    {
        if (check_pi())
        {
            int result = ::i2c_write_word_data(pi, handle, reg, data);
            if (result < 0)
            {
                LOG(ERROR) << sk_gpio_error_message(result);
            }
        }
    }

    #endif
    
    // Remote GPIO Functions
    int sk_gpio_init(const std::string &host)
    {
        int pi = pigpio_start(host, NULL);
        if (pi < 0) {
            printf("Failed to connect to pigpio daemon\n");
            return -1;
        }
        return pi;
    }
    
    connection sk_remote_gpio_init(std::string name, const std::string &host, unsigned short int port)
    {
        return open_connection(name, host, port);
    }

    void sk_remote_gpio_set_mode(connection pi, int pin, int mode)
    {
        sk_pigpio_cmd_t set_cmd;
        set_cmd.cmd_code = GPIO_CMD_SET_MODE;
        set_cmd.param1 = pin;
        set_cmd.param2 = mode;

        sk_gpio_send_cmd(pi, set_cmd);
    }

    int sk_remote_gpio_get_mode(connection pi, int pin)
    {
        sk_pigpio_cmd_t get_cmd;
        get_cmd.cmd_code = GPIO_CMD_GET_MODE;
        get_cmd.param1 = pin;

        return sk_gpio_send_cmd(pi, get_cmd);
    }

    void sk_remote_gpio_set_pull_up_down(connection pi, int pin, int pud)
    {
        sk_pigpio_cmd_t set_pud_cmd;
        set_pud_cmd.cmd_code = GPIO_CMD_SET_PUD;
        set_pud_cmd.param1 = pin;
        set_pud_cmd.param2 = pud;

        sk_gpio_send_cmd(pi, set_pud_cmd);
    }

    int sk_remote_gpio_read(connection pi, int pin)
    {
        sk_pigpio_cmd_t read_cmd;
        read_cmd.cmd_code = GPIO_CMD_READ;
        read_cmd.param1 = pin;

        return sk_gpio_send_cmd(pi, read_cmd);
    }

    void sk_remote_gpio_write(connection pi, int pin, int value)
    {
        sk_pigpio_cmd_t write_cmd;
        write_cmd.cmd_code = GPIO_CMD_WRITE;
        write_cmd.param1 = pin;
        write_cmd.param2 = value;

        sk_gpio_send_cmd(pi, write_cmd);
    }

    void sk_remote_set_pwm_range(connection pi, int pin, int range)
    {
        sk_pigpio_cmd_t set_range_cmd;
        set_range_cmd.cmd_code = GPIO_CMD_SET_PWM_RANGE;
        set_range_cmd.param1 = pin;
        set_range_cmd.param2 = range;

        sk_gpio_send_cmd(pi, set_range_cmd);
    }

    void sk_remote_set_pwm_frequency(connection pi, int pin, int frequency)
    {
        sk_pigpio_cmd_t set_freq_cmd;
        set_freq_cmd.cmd_code = GPIO_CMD_SET_PWM_FREQ;
        set_freq_cmd.param1 = pin;
        set_freq_cmd.param2 = frequency;

        sk_gpio_send_cmd(pi, set_freq_cmd);
    }

    void sk_remote_set_pwm_dutycycle(connection pi, int pin, int dutycycle)
    {
        sk_pigpio_cmd_t set_dutycycle_cmd;
        set_dutycycle_cmd.cmd_code = GPIO_CMD_SET_PWM_DUTYCYCLE;
        set_dutycycle_cmd.param1 = pin;
        set_dutycycle_cmd.param2 = dutycycle;

        sk_gpio_send_cmd(pi, set_dutycycle_cmd);
    }

    void sk_remote_clear_bank_1(connection pi)
    {
        sk_pigpio_cmd_t clear_bank_cmd;
        clear_bank_cmd.cmd_code = GPIO_CMD_CLEAR_BANK_1;
        clear_bank_cmd.param1 = PI4B_GPIO_BITMASK;

        sk_gpio_send_cmd(pi, clear_bank_cmd);
    }

    bool sk_remote_gpio_cleanup(connection pi)
    {
        if(!is_connection_open(pi))
        {
            LOG(ERROR) << "Remote GPIO: Connection not open.";
            return false;
        }
        LOG(INFO) << "Cleaning Pins on Remote Pi Named: " << pi->name << endl;
        sk_remote_clear_bank_1(pi);
        return close_connection(pi);
    }

    int sk_gpio_send_cmd(connection pi, sk_pigpio_cmd_t &cmd)
    {
        if(!is_connection_open(pi))
        {
            LOG(ERROR) << sk_gpio_error_message(PI_PIGIF_BAD_CONNECT); 
            return PI_PIGIF_BAD_CONNECT;
        }

        if(pi->protocol == TCP)
        {
            int num_send_bytes = sizeof(cmd);

            std::vector<char> buffer(num_send_bytes);
            memcpy(buffer.data(), &cmd, num_send_bytes);

            if(sk_send_bytes(&pi->socket, buffer.data(), num_send_bytes)) 
            {
                int num_bytes_recv = sk_read_bytes(&pi->socket, buffer.data(), num_send_bytes); 
                if(num_bytes_recv == num_send_bytes) 
                {
                    sk_pigpio_cmd_t resp;
                    memcpy(&resp, buffer.data(), num_send_bytes);
                    
                    // We cast it back to a signed type so we can get the negative error codes.
                    int32_t result = static_cast<int32_t>(resp.result);

                    if (result < 0)
                    {
                        LOG(ERROR) << sk_gpio_error_message(result);
                    }

                    return result;
                }
                else
                {
                    LOG(ERROR) << sk_gpio_error_message(PI_PIGIF_BAD_RECV);
                    return PI_PIGIF_BAD_RECV;
                }
            }
            else
            {
                LOG(ERROR) << sk_gpio_error_message(PI_PIGIF_BAD_SEND);
                return PI_PIGIF_BAD_SEND;
            }
        }
        else
        {
            LOG(ERROR) << "Remote GPIO: Connection has UDP Protocol";
            return -1;
        }
    }
    
    std::string sk_gpio_error_message(int error_code)
    {
        switch (error_code)
        {
            case PI_INIT_FAILED:
                return "GPIO initialization failed. Please check your setup and try again.";
            case PI_BAD_USER_GPIO:
            case PI_BAD_GPIO:
                return "Invalid GPIO pin number.";
            case PI_BAD_MODE:
                return "Invalid GPIO mode. Valid modes are 0-7.";
            case PI_BAD_LEVEL:
                return "Invalid GPIO level. Valid levels are 0 (LOW) or 1 (HIGH).";
            case PI_BAD_PUD:
                return "Invalid pull-up/down configuration. Valid options are 0 (OFF), 1 (Pull-down), 2 (Pull-up).";
            case PI_BAD_DUTYCYCLE:
                return "Invalid PWM duty cycle. Duty cycle must be between 0 and the range value (default 255).";
            case PI_BAD_DUTYRANGE:
                return "Invalid PWM range. Range must be between 25 and 40000.";
            case PIGIF_ERR_BAD_SEND:
                return "Failed to send command to remote GPIO daemon (pigpiod).";
            case PIGIF_ERR_BAD_RECV:
                return "Failed to receive response from remote GPIO daemon (pigpiod).";
            case PIGIF_ERR_BAD_CONNECT:
                return "Failed to connect to remote GPIO daemon (pigpiod).";
            default:
                return "Unknown error code " + std::to_string(error_code);
        }
    }
}
