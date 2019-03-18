/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <limits.h>
#include <time.h>

#include "fpc_ta_targets.h"
#include "fpc_ta_kpi_interface.h"
#include "fpc_tee_internal.h"
#include "fpc_tee.h"
#include "fpc_log.h"

#define MAX_SUB_CAPTURES 10
#define MAX_FASTTAP_CAPTURE 3
#define KPI_RESULTS_SIZE 50
#define KPI_PATH "/data/diag_logs"
#define MANIFEST_TAG_MAX 64

/* This struct must be kept in sync with fpc_lib/src/fpc_kpi.c */
typedef struct {
    /* Duration of finger qualification in ms */
    int16_t fq;
    /* Duration of fallback image in ms */
    int16_t fallback;
    /* Duration of FSD in ms */
    int16_t fsd[MAX_SUB_CAPTURES];
    /* Duration of CAC in ms */
    int16_t cac[MAX_SUB_CAPTURES];
    /* Duration of capture in ms */
    int16_t capture;
    /* Duration of enrol/identify in ms */
    int16_t enrol_identify;
    /* Duration of identify @ enrol in ms */
    int16_t identify_at_enrol;
    /* Duration since finger detected in ms */
    int16_t total;

    char usecase;
    char selected_image; /* '?' - no clue, 'C' - CAC search image, 'F' - fallback image */
    int8_t progress;
    int8_t decision;
    int8_t coverage;
    int8_t quality;
    int8_t nbr_of_templates;
    int8_t template_id;
    int8_t fasttap_searches;
    int8_t fasttap_captures;
    uint32_t fasttap_start;
    uint32_t fasttap_search[MAX_FASTTAP_CAPTURE];
    uint32_t fasttap_capture[MAX_FASTTAP_CAPTURE];
} kpi_results_t;

typedef struct {
    uint16_t hwid;
} kpi_metadata_t;

static int kpi_active = 0;
static char csv_file[PATH_MAX] = {0};
static char manifest_tag[MANIFEST_TAG_MAX] = {0};
static kpi_metadata_t kpi_metadata;

static int get_manifest_tag(fpc_tee_t *tee, char *sw_tag, int max_len);

static int print_csv_metadata(FILE *csv)
{
    int len;
    int ret = 0;
    char value[PROP_VALUE_MAX];

    len = fprintf(csv, "0x%04x\t", kpi_metadata.hwid);
    if (len < 0) return 0;
    ret += len;

    property_get("ro.board.platform", value, "n/a");
    len = fprintf(csv, "%s\t", value);
    if (len < 0) return 0;
    ret += len;

    property_get("ro.boot.hardware", value, "n/a");
    len = fprintf(csv, "%s\t", value);
    if (len < 0) return 0;
    ret += len;

    property_get("ro.build.type", value, "n/a");
    len = fprintf(csv, "%s\t", value);
    if (len < 0) return 0;
    ret += len;


    return ret;
}

static int kpi_print_results(kpi_results_t* results, FILE *csv)
{
    int ret;
    int32_t fsd_total = 0;
    int32_t cac_total = 0;

    print_csv_metadata(csv);

    ret = fprintf(csv, "%c\t%c\t%d\t%d\t%d\t%d\t%d\t%s\t",
            results->usecase,
            results->selected_image,
            results->coverage,
            results->quality,
            results->decision,
            results->nbr_of_templates,
            results->template_id,
            "img.id");

    uint32_t i = 0;
    while (i < MAX_SUB_CAPTURES && results->fsd[i] != -1)
    {
        fsd_total += results->fsd[i];
        fprintf(csv, "%s%d", (i) ? ";" : "", results->fsd[i]);
        i++;
    }
    fprintf(csv, "%s\t", (i) ? "" : "-" );

    i = 0;
    while (i < MAX_SUB_CAPTURES && results->cac[i] != -1)
    {
        cac_total += results->cac[i];
        fprintf(csv, "%s%d", (i) ? ";" : "", results->cac[i]);
        i++;
    }
    fprintf(csv, "%s\t", (i) ? "" : "-" );

    ret = fprintf(csv, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",
            results->fallback,
            fsd_total,
            cac_total,
            results->capture,
            results->enrol_identify,
            results->identify_at_enrol,
            results->total,
            results->fq);

    i = 0;
    while (i < MAX_FASTTAP_CAPTURE && results->fasttap_search[i])
    {
        fprintf(csv, "%s%u", (i) ? ";" : "",
                (uint32_t)(results->fasttap_search[i] - (i == 0 ? results->fasttap_start :
                                                         results->fasttap_capture[i - 1])));
        i++;
    }
    fprintf(csv, "%s\t", (i) ? "" : "-");

    i = 0;
    while (i < MAX_FASTTAP_CAPTURE && results->fasttap_capture[i])
    {
        fprintf(csv, "%s%u", (i) ? ";" : "",
                (uint32_t)(results->fasttap_capture[i] - results->fasttap_search[i]));
        i++;
    }
    fprintf(csv, "%s\t", (i) ? "" : "-");

    fprintf(csv, "%d\n", results->progress);

    return ret;
}

