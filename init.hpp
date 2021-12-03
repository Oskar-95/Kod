#pragma once

void iopins_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

void encoder_init(const pcnt_unit_t unit, const gpio_num_t pinA, const gpio_num_t pinB)
{
    pcnt_config_t pcnt_config_0 = {
        .pulse_gpio_num = pinA,
        .ctrl_gpio_num = pinB,
        .lctrl_mode = PCNT_MODE_KEEP,
        .hctrl_mode = PCNT_MODE_REVERSE,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_DEC,
        .counter_h_lim = ENCODER_H_LIM_VAL,
        .counter_l_lim = ENCODER_L_LIM_VAL,
        .unit = unit,
        .channel = PCNT_CHANNEL_0,
    };
    pcnt_config_t pcnt_config_1 = {
        .pulse_gpio_num = pinB,
        .ctrl_gpio_num = pinA,
        .lctrl_mode = PCNT_MODE_REVERSE,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_DEC,
        .counter_h_lim = ENCODER_H_LIM_VAL,
        .counter_l_lim = ENCODER_L_LIM_VAL,
        .unit = unit,
        .channel = PCNT_CHANNEL_1,
    };
    
    /* Initialize PCNT unit */
    
    pcnt_unit_config(&pcnt_config_0);
    pcnt_unit_config(&pcnt_config_1);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(unit, 100);
    pcnt_filter_enable(unit);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(unit);
    
}

void nvs_init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was trun_0cated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}
