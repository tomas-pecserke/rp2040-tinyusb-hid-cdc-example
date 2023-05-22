#include <ctype.h>
#include <tusb.h>

#define PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE 1

#include "stdio_tinyusb_cdc.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "pico/stdio_uart.h"
#include "pico/stdio_usb.h"

// Prototypes
static void cdc_task(void);
static void uart_task(void);

// Main

int main() {
    //stdio_init_all();
    stdio_uart_init();

    // init device stack on configured root-hub port
    tud_init(BOARD_TUD_RHPORT);
    //stdio_tinyusb_cdc_init();
    stdio_usb_init();

    printf("Start\n");

    while (1) {
        tud_task(); // tinyusb device task
        cdc_task();
        uart_task();
    }
}

// echo to either Serial0 or Serial1
// with Serial0 as all lower case, Serial1 as all upper case
static void echo_serial_port(uint8_t itf, uint8_t buf[], uint32_t count) {
    uint8_t const case_diff = 'a' - 'A';

    for (uint32_t i = 0; i < count; i++) {
        if (itf == 0) {
            // echo back 1st port as lower case
            if (isupper(buf[i])) buf[i] += case_diff;
        } else {
            // echo back 2nd port as upper case
            if (islower(buf[i])) buf[i] -= case_diff;
        }

        tud_cdc_n_write_char(itf, buf[i]);
    }
    tud_cdc_n_write_flush(itf);
}

// USB CDC

static void cdc_task(void) {
    uint8_t itf;

    for (itf = 1; itf < CFG_TUD_CDC; itf++) {
        // connected() check for DTR bit
        // Most but not all terminal client set this when making connection
        // if ( tud_cdc_n_connected(itf) )
        {
            if (tud_cdc_n_available(itf)) {
                uint8_t buf[64];

                uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

                // echo back to both serial ports
                echo_serial_port(0, buf, count);
                echo_serial_port(1, buf, count);
            }
        }
    }
}


static void uart_task(void) {
    char input = getchar_timeout_us(0);
    switch (input) {
        case 'r':
        case 'R':
            printf("UART: Restart\n");
            watchdog_enable(100, false);
            break;
        case 'b':
        case 'B':
            printf("UART: Bootsel mode\n");
            reset_usb_boot(0, 0);
            break;
        case 'p':
        case 'P':
            printf("UART: Hello, world!\n");
            break;
        default:
            // Ignore
            break;
    }
}
