/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trusty_std.h>
#include <trusty_ipc.h>
#include <err.h>

#include "fpc_ta_targets.h"
#include "fpc_ta_interface.h"
#include "fpc_ta_bio.h"
#include "fpc_log.h"
#include "fpc_ta_router.h"
#include "fpc_types.h"

#define FPC_FINGERPRINT_PORT "com.android.trusty.fpcteeapp"
#define FPC_FINGERPRINT_SECURE_PORT "com.android.trusty.fpcteeapp.secure"

#define FPC_FINGERPRINT_MAX_BUFFER_LENGTH    (4 * 1024)

typedef void (*event_handler_proc_t)(const uevent_t* ev, void* ctx);

typedef struct tipc_event_handler_s {
    event_handler_proc_t proc;
    void* priv;
} tipc_event_handler_t;

typedef struct fpc_fingerprint_chan_ctx_s {
    tipc_event_handler_t handler;
    uuid_t uuid;
    handle_t chan;
    int32_t (*dispatch)(struct fpc_fingerprint_chan_ctx_s*, void*, uint32_t);
} fpc_fingerprint_chan_ctx_t;

typedef struct fpc_fingerprint_srv_ctx_s {
    handle_t port_secure;
    handle_t port_non_secure;
} fpc_fingerprint_srv_ctx_t;

static void fpc_fingerprint_port_handler_secure(const uevent_t* ev, void* priv);
static void fpc_fingerprint_port_handler_non_secure(const uevent_t* ev, void* priv);

static tipc_event_handler_t fpc_fingerprint_port_evt_handler_secure = {
    .proc = fpc_fingerprint_port_handler_secure, .priv = NULL,
};

static tipc_event_handler_t fpc_fingerprint_port_evt_handler_non_secure = {
    .proc = fpc_fingerprint_port_handler_non_secure, .priv = NULL,
};

static void fpc_fingerprint_chan_handler(const uevent_t* ev, void* priv);

static long handle_port_errors(const uevent_t* ev) {
    if ((ev->event & IPC_HANDLE_POLL_ERROR) || (ev->event & IPC_HANDLE_POLL_HUP) ||
        (ev->event & IPC_HANDLE_POLL_MSG) || (ev->event & IPC_HANDLE_POLL_SEND_UNBLOCKED)) {
        /* should never happen with port handles */
        LOGE("error event (0x%x) for port (%d)", ev->event, ev->handle);
        return ERR_BAD_STATE;
    }

    return NO_ERROR;
}

static long send_response(handle_t chan,
        void* data, uint32_t data_len, int32_t* rsp)
{
    iovec_t iov[2] = {
        { data, data_len },
        { (void*)rsp, sizeof(int32_t) },
    };
    ipc_msg_t msg = { 2, iov, 0, NULL };

    /* send message back to the caller */
    int rc = send_msg(chan, &msg);
    // fatal error
    if (rc < 0) {
        LOGE("failed (%d) to send_msg for chan (%d)", rc, chan);
        goto out;
    }

    rc = 0;
out:
    return rc;
}

static int32_t fpc_fingerprint_dispatch_secure(fpc_fingerprint_chan_ctx_t* ctx, void* addr,
                                      uint32_t buf_len)
{
    return fpc_ta_route_command(addr, buf_len);
}

static int32_t fpc_fingerprint_dispatch_non_secure(fpc_fingerprint_chan_ctx_t* ctx, void* addr,
                                      uint32_t buf_len)
{
    return fpc_ta_route_command(addr, buf_len);
}

/* TODO to be implemented*/
static bool fpc_fingerprint_port_accessible(uuid_t* uuid, bool secure) {
    return true;
}

