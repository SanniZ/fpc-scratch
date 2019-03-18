/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TA_INTERFACE
#define INCLUSION_GUARD_FPC_TA_INTERFACE

#include <stdint.h>

/* max size for tipc
 * payload is not contain tipc_header and a word for data transfer response.
 * max_chunk size needs to reduce 16B for tipc_header and 4B for response.
 */
#define MAX_CHUNK \
	((4*1024) - sizeof(fpc_ta_bio_command_t) - sizeof(int32_t) - 16 /* tipc header */)


typedef struct {
    int32_t target;
    int32_t command;
} fpc_ta_cmd_header_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    int32_t response;
} fpc_ta_simple_command_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    int32_t response;
    uint32_t size;
    uint8_t array[];
} fpc_ta_byte_array_msg_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    int32_t response;
    uint32_t size;
} fpc_ta_size_msg_t;

#endif // INCLUSION_GUARD_FPC_TA_INTERFACE
