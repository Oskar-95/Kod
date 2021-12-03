#pragma once

class Driver {
    Driver() = delete;
    Driver(const Driver&) = delete;
    void operator = (const Driver&) = delete;

    typedef uint32_t data_t;
public:
    typedef uint8_t driver_address_t;
    typedef uint8_t register_address_t;

    Driver(Uart& uart, const driver_address_t address, const gpio_num_t enable_pin)
        : m_uart {uart},
          c_address {address},
          c_enable_pin(enable_pin)
    {}

    bool init() {
        gpio_config_t en_pin_conf {
            .pin_bit_mask = 1UL<<uint64_t(c_enable_pin),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&en_pin_conf);
        disable();
        data_t data = 0;
        int result = _read(0, data);
        printf("registr 0x00 pred zapisem  %d %X\n", result, data);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        _write(0, 0x000000E0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        result = _read(0, data);
        printf("registr 0x00 po zapisu %d %X\n", result, data);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        return result == 0;
    }

    void enable(bool v = true) {
        gpio_set_level(c_enable_pin, !v);
    }

    void disable() { enable(false); }

    bool set_speed(int speed) {
        return _write(0x22, speed);
    }

    int read_speed(uint32_t& read) {
        return _read(0x22, read);
    }
    


    bool set_IHOLD_IRUN(int irun, int ihold) {
        int32_t data = irun;
        data =(data << 8) | ihold;
        //printf("data pred zapisem do IHOLD_IRUN %d\n", data);
        return _write(0x10, data);
    }
    
    int get_PWMCONF(uint32_t& read) {
        return _read(0x70, read);
    }

    int get_DRV_STATUS(uint32_t& read) {
        return _read(0x6F, read);
    }
 
    int get_SG(uint32_t& read) {
        return _read(0x41, read);
    }

    int get_MSCNT(uint32_t& read) {
        return _read(0x6A, read);
    }

    driver_address_t address() const { return c_address; }
     
private:
    bool _write(const register_address_t register_address, const data_t register_data)
    {
        static constexpr size_t packet_size = 8;
        uint8_t data[packet_size] = { DRIVERS_UART_START_BYTE, c_address, static_cast<uint8_t>(register_address + 0x80) };
        data[3] = register_data >> 24;
        data[4] = register_data >> 16;
        data[5] = register_data >> 8;
        data[6] = register_data;
        data[7] = _calcCRC(data, packet_size-1);
        uint8_t data_read[packet_size];
        const int res = m_uart.transfer(data, packet_size, data_read, packet_size, DRIVERS_RX_TIMEOUT);
        if (res < 0) {
            printf("Only %d bytes written to driver %d (register %d, value 0x%08X), but has to be %u\n", -res, int(c_address), int(register_address), register_data, packet_size);
            return false;
        }
        if (res != packet_size) {
            printf("Read back only %d bytes, but %u written to driver %d (register %d, value 0x%08X)\n", res, packet_size, int(c_address), int(register_address), register_data);
            return false;
        }
        for (size_t i = 0; i != packet_size; ++i) {
            if (data[i] != data_read[i]) {
                printf("Sent byte %u (0x%02X) does not match received (0x%02X) - driver %d (register %d, value 0x%08X)\n", i, int(data[i]), int(data_read[i]), int(c_address), int(register_address), register_data);
                return false;
            }
        }
        return true;
    }

    int _read(const register_address_t register_address, data_t& register_data)
    {
        static constexpr size_t tx_packet_size = 4;
        static constexpr size_t rx_packet_size = 8;
        static constexpr size_t rx_data_begin = tx_packet_size + 3;
        static constexpr size_t rx_data_end = rx_data_begin + 4;
        uint8_t data_write[tx_packet_size] = { DRIVERS_UART_START_BYTE, c_address, register_address };
        data_write[tx_packet_size-1] = _calcCRC(data_write, tx_packet_size-1);
        uint8_t data_read[tx_packet_size + rx_packet_size];
        const int res = m_uart.transfer(data_write, tx_packet_size, data_read, rx_packet_size + tx_packet_size, DRIVERS_RX_TIMEOUT);//**************
        if (res < 0) {
            printf("Only %d bytes written to driver %d (register %d), but has to be %u\n", -res, int(c_address), int(register_address), tx_packet_size);
            return -res + 300;
        }
        if (res < tx_packet_size) {
            printf("Read back only %d bytes, but %u written to driver %d (register %d)\n", res, tx_packet_size, int(c_address), int(register_address));
            return res + 200;
        }
        for (size_t i = 0; i != tx_packet_size; ++i) {
            if (data_write[i] != data_read[i]) {
                printf("Sent byte %u (0x%02X) does not match received (0x%02X) - driver %d (register %d)\n", i, int(data_write[i]), int(data_read[i]), int(c_address), int(register_address));
                return 3;
            }
        }
        if (res < (tx_packet_size + rx_packet_size)) {
            printf("Read only %d bytes, but responce must has %u - driver %d (register %d)\n", res - tx_packet_size, rx_packet_size, int(c_address), int(register_address));
            return res + 100 - tx_packet_size;
        }
        const uint8_t crc = _calcCRC(data_read + tx_packet_size, rx_packet_size-1);
        if (crc != data_read[tx_packet_size + rx_packet_size-1]) {  //*******************************
            printf("crc error during reading driver %d register %d\n", int(register_address), tx_packet_size);
            return 2;
        }
        register_data = data_read[rx_data_begin];
        for(size_t i = rx_data_begin+1; i != rx_data_end; ++i)
            register_data = (register_data << 8) | data_read[i];
        return 0;
    }

    Uart& m_uart;
    const driver_address_t c_address;
    const gpio_num_t c_enable_pin;

// static methods
    static uint8_t _calcCRC(const uint8_t* datagram, uint8_t datagramLength) {
        uint8_t crc = 0;
        for(uint8_t i = 0; i != datagramLength; ++i) {   // Execute for all bytes of a message
            uint8_t currentByte = datagram[i];	         // Retrieve a byte to be sent from Array
            for(uint8_t j = 0; j != 8; ++j) {
                if((crc >> 7) ^ (currentByte & 0x01))    // update CRC based result of XOR operation
                    crc = (crc << 1) ^ 0x07;
                else
                    crc = (crc << 1);
                currentByte = currentByte >> 1;
            }                                            // for CRCbit
        }
        return crc;                                      // for message byte
    }
};
