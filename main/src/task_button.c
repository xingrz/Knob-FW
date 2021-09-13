#define TAG "BUTTON"

#include <esp_log.h>
#include <esp_system.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <driver/gpio.h>

#include "task_rotary.h"

#include "tasks.h"
#include "pinout.h"

#include "hid.h"

static xQueueHandle btn_q = NULL;

static void
button_handler(void *arg)
{
	uint8_t state = gpio_get_level(PIN_ENTER);
	BaseType_t woken = pdFALSE;
	xQueueSendFromISR(btn_q, &state, &woken);
	if (woken) portYIELD_FROM_ISR();
}

void
task_button(void *arg)
{
	btn_q = xQueueCreate(2, sizeof(uint8_t));

	gpio_config_t input = {
			.intr_type = GPIO_INTR_ANYEDGE,
			.pin_bit_mask = (1ULL << PIN_ENTER),
			.mode = GPIO_MODE_INPUT,
			.pull_up_en = GPIO_PULLUP_ENABLE,
	};

	gpio_config(&input);

	gpio_isr_handler_add(PIN_ENTER, button_handler, NULL);

	uint8_t msg;
	while (1) {
		if (xQueueReceive(btn_q, &msg, portMAX_DELAY) != pdPASS) {
			continue;
		}

		rotary_lock(500);
		hid_report_key(KEY_MUTE, msg);
	}

	vTaskDelete(NULL);
}
