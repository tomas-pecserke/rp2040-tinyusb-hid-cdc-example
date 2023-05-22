#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <pico/stdio.h>

#ifndef PICO_STDIO_TINYUSB_CDC_DEFAULT_ITF
#define PICO_STDIO_TINYUSB_CDC_DEFAULT_ITF 0
#endif

#ifndef PICO_STDIO_TINYUSB_DEFAULT_CRLF
#define PICO_STDIO_TINYUSB_DEFAULT_CRLF PICO_STDIO_DEFAULT_CRLF
#endif

// PICO_CONFIG: PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS,
// Maximum number of milliseconds to wait during initialization for a CDC connection from the host
// (negative means indefinite) during initialization,
// default=0, group=pico_stdio_usb
#ifndef PICO_STDIO_TINYUSB_CDC_CONNECT_WAIT_TIMEOUT_MS
#define PICO_STDIO_TINYUSB_CDC_CONNECT_WAIT_TIMEOUT_MS 0
#endif

// PICO_CONFIG: PICO_STDIO_TINYUSB_CDC_POST_CONNECT_WAIT_DELAY_MS,
// Number of extra milliseconds to wait when using PICO_STDIO_TINYUSB_CDC_CONNECT_WAIT_TIMEOUT_MS
// after a host CDC connection is detected
// (some host terminals seem to sometimes lose transmissions sent right after connection),
// default=50,
// group=stdio_tinyusb_cdc
#ifndef PICO_STDIO_TINYUSB_CDC_POST_CONNECT_WAIT_DELAY_MS
#define PICO_STDIO_TINYUSB_CDC_POST_CONNECT_WAIT_DELAY_MS 50
#endif

// PICO_CONFIG: PICO_STDIO_TINYUSB_CDC_CONNECTION_WITHOUT_DTR,
// Disable use of DTR for connection checking meaning connection is assumed to be valid,
// type=bool, default=0, group=pico_stdio_usb
#ifndef PICO_STDIO_TINYUSB_CDC_CONNECTION_WITHOUT_DTR
#define PICO_STDIO_TINYUSB_CDC_CONNECTION_WITHOUT_DTR 0
#endif

// PICO_CONFIG: PICO_STDIO_TINYUSB_CDC_SUPPORT_CHARS_AVAILABLE_CALLBACK,
// Enable USB STDIO support for stdio_set_chars_available_callback.
// Can be disabled to make use of USB CDC RX callback elsewhere,
// type=bool default=1, group=pico_stdio_usb
#ifndef PICO_STDIO_TINYUSB_CDC_SUPPORT_CHARS_AVAILABLE_CALLBACK
#define PICO_STDIO_TINYUSB_CDC_SUPPORT_CHARS_AVAILABLE_CALLBACK 1
#endif

// PICO_CONFIG: PICO_STDIO_TINYUSB_CDC_STDOUT_TIMEOUT_US,
// Number of microseconds to be blocked trying to write USB output before assuming
// the host has disappeared and discarding data, default=500000, group=pico_stdio_usb
#ifndef PICO_STDIO_TINYUSB_CDC_STDOUT_TIMEOUT_US
#define PICO_STDIO_TINYUSB_CDC_STDOUT_TIMEOUT_US 500000
#endif

extern stdio_driver_t stdio_tinyusb_cdc;

/*! \brief Explicitly initialize stdin/stdout over TinyUSB CDC and add it to the current set of stdin/stdout drivers
 *  \ingroup stdio_tinyusb_cdc
 *
 * This method sets up TinyUSB CDC with TFF od 0 for input and output (if defined).
 *
 *  \ref PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS can be set to cause this method to wait for a TinyUSB CDC connection
 *  from the host before returning, which is useful if you don't want any initial stdout output to be discarded
 *  before the connection is established.
 *
 *  \return true if the USB CDC was initialized, false if an error occurred
 */
bool stdio_tinyusb_cdc_init(void);

/*! \brief Perform custom initialization of TinyUSB CDC stdio and add it to the current set of stdin/stdout drivers
 *  \ingroup stdio_tinyusb_cdc
 *
 * \param itf the ITF of the CDC device to use
 * \param timeout_ms the number of milliseconds to wait for CDC connection, negative means no timeout
 *
 *  \return true if the USB CDC was initialized, false if an error occurred
 */
bool stdio_tinyusb_cdc_init_full(uint8_t itf, int32_t connect_timeout_ms);

/*! \brief Check if there is an active stdio CDC connection to a host
 *  \ingroup stdio_tinyusb_cdc
 *
 *  \return true if stdio is connected over CDC
 */
bool stdio_tinyusb_cdc_connected(void);

#ifdef __cplusplus
}
#endif