static fpc_fingerprint_chan_ctx_t* fpc_fingerprint_ctx_open(handle_t chan, uuid_t* uuid, bool secure) {
    if (!fpc_fingerprint_port_accessible(uuid, secure)) {
        LOGE("access denied for client uuid", 0);
        return NULL;
    }

    fpc_fingerprint_chan_ctx_t* ctx = (fpc_fingerprint_chan_ctx_t*) malloc(sizeof(fpc_fingerprint_chan_ctx_t));
    if (ctx == NULL) {
        return ctx;
    }

    memset(ctx, 0, sizeof(fpc_fingerprint_chan_ctx_t));
    ctx->handler.proc = &fpc_fingerprint_chan_handler;
    ctx->handler.priv = ctx;
    ctx->uuid = *uuid;
    ctx->chan = chan;
    ctx->dispatch = secure ? &fpc_fingerprint_dispatch_secure : &fpc_fingerprint_dispatch_non_secure;
    return ctx;
}

static void fpc_fingerprint_ctx_close(fpc_fingerprint_chan_ctx_t* ctx) {
    if (ctx) {
        close(ctx->chan);
        free(ctx);
    }
}

static long handle_msg(fpc_fingerprint_chan_ctx_t* ctx) {
    handle_t chan = ctx->chan;

    /* get message info */
    ipc_msg_info_t msg_inf;
    uint8_t* msg_buf = NULL;
    long rc = get_msg(chan, &msg_inf);
    if (rc == ERR_NO_MSG)
        return NO_ERROR; /* no new messages */

    // fatal error
    if (rc != NO_ERROR) {
        LOGE("failed (%ld) to get_msg for chan (%d), closing connection", rc, chan);
        return rc;
    }

    // allocate msg_buf, with one extra byte for null-terminator
    msg_buf = (uint8_t*) malloc(msg_inf.len + 1);
    if (!msg_buf) {
        LOGE("msg_buf failed to malloc");
        return ERR_NO_MEMORY;
    }

    /* read msg content */
    iovec_t iov = {msg_buf, msg_inf.len};
    ipc_msg_t msg = {1, &iov, 0, NULL};

    memset(msg_buf, 0, msg_inf.len + 1);
    rc = read_msg(chan, msg_inf.id, 0, &msg);

    // fatal error
    if (rc < 0) {
        LOGE("failed to read msg (%ld)", rc);
        goto out;
    }

    if (((unsigned long)rc) < msg_inf.len) {
        LOGE("invalid message of size (%ld)", rc);
        rc =  ERR_NOT_VALID;
        goto out;
    }

    int32_t rsp;
    void* addr = (void *)msg_buf;
    rsp = ctx->dispatch(ctx, addr, msg_inf.len);

    rc = send_response(chan, addr, msg_inf.len, &rsp);
out:
    if (msg_buf) {
        free(msg_buf);
    }
    return rc;
}

static void fpc_fingerprint_chan_handler(const uevent_t* ev, void* priv) {
    fpc_fingerprint_chan_ctx_t* ctx = (fpc_fingerprint_chan_ctx_t*)(priv);
    if (ctx == NULL) {
        LOGE("error: no context on channel %d", ev->handle);
        close(ev->handle);
        return;
    }

    if ((ev->event & IPC_HANDLE_POLL_ERROR) || (ev->event & IPC_HANDLE_POLL_READY)) {
        /* close it as it is in an error state */
        LOGE("error event (0x%x) for chan (%d)", ev->event, ev->handle);
        close(ev->handle);
        return;
    }

    if (ev->event & IPC_HANDLE_POLL_MSG) {
        long rc = handle_msg(ctx);
        if (rc != NO_ERROR) {
            /* report an error and close channel */
            LOGE("failed (%d) to handle event on channel %d", rc, ev->handle);
            fpc_fingerprint_ctx_close(ctx);
            return;
        }
    }

    if (ev->event & IPC_HANDLE_POLL_HUP) {
        /* closed by peer. */
        fpc_fingerprint_ctx_close(ctx);
        return;
    }
}

