

#ifndef _SPI_H_
#define _SPI_H_

void spi_send_char_blocking(uint8_t c);
void spi_init(void);
void spi_send_buffer_start(void);
void spi_send_buffer_fill(uint8_t c);
void spi_send_buffer_end(void);

#endif
