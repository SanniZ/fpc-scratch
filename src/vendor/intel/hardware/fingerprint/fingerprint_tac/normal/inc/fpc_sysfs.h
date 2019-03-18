/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_SYSFS_H
#define FPC_SYSFS_H

int fpc_sysfs_path_by_attr(const char *attr, const char *attr_val,
                                 const char *base, char *path, int path_max);

int fpc_sysfs_node_write(int base_fd, const char* name, const char* value);

#endif // FPC_SYSFS_H

