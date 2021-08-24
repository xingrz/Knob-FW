#ifndef __PROJECT_TASKS__
#define __PROJECT_TASKS__

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TASK_ROTARY_STACK_SIZE (2 * 1024)
#define TASK_BUTTON_STACK_SIZE (2 * 1024)

void task_rotary(void *arg);
void task_button(void *arg);

#endif  // __PROJECT_TASKS__
