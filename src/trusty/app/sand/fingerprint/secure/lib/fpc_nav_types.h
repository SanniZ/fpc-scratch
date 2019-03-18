/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_NAV_TYPES_H
#define FPC_NAV_TYPES_H

#include <stdint.h>

enum {
    FPC_NAV_REQUEST_POLL_DATA,
    FPC_NAV_REQUEST_WAIT_IRQ_HIGH,
    FPC_NAV_REQUEST_WAIT_IRQ_LOW,
};

typedef enum {
    FPC_NAV_EVENT_NONE = 0,
    FPC_NAV_EVENT_SINGLE_CLICK,
    FPC_NAV_EVENT_HOLD_CLICK,
    FPC_NAV_EVENT_SLIDE_UP,
    FPC_NAV_EVENT_SLIDE_DOWN,
    FPC_NAV_EVENT_SLIDE_LEFT,
    FPC_NAV_EVENT_SLIDE_RIGHT,
    FPC_NAV_EVENT_DOUBLE_CLICK,
} fpc_nav_event_t;

typedef struct {
    uint32_t single_click_min_time_threshold;
    uint32_t hold_click_time_threshold;
    uint32_t double_click_time_interval;

    uint32_t fast_move_tolerance;

    uint32_t slow_swipe_up_threshold;
    uint32_t slow_swipe_down_threshold;
    uint32_t slow_swipe_left_threshold;
    uint32_t slow_swipe_right_threshold;
    uint32_t fast_swipe_up_threshold;
    uint32_t fast_swipe_down_threshold;
    uint32_t fast_swipe_left_threshold;
    uint32_t fast_swipe_right_threshold;
} fpc_nav_config_t;

enum {
    FORCE_SENSOR_NOT_AVAILABLE = -1,
};

typedef struct {
    fpc_nav_event_t nav_event;
    int32_t force;
    int32_t finger_down;
    int32_t request;
} fpc_nav_data_t;

#endif // FPC_NAV_TYPES_H
