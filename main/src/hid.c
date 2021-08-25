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

const unsigned char hidapiReportMap[] = {
		// 8 bytes input, 8 bytes feature
		0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
		0x0A, 0x00, 0x01,  // Usage (0x0100)
		0xA1, 0x01,  // Collection (Application)
		0x85, 0x01,  //   Report ID (1)
		0x15, 0x00,  //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,  //   Report Size (8)
		0x95, 0x08,  //   Report Count (8)
		0x09, 0x01,  //   Usage (0x01)
		0x82, 0x02, 0x01,  //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null
		//   Position,Buffered Bytes)
		0x95, 0x08,  //   Report Count (8)
		0x09, 0x02,  //   Usage (0x02)
		0xB2, 0x02, 0x01,  //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null
		//   Position,Non-volatile,Buffered Bytes)
		0x95, 0x08,  //   Report Count (8)
		0x09, 0x03,  //   Usage (0x03)
		0x91, 0x02,  //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null
		//   Position,Non-volatile)
		0xC0,  // End Collection
};

const unsigned char mouseReportMap[] = {
		0x05, 0x01,  // Usage Page (Generic Desktop)
		0x09, 0x02,  // Usage (Mouse)
		0xA1, 0x01,  // Collection (Application)
		0x85, 0x01,  // Report Id (1)
		0x09, 0x01,  //   Usage (Pointer)
		0xA1, 0x00,  //   Collection (Physical)
		0x05, 0x09,  //     Usage Page (Buttons)
		0x19, 0x01,  //     Usage Minimum (01) - Button 1
		0x29, 0x03,  //     Usage Maximum (03) - Button 3
		0x15, 0x00,  //     Logical Minimum (0)
		0x25, 0x01,  //     Logical Maximum (1)
		0x75, 0x01,  //     Report Size (1)
		0x95, 0x03,  //     Report Count (3)
		0x81, 0x02,  //     Input (Data, Variable, Absolute) - Button states
		0x75, 0x05,  //     Report Size (5)
		0x95, 0x01,  //     Report Count (1)
		0x81, 0x01,  //     Input (Constant) - Padding or Reserved bits
		0x05, 0x01,  //     Usage Page (Generic Desktop)
		0x09, 0x30,  //     Usage (X)
		0x09, 0x31,  //     Usage (Y)
		0x09, 0x38,  //     Usage (Wheel)
		0x15, 0x81,  //     Logical Minimum (-127)
		0x25, 0x7F,  //     Logical Maximum (127)
		0x75, 0x08,  //     Report Size (8)
		0x95, 0x03,  //     Report Count (3)
		0x81, 0x06,  //     Input (Data, Variable, Relative) - X & Y coordinate
		0xC0,  //   End Collection
		0xC0,  // End Collection
};

const unsigned char keyboardReportMap[] = {
		0x05, 0x01,  // Usage Pg (Generic Desktop)
		0x09, 0x06,  // Usage (Keyboard)
		0xA1, 0x01,  // Collection: (Application)
		0x85, 0x02,  // Report Id (2)
		//
		0x05, 0x07,  //   Usage Pg (Key Codes)
		0x19, 0xE0,  //   Usage Min (224)
		0x29, 0xE7,  //   Usage Max (231)
		0x15, 0x00,  //   Log Min (0)
		0x25, 0x01,  //   Log Max (1)
		//
		//   Modifier byte
		0x75, 0x01,  //   Report Size (1)
		0x95, 0x08,  //   Report Count (8)
		0x81, 0x02,  //   Input: (Data, Variable, Absolute)
		//
		//   Reserved byte
		0x95, 0x01,  //   Report Count (1)
		0x75, 0x08,  //   Report Size (8)
		0x81, 0x01,  //   Input: (Constant)
		//
		//   LED report
		0x95, 0x05,  //   Report Count (5)
		0x75, 0x01,  //   Report Size (1)
		0x05, 0x08,  //   Usage Pg (LEDs)
		0x19, 0x01,  //   Usage Min (1)
		0x29, 0x05,  //   Usage Max (5)
		0x91, 0x02,  //   Output: (Data, Variable, Absolute)
		//
		//   LED report padding
		0x95, 0x01,  //   Report Count (1)
		0x75, 0x03,  //   Report Size (3)
		0x91, 0x01,  //   Output: (Constant)
		//
		//   Key arrays (6 bytes)
		0x95, 0x06,  //   Report Count (6)
		0x75, 0x08,  //   Report Size (8)
		0x15, 0x00,  //   Log Min (0)
		0x25, 0x65,  //   Log Max (101)
		0x05, 0x07,  //   Usage Pg (Key Codes)
		0x19, 0x00,  //   Usage Min (0)
		0x29, 0x65,  //   Usage Max (101)
		0x81, 0x00,  //   Input: (Data, Array)
		//
		0xC0,  // End Collection
};

