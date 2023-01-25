// This file is automatically @generated by Tailor
// It is not inteded for manual editing.
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include "bridge.h"

#define WORK_QUEUE_STACK_SIZE 512
#define WORK_QUEUE_PRIORITY 5
#define OP_INFOS_MAX_SIZE 20

typedef struct zbus_channel struct_zbus_channel;

K_THREAD_STACK_DEFINE(work_queue_stack_area, WORK_QUEUE_STACK_SIZE);

typedef enum op_kind {
    OP_PUBLISH,
    OP_READ,
    OP_CLAIM,
    OP_NOTIFY,
    OP_WAIT,
} op_kind_t;

typedef struct op_info {
    struct k_work work;
    uint8_t index;
    op_kind_t kind;
    struct_zbus_channel *chan;
    void *msg;
    void (*callback)(const void *);
    void *state;
} op_info_t;

static struct zbus_fields {
    bool is_init;
    struct k_work_q work_queue;
    op_info_t op_infos[OP_INFOS_MAX_SIZE];
    size_t op_infos_len;
} self = {
    .is_init = false,
    .op_infos = {},
    .op_infos_len = 0U,
};

static void init_work_queue(void);
static op_info_t *create_new_op_info(op_kind_t kind);
static void remove_op_info(op_info_t *op_info);
static void publish_work(struct k_work *item);
static void read_work(struct k_work *item);
static void claim_work(struct k_work *item);
static void notify_work(struct k_work *item);

static void hat_listener_callback(const struct zbus_channel *chan);

ZBUS_LISTENER_DEFINE(hat_listener, hat_listener_callback);

const uint8_t *zbus_channel_name(const struct_zbus_channel *chan) {
    return chan->name;
}

int zbus_finish(const struct_zbus_channel *chan) {
    return zbus_chan_finish(chan);
}

void *zbus_get_msg(const struct_zbus_channel *chan) {
    return chan->message;
}

void *zbus_get_user_data(const struct_zbus_channel *chan) {
    return chan->user_data;
}

int hat_zbus_publish_work_queue(const struct_zbus_channel *chan, const void *msg, void (*callback)(const void *),
                                 const void *state) {
    if (!self.is_init) {
        init_work_queue();
    }

    op_info_t *op_info = create_new_op_info(OP_PUBLISH);
    if (op_info == NULL) {
        return -ENOBUFS;
    }

    op_info->chan = (struct_zbus_channel *) chan;
    op_info->msg = (void *) msg;
    op_info->callback = callback;
    op_info->state = (void *) state;

    k_work_submit(&op_info->work);

    return 0;
}

int hat_zbus_claim_work_queue(const struct_zbus_channel *chan, void (*callback)(const void *),
                               const void *state) {
    if (!self.is_init) {
        init_work_queue();
    }

    op_info_t *op_info = create_new_op_info(OP_CLAIM);
    if (op_info == NULL) {
        return -ENOBUFS;
    }

    op_info->chan = (struct_zbus_channel *) chan;
    op_info->msg = NULL;
    op_info->callback = callback;
    op_info->state = (void *) state;

    k_work_submit(&op_info->work);

    return 0;
}

int hat_zbus_notify_work_queue(const struct_zbus_channel *chan, void (*callback)(const void *),
                                const void *state) {
    if (!self.is_init) {
        init_work_queue();
    }

    op_info_t *op_info = create_new_op_info(OP_NOTIFY);
    if (op_info == NULL) {
        return -ENOBUFS;
    }

    op_info->chan = (struct_zbus_channel *) chan;
    op_info->msg = NULL;
    op_info->callback = callback;
    op_info->state = (void *) state;

    k_work_submit(&op_info->work);

    return 0;
}

int hat_zbus_read_work_queue(const struct_zbus_channel *chan, void *msg, void (*callback)(const void *),
                              const void *state) {
    if (!self.is_init) {
        init_work_queue();
    }

    op_info_t *op_info = create_new_op_info(OP_READ);
    if (op_info == NULL) {
        return -ENOBUFS;
    }

    op_info->chan = (struct_zbus_channel *) chan;
    op_info->msg = msg;
    op_info->callback = callback;
    op_info->state = (void *) state;

    k_work_submit(&op_info->work);

    return 0;
}

