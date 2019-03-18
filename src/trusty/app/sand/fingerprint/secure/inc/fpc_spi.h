/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef __FPC_SPI_H__
#define __FPC_SPI_H__

#include <stddef.h>
#include <stdint.h>

int fpc_spi_init(uint32_t freq_low_khz, uint32_t freq_high_khz);

int fpc_spi_destroy(void);

int fpc_spi_select_low_freq(void);
int fpc_spi_select_high_freq(void);

int fpc_spi_read(uint8_t *buff, size_t num_bytes);
int fpc_spi_write(uint8_t *buff, size_t num_bytes);

/**
 * Setup the platform SPI API to do a full duplex (read and write) action.
 *
 * NOTE!
 * This function REQUIRES that the rx buffer is large enough to fit both tx data and expected rx
 * data ie. that the size of rx buffer is at least tx_bytes + rx_bytes!
 * NOTE!
 *
 * The first received rx data will be at tx_bytes offset into the rx buffer.
 *
 * @param[in]      tx       - The data buffer for tx data
 * @param[in]      tx_bytes - The size of tx data to be written on SPI bus
 * @param[in/out]  rx       - The data buffer for rx data
 * @param[in]      rx_bytes - The expected size of rx data to be received (not the actual rx buffer size
 *                            read note above)
 *
 */
int fpc_spi_writeread(uint8_t *tx, size_t tx_bytes, uint8_t *rx, size_t rx_bytes);

int fpc_spi_cs_low(void);
int fpc_spi_cs_high(void);

#endif // __FPC_SPI_H__

