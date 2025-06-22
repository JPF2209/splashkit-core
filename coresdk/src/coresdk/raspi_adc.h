/**
 * @header raspi_adc
 * @brief Provides support for using an ADC device with the GPIO pins on the Raspberry Pi.
 *        Supports both basic and register-based I2C communication (8-bit and 16-bit).
 * @author Aditya Parmar
 * 
 * @attribute group  raspberry
 * @attribute static raspberry
 */

 #ifndef raspi_adc_hpp
 #define raspi_adc_hpp
 
 #include "types.h"
 #include <string>
 
 namespace splashkit_lib
 {
     /**
      * The `adc_device` type represents an ADC device that can be read from via I2C.
      * Devices can be:
      *   - loaded via `open_adc`,
      *   - accessed via `adc_device_named`,
      *   - read using `read_adc`,
      *   - and closed using `close_adc`.
      *
      * @attribute class adc_device
      */
     typedef struct _adc_data *adc_device;
 
     /**
      * Checks if an ADC device with the given name is already loaded.
      *
      * @param name  The name of the ADC device.
      * @returns     true if an ADC with that name exists.
      */
     bool has_adc_device(const std::string &name);
 
     /**
      * Retrieves a loaded ADC device by name.
      *
      * @param name  The name of the ADC device.
      * @returns     A pointer to the device, or nullptr if not found.
      */
     adc_device adc_device_named(const std::string &name);
 
     /**
      * Opens an ADC device with a specified I2C bus, address, and type.
      *
      * @param name     The identifier name for the device.
      * @param bus      The I2C bus number.
      * @param address  The I2C device address.
      * @param type     The ADC type (e.g., ADS7830, PCF8591).
      * @returns        The adc_device pointer, or nullptr on failure.
      *
      * @attribute class  adc_device
      * @attribute constructor true
      * @attribute suffix with_bus
      */
     adc_device open_adc(const std::string &name, int bus, int address, adc_type type);
 
     /**
      * Opens an ADC device with a default bus and address.
      *
      * @param name  The identifier name for the device.
      * @param type  The ADC type.
      * @returns     The adc_device pointer, or nullptr on failure.
      *
      * @attribute class        adc_device
      * @attribute constructor  true
      */
     adc_device open_adc(const std::string &name, adc_type type);
 
     /**
      * Reads an analog value from the given ADC device and channel.
      *
      * Supports both 8-bit and 16-bit I2C reads depending on the ADC type.
      *
      * @param adc      The device to read from.
      * @param channel  The ADC channel or register.
      * @returns        The value read (0?255 or 0?65535), or -1 on failure.
      *
      * @attribute class   adc_device
      * @attribute self    adc
      * @attribute method  read
      */
     int read_adc(adc_device adc, adc_pin channel);
 
     /**
      * Reads an analog value using the ADC device name and channel.
      *
      * @param name     The name of the ADC device.
      * @param channel  The ADC channel or register.
      * @returns        The value read (0?255 or 0?65535), or -1 on failure.
      *
      * @attribute suffix  named
      */
     int read_adc(const std::string &name, adc_pin channel);
 
     /**
      * Closes a specific ADC device.
      *
      * @param adc  The ADC device to close.
      *
      * @attribute class       adc_device
      * @attribute self        adc
      * @attribute destructor  true
      */
     void close_adc(adc_device adc);
 
     /**
      * Closes an ADC device by its name.
      *
      * @param name  The device name.
      *
      * @attribute suffix  named
      */
     void close_adc(const std::string &name);
 
     /**
      * Closes all ADC devices currently loaded.
      */
     void close_all_adc();
 }
 #endif /* raspi_adc_hpp */
 