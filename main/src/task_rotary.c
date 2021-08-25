#define TAG "ROTARY"

#include <esp_log.h>
#include <esp_system.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <driver/gpio.h>

#include "task_rotary.h"

#include "tasks.h"
#include "pinout.h"

#include "hid.h"

#define DIR_CW 0x10  // Clockwise step.
#define DIR_CCW 0x20  // Anti-clockwise step.

#define R_START 0x0
#define F_CW_FINAL 0x1
#define F_CW_BEGIN 0x2
#define F_CW_NEXT 0x3
#define F_CCW_BEGIN 0x4
#define F_CCW_FINAL 0x5
#define F_CCW_NEXT 0x6

static const uint8_t ENC_TABLE[7][4] = {
		{R_START, F_CW_BEGIN, F_CCW_BEGIN, R_START},  // R_START
		{F_CW_NEXT, R_START, F_CW_FINAL, R_START | DIR_CW},  // F_CW_FINAL
		{F_CW_NEXT, F_CW_BEGIN, R_START, R_START},  // F_CW_BEGIN
		{F_CW_NEXT, F_CW_BEGIN, F_CW_FINAL, R_START},  // F_CW_NEXT
		{F_CCW_NEXT, R_START, F_CCW_BEGIN, R_START},  // F_CCW_BEGIN
		{F_CCW_NEXT, F_CCW_FINAL, R_START, R_START | DIR_CCW},  // F_CCW_FINAL
		{F_CCW_NEXT, F_CCW_FINAL, F_CCW_BEGIN, R_START},  // F_CCW_NEXT
};

static xQueueHandle enc_q = NULL;
static uint8_t enc_state = R_START;

static volatile TickType_t lock_until = 0;

static void
rotary_handler(void *arg)
{
	uint8_t pin_state = (gpio_get_level(PIN_ROTARY_A) << 1) | gpio_get_level(PIN_ROTARY_B);
	enc_state = ENC_TABLE[enc_state & 0xf][pin_state];
	uint8_t event = enc_state & 0x30;
	if (event == DIR_CW || event == DIR_CCW) {
		BaseType_t woken = pdFALSE;
		xQueueSendFromISR(enc_q, &event, &woken);
		if (woken) portYIELD_FROM_ISR();
	}
}

static void
report_key(uint8_t key)
{
	hid_report_key(key, true);
	vTaskDelay(5 / portTICK_PERIOD_MS);
	hid_report_key(key, false);
}

void
task_rotary(void *arg)
{
	enc_q = xQueueCreate(20, sizeof(uint8_t));

	gpio_config_t input = {
			.intr_type = GPIO_INTR_ANYEDGE,
			.pin_bit_mask = (1ULL << PIN_ROTARY_A) | (1ULL << PIN_ROTARY_B),
			.mode = GPIO_MODE_INPUT,
			.pull_up_en = GPIO_PULLUP_ENABLE,
	};

	gpio_config(&input);

	gpio_isr_handler_add(PIN_ROTARY_A, rotary_handler, NULL);
	gpio_isr_handler_add(PIN_ROTARY_B, rotary_handler, NULL);

	uint8_t msg;
	while (1) {
		if (xQueueReceive(enc_q, &msg, portMAX_DELAY) != pdPASS) {
			continue;
		}

		if (xTaskGetTickCount() < lock_until) {
			continue;
		}

		if (msg == DIR_CW) {
			report_key(HID_KEY_VOLUME_UP);
		} else if (msg == DIR_CCW) {
			report_key(HID_KEY_VOLUME_DOWN);
		}
	}

	vTaskDelete(NULL);
}

void
rotary_lock(int timeout_ms)
{
	TickType_t current = xTaskGetTickCount();
	lock_until = current + timeout_ms / portTICK_PERIOD_MS;
}
