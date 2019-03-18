/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include "fpc_types.h"
#include "fpc_tac.h"
#include "fpc_log.h"
#include "fpc_ta_targets.h"
#include "fpc_ta_interface.h"
#include "fpc_tee.h"
#include "fpc_tee_internal.h"
#include "fpc_irq_device.h"
#include "fpc_reset_device.h"
#include "fpc_ta_sensor_interface.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_sensor_internal.h"

#define CHECK_FINGER_UP_SLEEP_TIME_MS   20
#define USECS_PER_MSEC                  1000
#define NS_PER_MSEC                     1000000l
#define FPC_MS_PER_SEC                  1000
#define MAX_FINGER_LOST_CHECK_TIME_MS   2000l

/* Retries are split between REE and TEE. The retry here(REE) is combined
 * with polling of finger present status register in TEE */
#define MAX_QUALIFY_CAPTURE_RETRIES 4

/*
 * If more time than this has elapsed while waiting for finger down
 * the counter in the qualification loop is reset.
 */
#define QUALIFICATION_COUNTER_RESET_TIME_MS 500

/* Number of times to retry the communication */
#ifndef MAX_TRANSFER_RETRIES
#define MAX_TRANSFER_RETRIES           2
#endif

/*
 * Increment in ms for the sleep time for each communcation attempt,
 * that means the total wait time is
 * MAX_TRANSFER_RETRIES * (MAX_TRANSFER_RETRIES - 1) / 2 * TRANSFER_RETRY_SLEEP_INC_MS
 */
#ifndef TRANSFER_RETRY_SLEEP_INC_MS
#define TRANSFER_RETRY_SLEEP_INC_MS     10
#endif

#ifndef TRANSFER_RETRY_MAX_SLEEP_TIME
#define TRANSFER_RETRY_MAX_SLEEP_TIME   500
#endif

/*** internal api ***/
int fpc_tee_sensor_cancelled(fpc_tee_sensor_t* sensor)
{
    pthread_mutex_lock(&sensor->mutex);
    int cancelled = sensor->cancelled;
    pthread_mutex_unlock(&sensor->mutex);
    return cancelled;
}

static int sensor_transfer(fpc_tee_sensor_t* sensor)
{
    int32_t status = -1;
    int32_t sleep_time = 0;

    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensor_command_t* command =
        (fpc_ta_sensor_command_t*) tee->shared_buffer->addr;

    for (int i = 0; i < MAX_TRANSFER_RETRIES; i++) {

        status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
        if (status) {
            goto out;
        }

        if (0 > command->sensor.response) {
            /* Grab the error before we continue */
            fpc_tee_get_error_log(tee);
        }

        /* Only reset and retry on hardware error */
        if (command->sensor.response != -FPC_ERROR_RESET_HARDWARE) {
            goto out;
        }

#ifdef FPC_CONFIG_NORMAL_SPI_RESET
        status = fpc_reset_spi(sensor->reset);
        if (status) {
            LOGE("%s, spi hardware reset failed", __func__);
        }
#endif

#ifdef FPC_CONFIG_NORMAL_SENSOR_RESET
        status = fpc_reset_sensor(sensor->reset);
        if (status) {
            LOGE("%s, spi hardware reset failed", __func__);
        }
#endif

        status = -FPC_ERROR_HARDWARE;

        /* Increase the wait time slightly to increase the chance of
         * recovery */
        if (sleep_time >= TRANSFER_RETRY_MAX_SLEEP_TIME) {
            sleep_time = TRANSFER_RETRY_MAX_SLEEP_TIME;
        } else {
            sleep_time += TRANSFER_RETRY_SLEEP_INC_MS;
        }
        usleep(sleep_time * USECS_PER_MSEC);

    }

out:

    return status;
}

int fpc_tee_wait_irq(fpc_tee_sensor_t* sensor, int irq_value)
{
    return fpc_irq_wait(sensor->irq, irq_value);
}

int fpc_tee_status_irq(fpc_tee_sensor_t* sensor)
{
    return fpc_irq_status(sensor->irq);
}

static int sensor_command(fpc_tee_sensor_t* sensor, int32_t command_id)
{

    if (!sensor) {
        return -FPC_ERROR_INPUT;
    }

    fpc_tee_t* tee = sensor->tee;

    if (!tee || !tee->shared_buffer) {
        return -FPC_ERROR_INPUT;
    }

    fpc_ta_sensor_command_t* command = (fpc_ta_sensor_command_t*) tee->shared_buffer->addr;
    command->header.command = command_id;
    command->header.target = TARGET_FPC_TA_SENSOR;

    int status = sensor_transfer(sensor);
    if (status) {
        LOGE("%s, Failed to send command: %d to TA, status code: %d", __func__, command_id, status);
        return status;
    }

    return command->sensor.response;
}