static void kpi_print_all_results(kpi_results_t* results, int count)
{
    int need_header = 1;
    int ret;
    int i;
    struct stat st;
    FILE *fp_csv = NULL;

    if (!count) {
        return;
    }
    if (count < 0) {
        LOGE("%s: count negative", __func__);
        return;
    }
    if (count > KPI_RESULTS_SIZE) {
        LOGE("%s: count too high ", __func__);
        return;
    }

    fp_csv = fopen(csv_file, "a");
    if (!fp_csv) {
        LOGE("%s: open error '%s'", __func__, csv_file);
        return;
    }

    if (!fstat(fileno(fp_csv), &st)) {
        if (st.st_size > 0) {
            need_header = 0;
        }
    }

    if (need_header) {
        fprintf(fp_csv,
                "hwid\t"
                "platform\t"
                "hardware\t"
                "buildtype\t"
                "usecase\t"
                "selected\t"
                "coverage\t"
                "quality\t"
                "decision\t"
                "templates\t"
                "template\t"
                "imgid\t"
                "cac-fsd\t"
                "cac-sub\t"
                "fallback\t"
                "fsd_total\t"
                "cac_total\t"
                "capture\t"
                "enrol_identify\t"
                "identify_at_enrol\t"
                "total\t"
                "fq\t"
                "fasttap_search\t"
                "fasttap_capture\t"
                "progress\n");
    }

    for (i = 0; i < count; i++)
    {
        ret = kpi_print_results(results + i, fp_csv);
        if (ret < 0)
        {
            break;
        }
    }
    if (ret < 0)
    {
        LOGE("%s: write error", __func__);
    }
    (void)fclose(fp_csv);
    (void)chmod(csv_file, 0777);
}

static int fpc_tee_kpi_ctrl(
    fpc_tee_t* tee,
    fpc_kpi_cmd_t cmd,
    uint32_t *size,
    uint8_t *data)
{
    int status = -1;
    uint32_t ipc_size = size ? sizeof(fpc_ta_kpi_command_t) + *size : 0;

    if (ipc_size < sizeof(fpc_ta_kpi_command_t))
    {
        ipc_size = sizeof(fpc_ta_kpi_command_t);
    }

    fpc_tac_shared_mem_t* shared_ipc_buffer =
        fpc_tac_alloc_shared(tee->tac, ipc_size);
    if (!shared_ipc_buffer)
    {
        LOGE("%s: could not allocate shared buffer", __func__);
        goto out;
    }

    fpc_ta_kpi_command_t* command = shared_ipc_buffer->addr;
    command->header.target = TARGET_FPC_TA_KPI;
    command->header.command = cmd;
    command->kpi_ctrl.size = size ? *size : 0;
    if (command->kpi_ctrl.size && data)
    {
        memcpy(command->kpi_ctrl.array, data, command->kpi_ctrl.size);
    }

    status = fpc_tac_transfer(tee->tac, shared_ipc_buffer);
    if (status)
    {
        LOGE("%s: fpc_tac_transfer() -> %d", __func__, status);
        goto out;
    }
    status = command->kpi_ctrl.response;
    LOGD("%s: kpi_ctrl.response=%d", __func__, status);

    if (size && data)
    {
        *size = *size < command->kpi_ctrl.size ?
            *size : command->kpi_ctrl.size;

        memcpy(data, command->kpi_ctrl.array, *size);
    }

out:
    if (shared_ipc_buffer)
    {
        fpc_tac_free_shared(shared_ipc_buffer);
    }
    LOGD("%s ret=%d", __func__, status);
    return status;
}


void fpc_tee_kpi_start(fpc_tee_t* tee)
{
    char value[PROP_VALUE_MAX];

    property_get("fpc_kpi", value, "0");
    kpi_active = atoi(value);
    LOGD("%s: property value='%s' kpi_active=%d", __func__, value, kpi_active);

    if (!kpi_active)
    {
        // fpc_kpi property was unset, invalidate file name
        csv_file[0] = 0;
        return;
    }

    if (!manifest_tag[0]) {
        int ret = get_manifest_tag(tee, manifest_tag, MANIFEST_TAG_MAX - 1);
        if (ret < 0) {
            strcpy(manifest_tag, "unknown");
        }
    }

    time_t rawtime;
    time(&rawtime);
    struct tm* t = gmtime(&rawtime);

    if (!csv_file[0])
    {
        char platform[PROP_VALUE_MAX];

        property_get("ro.boot.serialno", value, "NA");

        property_get("ro.board.platform", platform, "na");

        snprintf(csv_file,
            PATH_MAX,
            "%s/oddc_%s%s_%04d%02d%02d%02d%02d%02d_%s_#%s#.csv",
            KPI_PATH,
#ifdef FPC_CONFIG_DEBUG
                "DEBUG_",
#else
                "",
#endif
            value,              // serial
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec,
            platform,
            manifest_tag);
        LOGD("%s csv_file='%s'", __func__, csv_file);
    }

    int status = fpc_tee_kpi_ctrl(tee, FPC_TA_KPI_START_CMD, NULL, NULL);
    if (status)
    {
        LOGE("%s: fpc_tee_kpi_ctrl failed with error: %d", __func__, status);
    }
}