const unsigned char consumerReportMap[] = {
		0x05, 0x0C,  // Usage Pg (Consumer Devices)
		0x09, 0x01,  // Usage (Consumer Control)
		0xA1, 0x01,  // Collection (Application)
		0x85, 0x03,  // Report Id (3)
		0x09, 0x02,  //   Usage (Numeric Key Pad)
		0xA1, 0x02,  //   Collection (Logical)
		0x05, 0x09,  //     Usage Pg (Button)
		0x19, 0x01,  //     Usage Min (Button 1)
		0x29, 0x0A,  //     Usage Max (Button 10)
		0x15, 0x01,  //     Logical Min (1)
		0x25, 0x0A,  //     Logical Max (10)
		0x75, 0x04,  //     Report Size (4)
		0x95, 0x01,  //     Report Count (1)
		0x81, 0x00,  //     Input (Data, Ary, Abs)
		0xC0,  //   End Collection
		0x05, 0x0C,  //   Usage Pg (Consumer Devices)
		0x09, 0x86,  //   Usage (Channel)
		0x15, 0xFF,  //   Logical Min (-1)
		0x25, 0x01,  //   Logical Max (1)
		0x75, 0x02,  //   Report Size (2)
		0x95, 0x01,  //   Report Count (1)
		0x81, 0x46,  //   Input (Data, Var, Rel, Null)
		0x09, 0xE9,  //   Usage (Volume Up)
		0x09, 0xEA,  //   Usage (Volume Down)
		0x15, 0x00,  //   Logical Min (0)
		0x75, 0x01,  //   Report Size (1)
		0x95, 0x02,  //   Report Count (2)
		0x81, 0x02,  //   Input (Data, Var, Abs)
		0x09, 0xE2,  //   Usage (Mute)
		0x09, 0x30,  //   Usage (Power)
		0x09, 0x83,  //   Usage (Recall Last)
		0x09, 0x81,  //   Usage (Assign Selection)
		0x09, 0xB0,  //   Usage (Play)
		0x09, 0xB1,  //   Usage (Pause)
		0x09, 0xB2,  //   Usage (Record)
		0x09, 0xB3,  //   Usage (Fast Forward)
		0x09, 0xB4,  //   Usage (Rewind)
		0x09, 0xB5,  //   Usage (Scan Next)
		0x09, 0xB6,  //   Usage (Scan Prev)
		0x09, 0xB7,  //   Usage (Stop)
		0x15, 0x01,  //   Logical Min (1)
		0x25, 0x0C,  //   Logical Max (12)
		0x75, 0x04,  //   Report Size (4)
		0x95, 0x01,  //   Report Count (1)
		0x81, 0x00,  //   Input (Data, Ary, Abs)
		0x09, 0x80,  //   Usage (Selection)
		0xA1, 0x02,  //   Collection (Logical)
		0x05, 0x09,  //     Usage Pg (Button)
		0x19, 0x01,  //     Usage Min (Button 1)
		0x29, 0x03,  //     Usage Max (Button 3)
		0x15, 0x01,  //     Logical Min (1)
		0x25, 0x03,  //     Logical Max (3)
		0x75, 0x02,  //     Report Size (2)
		0x81, 0x00,  //     Input (Data, Ary, Abs)
		0xC0,  //   End Collection
		0x81, 0x03,  //   Input (Const, Var, Abs)
		0xC0,  // End Collectionq
};

static esp_hid_raw_report_map_t report_maps[] = {
		{.data = hidapiReportMap, .len = sizeof(hidapiReportMap)},
		{.data = mouseReportMap, .len = sizeof(mouseReportMap)},
		{.data = keyboardReportMap, .len = sizeof(keyboardReportMap)},
		{.data = consumerReportMap, .len = sizeof(consumerReportMap)},
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
			ESP_LOGI(TAG, "OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:", param->output.map_index,
					esp_hid_usage_str(param->output.usage), param->output.report_id,
					param->output.length);
			ESP_LOG_BUFFER_HEX(TAG, param->output.data, param->output.length);
			break;
		}
		case ESP_HIDD_FEATURE_EVENT: {
			ESP_LOGI(TAG, "FEATURE[%u]: %8s ID: %2u, Len: %d, Data:", param->feature.map_index,
					esp_hid_usage_str(param->feature.usage), param->feature.report_id,
					param->feature.length);
			ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
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
