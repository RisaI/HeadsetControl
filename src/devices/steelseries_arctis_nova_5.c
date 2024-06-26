#include "../device.h"
#include "../utility.h"

#include <hidapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_SIZE 64

static struct device device_arctis;

#define ID_ARCTIS_NOVA_5           0x2232

#define BATTERY_MAX 0x04
#define BATTERY_MIN 0x00

#define HEADSET_OFFLINE 0x00
#define STATUS_BUF_SIZE 8

#define EQUALIZER_BANDS_SIZE 10
#define EQUALIZER_BASELINE   0x14
#define EQUALIZER_BAND_MIN   -10
#define EQUALIZER_BAND_MAX   +10

static const uint16_t PRODUCT_IDS[] = { ID_ARCTIS_NOVA_5 };

static int arctis_nova_5_send_sidetone(hid_device* device_handle, uint8_t num);
static int arctis_nova_5_send_inactive_time(hid_device* device_handle, uint8_t num);
static int arctis_nova_5_send_equalizer_preset(hid_device* device_handle, uint8_t num);
static int arctis_nova_5_send_equalizer(hid_device* device_handle, struct equalizer_settings* settings);
static BatteryInfo arctis_nova_5_request_battery(hid_device* device_handle);
static int arctis_nova_5_bluetooth_when_powered_on(hid_device* device_handle, uint8_t num);
static int arctis_nova_5_bluetooth_call_volume(hid_device* device_handle, uint8_t num);
static int arctis_nova_5_mic_light(hid_device* device_handle, uint8_t num);
static int arctis_nova_5_mic_volume(hid_device* device_handle, uint8_t num);
static int arctis_nova_5_volume_limiter(hid_device* device_handle, uint8_t num);

int arctis_nova_5_read_device_status(hid_device* device_handle, unsigned char* data_read);

void arctis_nova_5_init(struct device** device)
{
    device_arctis.idVendor            = VENDOR_STEELSERIES;
    device_arctis.idProductsSupported = PRODUCT_IDS;
    device_arctis.numIdProducts       = sizeof(PRODUCT_IDS) / sizeof(PRODUCT_IDS[0]);

    strncpy(device_arctis.device_name, "SteelSeries Arctis Nova 5", sizeof(device_arctis.device_name));

    device_arctis.capabilities                                           = B(CAP_SIDETONE) | B(CAP_BATTERY_STATUS)  | B(CAP_INACTIVE_TIME) | B(CAP_EQUALIZER) | B(CAP_EQUALIZER_PRESET) | B(CAP_MICROPHONE_MUTE_LED_BRIGHTNESS) | B(CAP_MICROPHONE_VOLUME) | B(CAP_VOLUME_LIMITER) | B(CAP_BT_WHEN_POWERED_ON) | B(CAP_BT_CALL_VOLUME);
    device_arctis.capability_details[CAP_SIDETONE]                       = (struct capability_detail) { .usagepage = 0xffc0, .usageid = 0x1, .interface = 3 };
    device_arctis.capability_details[CAP_BATTERY_STATUS]                 = (struct capability_detail) { .usagepage = 0xffc0, .usageid = 0x1, .interface = 3 };
    device_arctis.capability_details[CAP_INACTIVE_TIME]                  = (struct capability_detail) { .usagepage = 0xffc0, .usageid = 0x1, .interface = 3 };
    device_arctis.capability_details[CAP_EQUALIZER_PRESET]               = (struct capability_detail) { .usagepage = 0xffc0, .usageid = 0x1, .interface = 3 };
    device_arctis.capability_details[CAP_EQUALIZER]                      = (struct capability_detail) { .usagepage = 0xffc0, .usageid = 0x1, .interface = 3 };
    device_arctis.capability_details[CAP_MICROPHONE_MUTE_LED_BRIGHTNESS] = (struct capability_detail) { .usagepage = 0xffc0, .usageid = 0x1, .interface = 3 };
    device_arctis.capability_details[CAP_MICROPHONE_VOLUME]              = (struct capability_detail) { .usagepage = 0xffc0, .usageid = 0x1, .interface = 3 };
    device_arctis.capability_details[CAP_VOLUME_LIMITER]                 = (struct capability_detail) { .usagepage = 0xffc0, .usageid = 0x1, .interface = 3 };
    device_arctis.capability_details[CAP_BT_WHEN_POWERED_ON]             = (struct capability_detail) { .usagepage = 0xffc0, .usageid = 0x1, .interface = 3 };
    device_arctis.capability_details[CAP_BT_CALL_VOLUME]                 = (struct capability_detail) { .usagepage = 0xffc0, .usageid = 0x1, .interface = 3 };

    // are working
    device_arctis.send_sidetone                       = &arctis_nova_5_send_sidetone;
    device_arctis.request_battery                     = &arctis_nova_5_request_battery;
    device_arctis.send_inactive_time                  = &arctis_nova_5_send_inactive_time;
    device_arctis.send_microphone_mute_led_brightness = &arctis_nova_5_mic_light;

    // unsure
    device_arctis.send_equalizer_preset               = &arctis_nova_5_send_equalizer_preset;
    device_arctis.send_equalizer                      = &arctis_nova_5_send_equalizer;
    device_arctis.send_microphone_volume              = &arctis_nova_5_mic_volume;
    device_arctis.send_volume_limiter                 = &arctis_nova_5_volume_limiter;
    device_arctis.send_bluetooth_when_powered_on      = &arctis_nova_5_bluetooth_when_powered_on;
    device_arctis.send_bluetooth_call_volume          = &arctis_nova_5_bluetooth_call_volume;

    *device = &device_arctis;
}

