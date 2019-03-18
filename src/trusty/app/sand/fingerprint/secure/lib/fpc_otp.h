/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef _FPC_OTP_H_
#define _FPC_OTP_H_

#include "fpc_hw_identification_types.h"

/*
 * Is OTP supported?
 *
 * @return 1  - Yes
 *         0  - No
 */
int fpc_otp_is_supported(void);

/*
 * Get sensor info
 *
 * @param[out] data  Destination buffer. The data buffer will be filled regardless of otp data validity.
 *
 * @return FPC_RESULT_OK - Data copied
 *         FPC_RESULT_ERROR_PARAMETER - data is NULL
 */
int fpc_otp_get_sensor_info(fpc_hw_module_info_t* data);

#endif /* _FPC_OTP_H_ */
