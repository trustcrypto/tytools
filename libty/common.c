/*
 * ty, a collection of GUI and command-line tools to manage Teensy devices
 *
 * Distributed under the MIT license (see LICENSE.txt or http://opensource.org/licenses/MIT)
 * Copyright (c) 2015 Niels Martignène <niels.martignene@gmail.com>
 */

#include "ty/common.h"
#include "compat.h"
#include <stdarg.h>
#include "ty/system.h"
#include "task_priv.h"

struct ty_task {
    TY_TASK
};

ty_log_level ty_config_quiet = TY_LOG_INFO;
bool ty_config_experimental = false;

static ty_message_func *handler = ty_message_default_handler;
static void *handler_udata = NULL;

static __thread ty_err mask[16];
static __thread unsigned int mask_count;

static __thread char last_error_msg[256];

TY_INIT()
{
    const char *value;

    value = getenv("TY_QUIET");
    if (value)
        ty_config_quiet = (ty_log_level)strtol(value, NULL, 10);

    value = getenv("TY_EXPERIMENTAL");
    if (value && strcmp(value, "0") != 0 && strcmp(value, "") != 0)
        ty_config_experimental = true;
}

static void print_log(const void *data)
{
    const ty_log_message *msg = data;

    if (msg->level < ty_config_quiet)
        return;

    if (msg->level == TY_LOG_INFO) {
        printf("%s\n", msg->msg);
        fflush(stdout);
    } else {
        fprintf(stderr, "%s\n", msg->msg);
    }
}

static void print_progress(const void *data)
{
    static bool init = false, show_progress;
    const ty_progress_message *msg = data;

    if (TY_LOG_INFO < ty_config_quiet)
        return;

    if (!init) {
        show_progress = ty_descriptor_get_modes(TY_DESCRIPTOR_STDOUT) & TY_DESCRIPTOR_MODE_TERMINAL;
        init = true;
    }

    if (show_progress) {
        if (msg->value)
            printf("\r");
        printf("%s... %u%%", msg->action, 100 * msg->value / msg->max);
        if (msg->value == msg->max)
            printf("\n");

        fflush(stdout);
    } else if (!msg->value) {
        printf("%s...\n", msg->action);
    }
    fflush(stdout);
}

void ty_message_default_handler(ty_task *task, ty_message_type type, const void *data, void *udata)
{
    TY_UNUSED(task);
    TY_UNUSED(udata);

    switch (type) {
    case TY_MESSAGE_LOG:
        print_log(data);
        break;
    case TY_MESSAGE_PROGRESS:
        print_progress(data);
        break;

    default:
        break;
    }
}

void ty_message_redirect(ty_message_func *f, void *udata)
{
    assert(f);
    assert(f != ty_message_default_handler || !udata);

    handler = f;
    handler_udata = udata;
}

TY_PRINTF_FORMAT(2, 0)
static void logv(ty_log_level level, const char *fmt, va_list ap)
{
    char buf[256];
    ty_log_message msg;

    vsnprintf(buf, sizeof(buf), fmt, ap);

    if (level >= TY_LOG_ERROR) {
        strncpy(last_error_msg, buf, sizeof(last_error_msg));
        last_error_msg[sizeof(last_error_msg) - 1] = 0;
    }

    msg.level = level;
    msg.msg = buf;

    _ty_message(NULL, TY_MESSAGE_LOG, &msg);
}

void ty_log(ty_log_level level, const char *fmt, ...)
{
    assert(fmt);

    va_list ap;

    va_start(ap, fmt);
    logv(level, fmt, ap);
    va_end(ap);
}

static const char *generic_error(int err)
{
    if (err >= 0)
        return "Success";

    switch ((ty_err)err) {
    case TY_ERROR_MEMORY:
        return "Memory error";
    case TY_ERROR_PARAM:
        return "Incorrect parameter";
    case TY_ERROR_UNSUPPORTED:
        return "Option not supported";
    case TY_ERROR_NOT_FOUND:
        return "Not found";
    case TY_ERROR_EXISTS:
        return "Already exists";
    case TY_ERROR_ACCESS:
        return "Permission error";
    case TY_ERROR_BUSY:
        return "Busy error";
    case TY_ERROR_IO:
        return "I/O error";
    case TY_ERROR_TIMEOUT:
        return "Timeout error";
    case TY_ERROR_MODE:
        return "Wrong mode";
    case TY_ERROR_RANGE:
        return "Out of range error";
    case TY_ERROR_SYSTEM:
        return "System error";
    case TY_ERROR_PARSE:
        return "Parse error";
    case TY_ERROR_FIRMWARE:
        return "Firmware error";

    case TY_ERROR_OTHER:
        break;
    }

    return "Unknown error";
}

void ty_error_mask(ty_err err)
{
    assert(mask_count < TY_COUNTOF(mask));

    mask[mask_count++] = err;
}

void ty_error_unmask(void)
{
    assert(mask_count);

    mask_count--;
}

const char *ty_error_last_message(void)
{
    return last_error_msg;
}

int ty_error(ty_err err, const char *fmt, ...)
{
    va_list ap;

    for (unsigned int i = 0; i < mask_count; i++) {
        if (mask[i] == err)
            return err;
    }

    if (!fmt)
        fmt = generic_error(err);

    va_start(ap, fmt);
    logv(TY_LOG_ERROR, fmt, ap);
    va_end(ap);

    return err;
}

void ty_progress(const char *action, unsigned int value, unsigned int max)
{
    assert(value <= max);
    assert(max);

    ty_progress_message msg;

    msg.action = action ?: "Processing";
    msg.value = value;
    msg.max = max;

    _ty_message(NULL, TY_MESSAGE_PROGRESS, &msg);
}

void _ty_message(ty_task *task, ty_message_type type, const void *data)
{
    if (!task)
        task = _ty_task_get_current();

    (*handler)(task, type, data, handler_udata);
    if (task && task->callback)
        (*task->callback)(task, type, data, task->callback_udata);
}
