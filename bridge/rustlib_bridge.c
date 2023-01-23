/**
 * @file rustlib_bridge.c
 * @author
 * @brief
 * @version 0.1
 * @date 22/01/2023
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

void main_task(void);

K_THREAD_DEFINE(main_task_id, 1024, main_task, NULL, NULL, NULL, 3, 0, 0);
