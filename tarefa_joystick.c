#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

#define JOYSTICK_X_ADC  26 // ADC0
#define JOYSTICK_Y_ADC  27 // ADC1
#define JOYSTICK_BTN    22
#define LED_RED_PIN     13
#define LED_BLUE_PIN    12
#define LED_GREEN_PIN   11
#define BUTTON_A_PIN    5

#define I2C_SDA         14
#define I2C_SCL         15
#define I2C_PORT        i2c1

static volatile bool joystick_btn_pressed = false;
static volatile bool button_a_pressed = false;

static uint32_t last_debounce_joystick_btn = 0;
static uint32_t last_debounce_button_a     = 0;
#define DEBOUNCE_MS 200

static uint red_slice,  red_channel;
static uint blue_slice, blue_channel;
static bool pwm_enabled = true;

static ssd1306_t ssd;
static int square_x = 60;
static int square_y = 60;
#define SQUARE_SIZE  8
#define DEADZONE     50  // Valor de tolerância para manter o quadrado centralizado

static int border_style = 0;

void gpio_callback(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (gpio == JOYSTICK_BTN) {
        bool level = gpio_get(JOYSTICK_BTN); // false = pressionado
        if (!level && (now - last_debounce_joystick_btn >= DEBOUNCE_MS)) {
            last_debounce_joystick_btn = now;
            joystick_btn_pressed = true;
        }
    }
    else if (gpio == BUTTON_A_PIN) {
        bool level = gpio_get(BUTTON_A_PIN);
        if (!level && (now - last_debounce_button_a >= DEBOUNCE_MS)) {
            last_debounce_button_a = now;
            button_a_pressed = true;
        }
    }
}

static void init_led_pwm(uint gpio, uint *slice, uint *channel) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    *slice   = pwm_gpio_to_slice_num(gpio);
    *channel = pwm_gpio_to_channel(gpio);
    pwm_set_wrap(*slice, 4095);
    pwm_set_clkdiv(*slice, 1.0f);
    pwm_set_chan_level(*slice, *channel, 0);
    pwm_set_enabled(*slice, true);
}

static void set_led_level(uint slice, uint channel, uint16_t level) {
    if (!pwm_enabled) {
        pwm_set_chan_level(slice, channel, 0);
        return;
    }
    if (level > 4095) level = 4095;
    pwm_set_chan_level(slice, channel, level);
}

static void draw_display(void) {
    ssd1306_fill(&ssd, false);

    switch (border_style) {
        case 0:
            break;
        case 1:
            ssd1306_rect(&ssd, 0, 0, ssd.width, ssd.height, true, false);
            break;
        case 2:
            ssd1306_rect(&ssd, 0, 0, ssd.width, ssd.height, true, true);
            break;
    }

    // Desenha quadrado 8x8
    ssd1306_rect(&ssd, square_x, square_y, SQUARE_SIZE, SQUARE_SIZE, true, true);

    ssd1306_send_data(&ssd);
}

static uint16_t map_adc_to_brightness(uint16_t val) {
    int diff = val - 2048;
    if (diff < 0) diff = -diff;
    int scaled = diff * 2; // 0..4096
    if (scaled > 4095) scaled = 4095;
    return (uint16_t)scaled;
}

static int map_adc_to_coord(uint16_t val, int max_coord) {
    return (val * max_coord) / 4095;
}

int main() {
    stdio_init_all();

    gpio_init(JOYSTICK_BTN);
    gpio_set_dir(JOYSTICK_BTN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN);

    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_set_irq_enabled_with_callback(JOYSTICK_BTN, GPIO_IRQ_EDGE_FALL, true, gpio_callback);
    gpio_set_irq_enabled(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true);

    adc_init();
    adc_gpio_init(JOYSTICK_X_ADC);
    adc_gpio_init(JOYSTICK_Y_ADC);

    init_led_pwm(LED_RED_PIN,  &red_slice,  &red_channel);
    init_led_pwm(LED_BLUE_PIN, &blue_slice, &blue_channel);

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, false);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd.width  = 128;
    ssd.height = 64;
    ssd1306_init(&ssd, 128, 64, false, 0x3C, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa o quadrado centralizado
    square_x = (ssd.width - SQUARE_SIZE) / 2;
    square_y = (ssd.height - SQUARE_SIZE) / 2;
    draw_display();

    while (true) {
        // Leitura do ADC
        adc_select_input(0);
        uint16_t raw_x = adc_read();
        adc_select_input(1);
        uint16_t raw_y = adc_read();

        // Ajusta brilho dos LEDs
        uint16_t brightness_blue = map_adc_to_brightness(raw_y);
        uint16_t brightness_red  = map_adc_to_brightness(raw_x);
        set_led_level(blue_slice, blue_channel, brightness_blue);
        set_led_level(red_slice, red_channel, brightness_red);

        // Atualiza posição do quadrado com deadzone para manter centralizado
        int max_x = ssd.width - SQUARE_SIZE;
        int max_y = ssd.height - SQUARE_SIZE;
        if (abs((int)raw_x - 2048) > DEADZONE) {
            square_x = map_adc_to_coord(raw_x, max_x);
        } else {
            square_x = (ssd.width - SQUARE_SIZE) / 2;
        }
        if (abs((int)raw_y - 2048) > DEADZONE) {
            square_y = map_adc_to_coord(raw_y, max_y);
        } else {
            square_y = (ssd.height - SQUARE_SIZE) / 2;
        }

        draw_display();

        // Verifica botões
        if (joystick_btn_pressed) {
            joystick_btn_pressed = false;
            bool current = gpio_get(LED_GREEN_PIN);
            gpio_put(LED_GREEN_PIN, !current);

            border_style = (border_style + 1) % 3;
            draw_display();
        }

        if (button_a_pressed) {
            button_a_pressed = false;
            pwm_enabled = !pwm_enabled;
            printf("Botão A => PWM %s\n", pwm_enabled ? "Ativado" : "Desativado");
        }

        sleep_ms(10);
    }

    return 0;
}
