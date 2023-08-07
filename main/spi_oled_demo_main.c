#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "ssd1306.h"

static const char* TAG = "OLED";

#define BLINK_GPIO GPIO_NUM_5
#define BLINK_PERIOD 200
#define BLINK_PRINT FALSE
#define BUTTON_GPIO GPIO_NUM_0


// spi pins for oled - defined in sdkconfig
//#define CONFIG_MOSI_GPIO  GPIO_NUM_23
//#define CONFIG_SCLK_GPIO  GPIO_NUM_18
//#define CONFIG_CS_GPIO    GPIO_NUM_27
//#define CONFIG_DC_GPIO    GPIO_NUM_26
//#define CONFIG_RESET_GPIO GPIO_NUM_25

static uint8_t s_led_state = 0;
static SSD1306_t oled_dev;
static const char* digitText[10] = {
    "zero","one", "two", "three","four", 
    "five", "six", "seven","eight", "nine"
};

static uint16_t loopcount = 0;

static void configure_led(void) {
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

static void configure_button(void) {
    gpio_config_t cfg = { 0 };
    gpio_reset_pin(BUTTON_GPIO);
    cfg.pin_bit_mask = (1 << (BUTTON_GPIO));
    cfg.mode = GPIO_MODE_INPUT;
    cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&cfg);
}

static void configure_spi_oled(void) {

    spi_master_init(&oled_dev, 
        CONFIG_MOSI_GPIO, 
        CONFIG_SCLK_GPIO, 
        CONFIG_CS_GPIO, 
        CONFIG_DC_GPIO, 
        CONFIG_RESET_GPIO);

    oled_dev._flip = false;

    ssd1306_init(&oled_dev, 128, 64);
    ssd1306_clear_screen(&oled_dev, false);
    ssd1306_contrast(&oled_dev, 0xff);
    ssd1306_display_text_x3(&oled_dev, 0, "ESP32", 5, false);
}

static void update_led(void) {
    gpio_set_level(BLINK_GPIO, s_led_state);
    // toggle
    s_led_state = !s_led_state;
}

static void update_oled(void){
    //if ((loopcount & 1) == 0)
    //    return;

    // clear screen, preserve top 3 lines
    ssd1306_clear_line(&oled_dev, 3, false);
    ssd1306_clear_line(&oled_dev, 4, false);
    ssd1306_clear_line(&oled_dev, 5, false);
    ssd1306_clear_line(&oled_dev, 6, false);
    ssd1306_clear_line(&oled_dev, 7, false);

    char digits[7];

    // show digit name e.g. "three"
    sprintf(digits, "  %s", (char *) digitText[(loopcount >> 1) % 10]);
    ssd1306_display_text(&oled_dev, 4, digits, strlen(digits), false);

    // show loop count numeric value
    sprintf(digits, " %d", loopcount % 10000);
    ssd1306_display_text_x3(&oled_dev, 5, digits, strlen(digits), false);
}

void app_main(void) {

    configure_led();
    configure_button();
    configure_spi_oled();

    while (1) {
        // block when button pressed
        if (gpio_get_level(BUTTON_GPIO) == 0) {
            do
                vTaskDelay(100);
            while (gpio_get_level(BUTTON_GPIO) != 1);
        }
        update_led();
        update_oled();
        vTaskDelay(BLINK_PERIOD / portTICK_PERIOD_MS);
        loopcount++;
    }
}

