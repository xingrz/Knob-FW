#define TAG "MAIN"

#include <esp_log.h>
#include <esp_system.h>

#include <driver/gpio.h>

#include "tasks.h"

void
app_main()
{
	ESP_ERROR_CHECK(gpio_install_isr_service(0));

	assert(pdPASS == xTaskCreate(task_rotary, "task_rotary", TASK_ROTARY_STACK_SIZE, NULL,
							 tskIDLE_PRIORITY + 1, NULL));

	ESP_LOGI(TAG, "SYSTEM READY");
}
