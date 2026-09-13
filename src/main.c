/*
FreeRTOS - Projeto Desafio
Este projeto implementa 4 tarefas que trabalham juntas:
- Tarefa 1: Lê o estado do botão e envia sinais via Event Group
- Tarefa 2: Recebe sinal de botão apertado, muda cor do LED e envia nome da cor pela Queue
- Tarefa 3: Escreve na primeira linha do OLED o estado do botão usando Mutex
- Tarefa 4: Recebe string da Queue e escreve na segunda linha do OLED usando Mutex
*/

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "FreeRTOSConfig.h"
#include "ssd1306_i2c.h"
#include "ssd1306.h"

// Macro para contar elementos de array (caso não esteja definida no SDK)
#ifndef count_of
#define count_of(a) (sizeof(a) / sizeof(*(a)))
#endif

// Definições de pinos
#define BUTTON_PIN 5        // Pino do botão (ajustar conforme hardware)
// Mapeamento identificado pelos testes:
// GPIO 11 = VERDE
// GPIO 12 = AZUL
// GPIO 13 = VERMELHO
#define LED_RED_PIN 13       // GPIO 13 controla LED vermelho
#define LED_GREEN_PIN 11     // GPIO 11 controla LED verde
#define LED_BLUE_PIN 12      // GPIO 12 controla LED azul
#define I2C_SDA_PIN 2        // Pino SDA do I2C para OLED 
#define I2C_SCL_PIN 3        // Pino SCL do I2C para OLED 

// Bits do Event Group
#define BUTTON_ON_BIT_1  (1 << 0)   // Bit para tarefa 2
#define BUTTON_ON_BIT_2  (1 << 1)   // Bit para tarefa 3
#define BUTTON_OFF_BIT   (1 << 2)   // Bit para tarefa 3 quando botão solto

// Handles globais
static EventGroupHandle_t xEventGroup;
static QueueHandle_t xQueueColor;
static SemaphoreHandle_t xMutex;
static uint8_t *ssd1306_buffer;

// Estado inicial do LED (vermelho)
// IMPORTANTE: O LED inicia em vermelho (0), então o primeiro clique muda para verde (1)
static int current_color = 0; // 0=vermelho, 1=verde, 2=azul

// Função para inicializar GPIOs
void init_gpios(void) {
    // Configura botão com pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    
    // Configura LEDs RGB como saídas
    gpio_init(LED_RED_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    
    // Inicia com LED vermelho aceso
    gpio_put(LED_RED_PIN, 1);
    gpio_put(LED_GREEN_PIN, 0);
    gpio_put(LED_BLUE_PIN, 0);
}

// Função para inicializar display OLED
void init_display(void) {
    // Aloca buffer para o display (ssd1306_buffer_length está definido em ssd1306_i2c.h)
    ssd1306_buffer = (uint8_t*) calloc(ssd1306_buffer_length, sizeof(uint8_t));
    if (ssd1306_buffer == NULL) {
        printf("Erro: Falha ao alocar buffer do display!\n");
        return;
    }
    
    // Inicialização do I2C para comunicação com o display OLED (mesma ordem do exemplo)
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);  // Pull-up interno para estabilidade do sinal
    gpio_pull_up(I2C_SCL_PIN);  // Pull-up interno para estabilidade do sinal
    ssd1306_init();
    
    // Configuração da área de renderização para o display OLED
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);
    
    // Limpa display
    memset(ssd1306_buffer, 0, ssd1306_buffer_length);
    
    // Escreve texto inicial
    ssd1306_draw_string(ssd1306_buffer, 0, 0, "BOTAO OFF");
    ssd1306_draw_string(ssd1306_buffer, 0, 16, "VERMELHO");
    
    // Atualiza display completo
    render_on_display(ssd1306_buffer, &frame_area);
}