static void fpc_fingerprint_port_handler(const uevent_t* ev, void* priv, bool secure) {
    long rc = handle_port_errors(ev);
    if (rc != NO_ERROR) {
        abort();
    }

    uuid_t peer_uuid;
    if (ev->event & IPC_HANDLE_POLL_READY) {
        /* incoming connection: accept it */
        int rc = accept(ev->handle, &peer_uuid);
        if (rc < 0) {
            LOGE("failed (%d) to accept on port %d", rc, ev->handle);
            return;
        }

        handle_t chan = (handle_t)rc;
        fpc_fingerprint_chan_ctx_t* ctx = fpc_fingerprint_ctx_open(chan, &peer_uuid, secure);
        if (ctx == NULL) {
            LOGE("failed to allocate context on chan %d", chan);
            close(chan);
            return;
        }

        rc = set_cookie(chan, ctx);
        if (rc < 0) {
            LOGE("failed (%d) to set_cookie on chan %d", rc, chan);
            fpc_fingerprint_ctx_close(ctx);
            return;
        }
    }
}

static void fpc_fingerprint_port_handler_secure(const uevent_t* ev, void* priv) {
    fpc_fingerprint_port_handler(ev, priv, true);
}

static void fpc_fingerprint_port_handler_non_secure(const uevent_t* ev, void* priv) {
    fpc_fingerprint_port_handler(ev, priv, false);
}

static void dispatch_event(const uevent_t* ev) {
    if (ev == NULL)
        return;

    if (ev->event == IPC_HANDLE_POLL_NONE) {
        /* not really an event, do nothing */
        LOGE("got an empty event");
        return;
    }

    /* check if we have handler */
    tipc_event_handler_t* handler = (tipc_event_handler_t*)(ev->cookie);
    if (handler && handler->proc) {
        /* invoke it */
        handler->proc(ev, handler->priv);
        return;
    }

    /* no handler? close it */
    LOGE("no handler for event (0x%x) with handle %d", ev->event, ev->handle);

    close(ev->handle);

    return;
}

static long fpc_fingerprint_ipc_init(fpc_fingerprint_srv_ctx_t* ctx) {
    int rc;

    /* Initialize secure-side service */
    rc = port_create(FPC_FINGERPRINT_SECURE_PORT, 1, FPC_FINGERPRINT_MAX_BUFFER_LENGTH,
                     IPC_PORT_ALLOW_TA_CONNECT);
    if (rc < 0) {
        LOGE("Failed (%d) to create port %s", rc, FPC_FINGERPRINT_SECURE_PORT);
        return rc;
    }

    ctx->port_secure = (handle_t)rc;

    rc = set_cookie(ctx->port_secure, &fpc_fingerprint_port_evt_handler_secure);
    if (rc) {
        LOGE("failed (%d) to set_cookie on port %d", rc, ctx->port_secure);
        close(ctx->port_secure);
        return rc;
    }

    /* initialize non-secure side service */
    rc = port_create(FPC_FINGERPRINT_PORT, 1, FPC_FINGERPRINT_MAX_BUFFER_LENGTH, IPC_PORT_ALLOW_NS_CONNECT);
    if (rc < 0) {
        LOGE("Failed (%d) to create port %s", rc, FPC_FINGERPRINT_PORT);
        return rc;
    }

    ctx->port_non_secure = (handle_t)rc;

    rc = set_cookie(ctx->port_non_secure, &fpc_fingerprint_port_evt_handler_non_secure);
    if (rc) {
        LOGE("failed (%d) to set_cookie on port %d", rc, ctx->port_non_secure);
        close(ctx->port_non_secure);
        return rc;
    }

    return NO_ERROR;
}

int main(void) {
    long rc;
    uevent_t event;

    LOGD("Initializing", 0);

    fpc_fingerprint_srv_ctx_t ctx;
    rc = fpc_fingerprint_ipc_init(&ctx);
    if (rc < 0) {
        LOGE("failed (%d) to initialize fpc_fingerprint", rc);
        return rc;
    }

    /* enter main event loop */
    while (true) {
        event.handle = INVALID_IPC_HANDLE;
        event.event = 0;
        event.cookie = NULL;

        rc = wait_any(&event, -1);
        if (rc < 0) {
            LOGE("wait_any failed (%d)", rc);
            break;
        }

        if (rc == NO_ERROR) { /* got an event */
            rc = fpc_ta_router_init();
            if (rc) {
                LOGE("%s ta route init failed %ld\n", __func__, rc);
                return -1;
            }
            dispatch_event(&event);
        }
    }

    fpc_ta_router_exit();

    return 0;
}
