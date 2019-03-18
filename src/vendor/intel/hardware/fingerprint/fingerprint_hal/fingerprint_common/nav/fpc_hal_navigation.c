/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "fpc_hal_ext_navigation.h"
#include "fpc_hal_navigation.h"
#include "fpc_log.h"
#include "fpc_worker.h"
#include "fpc_irq_device.h"
#include "fpc_tee_nav.h"
#include "fpc_types.h"
#include "fpc_nav_types.h"
#include "fpc_tee.h"
#include "fpc_hal_input_device.h"
#if defined(FPC_CONFIG_FORCE_SENSOR) || defined(FPC_CONFIG_NAVIGATION_FORCE_SW)
#include "fpc_hal_sense_touch.h"
#include "fpc_hal_sense_touch_types.h"
#endif


static uint64_t current_u_time()
{
    struct timeval t1;
    gettimeofday(&t1, NULL);
    return (t1.tv_sec * 1000 * 1000) + (t1.tv_usec);
}


typedef struct {
    fpc_navigation_t navigation;
    pthread_mutex_t mutex;
    pthread_mutex_t cancel_mutex;
    fpc_tee_t* tee;
    fpc_irq_t* irq;
    fpc_worker_t* worker_thread;
    bool enabled;
    bool paused;
    bool cancel;
    fpc_nav_config_t config;
    bool configured;
#if defined(FPC_CONFIG_FORCE_SENSOR) || defined(FPC_CONFIG_NAVIGATION_FORCE_SW)
    const fpc_sense_touch_config_t* st_config;
#endif
} nav_module_t;

static void nav_loop(void* data)
{
    LOGD("%s", __func__);
    /* These are used to block colliding events from being reported,
       Such as force_press and nav_click */
    bool block_nav_events = false;
    bool block_st_events = false;
#if defined(FPC_CONFIG_FORCE_SENSOR) || defined(FPC_CONFIG_NAVIGATION_FORCE_SW)
    int32_t prev_force = 0;
    bool force_button_down = false;
#endif
    nav_module_t* module = (nav_module_t*) data;

    int status = 0;
    if (module->configured) {
        status = fpc_tee_nav_set_config(module->tee, &module->config);
        if (status) {
            return;
        }
    }

    status = fpc_tee_nav_init(module->tee);
    if (status) {
        return;
    }

    const uint32_t frame_rate_limit = 150;
    const uint64_t frame_time = 1000 * 1000 / frame_rate_limit;
    uint64_t time = 0;
    uint64_t frame_delta;

    for (;;) {
        pthread_mutex_lock(&module->cancel_mutex);
        bool cancel = module->cancel;
        pthread_mutex_unlock(&module->cancel_mutex);

        if (cancel) {
            status = -FPC_ERROR_CANCELLED;
            goto out;
        }

        fpc_nav_data_t nav;
        int status = fpc_tee_nav_poll_data(module->tee, &nav);
        if (status) {
            goto out;
        }

        if (!block_nav_events && nav.nav_event != FPC_NAV_EVENT_NONE) {
            block_st_events = true;
            report_input_event(FPC_NAV_EVENT, nav.nav_event, FPC_HAL_INPUT_KEY_DOWN);
            report_input_event(FPC_NAV_EVENT, nav.nav_event, FPC_HAL_INPUT_KEY_UP);
        }

#if defined(FPC_CONFIG_FORCE_SENSOR) || defined(FPC_CONFIG_NAVIGATION_FORCE_SW)
        if (nav.force != FORCE_SENSOR_NOT_AVAILABLE && nav.force != prev_force) {
            LOGD("%s Reporting raw force: %d over input device.", __func__, nav.force);
            report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_RAW, nav.force);
            prev_force = nav.force;
        }

        if(module->st_config != NULL) {
            if(!block_st_events && module->st_config->trigger_threshold != 0 &&
                nav.force >= module->st_config->trigger_threshold) {
                if(!force_button_down) {
                    force_button_down = true;
                    block_nav_events = true;
                    report_input_event(FPC_SENSE_TOUCH_EVENT,
                                       FPC_SENSE_TOUCH_PRESS,
                                       FPC_HAL_INPUT_KEY_DOWN);
                }
            } else if(nav.force <= module->st_config->untrigger_threshold) {
                if(force_button_down) {
                    force_button_down = false;
                    report_input_event(FPC_SENSE_TOUCH_EVENT,
                                       FPC_SENSE_TOUCH_PRESS,
                                       FPC_HAL_INPUT_KEY_UP);
                }
            }
        }

        if(!nav.finger_down) {
            block_nav_events = false;
            block_st_events = false;
        }
#endif
        switch (nav.request) {
            case FPC_NAV_REQUEST_WAIT_IRQ_HIGH:
            LOGD("%s FPC_NAV_REQUEST_WAIT_IRQ_HIGH", __func__);
            status = fpc_irq_wait(module->irq, 1);
            if (status) {
               goto out;
            }
            break;
            case FPC_NAV_REQUEST_WAIT_IRQ_LOW:
            LOGD("%s FPC_NAV_REQUEST_WAIT_IRQ_LOW", __func__);
            status = fpc_irq_wait(module->irq, 0);
            if (status) {
                goto out;
            }
            break;
            case FPC_NAV_REQUEST_POLL_DATA:
            frame_delta = current_u_time() - time;
            if (frame_delta < frame_time) {
               usleep((frame_time - frame_delta));
            }
            time = current_u_time();
            break;
        }
    }