int hat_zbus_wait_work_queue(const struct_zbus_channel *chan, void *msg, void (*callback)(const void *),
                             const void *state)
{
    if (!self.is_init) {
        init_work_queue();
    }

    op_info_t *op_info = create_new_op_info(OP_WAIT);
    if (op_info == NULL) {
        return -ENOBUFS;
    }

    op_info->chan = (struct_zbus_channel *) chan;
    op_info->msg = msg;
    op_info->callback = callback;
    op_info->state = (void *) state;

    return 0;
}

static void init_work_queue(void)
{
    k_work_queue_init(&self.work_queue);

    k_work_queue_start(&self.work_queue, work_queue_stack_area,
                       K_THREAD_STACK_SIZEOF(work_queue_stack_area), WORK_QUEUE_PRIORITY,
                       NULL);

    self.is_init = true;
}

static void publish_work(struct k_work *item)
{
    int err;
    op_info_t *op_info = CONTAINER_OF(item, struct op_info, work);

    err = zbus_chan_pub(op_info->chan, op_info->msg, K_FOREVER);
    // TODO check err

    op_info->callback(op_info->state);

    remove_op_info(op_info);
}

static void read_work(struct k_work *item)
{
    int err;
    op_info_t *op_info = CONTAINER_OF(item, struct op_info, work);

    err = zbus_chan_read(op_info->chan, op_info->msg, K_FOREVER);
    // TODO check err

    op_info->callback(op_info->state);

    remove_op_info(op_info);
}

static void claim_work(struct k_work *item)
{
    int err;
    op_info_t *op_info = CONTAINER_OF(item, struct op_info, work);

    err = zbus_chan_claim(op_info->chan, K_FOREVER);
    // TODO check err

    op_info->callback(op_info->state);

    remove_op_info(op_info);
}

static void notify_work(struct k_work *item)
{
    int err;
    op_info_t *op_info = CONTAINER_OF(item, struct op_info, work);

    err = zbus_chan_notify(op_info->chan, K_FOREVER);
    // TODO check err

    op_info->callback(op_info->state);

    remove_op_info(op_info);
}

static void hat_listener_callback(const struct zbus_channel *chan)
{
    int err;
    op_info_t *info;

    for (int i = 0; i < self.op_infos_len; ++i) {
        info = &self.op_infos[i];

        if (!(info->chan == chan && info->kind == OP_WAIT)) {
            continue;
        }

        err = zbus_chan_read(chan, info->msg, K_NO_WAIT);
        // TODO check err

        info->callback(info->state);

        remove_op_info(info);
        i -= 1;
    }
}

static op_info_t *create_new_op_info(op_kind_t kind)
{
    if (self.op_infos_len >= OP_INFOS_MAX_SIZE) {
        return NULL;
    }

    op_info_t *op_info = &self.op_infos[self.op_infos_len];
    op_info->index = self.op_infos_len;
    op_info->kind = kind;
    self.op_infos_len++;

    switch (kind) {
    case OP_PUBLISH:
        k_work_init(&op_info->work, publish_work);
        break;
    case OP_READ:
        k_work_init(&op_info->work, read_work);
        break;
    case OP_CLAIM:
        k_work_init(&op_info->work, claim_work);
        break;
    case OP_NOTIFY:
        k_work_init(&op_info->work, notify_work);
        break;
    case OP_WAIT:
        break;
    }

    return op_info;
}

static void remove_op_info(op_info_t *op_info)
{
    if (op_info->index < (self.op_infos_len - 1U)) {
        for (int i = op_info->index; i < (self.op_infos_len - 1U); ++i) {
            memcpy(&self.op_infos[i], &self.op_infos[i+1], sizeof(op_info_t));
            self.op_infos[i].index = i;
        }
    }

    self.op_infos_len -= 1U;
}
