/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "usb_descriptors.h"
#include "pico/unique_id.h"
#include "tusb.h"

/* A combination of interfaces must have a unique product id, since PC will save
 * device driver after the first plug. Same VID/PID with different interface e.g
 * MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USB_PID                                                      \
    (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
     _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t desc_device_dev = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0xCaff,
    .idProduct = USB_PID,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*)&desc_device_dev;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report_cardio[] = {
    AIC_PICO_REPORT_DESC_CARDIO,
};

uint8_t const desc_hid_report_nkro[] = {
    AIC_PICO_REPORT_DESC_NKRO,
};

uint8_t const desc_hid_report_light[] = {
    AIC_PICO_REPORT_DESC_LIGHT,
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf)
{
    switch (itf) {
        case 0:
            return desc_hid_report_cardio;
        case 1:
            return desc_hid_report_nkro;
        case 2:
            return desc_hid_report_light;
        default:
            return NULL;
    }
}
//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum { ITF_NUM_CARDIO, ITF_NUM_NKRO, ITF_NUM_LIGHT,
       ITF_NUM_CLI, ITF_NUM_CLI_DATA,
       ITF_NUM_AIME, ITF_NUM_AIME_DATA,
       ITF_NUM_TOTAL };

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN * 3 + TUD_CDC_DESC_LEN * 2)

#define EPNUM_CARDIO 0x81
#define EPNUM_KEY 0x82
#define EPNUM_LIGHT 0x83

#define EPNUM_CLI_NOTIF 0x85
#define EPNUM_CLI_OUT   0x06
#define EPNUM_CLI_IN    0x86

#define EPNUM_AIME_NOTIF 0x87
#define EPNUM_AIME_OUT   0x08
#define EPNUM_AIME_IN    0x88

uint8_t const desc_configuration_dev[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 200),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_CARDIO, 4, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report_cardio), EPNUM_CARDIO,
                       CFG_TUD_HID_EP_BUFSIZE, 1),

    TUD_HID_DESCRIPTOR(ITF_NUM_NKRO, 5, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report_nkro), EPNUM_KEY,
                       CFG_TUD_HID_EP_BUFSIZE, 1),

    TUD_HID_DESCRIPTOR(ITF_NUM_LIGHT, 6, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report_light), EPNUM_LIGHT,
                       CFG_TUD_HID_EP_BUFSIZE, 4),

    TUD_CDC_DESCRIPTOR(ITF_NUM_CLI, 7, EPNUM_CLI_NOTIF,
                       8, EPNUM_CLI_OUT, EPNUM_CLI_IN, 64),

    TUD_CDC_DESCRIPTOR(ITF_NUM_AIME, 8, EPNUM_AIME_NOTIF,
                       8, EPNUM_AIME_OUT, EPNUM_AIME_IN, 64),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    return desc_configuration_dev;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

static char serial_number_str[24] = "123456\0";

// array of pointer to string descriptors
const char *string_desc_arr[] = {
    (const char[]){0x09, 0x04},  // 0: is supported language is English (0x0409)
    "WHowe",                     // 1: Manufacturer
    "AIC Pico",                  // 2: Product
    serial_number_str,           // 3: Serials, should use chip ID
    "AIC Pico CardIO",
    "AIC Pico Keypad",
    "AIC Pico LED",
    "AIC Pico CLI Port",
    "AIC Pico AIME Port",
    "AIC Pico Red Light",
    "AIC Pico Green Light",
    "AIC Pico Blue Light",
};

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    static uint16_t _desc_str[64];

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 + 2);
        return _desc_str;
    }

    if (index == 3) {
        pico_unique_board_id_t board_id;
        pico_get_unique_board_id(&board_id);
        sprintf(serial_number_str, "%016llx", *(uint64_t *)&board_id);
    }
    
    const size_t base_num = sizeof(string_desc_arr) / sizeof(string_desc_arr[0]);
    char str[64];

    if (index < base_num) {
        strcpy(str, string_desc_arr[index]);
    } else {
        sprintf(str, "Unknown %d", index);
    }

    uint8_t chr_count = strlen(str);
    if (chr_count > 63) {
        chr_count = 63;
    }

    // Convert ASCII string into UTF-16
    for (uint8_t i = 0; i < chr_count; i++) {
        _desc_str[1 + i] = str[i];
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}
