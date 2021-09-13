#ifndef __PROJECT_HID__
#define __PROJECT_HID__

#include <stdint.h>
#include <stdbool.h>

#define HID_USE_CONSUMER_CONTROL

#define KEY_MUTE 113
#define KEY_VOLUMEDOWN 114
#define KEY_VOLUMEUP 115

void hid_init(const char *manufacturer_name, const char *device_name_prefix);
void hid_report_key(uint8_t key, bool pressed);

#endif  // __PROJECT_HID__
