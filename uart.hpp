#pragma once

#include "driver/uart.h"
#include "soc/uart_struct.h"

class Uart {
    Uart() = delete;
    Uart(const Uart&) = delete;
    void operator = (const Uart&) = delete;
public:
    typedef uart_config_t config_t;
    struct pins_t
    {
        int pin_txd;
        int pin_rxd;
        int pin_rts;
        int pin_cts;
    };
    struct buffers_t {
        size_t rx_buffer_size;
        size_t tx_buffer_size;
        size_t event_queue_size;
    };
    Uart(uart_port_t port, config_t config, pins_t pins, buffers_t buffers)
        : m_port {port},
          m_mutex {xSemaphoreCreateMutex()}
    {
        if (uart_is_driver_installed(m_port)) {
            printf("Uart on port %d is already installed, aborting\n", m_port);
            abort();
        }
        ESP_ERROR_CHECK(uart_param_config(m_port, &config));
        ESP_ERROR_CHECK(uart_set_pin(m_port, pins.pin_txd, pins.pin_rxd, pins.pin_rts, pins.pin_cts));
        ESP_ERROR_CHECK(uart_driver_install(m_port, buffers.rx_buffer_size, buffers.tx_buffer_size, buffers.event_queue_size, NULL, 0));
        if (!m_mutex) {
            printf("Can not create Uart%d mutex, aborting\n", m_port);
            abort();
        }
    }
    ~Uart() {
        if (m_mutex) {
            uart_driver_delete(m_port);
            vSemaphoreDelete(m_mutex);
            m_mutex = nullptr;
        }
    }
    int write(const uint8_t* data, size_t size) {
        MutexGuard guard {m_mutex};
        guard.acquire();
        const int sent = uart_write_bytes(m_port, reinterpret_cast<const char*>(data), size);
        return sent;
    }
    int write (uint8_t v) {
        return write(&v, 1);
    }
    size_t available() {
        MutexGuard guard {m_mutex};
        guard.acquire();
        size_t ret = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(m_port, &ret));
        return ret;
    }
    int read(TickType_t timeout = portMAX_DELAY) {
        MutexGuard guard {m_mutex};
        guard.acquire();
        uint8_t v = 0;
        if (uart_read_bytes(m_port, &v, 1, timeout) == 1)
            return v;
        return -1;
    }
    int read(uint8_t* data, size_t size, TickType_t timeout = portMAX_DELAY) {
        MutexGuard guard {m_mutex};
        guard.acquire();
        return uart_read_bytes(m_port, data, size, timeout);
    }
    int transfer(const uint8_t* tx_data, size_t tx_size, uint8_t* rx_data, size_t rx_size, TickType_t timeout = portMAX_DELAY) {
        MutexGuard guard {m_mutex};
        guard.acquire();
        ESP_ERROR_CHECK(uart_flush(m_port));
        const int sent = uart_write_bytes(m_port, reinterpret_cast<const char*>(tx_data), tx_size);
        if (sent != int(tx_size))
            return -sent;
        return uart_read_bytes(m_port, rx_data, rx_size, timeout);
    }
    void flush() {
        MutexGuard guard {m_mutex};
        guard.acquire();
        ESP_ERROR_CHECK(uart_flush(m_port));
    }
private:
    const uart_port_t m_port;
    SemaphoreHandle_t m_mutex;
};
