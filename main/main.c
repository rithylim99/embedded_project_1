#include <stdio.h>
#include <stdint.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// BMP280 I2C address and registers
#define BMP280_I2C_ADDR        0x76
#define BMP280_REG_PRESS_MSB   0xF7
#define BMP280_REG_CALIB_START 0x88

// I2C Configuration
#define I2C_MASTER_NUM         I2C_NUM_0
#define I2C_MASTER_SCL_IO      20
#define I2C_MASTER_SDA_IO      19
#define I2C_MASTER_FREQ_HZ     100000

//KY-003 Configuration
#define KY003_PIN GPIO_NUM_6

// Calibration data structure
typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
} bmp280_calib_data_t;

static bmp280_calib_data_t calib_data;

static const char *TAG_BMP = "BMP280";
static const char *TAG_KY = "KY003";

// I2C Master initialization
void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// I2C read helper
esp_err_t i2c_read(uint8_t reg, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP280_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP280_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return err;
}

// Read calibration data
esp_err_t bmp280_read_calibration_data() {
    uint8_t calib[6];
    esp_err_t err = i2c_read(BMP280_REG_CALIB_START, calib, 6);
    if (err == ESP_OK) {
        calib_data.dig_T1 = (calib[1] << 8) | calib[0];
        calib_data.dig_T2 = (calib[3] << 8) | calib[2];
        calib_data.dig_T3 = (calib[5] << 8) | calib[4];
        ESP_LOGI(TAG_BMP, "Calibration Data Loaded: T1=%u, T2=%d, T3=%d",
                 calib_data.dig_T1, calib_data.dig_T2, calib_data.dig_T3);
    } else {
        ESP_LOGE(TAG_BMP, "Failed to read calibration data");
    }
    return err;
}

// Temperature conversion
int32_t bmp280_compensate_temperature(int32_t adc_T, int32_t *t_fine) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)calib_data.dig_T1 << 1))) *
            ((int32_t)calib_data.dig_T2)) >>
           11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib_data.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)calib_data.dig_T1))) >>
             12) *
            ((int32_t)calib_data.dig_T3)) >>
           14;
    *t_fine = var1 + var2;
    T = (*t_fine * 5 + 128) >> 8; // Temperature in 0.01°C
    return T;
}

void bmp280_task() {
    ESP_LOGI(TAG_BMP, "Initializing I2C...");
    i2c_master_init();

    ESP_LOGI(TAG_BMP, "Reading BMP280 Calibration Data...");
    if (bmp280_read_calibration_data() != ESP_OK) {
        ESP_LOGE(TAG_BMP, "Failed to initialize BMP280");
        return;
    }
    int32_t t_fine = 0;

    for (;;) {
        uint8_t data[6];
        esp_err_t err = i2c_read(BMP280_REG_PRESS_MSB, data, 6);
        if (err == ESP_OK) {
            int32_t adc_T = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
            int32_t temp = bmp280_compensate_temperature(adc_T, &t_fine);
            float temp_celsius = temp / 100.0;
            ESP_LOGI(TAG_BMP, "Temperature: %.2f °C", temp_celsius);
        } else {
            ESP_LOGE(TAG_BMP, "Failed to read sensor data");
        }
        vTaskDelay(3000/portTICK_PERIOD_MS);
    }
}

void ky_003_task(){
    gpio_config_t io_config = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << KY003_PIN,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0x00,
        .pull_up_en = 0x01
    };
    gpio_config(&io_config);

    for(;;){
        int ky_003_state = gpio_get_level(KY003_PIN);
        if(ky_003_state == 0){
            ESP_LOGI(TAG_KY, "Magnetic is detected");
        }else{
            ESP_LOGI(TAG_KY, "Magnetic is not detected");
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
}
}

// Main application
void app_main() {
    xTaskCreate(bmp280_task, "bmp280_task", 4096, NULL, 5, NULL);
    xTaskCreate(ky_003_task, "ky_003_task", 4096, NULL, 5, NULL);
}
