#define TAG "LED"

#include <esp_log.h>
#include <esp_system.h>

#include <driver/gpio.h>

#include "pinout.h"
#include "led.h"

void
led_init(void)
{
	gpio_config_t output = {
			.pin_bit_mask = (1ULL << PIN_STATUS),
			.mode = GPIO_MODE_OUTPUT,
	};

	gpio_config(&output);
	gpio_set_level(PIN_STATUS, true);
}

void
led_set(bool on)
{
	gpio_set_level(PIN_STATUS, on);
}
