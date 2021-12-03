#pragma once

esp_reset_reason_t check_reset() {
    esp_reset_reason_t resetReason = esp_reset_reason();
    switch (resetReason) {
    case ESP_RST_UNKNOWN:
        printf("\tUnknown reset - strange\n");
        break;
    case ESP_RST_POWERON:
        printf("\tPoweron reset\n");
        break;
    case ESP_RST_EXT:
        printf("\tExternal reset\n");
        break;
    case ESP_RST_SW:
        printf("\tSoftware reset\n");
        break;
    case ESP_RST_PANIC:
        printf("\tReset due to core panic - stop program\n");
        vTaskSuspend(nullptr);
        break;
    case ESP_RST_INT_WDT:
        printf("\tReset due to interrupt watchdog - stop program\n");
        vTaskSuspend(nullptr);
        break;
    case ESP_RST_TASK_WDT:
        printf("\tReset due to task watchdog - stop program\n");
        vTaskSuspend(nullptr);
        break;
    case ESP_RST_WDT:
        printf("\tReset due to some watchdog - stop program\n");
        vTaskSuspend(nullptr);
        break;
    case ESP_RST_DEEPSLEEP:
        printf("\tWaked from deep sleep\n");
        break;
    case ESP_RST_BROWNOUT:
        printf("\tBrownout reset - please check power\n");
        break;
    case ESP_RST_SDIO:
        printf("\tSDIO reset - strange\n");
        break;
    }
    return resetReason;
}

class MutexGuard {
    MutexGuard() = delete;
    MutexGuard(const MutexGuard&) = delete;
    void operator = (const MutexGuard&) = delete;
public:
    MutexGuard(SemaphoreHandle_t& mutex)
        : m_mutex {mutex}
    {}
    int acquire (TickType_t timeout = portMAX_DELAY){
        return xSemaphoreTake(m_mutex, timeout);
    }
    int release () {
        return xSemaphoreGive(m_mutex);
    }
    ~MutexGuard() {
        xSemaphoreGive(m_mutex);
    }
private:
    SemaphoreHandle_t& m_mutex;
};