static long fpc_get_ms_diff(struct timespec *ts_start, struct timespec *ts_stop)
{
    time_t sec_diff = ts_stop->tv_sec - ts_start->tv_sec;
    long ms_diff = (ts_stop->tv_nsec - ts_start->tv_nsec) / NS_PER_MSEC;

    if (sec_diff > 0) {
        ms_diff += FPC_MS_PER_SEC * sec_diff;
    }
    return ms_diff;
}

#if (FPC_CONFIG_FINGER_LOST_INTERRUPT == 1)
int fpc_tee_wait_finger_lost(fpc_tee_sensor_t* sensor) {
    int status;

    status = sensor_command(sensor, FPC_TA_SENSOR_FINGER_LOST_WAKEUP_SETUP_CMD);
    if (status) {
        goto out;
    }

    status = fpc_irq_wait(sensor->irq, 1);

    (void) sensor_command(sensor, FPC_TA_SENSOR_CHECK_FINGER_LOST_CMD);
out:
    return status;

}
#else

int fpc_tee_wait_finger_lost(fpc_tee_sensor_t* sensor)
{
    int status = 0;
    struct timespec ts_start;
    struct timespec ts_now;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    for (;;) {

        if (fpc_tee_sensor_cancelled(sensor)) {
            status = -FPC_ERROR_CANCELLED;
            goto out;
        }

        status = sensor_command(sensor, FPC_TA_SENSOR_CHECK_FINGER_LOST_CMD);
        if (status > 0) {
            status = 0;
            break;
        } else if (status < 0) {
            goto out;
        } else {
            clock_gettime(CLOCK_MONOTONIC, &ts_now);

            if (fpc_get_ms_diff(&ts_start, &ts_now) < MAX_FINGER_LOST_CHECK_TIME_MS) {
                usleep(CHECK_FINGER_UP_SLEEP_TIME_MS * USECS_PER_MSEC);
            } else {
                status = FPC_CAPTURE_FINGER_STUCK;
                LOGD("Finger stuck - aborting");
                break;
            }
        }
    }

out:
    return status;
}
#endif /*FPC_USE_FINGER_LOST_INTERRUPT */

int fpc_tee_wait_finger_down(fpc_tee_sensor_t* sensor)
{

    int status = sensor_command(sensor, FPC_TA_SENSOR_WAKEUP_SETUP_CMD);

    if (status) {
        return status;
    }

    status = fpc_irq_wait(sensor->irq, 1);

    return status;
}

int fpc_tee_capture_image(fpc_tee_sensor_t* sensor)
{
    LOGD("%s", __func__);
    sensor->cac_result = 0;

    int status = fpc_irq_wakeup_enable(sensor->irq);
    if (status) {
        goto out;
    }

    status = fpc_tee_wait_finger_lost(sensor);
    if (status) {
        goto out;
    }

    // wait for irq(finger down)->qualify->capture
    for (int i = 0; i < MAX_QUALIFY_CAPTURE_RETRIES; i++) {
        struct timespec wait_start;
        struct timespec wait_end;
        clock_gettime(CLOCK_MONOTONIC_RAW, &wait_start);
        status = fpc_tee_wait_finger_down(sensor);
        clock_gettime(CLOCK_MONOTONIC_RAW, &wait_end);
        if (fpc_get_ms_diff(&wait_start, &wait_end) > QUALIFICATION_COUNTER_RESET_TIME_MS) {
            i = 0;
            LOGD("Waited more than %d ms, reset qualification counter",
                 QUALIFICATION_COUNTER_RESET_TIME_MS);
        }
        if (status) {
            goto out;
        }

        // Qualify and capture image
        status = sensor_command(sensor, FPC_TA_SENSOR_CAPTURE_IMAGE_CMD);
        fpc_ta_sensor_capture_info_t* command =
            (fpc_ta_sensor_capture_info_t*) sensor->tee->shared_buffer->addr;
        sensor->cac_result = command->cac_result;

        if (status != FPC_CAPTURE_QUALIFY_ABORT) {
            goto out;
        }
    }

    if (status == FPC_CAPTURE_QUALIFY_ABORT) {
        LOGD("%s Qualify capture exceeded MAX_QUALIFY_CAPTURE_RETRIES", __func__);
    }
out:
    fpc_irq_wakeup_disable(sensor->irq);

    if (status) {
        sensor_command(sensor, FPC_TA_SENSOR_DEEP_SLEEP_CMD);
    }

    return status;
}

