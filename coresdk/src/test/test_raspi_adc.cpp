/***********************************************
 * XQuestCode || Aditya Parmar
 *Â© 2024 Aditya Parmar. All Rights Reserved.
 ***********************************************/

#include <iostream>
#include "raspi_adc.h"  // Your ADC header
#include "utils.h"      // For any_key_pressed() and delay()
#include "gpio_driver.h" // For raspi_init and raspi_cleanup

using namespace std;
using namespace splashkit_lib;

void run_gpio_adc_tests()
{
    cout << "Testing ADC with a ADS7830 and a potentiometer" << endl;

    // Initialize GPIO/I2C subsystem
    raspi_init();

    cout << "Plug a potentiometer at A0 channel of the ADS7830" << endl;

    // Open ADC device on I2C bus 1, address 0x48, type ADS7830
    adc_device dev = open_adc("ADC1", 1, 0x48, ADS7830);
    if (dev == nullptr)
    {
        cout << "Failed to open ADC device." << endl;
        raspi_cleanup();
        return;
    }

    adc_pin channel = ADC_PIN_0; // Use channel 0 (A0) on ADS7830

    cout << "Press any key to stop the test." << endl;

    while (!any_key_pressed())
    {
        int value = read_adc(dev, channel);
        if (value < 0)
        {
            cout << "Error reading ADC value." << endl;
        }
        else
        {
            cout << "ADC value: " << value << endl;
        }
        delay(100); // 100 ms delay between reads (adjust as needed)
    }

    // Clean up: close ADC and GPIO subsystem
    close_adc(dev);
    raspi_cleanup();

    cout << "ADC test completed." << endl;
}
