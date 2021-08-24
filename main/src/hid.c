#define TAG "HID"

#include <stdint.h>
#include <string.h>

#include <esp_log.h>
#include <esp_system.h>

#include <esp_bt.h>
#include <esp_bt_defs.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_gatt_defs.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>
#include <esp_hidd.h>

#include "hid.h"
#include "esp_hid_gap.h"

#define COUNT(x) (sizeof(x) / sizeof(x[0]))

#include "hid_descriptors.h"

static esp_hid_raw_report_map_t report_maps[] = {
		{.data = hidapiReportMap, .len = sizeof(hidapiReportMap)},
		{.data = keyboardReportMap, .len = sizeof(keyboardReportMap)},
};

static esp_hid_device_config_t hid_config = {
		.vendor_id = 0x16C0,
		.product_id = 0x05DF,
		.version = 0x0100,
		.serial_number = "1234567890",
		.report_maps = report_maps,
		.report_maps_len = COUNT(report_maps),
};

static esp_hidd_dev_t *hid_dev = NULL;
static bool dev_connected = false;

// Keyboard input report ID
#define HID_RPT_ID_KEY_IN 2

// HID keyboard input report length
#define HID_KEYBOARD_IN_RPT_LEN 8

void
esp_hidd_send_keyboard_value(uint8_t special_key_mask, uint8_t *keyboard_cmd, uint8_t num_key)
{
	if (num_key > HID_KEYBOARD_IN_RPT_LEN - 2) {
		ESP_LOGE(TAG, "%s(), the number key should not be more than %d", __func__,
				HID_KEYBOARD_IN_RPT_LEN);
		return;
	}

	uint8_t buffer[HID_KEYBOARD_IN_RPT_LEN] = {0};

	buffer[0] = special_key_mask;

	for (int i = 0; i < num_key; i++) {
		buffer[i + 2] = keyboard_cmd[i];
	}

	ESP_LOGD(TAG, "the key vaule = %d,%d,%d, %d, %d, %d,%d, %d", buffer[0], buffer[1], buffer[2],
			buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);

	esp_hidd_dev_input_set(hid_dev, 1, HID_RPT_ID_KEY_IN, buffer, HID_KEYBOARD_IN_RPT_LEN);
}

static void
hidd_event_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
	esp_hidd_event_t event = (esp_hidd_event_t)id;
	esp_hidd_event_data_t *param = (esp_hidd_event_data_t *)event_data;

	switch (event) {
		case ESP_HIDD_START_EVENT: {
			ESP_LOGI(TAG, "START");
			esp_hid_ble_gap_adv_start();
			break;
		}
		case ESP_HIDD_CONNECT_EVENT: {
			ESP_LOGI(TAG, "CONNECT");
			dev_connected = true;  // todo: this should be on auth_complete (in GAP)
			break;
		}
		case ESP_HIDD_PROTOCOL_MODE_EVENT: {
			ESP_LOGI(TAG, "PROTOCOL MODE[%u]: %s", param->protocol_mode.map_index,
					param->protocol_mode.protocol_mode ? "REPORT" : "BOOT");
			break;
		}
		case ESP_HIDD_CONTROL_EVENT: {
			ESP_LOGI(TAG, "CONTROL[%u]: %sSUSPEND", param->control.map_index,
					param->control.control ? "EXIT_" : "");
			break;
		}
		case ESP_HIDD_OUTPUT_EVENT: {
			ESP_LOGI(TAG, "OUTPUT[%u]: %8s ID: %2u, Len: %d", param->output.map_index,
					esp_hid_usage_str(param->output.usage), param->output.report_id,
					param->output.length);
			break;
		}
		case ESP_HIDD_FEATURE_EVENT: {
			ESP_LOGI(TAG, "FEATURE[%u]: %8s ID: %2u, Len: %d", param->feature.map_index,
					esp_hid_usage_str(param->feature.usage), param->feature.report_id,
					param->feature.length);
			break;
		}
		case ESP_HIDD_DISCONNECT_EVENT: {
			ESP_LOGI(TAG, "DISCONNECT: %s",
					esp_hid_disconnect_reason_str(esp_hidd_dev_transport_get(param->disconnect.dev),
							param->disconnect.reason));
			dev_connected = false;
			esp_hid_ble_gap_adv_start();
			break;
		}
		case ESP_HIDD_STOP_EVENT: {
			ESP_LOGI(TAG, "STOP");
			break;
		}
		default:
			break;
	}
	return;
}

void
hid_init(const char *manufacturer_name, const char *device_name)
{
	esp_err_t ret;

	ret = esp_hid_gap_init(ESP_BT_MODE_BLE);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "esp_hid_gap_init failed: %d", ret);
		return;
	}

	hid_config.manufacturer_name = manufacturer_name;
	hid_config.device_name = device_name;

	ret = esp_hid_ble_gap_adv_init(ESP_HID_APPEARANCE_KEYBOARD, hid_config.device_name);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "esp_hid_ble_gap_adv_init failed: %d", ret);
		return;
	}

	ret = esp_ble_gatts_register_callback(esp_hidd_gatts_event_handler);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "esp_ble_gatts_register_callback failed: %d", ret);
		return;
	}

	ret = esp_hidd_dev_init(&hid_config, ESP_HID_TRANSPORT_BLE, hidd_event_callback, &hid_dev);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "esp_hidd_dev_init failed: %d", ret);
		return;
	}
}

void
hid_report_key(uint8_t key, bool pressed)
{
	if (!dev_connected) return;
	uint8_t keys[] = {key};
	esp_hidd_send_keyboard_value(0, keys, pressed ? 1 : 0);
}
