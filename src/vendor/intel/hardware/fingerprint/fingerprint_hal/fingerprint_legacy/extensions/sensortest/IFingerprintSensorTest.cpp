/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <binder/IPCThreadState.h>
#include <binder/PermissionCache.h>
#include <utils/String16.h>
#include "IFingerprintSensorTest.h"
#include <android/log.h>

/* When a interface operation uses a parcel class to pass data the first 4 bytes written/read is
 * header information.
 * Use reply->writeInt32(HAS_PARCEL_DATA) to write a valid parcel header then proceed to
 * write the actual data.
 * Use reply->writeInt32(NO_PARCEL_DATA) to write null as reply parcel.
 * Use int parcel_header = data.readInt32() to read the header when receiving a parcel before
 * proceeding to read the actual data.
 */
#define NO_PARCEL_DATA 0
#define HAS_PARCEL_DATA 1

namespace android {

static const String16 FINGERPRINT_EXTENSION_SENSOR_TEST(
        "com.fingerprints.extension.SENSOR_TEST");

/***** ISensorTestCallback *****/

class BpSensorTestCallback : public BpInterface<ISensorTestCallback> {
     public:
        BpSensorTestCallback(const sp<IBinder>& impl)
                : BpInterface<ISensorTestCallback>(impl) {
        }

