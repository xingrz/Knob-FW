#define TAG "MAIN"

#include <esp_log.h>
#include <esp_system.h>

#include <nvs.h>
#include <nvs_flash.h>

#include <driver/gpio.h>

#include "tasks.h"
#include "hid.h"
#include "led.h"

static esp_err_t
app_nvs_init()
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	return ret;
}

void
app_main()
{
	ESP_ERROR_CHECK(app_nvs_init());

	ESP_ERROR_CHECK(gpio_install_isr_service(0));

	hid_init("XiNGRZ", "knob");
	led_init();

	assert(pdPASS == xTaskCreate(task_rotary, "task_rotary", TASK_ROTARY_STACK_SIZE, NULL,
							 tskIDLE_PRIORITY + 1, NULL));

	assert(pdPASS == xTaskCreate(task_button, "task_button", TASK_BUTTON_STACK_SIZE, NULL,
							 tskIDLE_PRIORITY + 1, NULL));

	ESP_LOGI(TAG, "SYSTEM READY");
}
