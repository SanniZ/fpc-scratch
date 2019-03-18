/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_FS
#define INCLUSION_GUARD_FPC_FS

int fpc_fs_remove(const char* path);
int fpc_fs_read(const char* path, uint8_t *buf, uint32_t size);
int fpc_fs_write(const char* path, uint8_t *buf, uint32_t size);
int fpc_fs_get_filesize(const char* path, uint32_t *size);

#endif // INCLUSION_GUARD_FPC_FS
