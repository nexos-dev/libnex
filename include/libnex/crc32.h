/*
    crc32.h - definitions to compute CRC32 checksum of buffer
    Licensed under the CC0 license
    See https://creativecommons.org/publicdomain/zero/1.0/legalcode
*/

#ifndef _CRC32_H
#define _CRC32_H

#include <libnex/base.h>
#include <libnex/libnex_config.h>
#include <stdint.h>

/**
 * @brief Calculates the CRC32 checksum of buffer buf
 * @param buf buffer to calculate checksum of
 * @param len length of buf
 * @return the CRC32 checksum
 */
LIBNEX_PUBLIC uint32_t Crc32Calc (uint8_t* buf, uint32_t len);

#endif