static int arctis_nova_5_send_sidetone(hid_device* device_handle, uint8_t num)
{
    /*
     * steps to make it work nicely with headset-charge-indicator
     */
    num = num / 12;

    uint8_t data[MSG_SIZE] = { 0x00, 0x39, num };

    return hid_write(device_handle, data, sizeof(data));
}

static int arctis_nova_5_send_inactive_time(hid_device* device_handle, uint8_t num)
{
    /*
     * Leave it default or set to 0 other ways' connection with headset is going to be permanently lost
     * On windows suspend packet information on interrupt is 0xbb but was not able to wake them up on windows in virtual box or linux.
     * My environment does not allow wake up scenario to happened.
     * https://support.steelseries.com/hc/en-us/articles/115000051472-The-SteelSeries-Engine-says-Reconnect-Headset-but-my-transmitter-is-connected-
     */

    uint8_t data[MSG_SIZE] = { 0x00, 0xa3, num };

    return hid_write(device_handle, data, MSG_SIZE);
}

static BatteryInfo arctis_nova_5_request_battery(hid_device* device_handle)
{
    // read device info
    unsigned char data_read[STATUS_BUF_SIZE];
    int r = arctis_nova_5_read_device_status(device_handle, data_read);

    BatteryInfo info = { .status = BATTERY_UNAVAILABLE, .level = -1 };

    if (r < 0) {
        info.status = BATTERY_HIDERROR;
        return info;
    }

    if (r == 0) {
        info.status = BATTERY_TIMEOUT;
        return info;
    }

    if (data_read[3] == HEADSET_OFFLINE)
        return info;

    if (data_read[3] == 0x01) {
        info.status = BATTERY_CHARGING;
    } else {
        info.status = BATTERY_AVAILABLE;
    }

    int bat = data_read[2];

    if (bat > BATTERY_MAX)
        info.level = 100;
    else
        info.level = map(bat, BATTERY_MIN, BATTERY_MAX, 0, 100);

    return info;
}