// Tarefa 1: Lê o estado do botão e envia sinais via Event Group
void vTaskButtonReader(void *pvParameters) {
    bool last_button_state = false;
    bool current_button_state;
    
    // Lê o estado inicial do botão
    last_button_state = !gpio_get(BUTTON_PIN);
    
    while (1) {
        // Lê o estado atual do botão (pull-up: 0 = apertado, 1 = solto)
        current_button_state = !gpio_get(BUTTON_PIN);
        
        // Verifica se houve mudança de estado
        if (current_button_state != last_button_state) {
            if (current_button_state) {
                // Botão foi apertado (OFF -> ON)
                printf("Botao PRESSIONADO!\n");
                xEventGroupSetBits(xEventGroup, BUTTON_ON_BIT_1 | BUTTON_ON_BIT_2);
            } else {
                // Botão foi solto (ON -> OFF)
                printf("Botao SOLTO!\n");
                xEventGroupSetBits(xEventGroup, BUTTON_OFF_BIT);
            }
            last_button_state = current_button_state;
        }
        
        // Delay de 50ms entre leituras
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Tarefa 2: Recebe sinal de botão apertado, muda cor do LED e envia nome da cor pela Queue
void vTaskLedController(void *pvParameters) {
    char color_string[10];

    printf("Tarefa LedController iniciada. Aguardando botao...\n");

    while (1) {
        // Espera o bit de botão apertado (para esta tarefa)
        xEventGroupWaitBits(xEventGroup,
                            BUTTON_ON_BIT_1,
                            pdTRUE,    // Limpa o bit após ler
                            pdFALSE,   // Não precisa de todos os bits
                            portMAX_DELAY);

        printf("Tarefa 2: Botao detectado, mudando cor do LED...\n");

        // Apaga todas as cores antes de mudar
        gpio_put(LED_RED_PIN, 0);
        gpio_put(LED_GREEN_PIN, 0);
        gpio_put(LED_BLUE_PIN, 0);

        // Muda a cor do LED (vermelho -> verde -> azul)
        current_color = (current_color + 1) % 3;

        switch (current_color) {
            case 0: // Vermelho
                gpio_put(LED_RED_PIN, 1);
                strcpy(color_string, "VERMELHO");
                printf("LED mudou para VERMELHO\n");
                break;
            case 1: // Verde
                gpio_put(LED_GREEN_PIN, 1);
                strcpy(color_string, "VERDE");
                printf("LED mudou para VERDE\n");
                break;
            case 2: // Azul
                gpio_put(LED_BLUE_PIN, 1);
                strcpy(color_string, "AZUL");
                printf("LED mudou para AZUL\n");
                break;
        }

        // Envia o conteúdo da string (cópia de 10 bytes)
        BaseType_t send_result = xQueueSend(xQueueColor, &color_string[0], pdMS_TO_TICKS(100));

        if (send_result != pdPASS) {
            printf("ERRO: Tarefa 2 nao conseguiu enviar '%s' para Queue (Queue cheia ou timeout)!\n", color_string);
        } else {
            printf("Tarefa 2: Cor '%s' enviada para Queue com SUCESSO\n", color_string);
        }
    }
}


// Tarefa 3: Escreve na primeira linha do OLED o estado do botão usando Mutex
void vTaskOledButtonStatus(void *pvParameters) {
    EventBits_t bits;
    char status_string[10];
    struct render_area area_line1;
    
    // Área para renderizar apenas a primeira linha
    area_line1.start_column = 0;
    area_line1.end_column = ssd1306_width - 1;
    area_line1.start_page = 0;
    area_line1.end_page = 0;
    calculate_render_area_buffer_length(&area_line1);
    
    printf("Tarefa OledButtonStatus iniciada. Aguardando eventos...\n");
    
    while (1) {
        // Espera por um dos bits: botão ON (bit 2) ou botão OFF
        bits = xEventGroupWaitBits(xEventGroup,
                                  BUTTON_ON_BIT_2 | BUTTON_OFF_BIT,
                                  pdTRUE,    // Limpa os bits após ler
                                  pdFALSE,   // Não precisa de todos os bits
                                  portMAX_DELAY);
        
        // Pega o Mutex antes de usar o display
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
            // Limpa a primeira linha
            for (int x = 0; x < ssd1306_width; x++) {
                ssd1306_set_pixel(ssd1306_buffer, x, 0, false);
                ssd1306_set_pixel(ssd1306_buffer, x, 1, false);
                ssd1306_set_pixel(ssd1306_buffer, x, 2, false);
                ssd1306_set_pixel(ssd1306_buffer, x, 3, false);
                ssd1306_set_pixel(ssd1306_buffer, x, 4, false);
                ssd1306_set_pixel(ssd1306_buffer, x, 5, false);
                ssd1306_set_pixel(ssd1306_buffer, x, 6, false);
                ssd1306_set_pixel(ssd1306_buffer, x, 7, false);
            }
            
            // Determina qual string escrever
            if (bits & BUTTON_ON_BIT_2) {
                strcpy(status_string, "BOTAO ON");
                printf("Tarefa 3: Escrevendo 'BOTAO ON' na PRIMEIRA linha (y=0)\n");
            } else {
                strcpy(status_string, "BOTAO OFF");
                printf("Tarefa 3: Escrevendo 'BOTAO OFF' na PRIMEIRA linha (y=0)\n");
            }
            
            // Escreve a string APENAS na primeira linha (y=0)
            ssd1306_draw_string(ssd1306_buffer, 0, 0, status_string);
            
            // Atualiza APENAS a primeira linha no display (página 0)
            // Calcula offset no buffer: página * largura + coluna_inicial
            int buffer_offset_line1 = area_line1.start_page * ssd1306_width + area_line1.start_column;
            render_on_display(ssd1306_buffer + buffer_offset_line1, &area_line1);
            printf("Tarefa 3: Display atualizado - apenas pagina 0 (primeira linha)\n");
            
            // Libera o Mutex
            xSemaphoreGive(xMutex);
        }
    }
}

// Tarefa 4: Recebe string da Queue e escreve na segunda linha do OLED usando Mutex
void vTaskOledColorDisplay(void *pvParameters) {
    char color_string[10] = {0};  // Inicializa para evitar lixo de memória
    struct render_area area_line2;
    
    // Define área da segunda linha (y = 16 está na página 2, pois cada página = 8px)
    area_line2.start_column = 0;
    area_line2.end_column = ssd1306_width - 1;
    area_line2.start_page = 2;
    area_line2.end_page = 2;
    calculate_render_area_buffer_length(&area_line2);

    printf("Tarefa OledColorDisplay iniciada. Aguardando Queue...\n");

    while (1) {
        // Espera string da Queue (bloqueia até chegar algo)
        BaseType_t result = xQueueReceive(xQueueColor, &color_string[0], portMAX_DELAY);
        if (result == pdPASS) {
            // Garante que a string está null-terminada
            color_string[sizeof(color_string) - 1] = '\0';
            printf("Tarefa 4: Recebeu '%s' da Queue e escrevendo na segunda linha\n", color_string);

            // Pega o Mutex antes de desenhar
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {

                // Limpa apenas a segunda linha (y = 16-23, página 2)
                for (int x = 0; x < ssd1306_width; x++) {
                    ssd1306_set_pixel(ssd1306_buffer, x, 16, false);
                    ssd1306_set_pixel(ssd1306_buffer, x, 17, false);
                    ssd1306_set_pixel(ssd1306_buffer, x, 18, false);
                    ssd1306_set_pixel(ssd1306_buffer, x, 19, false);
                    ssd1306_set_pixel(ssd1306_buffer, x, 20, false);
                    ssd1306_set_pixel(ssd1306_buffer, x, 21, false);
                    ssd1306_set_pixel(ssd1306_buffer, x, 22, false);
                    ssd1306_set_pixel(ssd1306_buffer, x, 23, false);
                }

                // Escreve a string da cor
                ssd1306_draw_string(ssd1306_buffer, 0, 16, color_string);
                printf("Tarefa 4: Escrevendo '%s' na segunda linha\n", color_string);

                // Atualiza apenas a área da segunda linha
                // Calcula offset no buffer: página * largura + coluna_inicial
                int buffer_offset_line2 = area_line2.start_page * ssd1306_width + area_line2.start_column;
                render_on_display(ssd1306_buffer + buffer_offset_line2, &area_line2);
                printf("Tarefa 4: Display atualizado com '%s'\n", color_string);

                // Libera o Mutex
                xSemaphoreGive(xMutex);
            }
        }
    }
}


int main() {
    // Inicializa console
    stdio_init_all();
    sleep_ms(100);  // Aguarda console estar pronto
    
    printf("Iniciando projeto FreeRTOS Desafio...\n");
    
    // Cria objetos FreeRTOS (antes de criar as tarefas)
    xEventGroup = xEventGroupCreate();
    if (xEventGroup == NULL) {
        printf("Erro: não conseguiu criar o Event Group!\n");
        while (1);
    }
    
    xQueueColor = xQueueCreate(5, sizeof(char[10]));  // 5 elementos, 10 bytes cada
    if (xQueueColor == NULL) {
        printf("Erro: não conseguiu criar a Queue!\n");
        while (1);
    }
    
    xMutex = xSemaphoreCreateMutex();
    if (xMutex == NULL) {
        printf("Erro: não conseguiu criar o Mutex!\n");
        while (1);
    }
    
    // Inicializa hardware
    init_gpios();
    init_display();
    
    // Cria as tarefas
    // Nota: Tarefa 4 criada antes da Tarefa 3 para estar pronta para receber dados da Queue
    if (xTaskCreate(vTaskButtonReader, "ButtonReader", 512, NULL, 1, NULL) != pdPASS) {
        printf("Erro ao criar tarefa ButtonReader!\n");
        while (1);
    }
    if (xTaskCreate(vTaskLedController, "LedController", 512, NULL, 1, NULL) != pdPASS) {
        printf("Erro ao criar tarefa LedController!\n");
        while (1);
    }
    if (xTaskCreate(vTaskOledColorDisplay, "OledColorDisplay", 1024, NULL, 1, NULL) != pdPASS) {
        printf("Erro ao criar tarefa OledColorDisplay!\n");
        while (1);
    }
    if (xTaskCreate(vTaskOledButtonStatus, "OledButtonStatus", 1024, NULL, 1, NULL) != pdPASS) {
        printf("Erro ao criar tarefa OledButtonStatus!\n");
        while (1);
    }
    
    printf("Todas as tarefas criadas. Iniciando scheduler...\n");
    vTaskStartScheduler();
    
    // Nunca deveria chegar aqui
    printf("Erro: Scheduler não iniciou!\n");
    while (1);
}