        virtual void onResult(fpc_hal_ext_sensortest_test_result_t* result) {
            Parcel data, reply;
            data.writeInterfaceToken(
                    ISensorTestCallback::getInterfaceDescriptor());
            if (result) {
                data.writeInt32(HAS_PARCEL_DATA);
                data.writeInt32(result->result_code);
                data.writeString16(String16(result->result_string));
                data.writeInt32(result->error_code);
                data.writeString16(String16(result->error_string));
                data.writeInt32((result->image_fetched ? 1:0));
                if (result->image_fetched) {
                    data.writeByteArray(result->image_result.buffer_size,
                                        result->image_result.buffer);
                }
            } else {
                data.writeInt32(NO_PARCEL_DATA);
            }
            remote()->transact(ON_RESULT, data, &reply, IBinder::FLAG_ONEWAY);
        }
};

IMPLEMENT_META_INTERFACE(SensorTestCallback, "com.fingerprints.extension.sensortest.ISensorTestCallback");

/***** ISensorTestCaptureCallback *****/

class BpSensorTestCaptureCallback : public BpInterface<
        ISensorTestCaptureCallback> {
     public:
        BpSensorTestCaptureCallback(const sp<IBinder>& impl)
                : BpInterface<ISensorTestCaptureCallback>(impl) {
        }

        virtual void onAcquired(int32_t acquiredInfo) {
            Parcel data, reply;
            data.writeInterfaceToken(
                    ISensorTestCaptureCallback::getInterfaceDescriptor());
            data.writeInt32(acquiredInfo);
            remote()->transact(ON_ACQUIRED, data, &reply, IBinder::FLAG_ONEWAY);
        }

        virtual void onError(int32_t error) {
            Parcel data, reply;
            data.writeInterfaceToken(
                    ISensorTestCaptureCallback::getInterfaceDescriptor());
            data.writeInt32(error);
            remote()->transact(ON_ERROR, data, &reply, IBinder::FLAG_ONEWAY);
        }
};

IMPLEMENT_META_INTERFACE(SensorTestCaptureCallback, "com.fingerprints.extension.sensortest.ICaptureCallback");

/***** IFingerprintSensorTest *****/

const android::String16 IFingerprintSensorTest::descriptor(
        "com.fingerprints.extension.sensortest.IFingerprintSensorTest");

const android::String16& IFingerprintSensorTest::getInterfaceDescriptor() const {
    return IFingerprintSensorTest::descriptor;
}

status_t BnFingerprintSensorTest::onTransact(uint32_t code, const Parcel& data,
                                             Parcel* reply, uint32_t flags) {
    switch (code) {
        case GET_SENSOR_INFO: {
            CHECK_INTERFACE(IFingerprintSensorTest, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSOR_TEST)) {
                return PERMISSION_DENIED;
            }
            fpc_hw_module_info_t info;
            info.sensor_die_info.hardware_id = INVALID_HARDWARE_ID;
            info.sensor_die_info.lot_id[0] = INVALID_LOT_ID;
            info.sensor_die_info.wafer_id = INVALID_WAFTER_ID;
            info.sensor_die_info.wafer_position_x = INVALID_WAFER_POSITION;
            info.sensor_die_info.wafer_position_y = INVALID_WAFER_POSITION;
            info.sensor_die_info.production_timestamp[0] = INVALID_PROD_TIMESTAMP;
            info.sensor_die_info.valid_field = 0;
            info.companion_die_info.hardware_id = INVALID_HARDWARE_ID;
            info.companion_die_info.lot_id[0] = INVALID_LOT_ID;
            info.companion_die_info.valid_field = 0;
            info.vendor_otp_info.vendor_data_size = 0;
            info.vendor_otp_info.valid_field = 0;
            info.otp_error_info.total_num_bit_errors = 0;
            info.otp_error_info.max_num_bit_errors_in_byte = 0;
            info.product_type = 0;

            getSensorInfo(&info);

            reply->writeNoException();
            reply->writeInt32(HAS_PARCEL_DATA);
            reply->writeInt32(info.sensor_die_info.valid_field);
            reply->writeInt32(info.sensor_die_info.hardware_id);
            reply->writeString16(String16(info.sensor_die_info.lot_id));
            reply->writeInt32(info.sensor_die_info.wafer_id);
            reply->writeInt32(info.sensor_die_info.wafer_position_x);
            reply->writeInt32(info.sensor_die_info.wafer_position_y);
            reply->writeString16(String16(info.sensor_die_info.production_timestamp));
            reply->writeInt32(info.companion_die_info.valid_field);
            reply->writeInt32(info.companion_die_info.hardware_id);
            reply->writeString16(String16(info.companion_die_info.lot_id));
            reply->writeInt32(info.vendor_otp_info.valid_field);
            reply->writeByteArray(info.vendor_otp_info.vendor_data_size,
                                  info.vendor_otp_info.vendor_data);
            reply->writeInt32(info.otp_error_info.total_num_bit_errors);
            reply->writeInt32(info.otp_error_info.max_num_bit_errors_in_byte);
            reply->writeInt32(info.product_type);
            return NO_ERROR;
        }
        case GET_SENSOR_TESTS: {
            CHECK_INTERFACE(IFingerprintSensorTest, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSOR_TEST)) {
                return PERMISSION_DENIED;
            }
            fpc_hal_ext_sensortest_tests_t* sensorTests =
                    (fpc_hal_ext_sensortest_tests_t*) malloc(
                            sizeof(fpc_hal_ext_sensortest_tests_t));

            if (sensorTests) {
                memset(sensorTests, 0, sizeof(fpc_hal_ext_sensortest_tests_t));

                getSensorTests(sensorTests);
                reply->writeNoException();
                if (sensorTests->size > 0) {
                    reply->writeInt32(sensorTests->size);
                    for (uint32_t i = 0; i < sensorTests->size; i++) {
                        reply->writeInt32(HAS_PARCEL_DATA);
                        reply->writeString16(
                                String16(sensorTests->tests[i].name));
                        reply->writeString16(
                                String16(sensorTests->tests[i].description));
                        reply->writeInt32(
                                sensorTests->tests[i].wait_for_finger_down ?
                                        1 : 0);
                        reply->writeString16(
                                String16(
                                        sensorTests->tests[i].rubber_stamp_type));
                    }
                } else {
                    reply->writeInt32(0);
                }
                free(sensorTests);
                sensorTests = NULL;
                return NO_ERROR;
            } else {
                ALOGE("Failed to allocate sensorTests");
                return FAILED_TRANSACTION;
            }
        }
        case RUN_SENSOR_TEST: {
            CHECK_INTERFACE(IFingerprintSensorTest, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSOR_TEST)) {
                return PERMISSION_DENIED;
            }
            sp < ISensorTestCallback > callback = interface_cast
                    < ISensorTestCallback > (data.readStrongBinder());