#ifdef FPC_CONFIG_ENGINEERING
int fpc_tee_early_stop_ctrl(fpc_tee_sensor_t *sensor, uint8_t *ctrl) {
    int status;

    fpc_tee_t *tee = sensor->tee;

    fpc_ta_sensor_command_t* command = (fpc_ta_sensor_command_t*) tee->shared_buffer->addr;
    command->header.command = FPC_TA_SENSOR_EARLY_STOP_CTRL_CMD;
    command->header.target = TARGET_FPC_TA_SENSOR;
    command->early_stop_ctrl.ctrl = *ctrl;

    status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        LOGE("%s, Failed to send command: EARLY_STOP_CTRL_CMD to TA, status code: %d",
                __func__, status);
        goto out;
    }

    status = command->early_stop_ctrl.header.response;
    *ctrl = command->early_stop_ctrl.ctrl;

out:
    return status;
}
#endif

int32_t fpc_tee_get_last_cac_result(fpc_tee_sensor_t *sensor)
{
    return sensor->cac_result;
}

int fpc_tee_deep_sleep(fpc_tee_sensor_t* sensor)
{
    return sensor_command(sensor, FPC_TA_SENSOR_DEEP_SLEEP_CMD);
}

int fpc_tee_capture_snapshot(fpc_tee_sensor_t* sensor)
{
    int status = sensor_command(sensor, FPC_TA_SENSOR_CAPTURE_IMAGE_CMD);
    fpc_ta_sensor_capture_info_t* command =
        (fpc_ta_sensor_capture_info_t*) sensor->tee->shared_buffer->addr;
    sensor->cac_result = command->cac_result;
    return status;
}

int fpc_tee_is_otp_supported(fpc_tee_sensor_t* sensor, int* result)
{
    LOGD("%s", __func__);

    fpc_tee_t *tee = sensor->tee;

    fpc_ta_simple_command_t* command =
        (fpc_ta_simple_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_SENSOR_IS_OTP_SUPPORTED_CMD;
    command->header.target = TARGET_FPC_TA_SENSOR;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        LOGE("%s, Failed to fetch otp support info, status code: %d", __func__, status);
        goto out;
    }

    *result= command->response;
out:
    return status;
}

int fpc_tee_get_sensor_force_value(fpc_tee_sensor_t* sensor, uint8_t* value)
{
    LOGD("%s", __func__);

    fpc_tee_t *tee = sensor->tee;

    fpc_ta_sensor_command_t* command =
            (fpc_ta_sensor_command_t*) tee->shared_buffer->addr;
    command->header.command = FPC_TA_SENSOR_GET_FORCE_VALUE;
    command->header.target = TARGET_FPC_TA_SENSOR;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    if (status) {
        LOGE("%s, Failed to fetch sensor force value, status code: %d", __func__, status);
        goto out;
    }

    *value = command->get_force_value.value;

out:
    return status;
}

int fpc_tee_is_sensor_force_supported(fpc_tee_sensor_t* sensor, uint8_t* is_supported)
{
    LOGD("%s", __func__);

    fpc_tee_t *tee = sensor->tee;

    fpc_ta_simple_command_t* command =
            (fpc_ta_simple_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_SENSOR_IS_SENSOR_FORCE_SUPPORTED;
    command->header.target = TARGET_FPC_TA_SENSOR;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        LOGE("%s, Failed to check sensor force support, status code: %d", __func__, status);
        goto out;
    }

    *is_supported = command->response;

out:
    return status;
}

#if FPC_CONFIG_FORCE_SENSOR == 1

int fpc_tee_wait_for_button_down_force(fpc_tee_sensor_t* sensor,
                                       uint32_t force_button_down_timeout_ms,
                                       uint8_t force_button_down_threshold)
{
    uint8_t force = 0;
    int force_result = 0;

    struct timespec start;
    struct timespec now;
    uint32_t delta_time_ms = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (force <= force_button_down_threshold &&
           delta_time_ms < force_button_down_timeout_ms) {
        force_result = fpc_tee_get_sensor_force_value(sensor, &force);
        if (force_result) {
            return force_result;
        }
        LOGD("%s force:%u th:%u\n", __func__, force, force_button_down_threshold);

        int status = sensor_command(sensor, FPC_TA_SENSOR_CHECK_FINGER_LOST_CMD);
        if (status < 0) {
            return status;
        } else if (status > 0) {
            return FPC_CAPTURE_FINGER_LOST;
        }
        if (fpc_tee_sensor_cancelled(sensor))
        {
            return -FPC_ERROR_CANCELLED;
        }

        clock_gettime(CLOCK_MONOTONIC, &now);
        delta_time_ms = fpc_get_ms_diff(&start, &now);
    }
    if (delta_time_ms >= force_button_down_timeout_ms) {
        return -FPC_ERROR_TIMEDOUT;
    }
    return 0;
}

