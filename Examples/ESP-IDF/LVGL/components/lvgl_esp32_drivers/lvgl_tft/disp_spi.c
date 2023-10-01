/**
 * @file disp_spi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

#define TAG "disp_spi"

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "lvgl/lvgl.h"

#include "disp_spi.h"
#include "disp_driver.h"

#include "../lvgl_helpers.h"
#include "../lvgl_spi_conf.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void IRAM_ATTR spi_ready (spi_transaction_t *trans);

/**********************
 *  STATIC VARIABLES
 **********************/
static spi_host_device_t spi_host;
static spi_device_handle_t spi;
static volatile uint8_t spi_pending_trans = 0;
static transaction_cb_t chained_post_cb;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void disp_spi_add_device_config(spi_host_device_t host, spi_device_interface_config_t *devcfg)
{
    spi_host=host;
    chained_post_cb=devcfg->post_cb;
    devcfg->post_cb=spi_ready;
    esp_err_t ret=spi_bus_add_device(host, devcfg, &spi);
    assert(ret==ESP_OK);
}

void disp_spi_add_device(spi_host_device_t host)
{
    disp_spi_add_device_with_speed(host, SPI_TFT_CLOCK_SPEED_HZ);
}

void disp_spi_add_device_with_speed(spi_host_device_t host, int clock_speed_hz)
{
	printf("%s->Adding SPI device\n",TAG);
	printf("%s->Clock speed: %dHz, mode: %d, CS pin: %d\n",TAG,clock_speed_hz, SPI_TFT_SPI_MODE, DISP_SPI_CS);
	spi_device_interface_config_t devcfg={
		.clock_speed_hz = clock_speed_hz,
		.mode = SPI_TFT_SPI_MODE,
		.spics_io_num=DISP_SPI_CS,              // CS pin
		.input_delay_ns=DISP_SPI_INPUT_DELAY_NS,
		.queue_size=1,
		.pre_cb=NULL,
		.post_cb=NULL,
		.flags = SPI_DEVICE_NO_DUMMY | SPI_DEVICE_HALFDUPLEX,
	};
	disp_spi_add_device_config(host, &devcfg);
}

void disp_spi_change_device_speed(int clock_speed_hz)
{
    if (clock_speed_hz <= 0) {
        clock_speed_hz = SPI_TFT_CLOCK_SPEED_HZ;
    }
    printf("%s->Changing SPI device clock speed: %d\n",TAG, clock_speed_hz);
    disp_spi_remove_device();
    disp_spi_add_device_with_speed(spi_host, clock_speed_hz);
}

void disp_spi_remove_device()
{
    /* Wait for previous pending transaction results */
    disp_wait_for_pending_transactions();

    esp_err_t ret=spi_bus_remove_device(spi);
    assert(ret==ESP_OK);
}

void disp_spi_transaction(const uint8_t *data, size_t length,
    disp_spi_send_flag_t flags, disp_spi_read_data *out,
    uint64_t addr)
{
    if (0 == length) {
        return;
    }

    /* Wait for previous pending transaction results */
    disp_wait_for_pending_transactions();

    spi_transaction_ext_t t = {0};

    /* transaction length is in bits */
    t.base.length = length * 8;

    if (length <= 4 && data != NULL) {
        t.base.flags = SPI_TRANS_USE_TXDATA;
        memcpy(t.base.tx_data, data, length);
    } else {
        t.base.tx_buffer = data;
    }

    if (flags & DISP_SPI_RECEIVE) {
        assert(out != NULL && (flags & (DISP_SPI_SEND_POLLING | DISP_SPI_SEND_SYNCHRONOUS)));
        t.base.rx_buffer = out;
        t.base.rxlength = 0; /* default, same as tx length */
    }

    if (flags & DISP_SPI_ADDRESS_8) {
        t.address_bits = 8;
    } else if (flags & DISP_SPI_ADDRESS_16) {
        t.address_bits = 16;
    } else if (flags & DISP_SPI_ADDRESS_24) {
        t.address_bits = 24;
    } else if (flags & DISP_SPI_ADDRESS_32) {
        t.address_bits = 32;
    }
    if (t.address_bits) {
        t.base.addr = addr;
        t.base.flags |= SPI_TRANS_VARIABLE_ADDR;
    }

    /* Save flags for pre/post transaction processing */
    t.base.user = (void *) flags;

    /* Poll/Complete/Queue transaction */
    if (flags & DISP_SPI_SEND_POLLING) {
        spi_device_polling_transmit(spi, (spi_transaction_t *) &t);
    } else if (flags & DISP_SPI_SEND_SYNCHRONOUS) {
        spi_device_transmit(spi, (spi_transaction_t *) &t);
    } else {
        static spi_transaction_ext_t queuedt;
        memcpy(&queuedt, &t, sizeof t);
        spi_pending_trans++;
        if (spi_device_queue_trans(spi, (spi_transaction_t *) &queuedt, portMAX_DELAY) != ESP_OK) {
            spi_pending_trans--; /* Clear wait state */
        }
    }
}


void disp_wait_for_pending_transactions(void)
{
    spi_transaction_t *presult;

    while (spi_pending_trans) {
        if (spi_device_get_trans_result(spi, &presult, portMAX_DELAY) == ESP_OK) {
            spi_pending_trans--;
        }
    }
}

void disp_spi_acquire(void)
{
    esp_err_t ret = spi_device_acquire_bus(spi, portMAX_DELAY);
    assert(ret == ESP_OK);
}

void disp_spi_release(void)
{
    spi_device_release_bus(spi);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void IRAM_ATTR spi_ready(spi_transaction_t *trans)
{
    disp_spi_send_flag_t flags = (disp_spi_send_flag_t) trans->user;

    if (flags & DISP_SPI_SIGNAL_FLUSH) {
        lv_disp_t * disp = NULL;

#if (LVGL_VERSION_MAJOR >= 7)
        disp = _lv_refr_get_disp_refreshing();
#else /* Before v7 */
        disp = lv_refr_get_disp_refreshing();
#endif

        lv_disp_flush_ready(&disp->driver);
    }

    if (chained_post_cb) {
        chained_post_cb(trans);
    }
}

