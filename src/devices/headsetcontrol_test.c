#include "../device.h"
#include "../utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct device device_headsetcontrol_test;

#define TESTBYTES_SEND 32

static const uint16_t PRODUCT_IDS[] = { PRODUCT_TESTDEVICE };

static int headsetcontrol_test_send_sidetone(hid_device* device_handle, uint8_t num);
static BatteryInfo headsetcontrol_test_request_battery(hid_device* device_handle);
static int headsetcontrol_test_notification_sound(hid_device* device_handle, uint8_t soundid);
static int headsetcontrol_test_lights(hid_device* device_handle, uint8_t on);
static int headsetcontrol_test_send_equalizer_preset(hid_device* device_handle, uint8_t num);
static int headsetcontrol_test_send_equalizer(hid_device* device_handle, struct equalizer_settings* settings);
static int headsetcontrol_test_send_microphone_mute_led_brightness(hid_device* device_handle, uint8_t num);
static int headsetcontrol_test_send_microphone_volume(hid_device* device_handle, uint8_t num);
static int headsetcontrol_test_switch_voice_prompts(hid_device* device_handle, uint8_t on);
static int headsetcontrol_test_switch_rotate_to_mute(hid_device* device_handle, uint8_t on);
static int headsetcontrol_test_request_chatmix(hid_device* device_handle);
static int headsetcontrol_test_set_inactive_time(hid_device* device_handle, uint8_t minutes);
static int headsetcontrol_test_volume_limiter(hid_device* device_handle, uint8_t num);
static int headsetcontrol_test_bluetooth_when_powered_on(hid_device* device_handle, uint8_t num);
static int headsetcontrol_test_bluetooth_call_volume(hid_device* device_handle, uint8_t num);

extern int test_profile;

void headsetcontrol_test_init(struct device** device)
{
    if (test_profile < 0 || test_profile > 10) {
        printf("test_profile must be between 0 and 10\n");
        abort();
    }

    device_headsetcontrol_test.idVendor            = VENDOR_TESTDEVICE;
    device_headsetcontrol_test.idProductsSupported = PRODUCT_IDS;
    device_headsetcontrol_test.numIdProducts       = 1;

    strncpy(device_headsetcontrol_test.device_name, "HeadsetControl Test device", sizeof(device_headsetcontrol_test.device_name));
    // normally filled by hid in main.c
    wcsncpy(device_headsetcontrol_test.device_hid_vendorname, L"HeadsetControl", sizeof(device_headsetcontrol_test.device_hid_vendorname) / sizeof(device_headsetcontrol_test.device_hid_vendorname[0]));
    wcsncpy(device_headsetcontrol_test.device_hid_productname, L"Test device", sizeof(device_headsetcontrol_test.device_hid_productname) / sizeof(device_headsetcontrol_test.device_hid_productname[0]));

    if (test_profile != 10) {
        device_headsetcontrol_test.capabilities = B(CAP_SIDETONE) | B(CAP_BATTERY_STATUS) | B(CAP_NOTIFICATION_SOUND) | B(CAP_LIGHTS) | B(CAP_INACTIVE_TIME) | B(CAP_CHATMIX_STATUS) | B(CAP_VOICE_PROMPTS) | B(CAP_ROTATE_TO_MUTE) | B(CAP_EQUALIZER_PRESET) | B(CAP_EQUALIZER) | B(CAP_MICROPHONE_MUTE_LED_BRIGHTNESS) | B(CAP_MICROPHONE_VOLUME) | B(CAP_VOLUME_LIMITER) | B(CAP_BT_WHEN_POWERED_ON) | B(CAP_BT_CALL_VOLUME);
    } else {
        device_headsetcontrol_test.capabilities = B(CAP_SIDETONE) | B(CAP_LIGHTS) | B(CAP_BATTERY_STATUS);
    }

    device_headsetcontrol_test.send_sidetone                       = &headsetcontrol_test_send_sidetone;
    device_headsetcontrol_test.request_battery                     = &headsetcontrol_test_request_battery;
    device_headsetcontrol_test.notifcation_sound                   = &headsetcontrol_test_notification_sound;
    device_headsetcontrol_test.switch_lights                       = &headsetcontrol_test_lights;
    device_headsetcontrol_test.send_inactive_time                  = &headsetcontrol_test_set_inactive_time;
    device_headsetcontrol_test.request_chatmix                     = &headsetcontrol_test_request_chatmix;
    device_headsetcontrol_test.switch_voice_prompts                = &headsetcontrol_test_switch_voice_prompts;
    device_headsetcontrol_test.switch_rotate_to_mute               = &headsetcontrol_test_switch_rotate_to_mute;
    device_headsetcontrol_test.send_equalizer_preset               = &headsetcontrol_test_send_equalizer_preset;
    device_headsetcontrol_test.send_equalizer                      = &headsetcontrol_test_send_equalizer;
    device_headsetcontrol_test.send_microphone_mute_led_brightness = &headsetcontrol_test_send_microphone_mute_led_brightness;
    device_headsetcontrol_test.send_microphone_volume              = &headsetcontrol_test_send_microphone_volume;
    device_headsetcontrol_test.send_volume_limiter                 = &headsetcontrol_test_volume_limiter;
    device_headsetcontrol_test.send_bluetooth_when_powered_on      = &headsetcontrol_test_bluetooth_when_powered_on;
    device_headsetcontrol_test.send_bluetooth_call_volume          = &headsetcontrol_test_bluetooth_call_volume;

    *device = &device_headsetcontrol_test;
}

