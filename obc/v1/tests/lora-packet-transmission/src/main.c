#include "pico/stdlib.h"
#include "rp2x_rfm69.h"
#include <hardware/gpio.h>
#include <hardware/spi.h>

#define SPI_INST spi0
#define PIN_MISO (16)
#define PIN_CS (17)
#define PIN_SCK (18)
#define PIN_MOSI (19)
#define PIN_RST (15)
#define LED_PIN 25

int main() {
    stdio_init_all();

    sleep_ms(2000);

    spi_init(SPI_INST, 1000 * 1000);
    spi_set_format(SPI_INST, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);

    gpio_put(PIN_RST, 0);
    sleep_ms(10);
    gpio_put(PIN_RST, 1);
    sleep_ms(10);

    rfm69_context_t rfm;
    struct rfm69_config_s config = {
        .spi = SPI_INST, .pin_cs = PIN_CS, .pin_rst = PIN_RST};

    gpio_put(LED_PIN, 1);

    if (!rfm69_init(&rfm, &config)) {
        gpio_put(LED_PIN, 0);
        printf("error initializing rfm69 module over SPI");
        return 1;
    }

    rfm69_bitrate_set(&rfm, RFM69_MODEM_BITRATE_57_6);
    rfm69_fdev_set(&rfm, 70000);
    rfm69_rxbw_set(&rfm, RFM69_RXBW_MANTISSA_20, 2);
    rfm69_dcfree_set(&rfm, RFM69_DCFREE_WHITENING);

    const uint8_t tx_address = 0x00;
    rfm69_node_address_set(&rfm, tx_address);

    const uint8_t rx_address = 0x01;

    rfm69_packet_format_set(&rfm, RFM69_PACKET_VARIABLE);

    uint8_t buffer[8] = {
        (sizeof buffer), rx_address, 'H', 'e', 'l', 'l', 'o', '!'};

    rfm69_mode_set(&rfm, RFM69_OP_MODE_SLEEP);

    uint sent = 0;
    bool led_on = true;

    while (true) {
        led_on = !led_on;
        gpio_put(LED_PIN, led_on);

        rfm69_write(&rfm, RFM69_REG_FIFO, buffer, (sizeof buffer));

        rfm69_mode_set(&rfm, RFM69_OP_MODE_TX);

        bool state = false;

        while (!state) {
            rfm69_irq2_flag_state(&rfm, RFM69_IRQ2_FLAG_PACKET_SENT, &state);
        }

        printf("%u: %u bytes sent!\n", ++sent, (sizeof buffer));

        rfm69_mode_set(&rfm, RFM69_OP_MODE_SLEEP);
        sleep_ms(1000);
    }

    return 0;
}