            int parcel_header = data.readInt32();
            if (parcel_header == HAS_PARCEL_DATA) {
                fpc_hal_ext_sensortest_test_t* test =
                        (fpc_hal_ext_sensortest_test_t*) malloc(
                                sizeof(fpc_hal_ext_sensortest_test_t));

                if (test) {
                    memset(test, 0, sizeof(fpc_hal_ext_sensortest_test_t));

                    String8 name(data.readString16());
                    strlcpy(test->name, name.string(),
                            FPC_HAL_EXT_SENSORTEST_TEST_NAME_MAX);

                    String8 description(data.readString16());
                    strlcpy(test->description, description.string(),
                            FPC_HAL_EXT_SENSORTEST_TEST_DESCRIPTION_MAX);

                    test->wait_for_finger_down = data.readInt32() != 0;

                    String8 rubberStampType(data.readString16());
                    strlcpy(test->rubber_stamp_type, rubberStampType.string(),
                            FPC_HAL_EXT_SENSORTEST_TEST_STAMP_TYPE_MAX);

                    fpc_hal_ext_sensortest_test_input_t* input =
                            (fpc_hal_ext_sensortest_test_input_t*) malloc(
                                    sizeof(fpc_hal_ext_sensortest_test_input_t));

                    if (input) {
                        memset(input, 0,
                               sizeof(fpc_hal_ext_sensortest_test_input_t));

                        if (data.readInt32() == HAS_PARCEL_DATA) {
                            String8 testLimitsKeyValuePair(data.readString16());
                            strlcpy(input->test_limits_key_value_pair,
                                    testLimitsKeyValuePair.string(),
                                    FPC_HAL_EXT_SENSORTEST_TEST_INPUT_MAX);
                        }
                    }

                    runSensorTest(callback, test, input);

                    free(test);
                    test = NULL;

                    if (input) {
                        free(input);
                        input = NULL;
                    }
                    reply->writeNoException();
                    return NO_ERROR;

                } else {
                    ALOGE("Failed to allocate sensorTests");
                    return FAILED_TRANSACTION;
                }
            } else if (parcel_header == NO_PARCEL_DATA) {
                ALOGE("Parcel header indicating no data, operation RUN_SENSOR_TEST(%d)"
                      "cannot be performed.",
                      RUN_SENSOR_TEST);
                return FAILED_TRANSACTION;
            } else {
                ALOGE("Received unrecognized parcel header: %d, operation RUN_SENSOR_TEST(%d)"
                      "will not be performed.",
                      parcel_header, RUN_SENSOR_TEST);
                return FAILED_TRANSACTION;
            }
        }
        case CANCEL_SENSOR_TEST: {
            CHECK_INTERFACE(IFingerprintSensorTest, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSOR_TEST)) {
                return PERMISSION_DENIED;
            }
            cancelSensorTest();
            reply->writeNoException();
            return NO_ERROR;
        }
        case CAPTURE: {
            CHECK_INTERFACE(IFingerprintSensorTest, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSOR_TEST)) {
                return PERMISSION_DENIED;
            }
            sp < ISensorTestCaptureCallback > callback = interface_cast
                    < ISensorTestCaptureCallback > (data.readStrongBinder());
            bool waitForFinger = data.readInt32() != 0;
            bool uncalibrated = data.readInt32() != 0;
            capture(callback, waitForFinger, uncalibrated);
            reply->writeNoException();
            return NO_ERROR;
        }
        case CANCEL_CAPTURE: {
            CHECK_INTERFACE(IFingerprintSensorTest, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSOR_TEST)) {
                return PERMISSION_DENIED;
            }
            cancelCapture();
            reply->writeNoException();
            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

bool BnFingerprintSensorTest::checkPermission(const String16& permission) {
    const IPCThreadState* ipc = IPCThreadState::self();
    const int calling_pid = ipc->getCallingPid();
    const int calling_uid = ipc->getCallingUid();
    return PermissionCache::checkPermission(permission, calling_pid,
                                            calling_uid);
}

}
// namespace android
