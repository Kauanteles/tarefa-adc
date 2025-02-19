// Bibliotecas utilizadas no programa
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"

// Definições de pinos, constantes e variáveis globais
#define PIN_HORIZONTAL 26     
#define PIN_VERTICAL 27       
#define PIN_JOYSTICK 22     
#define PIN_LED_VERMELHO 13   
#define PIN_LED_VERDE 11    
#define PIN_LED_AZUL 12       
#define PIN_BOTAO_A 5        
#define PIN_I2C_SDA 14        
#define PIN_I2C_SCL 15        
#define ENDERECO_I2C 0x3C      
#define INTERFACE_I2C i2c1     
#define LIMITE 200             
#define CENTRO 2048            
#define PWM_MAX 4095          

// Variáveis globais para controle de tempo e estados
volatile uint32_t ultimo_tempo = 0;  
volatile bool borda_pontilhada = false;
volatile bool leds_ativos = true;  

// Protótipos das funções
void configurar_pwm(uint pino);
void configurar_adc(uint pino);
void configurar_gpio_saida(uint pino);
void configurar_botao(uint pino);
void configurar_i2c();
void configurar_display(ssd1306_t* display);
void tratar_interrupcao_gpio(uint pino, uint32_t eventos);
int mapear_valor(int valor, int entrada_min, int entrada_max, int saida_min, int saida_max);
void desenhar_borda_pontilhada(ssd1306_t* display);

int main() {
    stdio_init_all();

    // Inicialização e configuração do hardware
    configurar_pwm(PIN_LED_AZUL);
    configurar_pwm(PIN_LED_VERMELHO);
    configurar_adc(PIN_HORIZONTAL);
    configurar_adc(PIN_VERTICAL);
    configurar_gpio_saida(PIN_LED_VERDE);
    configurar_botao(PIN_JOYSTICK);
    configurar_botao(PIN_BOTAO_A);
    configurar_i2c();

    // Configuração das interrupções para os botões
    gpio_set_irq_enabled_with_callback(PIN_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &tratar_interrupcao_gpio);
    gpio_set_irq_enabled_with_callback(PIN_BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &tratar_interrupcao_gpio);

    // Inicialização do display OLED
    ssd1306_t display;
    configurar_display(&display);

    // Variáveis para leitura dos eixos do joystick
    uint16_t eixo_x, eixo_y;
    uint intensidade_vermelho, intensidade_azul;

    while (1) {
        // Leitura dos valores do joystick
        adc_select_input(1);
        eixo_x = adc_read();
        adc_select_input(0);
        eixo_y = adc_read();

        intensidade_vermelho = 0;
        intensidade_azul = 0;

        // Ajusta a intensidade dos LEDs com base no movimento do joystick
        if (leds_ativos) {
            if (eixo_y < CENTRO - LIMITE) {
                intensidade_azul = mapear_valor(eixo_y, 0, CENTRO - LIMITE, PWM_MAX, 0);
            } else if (eixo_y > CENTRO + LIMITE) {
                intensidade_azul = mapear_valor(eixo_y, CENTRO + LIMITE, PWM_MAX, 0, PWM_MAX);
            }

            if (eixo_x < CENTRO - LIMITE) {
                intensidade_vermelho = mapear_valor(eixo_x, 0, CENTRO - LIMITE, PWM_MAX, 0);
            } else if (eixo_x > CENTRO + LIMITE) {
                intensidade_vermelho = mapear_valor(eixo_x, CENTRO + LIMITE, PWM_MAX, 0, PWM_MAX);
            }
        }

        // Mapeia os valores do joystick para as coordenadas do display
        uint pos_x = mapear_valor(eixo_x, 0, 4095, 0, 119);
        uint pos_y = mapear_valor(eixo_y, 0, 4095, 55, 0);

        // Ajusta os limites para evitar sair da tela
        pos_x = pos_x == 118 ? 116 : (pos_x == 0 ? 4 : pos_x);
        pos_y = pos_y == 55 ? 52 : (pos_y == 1 ? 4 : pos_y);

        // Atualiza o display
        ssd1306_fill(&display, false);
        
        if (borda_pontilhada) {
            desenhar_borda_pontilhada(&display);
        } else {
            ssd1306_rect(&display, 0, 0, 128, 64, true, false);
        }

        // Desenha o cursor na tela
        ssd1306_rect(&display, pos_y, pos_x, 8, 8, true, true);
        ssd1306_send_data(&display);

        // Atualiza os LEDs
        pwm_set_gpio_level(PIN_LED_AZUL, intensidade_azul);
        pwm_set_gpio_level(PIN_LED_VERMELHO, intensidade_vermelho);

    }
}

// Configura o PWM para um determinado pino
void configurar_pwm(uint pino) {
    gpio_set_function(pino, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pino);
    pwm_set_wrap(slice, PWM_MAX);
    pwm_set_enabled(slice, true);
}

// Configura o ADC para um determinado pino
void configurar_adc(uint pino) {
    adc_init();
    adc_gpio_init(pino);
}

// Configura um pino como saída
void configurar_gpio_saida(uint pino) {
    gpio_init(pino);
    gpio_set_dir(pino, GPIO_OUT);
}

// Configura um pino de botão como entrada com pull-up
void configurar_botao(uint pino) {
    gpio_init(pino);
    gpio_set_dir(pino, GPIO_IN);
    gpio_pull_up(pino);
}

// Configura o barramento I2C
void configurar_i2c() {
    i2c_init(INTERFACE_I2C, 400 * 1000);
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);
}

// Configura o display OLED
void configurar_display(ssd1306_t* display) {
    ssd1306_init(display, WIDTH, HEIGHT, false, ENDERECO_I2C, INTERFACE_I2C);
    ssd1306_config(display);
    ssd1306_send_data(display);
}

// Trata as interrupções dos botões
void tratar_interrupcao_gpio(uint pino, uint32_t eventos) {
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
    if (tempo_atual - ultimo_tempo > 250) {
        ultimo_tempo = tempo_atual;
        if (pino == PIN_JOYSTICK) {
            gpio_put(PIN_LED_VERDE, !gpio_get(PIN_LED_VERDE));
            borda_pontilhada = !borda_pontilhada;
        } else if (pino == PIN_BOTAO_A) {
            leds_ativos = !leds_ativos;
        }
    }
}

// Mapeia um valor de um intervalo para outro
int mapear_valor(int valor, int entrada_min, int entrada_max, int saida_min, int saida_max) {
    return (valor - entrada_min) * (saida_max - saida_min) / (entrada_max - entrada_min) + saida_min;
}

// Desenha uma borda pontilhada no display
void desenhar_borda_pontilhada(ssd1306_t* display) {
    for (int i = 0; i <= 127; i += 4) {
        ssd1306_pixel(display, i, 0, true);
        ssd1306_pixel(display, i, 63, true);
    }
    for (int i = 0; i <= 63; i += 4) {
        ssd1306_pixel(display, 0, i, true);
        ssd1306_pixel(display, 127, i, true);
    }
    ssd1306_pixel(display, 127, 63, true);
}