/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef _FPC_IMAGE_STORAGE_
#define _FPC_IMAGE_STORAGE_

#include <stdint.h>
#include "fpc_external.h"

/**
 * Retrieve required size for storing the global fpc_image_data_t.
 *
 * @param image        - image_t containing the blob
 * @param[out] size    - required size of buffer in bytes
 *
 * @return 0 on success, else -1
 */
int32_t fpc_storage_get_raw_size(
        const image_t *image, uint32_t *size);

/**
 * Retrieve global fpc_image_data_t instance as raw data.
 *
 * @param image        - image_t containing the blob
 * @param[out] buffer  - target buffer
 * @param size         - size of target buffer, same size as returned by
 *                       fpc_storage_get_raw_size()
 *
 * @return 0 on success, -1 if size is too small or buffer is NULL
 */
int32_t fpc_storage_retrieve_raw(
        const image_t *image, uint8_t *buffer, const uint32_t size);

/*
 * Inject and replace content of global fpc_image_data_t.
 *
 * This function allows you to inject an arbitrary fpc_image_data_t.
 * This can then be sent to preprocessing or matching.
 *
 * @param[out] buffer fpc_image_data_t to be injected
 * @param[in] size    size of 'buffer'. Must match current
 *                    fpc_image_data's capacity exactly.
 * @param[in] image   target image_t
 *
 * @return 0 on success, -1 if size is to small or buffer is NULL.
 */
int32_t fpc_storage_inject_raw(
        uint8_t* buffer, uint32_t size, image_t* image);

/*
 * Extract a dead pixel list from image_t
 * A dead pixel list is a list of indexes of malfunctioning pixels.
 *
 * @param[out] buffer target buffer to store pixel list in.
 * @param[in] size    size of target buffer.
 * @param[in] image   image_t containing the list to be exctracted.
 * @return 0 on success, -1 if size is to small or buffer is NULL.
 */
int32_t fpc_storage_retrieve_dead_pixel_list(
                    uint8_t* buffer,
                    uint32_t* size,
                    image_t* image);

#endif /* _FPC_IMAGE_STORAGE_ */
