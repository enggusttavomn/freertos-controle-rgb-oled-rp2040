#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/**
 * @file ssd1306_i2c.h
 * @brief Definições e tipos para controle do display OLED SSD1306 via I2C.
 *
 * Este arquivo reúne:
 * - dimensões do display;
 * - endereço I2C;
 * - comandos do controlador SSD1306;
 * - estrutura de área de renderização;
 * - estrutura de contexto @c ssd1306_t usada pelas funções do driver.
 *
 * É o cabeçalho correspondente a @c ssd1306_i2c.c.
 */

#ifndef ssd1306_inc_h
#define ssd1306_inc_h

/** @brief Altura do display em pixels. */
#define ssd1306_height 64
/** @brief Largura do display em pixels. */
#define ssd1306_width 128

/** @brief Endereço I2C padrão do módulo SSD1306. */
#define ssd1306_i2c_address _u(0x3C)

/** @brief Frequência do barramento I2C (kHz). */
#define ssd1306_i2c_clock 400

/* -------------------- Comandos de configuração (endereços) -------------------- */

/** @brief Define o modo de memória. */
#define ssd1306_set_memory_mode _u(0x20)
/** @brief Define intervalo de colunas. */
#define ssd1306_set_column_address _u(0x21)
/** @brief Define intervalo de páginas. */
#define ssd1306_set_page_address _u(0x22)
/** @brief Configura scroll horizontal. */
#define ssd1306_set_horizontal_scroll _u(0x26)
/** @brief Ativa/desativa scroll. */
#define ssd1306_set_scroll _u(0x2E)

/** @brief Linha inicial de exibição. */
#define ssd1306_set_display_start_line _u(0x40)

/** @brief Ajuste de contraste. */
#define ssd1306_set_contrast _u(0x81)
/** @brief Comando para carga da bomba interna. */
#define ssd1306_set_charge_pump _u(0x8D)

/** @brief Remapeia segmentos. */
#define ssd1306_set_segment_remap _u(0xA0)
/** @brief Liga todos os pixels (modo RAM). */
#define ssd1306_set_entire_on _u(0xA4)
/** @brief Liga todos os pixels (forçado). */
#define ssd1306_set_all_on _u(0xA5)
/** @brief Exibição normal. */
#define ssd1306_set_normal_display _u(0xA6)
/** @brief Exibição invertida. */
#define ssd1306_set_inverse_display _u(0xA7)
/** @brief Define razão de multiplexação. */
#define ssd1306_set_mux_ratio _u(0xA8)
/** @brief Liga/desliga display. */
#define ssd1306_set_display _u(0xAE)
/** @brief Direção de varredura COM. */
#define ssd1306_set_common_output_direction _u(0xC0)
/** @brief Direção invertida de varredura COM. */
#define ssd1306_set_common_output_direction_flip _u(0xC0)

/** @brief Deslocamento vertical de exibição. */
#define ssd1306_set_display_offset _u(0xD3)
/** @brief Clock interno. */
#define ssd1306_set_display_clock_divide_ratio _u(0xD5)
/** @brief Tempo de pré-carga. */
#define ssd1306_set_precharge _u(0xD9)
/** @brief Configuração de pinos COM. */
#define ssd1306_set_common_pin_configuration _u(0xDA)
/** @brief Nível de deseleção VCOMH. */
#define ssd1306_set_vcomh_deselect_level _u(0xDB)

/** @brief Altura de uma página (8 px). */
#define ssd1306_page_height _u(8)
/** @brief Número de páginas do display. */
#define ssd1306_n_pages (ssd1306_height / ssd1306_page_height)
/** @brief Tamanho do buffer necessário para desenhar o display inteiro. */
#define ssd1306_buffer_length (ssd1306_n_pages * ssd1306_width)

/** @brief Modo de escrita (usado pelo driver). */
#define ssd1306_write_mode _u(0xFE)
/** @brief Modo de leitura (usado pelo driver). */
#define ssd1306_read_mode _u(0xFF)

/**
 * @brief Área de renderização dentro do display.
 *
 * Essa estrutura é usada para atualizar apenas um retângulo do OLED, evitando
 * regravar o display inteiro.
 */
struct render_area {
    uint8_t start_column;  /**< Coluna inicial.  */
    uint8_t end_column;    /**< Coluna final (inclusiva). */
    uint8_t start_page;    /**< Página inicial. */
    uint8_t end_page;      /**< Página final (inclusiva). */

    int buffer_length;     /**< Quantidade de bytes correspondente à área. */
};

/**
 * @brief Estrutura de contexto para operação do SSD1306 em modo bitmap.
 *
 * É preenchida pelas funções de inicialização e passada às demais rotinas
 * para envio de dados.
 */
typedef struct {
  uint8_t width, height, pages, address;
  i2c_inst_t * i2c_port;
  bool external_vcc;
  uint8_t *ram_buffer;
  size_t bufsize;
  uint8_t port_buffer[2];
} ssd1306_t;

#endif
