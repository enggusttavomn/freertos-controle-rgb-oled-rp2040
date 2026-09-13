/**
 * @file ssd1306.h
 * @brief Declarações das funções de alto e baixo nível para o display SSD1306.
 *
 * Este cabeçalho expõe as rotinas implementadas em @c ssd1306_i2c.c para:
 * - inicializar o display;
 * - montar área de renderização;
 * - desenhar pixels, linhas, caracteres e strings;
 * - enviar buffers e comandos;
 * - operar também no modo bitmap com a estrutura @c ssd1306_t.
 */

#include "ssd1306_i2c.h"

/**
 * @brief Calcula quantos bytes o buffer deve ter para uma determinada área.
 * @param area Ponteiro para a área de renderização a ser preenchida.
 */
extern void calculate_render_area_buffer_length(struct render_area *area);

/**
 * @brief Envia um único comando ao SSD1306 (via I2C).
 * @param cmd código do comando.
 */
extern void ssd1306_send_command(uint8_t cmd);

/**
 * @brief Envia uma lista sequencial de comandos ao SSD1306.
 * @param ssd ponteiro para array com os comandos.
 * @param number quantidade de comandos no array.
 */
extern void ssd1306_send_command_list(uint8_t *ssd, int number);

/**
 * @brief Envia ao display um buffer de dados já pronto.
 * @param ssd buffer de dados de vídeo.
 * @param buffer_length tamanho do buffer, em bytes.
 */
extern void ssd1306_send_buffer(uint8_t ssd[], int buffer_length);

/**
 * @brief Inicializa o display no modo de texto/gráfico padrão.
 */
extern void ssd1306_init(void);

/**
 * @brief Ativa ou desativa o scroll do display.
 * @param set @c true para ativar, @c false para desativar.
 */
extern void ssd1306_scroll(bool set);

/**
 * @brief Atualiza no display apenas a área informada.
 * @param ssd ponteiro para o framebuffer na RAM.
 * @param area área de renderização a ser enviada.
 */
extern void render_on_display(uint8_t *ssd, struct render_area *area);

/**
 * @brief Define (ou apaga) um único pixel do framebuffer.
 * @param ssd framebuffer.
 * @param x coordenada x (0 .. largura-1).
 * @param y coordenada y (0 .. altura-1).
 * @param set @c true para acender, @c false para apagar.
 */
extern void ssd1306_set_pixel(uint8_t *ssd, int x, int y, bool set);

/**
 * @brief Desenha uma linha usando o algoritmo de Bresenham.
 */
extern void ssd1306_draw_line(uint8_t *ssd, int x_0, int y_0, int x_1, int y_1, bool set);

/**
 * @brief Desenha um caractere (8x8) na posição indicada.
 */
extern void ssd1306_draw_char(uint8_t *ssd, int16_t x, int16_t y, uint8_t character);

/**
 * @brief Desenha uma string chamando várias vezes @c ssd1306_draw_char().
 */
extern void ssd1306_draw_string(uint8_t *ssd, int16_t x, int16_t y, char *string);

/**
 * @brief Envia um comando usando a estrutura de contexto @c ssd1306_t.
 */
extern void ssd1306_command(ssd1306_t *ssd, uint8_t command);

/**
 * @brief Configura o display para o modo bitmap (uso de RAM interna).
 */
extern void ssd1306_config(ssd1306_t *ssd);

/**
 * @brief Inicializa o display no modo bitmap, preenchendo o contexto.
 */
extern void ssd1306_init_bm(ssd1306_t *ssd,
                            uint8_t width,
                            uint8_t height,
                            bool external_vcc,
                            uint8_t address,
                            i2c_inst_t *i2c);

/**
 * @brief Envia o conteúdo atual do buffer @c ram_buffer para o display.
 */
extern void ssd1306_send_data(ssd1306_t *ssd);

/**
 * @brief Desenha um bitmap completo no display, usando o contexto.
 */
extern void ssd1306_draw_bitmap(ssd1306_t *ssd, const uint8_t *bitmap);