void fpc_tee_kpi_stop(fpc_tee_t* tee)
{
    if (!kpi_active)
    {
        return;
    }
    uint32_t size = KPI_RESULTS_SIZE * sizeof(kpi_results_t) + sizeof(kpi_metadata);
    uint8_t *io_buffer = malloc(size);
    if (!io_buffer)
    {
        LOGE("%s: failed to allocate kpi_results", __func__);
        return;
    }

    int status = fpc_tee_kpi_ctrl(
        tee,
        FPC_TA_KPI_STOP_CMD,
        &size,
        io_buffer);

    if (status)
    {
        LOGE("%s: fpc_tee_kpi_ctrl failed with error: %d", __func__, status);
        goto out;
    }

    memcpy(&kpi_metadata, io_buffer, sizeof(kpi_metadata));
    size -= sizeof(kpi_metadata);
    kpi_results_t *results = (kpi_results_t *)(io_buffer + sizeof(kpi_metadata));

    if (size % sizeof(kpi_results_t))
    {
        LOGE("%s: size mismatch, expected multiple of %zu, got %" PRIu32,
                __func__,
                sizeof(kpi_results_t),
                size);
        goto out;
    }

    kpi_print_all_results(results, size / sizeof(kpi_results_t));
    kpi_active = 0;

out:
    free(io_buffer);
}

static int get_manifest_tag(fpc_tee_t *tee, char *manifest_tag, int max_len)
{
    LOGD("%s", __func__);
    int status = -1;
    char *search = "Manifesttag=";

    uint32_t size = MAX_BUILDINFO_SIZE + sizeof(fpc_ta_byte_array_msg_t);
    fpc_tac_shared_mem_t* shared_ipc_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_ipc_buffer) {
        goto out;
    }

    fpc_ta_kpi_command_t* command = shared_ipc_buffer->addr;
    command->header.target = TARGET_FPC_TA_KPI;
    command->header.command = FPC_TA_GET_BUILD_INFO_CMD;
    command->build_info.size = MAX_BUILDINFO_SIZE;

    status = fpc_tac_transfer(tee->tac, shared_ipc_buffer);
    if (status) {
        LOGE("%s, fpc_tac_transfer failed %d.", __func__, status);
        goto out;
    }

    status = command->build_info.response;
    if (status) {
        LOGE("%s, get_build_info ta cmd failed %d.", __func__, status);
        goto out;
    }

    char *tag = strstr((char*)command->build_info.array, search);
    if (!tag) {
        *manifest_tag = 0;
        goto out;
    }

    tag += strlen(search);
    char *end = strchr(tag, ':');
    if (!end) {
        *manifest_tag = 0;
        goto out;
    }
    int len = (end - tag);
    if (len > max_len) {
        len = max_len;
    }
    memcpy(manifest_tag, tag, len);
    manifest_tag[len] = 0;

    for (int i = 0; i < len; i++) {
        if (!(manifest_tag[i] >= '0' && manifest_tag[i] <= '9') &&
            !(manifest_tag[i] >= 'A' && manifest_tag[i] <= 'Z') &&
            !(manifest_tag[i] >= 'a' && manifest_tag[i] <= 'z') &&
            (manifest_tag[i] != '.') &&
            (manifest_tag[i] != '-') &&
            (manifest_tag[i] != '_')) {
            manifest_tag[i] = 'X';
        }
    }
    status = 0;

out:
    if (shared_ipc_buffer) {
        fpc_tac_free_shared(shared_ipc_buffer);
    }
    return status;
}

int fpc_tee_print_build_info(fpc_tee_t* tee)
{
    LOGD("%s", __func__);
    int status = -1;

    uint32_t size = MAX_BUILDINFO_SIZE + sizeof(fpc_ta_byte_array_msg_t);
    fpc_tac_shared_mem_t* shared_ipc_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_ipc_buffer) {
        goto out;
    }

    fpc_ta_kpi_command_t* command = shared_ipc_buffer->addr;
    command->header.target = TARGET_FPC_TA_KPI;
    command->header.command = FPC_TA_GET_BUILD_INFO_CMD;
    command->build_info.size = MAX_BUILDINFO_SIZE;

    status = fpc_tac_transfer(tee->tac, shared_ipc_buffer);
    if (status) {
        LOGE("%s, fpc_tac_transfer failed %d.", __func__, status);
        goto out;
    }

    status = command->build_info.response;
    if (status) {
        LOGE("%s, get_build_info ta cmd failed %d.", __func__, status);
        goto out;
    }

    char *build_type = strchr((char*)command->build_info.array, '\n');
    if (build_type) {
        *build_type = 0;
        build_type++;
    }

    LOGD("%s : fpcbuildinfo : %s", __func__, command->build_info.array);
    if (build_type) {
        LOGD("%s : fpcbuildtype : %s", __func__, build_type);
    }

out:
    if (shared_ipc_buffer) {
        fpc_tac_free_shared(shared_ipc_buffer);
    }
    return status;
}
