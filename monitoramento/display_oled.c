#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

// Configurações de pinos
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;
const uint MIC_PIN = 28;     
const uint BUZZER_PIN = 21;   
const uint BUTTON_A_PIN = 5;  
const uint BUTTON_B_PIN = 6;  
const uint LED_RED_PIN = 10;  
const uint LED_GREEN_PIN = 11; 
const uint LED_BLUE_PIN = 12; 

// Configurações do ADC para detecção de vibração
const float SOUND_OFFSET = 1.65; 
const float SOUND_THRESHOLD = 0.25; 
const float ADC_REF = 3.3;         
const int ADC_RES = 4095;          

// Estado do sistema
bool system_active = false;
bool alert_active = false;

// Inicialização do PWM para o buzzer
void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f); 
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0); 
}

// Configuração dos LEDs de estado
void configure_leds() {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, 0);

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, 0);

    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, 1); 
}

// Atualiza estado dos LEDs
void update_led_status(bool active, bool alert_triggered) {
    if (!active) {
        gpio_put(LED_RED_PIN, 0);
        gpio_put(LED_GREEN_PIN, 0);
        gpio_put(LED_BLUE_PIN, 1); 
    } else if (alert_triggered) {
        gpio_put(LED_RED_PIN, 1);   // 🔴 Acende o LED vermelho quando há alerta
        gpio_put(LED_GREEN_PIN, 0);
        gpio_put(LED_BLUE_PIN, 0);
    } else {
        gpio_put(LED_RED_PIN, 0);
        gpio_put(LED_GREEN_PIN, 1);
        gpio_put(LED_BLUE_PIN, 0);
    }
}

// Emitir alerta sonoro
void play_alert(uint pin) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t top = clock_freq / 2000 - 1;

    pwm_set_wrap(slice_num, top);
    
    for (int i = 0; i < 5; i++) {
        pwm_set_gpio_level(pin, top * 0.9); // 🔊 Aumenta a intensidade do som
        sleep_ms(300);
        pwm_set_gpio_level(pin, 0);
        sleep_ms(100);
    }
}

int main() {
    stdio_init_all();

    adc_init();
    adc_gpio_init(MIC_PIN);
    adc_select_input(2); 

    pwm_init_buzzer(BUZZER_PIN);
    configure_leds();

    // Inicialização do display OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();

    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // Limpar a tela OLED antes de iniciar
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Mensagem inicial com quebras de linha
    ssd1306_draw_string(ssd, 0, 0, "Monitoramento");
    ssd1306_draw_string(ssd, 0, 16, "de Estruturas");
    ssd1306_draw_string(ssd, 0, 32, "Aguardando");
    ssd1306_draw_string(ssd, 0, 48, "sensores...");
    render_on_display(ssd, &frame_area);

    while (true) {
        // Botão A - Ativar monitoramento
        if (gpio_get(BUTTON_A_PIN) == 0) {
            system_active = true;
            alert_active = false;
            update_led_status(true, false);

            memset(ssd, 0, ssd1306_buffer_length);
            render_on_display(ssd, &frame_area);

            ssd1306_draw_string(ssd, 0, 16, "Monitoramento");
            ssd1306_draw_string(ssd, 0, 32, "ATIVO");
            render_on_display(ssd, &frame_area);
            sleep_ms(200);
        }

        // Botão B - Desativar monitoramento e alerta
        if (gpio_get(BUTTON_B_PIN) == 0) {
            system_active = false;
            alert_active = false;

            gpio_put(LED_RED_PIN, 0);  // 🔴 Apaga o LED vermelho
            update_led_status(false, false);

            memset(ssd, 0, ssd1306_buffer_length);  // 🟢 Limpa o display OLED
            render_on_display(ssd, &frame_area);
            
            ssd1306_draw_string(ssd, 0, 16, "Monitoramento");
            ssd1306_draw_string(ssd, 0, 32, "INATIVO");
            render_on_display(ssd, &frame_area);
            
            sleep_ms(200);
        }

        if (system_active) {
            uint16_t raw_adc = adc_read();
            float voltage = (raw_adc * ADC_REF) / ADC_RES;
            float vibration_level = fabs(voltage - SOUND_OFFSET);

            if (vibration_level > SOUND_THRESHOLD && !alert_active) {
                alert_active = true;
                update_led_status(true, true);
                
                gpio_put(LED_RED_PIN, 1); // 🔴 Acende o LED vermelho

                memset(ssd, 0, ssd1306_buffer_length);
                render_on_display(ssd, &frame_area);

                // 📌 Ajuste da exibição no OLED para telas pequenas
                ssd1306_draw_string(ssd, 0, 16, "ALERTA:");
                ssd1306_draw_string(ssd, 0, 32, "VIBRACAO");
                ssd1306_draw_string(ssd, 0, 48, "EXCESSIVA!");
                render_on_display(ssd, &frame_area);

                while (alert_active) {  // 🔄 Mantém o alerta até o botão B ser pressionado
                    play_alert(BUZZER_PIN);
                    sleep_ms(500);

                    if (gpio_get(BUTTON_B_PIN) == 0) {
                        alert_active = false;
                    }
                }
            }
        }
        sleep_ms(100);
    }
}