static int arctis_nova_5_send_equalizer_preset(hid_device* device_handle, uint8_t num)
{
    // This headset supports only 4 presets:
    // flat (default), bass boost, smiley, focus

    switch (num) {
    case 0: {
        uint8_t flat[MSG_SIZE] = { 0x0, 0x33, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x0 };
        return hid_write(device_handle, flat, MSG_SIZE);
    }
    case 1: {
        uint8_t bass[MSG_SIZE] = { 0x0, 0x33, 0x1b, 0x1f, 0x1c, 0x16, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12, 0x0 };
        return hid_write(device_handle, bass, MSG_SIZE);
    }
    case 2: {
        uint8_t smiley[MSG_SIZE] = { 0x0, 0x33, 0x0a, 0x0d, 0x12, 0x0d, 0x0f, 0x1c, 0x20, 0x1b, 0x0d, 0x14, 0x0 };
        return hid_write(device_handle, smiley, MSG_SIZE);
    }
    case 3: {
        uint8_t focus[MSG_SIZE] = { 0x0, 0x33, 0x1a, 0x1b, 0x17, 0x11, 0x0c, 0x0c, 0x0f, 0x17, 0x1a, 0x1c, 0x0 };
        return hid_write(device_handle, focus, MSG_SIZE);
    }
    default: {
        printf("Device only supports 0-3 range for presets.\n");
        return HSC_OUT_OF_BOUNDS;
    }
    }
}

static int arctis_nova_5_send_equalizer(hid_device* device_handle, struct equalizer_settings* settings)
{
    if (settings->size != EQUALIZER_BANDS_SIZE) {
        printf("Device only supports %d bands.\n", EQUALIZER_BANDS_SIZE);
        return HSC_OUT_OF_BOUNDS;
    }

    uint8_t data[MSG_SIZE] = { 0x0, 0x33 };
    for (int i = 0; i < settings->size; i++) {
        float band_value = settings->bands_values[i];
        if (band_value < EQUALIZER_BAND_MIN || band_value > EQUALIZER_BAND_MAX) {
            printf("Device only supports bands ranging from %d to %d.\n", EQUALIZER_BAND_MIN, EQUALIZER_BAND_MAX);
            return HSC_OUT_OF_BOUNDS;
        }

        data[i + 2] = (uint8_t)(EQUALIZER_BASELINE + band_value);
    }
    data[settings->size + 3] = 0x0;

    return hid_write(device_handle, data, MSG_SIZE);
}

int arctis_nova_5_read_device_status(hid_device* device_handle, unsigned char* data_read)
{
    unsigned char data_request[2] = { 0x00, 0xb0 };
    int r                         = hid_write(device_handle, data_request, sizeof(data_request));

    if (r < 0)
        return r;

    return hid_read_timeout(device_handle, data_read, STATUS_BUF_SIZE, hsc_device_timeout);
}

static int arctis_nova_5_bluetooth_when_powered_on(hid_device* device_handle, uint8_t num)
{
    unsigned char data[MSG_SIZE]  = { 0x00, 0xb2, num };
    unsigned char data2[MSG_SIZE] = { 0x00, 0x09, 0 };
    if (hid_write(device_handle, data, MSG_SIZE) >= 0) {
        return hid_write(device_handle, data2, MSG_SIZE);
    }
    return HSC_READ_TIMEOUT;
}

static int arctis_nova_5_bluetooth_call_volume(hid_device* device_handle, uint8_t num)
{
    // 0x00 do nothing
    // 0x01 lower volume by 12db
    // 0x02 mute game during call
    unsigned char data[MSG_SIZE] = { 0x00, 0xb3, num };
    return hid_write(device_handle, data, MSG_SIZE);
}

static int arctis_nova_5_mic_light(hid_device* device_handle, uint8_t num)
{
    // 0x00 off
    // 0x01
    // 0x02
    // 0x03 max
    unsigned char data[MSG_SIZE] = { 0x00, 0xae, num };
    return hid_write(device_handle, data, MSG_SIZE);
}

static int arctis_nova_5_mic_volume(hid_device* device_handle, uint8_t num)
{
    // 0x00 off
    // step + 0x01
    // 0x07 max
    num = num / 16;
    if (num == 8)
        num--;
    unsigned char data[MSG_SIZE] = { 0x00, 0x37, num };
    return hid_write(device_handle, data, MSG_SIZE);
}

static int arctis_nova_5_volume_limiter(hid_device* device_handle, uint8_t num)
{
    // 0x00 off
    // 0x01 on
    unsigned char data[MSG_SIZE] = { 0x00, 0x3a, num };
    return hid_write(device_handle, data, MSG_SIZE);
}