int fpc_tee_wait_for_button_up_force(fpc_tee_sensor_t* sensor,
                                     uint32_t force_button_up_timeout_ms,
                                     uint8_t force_button_up_threshold)
{
    uint8_t force = 255;
    int force_result = 0;

    struct timespec start;
    struct timespec now;
    uint32_t delta_time_ms = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (force >= force_button_up_threshold &&
           delta_time_ms < force_button_up_timeout_ms) {
        force_result = fpc_tee_get_sensor_force_value(sensor, &force);
        if (force_result) {
            return force_result;
        }
        LOGD("%s force:%u th:%u\n", __func__, force, force_button_up_threshold);
        if (fpc_tee_sensor_cancelled(sensor))
        {
            return -FPC_ERROR_CANCELLED;
        }

        clock_gettime(CLOCK_MONOTONIC, &now);
        delta_time_ms = fpc_get_ms_diff(&start, &now);
    }

    if (delta_time_ms >= force_button_up_timeout_ms) {
        return -FPC_ERROR_TIMEDOUT;
    }
    return 0;
}
#endif

/*********************************************************
 * Fetches sensor OTP information from TA.
 *
 * @param[IN]: sensor - handle containg the HAL/TA transfer buffer.
 * @param[OUT]: otp_info - Output data.
 *
 * @return: On Success returns 0.
 *          On Failure returns non-zero error code.
 *********************************************************/
int fpc_tee_get_sensor_otp_info(fpc_tee_sensor_t* sensor, fpc_hw_module_info_t* otp_info)
{
    LOGD("%s", __func__);
    int status = 0;

    fpc_ta_sensor_otp_info_t* command =
        (fpc_ta_sensor_otp_info_t*) sensor->tee->shared_buffer->addr;

    command->data = *otp_info;
    status = sensor_command(sensor, FPC_TA_SENSOR_GET_OTP_INFO_CMD);

    if (status) {
        LOGE("%s, Failed to fetch sensor otp data, status code: %d", __func__, status);
        goto out;
    }

    *otp_info = command->data;
    status = command->response;
out:
    return status;
}

fpc_tee_sensor_t* fpc_tee_sensor_init(fpc_tee_t* tee) {

    fpc_tee_sensor_t* sensor;

    if (!tee) {
        return NULL;
    }

    sensor = malloc(sizeof(fpc_tee_sensor_t));
    if (!sensor) {
        return NULL;
    }

    memset(sensor, 0, sizeof(*sensor));
    sensor->tee = tee;
    pthread_mutex_init(&sensor->mutex, NULL);

    sensor->irq = fpc_irq_init();
    if (!sensor->irq) {
        goto err;
    }

#if defined(FPC_CONFIG_NORMAL_SPI_RESET) || defined(FPC_CONFIG_NORMAL_SENSOR_RESET)
    sensor->reset = fpc_reset_init();
    if (!sensor->reset) {
        goto err;
    }
#endif
    if (0 != fpc_tee_deep_sleep(sensor)) {
        goto err;
    }


    return sensor;

err:
    pthread_mutex_destroy(&sensor->mutex);
    if (sensor->irq) {
        fpc_irq_release(sensor->irq);
    }
#if defined(FPC_CONFIG_NORMAL_SPI_RESET) || defined(FPC_CONFIG_NORMAL_SENSOR_RESET)
    if (sensor->reset) {
        fpc_reset_release(sensor->reset);
    }
#endif
    free(sensor);
    return NULL;
}

void fpc_tee_sensor_release(fpc_tee_sensor_t* sensor)
{
    if (!sensor) {
        return;
    }

    fpc_tee_deep_sleep(sensor);

    pthread_mutex_destroy(&sensor->mutex);
    fpc_irq_release(sensor->irq);
#if defined(FPC_CONFIG_NORMAL_SPI_RESET) || defined(FPC_CONFIG_NORMAL_SENSOR_RESET)
    fpc_reset_release(sensor->reset);
#endif
     free(sensor);
}

int fpc_tee_set_cancel(fpc_tee_sensor_t* sensor)
{
    if (!sensor) {
        return -FPC_ERROR_INPUT;
    }

    LOGD("%s", __func__);
    pthread_mutex_lock(&sensor->mutex);
    sensor->cancelled = 1;
    int status = fpc_irq_set_cancel(sensor->irq);
    pthread_mutex_unlock(&sensor->mutex);
    return status;
}

int fpc_tee_clear_cancel(fpc_tee_sensor_t* sensor)
{
    if (!sensor) {
        return -FPC_ERROR_INPUT;
    }

    LOGD("%s", __func__);
    pthread_mutex_lock(&sensor->mutex);
    sensor->cancelled = 0;
    int status = fpc_irq_clear_cancel(sensor->irq);
    pthread_mutex_unlock(&sensor->mutex);
    return status;
}