out:
    fpc_tee_nav_exit(module->tee);
}

static void set_config(fpc_navigation_t* self, const fpc_nav_config_t* config)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);
    nav->config = *config;
    nav->configured = true;
    pthread_mutex_unlock(&nav->mutex);
}

static void get_config(fpc_navigation_t* self, fpc_nav_config_t* config)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);
    *config = nav->config;
    pthread_mutex_unlock(&nav->mutex);
}

static void cancel(nav_module_t* nav)
{
    /* Set cancel states */
    fpc_irq_set_cancel(nav->irq);
    pthread_mutex_lock(&nav->cancel_mutex);
    nav->cancel = true;
    pthread_mutex_unlock(&nav->cancel_mutex);
    fpc_worker_join_task(nav->worker_thread);

    /* Reset cancel states */
    pthread_mutex_lock(&nav->cancel_mutex);
    nav->cancel = false;
    pthread_mutex_unlock(&nav->cancel_mutex);
    fpc_irq_clear_cancel(nav->irq);
}

static void set_enabled(fpc_navigation_t* self, bool enabled)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);

    bool is_running = nav->enabled && !nav->paused;
    bool should_run = enabled && !nav->paused;

    if (should_run && !is_running) {
        fpc_worker_run_task(nav->worker_thread, nav_loop, nav);
    } else if (!should_run && is_running) {
        cancel(nav);
    }

    nav->enabled = enabled;

    pthread_mutex_unlock(&nav->mutex);
}

static bool get_enabled(fpc_navigation_t* self)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);
    bool enabled = nav->enabled;
    pthread_mutex_unlock(&nav->mutex);
    return enabled;
}

void fpc_navigation_resume(fpc_navigation_t* self)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);

    if (nav->enabled && nav->paused) {
        fpc_worker_run_task(nav->worker_thread, nav_loop, nav);
    }
    nav->paused = false;
    pthread_mutex_unlock(&nav->mutex);
}

void fpc_navigation_pause(fpc_navigation_t* self)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);

    if (nav->enabled && !nav->paused) {
        cancel(nav);
    }

    nav->paused = true;
    pthread_mutex_unlock(&nav->mutex);
}

fpc_navigation_t* fpc_navigation_new(fpc_tee_t* tee_handle)
{
    nav_module_t* module = (nav_module_t*) calloc(sizeof(nav_module_t), 1);
    if (!module) {
        return NULL;
    }

    pthread_mutex_init(&module->mutex, NULL);
    pthread_mutex_init(&module->cancel_mutex, NULL);

    module->irq = fpc_irq_init();
    if (!module->irq) {
        goto err;
    }

    module->tee = tee_handle;
#if defined(FPC_CONFIG_FORCE_SENSOR) || defined(FPC_CONFIG_NAVIGATION_FORCE_SW)
    module->st_config = fpc_sense_touch_get_config();
#endif

    module->worker_thread = fpc_worker_new();
    if (!module->worker_thread) {
        goto err;
    }

    module->navigation.set_config = set_config;
    module->navigation.get_config = get_config;
    module->navigation.set_enabled = set_enabled;
    module->navigation.get_enabled = get_enabled;
    module->enabled = true;
    module->paused = true;

    if (fpc_tee_nav_get_config(module->tee, &module->config)) {
        LOGE("%s fpc_tee_nav_get_config failed", __func__);
        goto err;
    }

    return (fpc_navigation_t*) module;

err:
    fpc_navigation_destroy( (fpc_navigation_t*) module);

    return NULL;
}

void fpc_navigation_destroy(fpc_navigation_t *self)
{
    if (!self) {
        return;
    }

    nav_module_t* module = (nav_module_t*) self;

    fpc_irq_release(module->irq);
    fpc_worker_destroy(module->worker_thread);
    pthread_mutex_destroy(&module->mutex);
    pthread_mutex_destroy(&module->cancel_mutex);
    free(module);
}