static int headsetcontrol_test_send_sidetone(hid_device* device_handle, uint8_t num)
{
    if (test_profile == 1) {
        return -1;
    }

    return TESTBYTES_SEND;
}

static BatteryInfo headsetcontrol_test_request_battery(hid_device* device_handle)
{
    BatteryInfo info = { .status = BATTERY_UNAVAILABLE, .level = -1 };

    switch (test_profile) {
    case 0:
        info.status = BATTERY_AVAILABLE;
        info.level  = 42;
        break;
    case 1:
        info.status = BATTERY_HIDERROR;
        break;
    case 2:
        info.status = BATTERY_CHARGING;
        info.level  = 50;
        break;
    case 3:
        info.status = BATTERY_AVAILABLE;
        info.level  = 64;
        break;
    case 4:
        info.status = BATTERY_UNAVAILABLE;
        break;
    case 5:
        info.status = BATTERY_TIMEOUT;
        break;
    }

    return info;
}

static int headsetcontrol_test_notification_sound(hid_device* device_handle, uint8_t soundid)
{
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_lights(hid_device* device_handle, uint8_t on)
{
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_send_equalizer_preset(hid_device* device_handle, uint8_t num)
{
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_send_equalizer(hid_device* device_handle, struct equalizer_settings* settings)
{
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_send_microphone_mute_led_brightness(hid_device* device_handle, uint8_t num)
{
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_send_microphone_volume(hid_device* device_handle, uint8_t num)
{
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_switch_voice_prompts(hid_device* device_handle, uint8_t on)
{
    if (test_profile == 1) {
        return -1;
    }
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_switch_rotate_to_mute(hid_device* device_handle, uint8_t on)
{
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_request_chatmix(hid_device* device_handle)
{
    if (test_profile == 1) {
        return -1;
    } else if (test_profile == 2) {
        return -1;
    }

    return 42;
}

static int headsetcontrol_test_set_inactive_time(hid_device* device_handle, uint8_t minutes)
{
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_volume_limiter(hid_device* device_handle, uint8_t num)
{
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_bluetooth_when_powered_on(hid_device* device_handle, uint8_t num)
{
    return TESTBYTES_SEND;
}

static int headsetcontrol_test_bluetooth_call_volume(hid_device* device_handle, uint8_t num)
{
    return TESTBYTES_SEND;
}
