/*
 * Copyright 2019 Eric Tremblay <habscup@gmail.com>
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "cli.h"

#include <app_timer.h>

#include <drivers/cli_uart.h>

#include "ble/abstract_ble_observer.h"
#include "ble/ble_device.h"

APP_TIMER_DEF(m_scan_timer);

typedef struct
{
    uint8_t * p_data;
    uint16_t  data_len;
} data_t;

static bool start_scan = false;
static bool scan_in_progress = false;

/* List of seen devices in the current scan. */
static struct {
    uint8_t addr[BLE_GAP_ADDR_LEN];
} device_list[250];
static uint8_t device_count = 0;

static bool is_in_device_list(const ble_gap_evt_adv_report_t* report)
{
    for (int i = 0; i < device_count; i++) {
        if (!memcmp(report->peer_addr.addr, device_list[i].addr,
                    BLE_GAP_ADDR_LEN)) {
            return true;
        }
    }
    return false;
}

static void add_to_device_list(const ble_gap_evt_adv_report_t* report)
{
    memcpy(device_list[device_count].addr, report->peer_addr.addr,
           BLE_GAP_ADDR_LEN);
    device_count++;
}

static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index + 2];
            p_typedata->data_len = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}

static void on_advertising_report(const ble_gap_evt_adv_report_t* report) {
    if (scan_in_progress) {
        if (!is_in_device_list(report)) {
            data_t adv_data;
            data_t dev_name;
            uint8_t data[32];
            char dev_str[32];

            memcpy(data, report->data, BLE_GAP_ADV_MAX_SIZE);
            adv_data.p_data = data;
            adv_data.data_len = report->dlen;

            if (adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, &adv_data, &dev_name) != NRF_SUCCESS) {
                if (adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, &adv_data, &dev_name) != NRF_SUCCESS) {
                    return;
                }
            }

            cli_printf("%02X:%02X:%02X:%02X:%02X:%02X ", report->peer_addr.addr[5],
                       report->peer_addr.addr[4], report->peer_addr.addr[3],
                       report->peer_addr.addr[2], report->peer_addr.addr[1],
                       report->peer_addr.addr[0]);

            strncpy(dev_str, (char*)dev_name.p_data, dev_name.data_len);
            dev_str[dev_name.data_len] = '\0';

            cli_printf("%s ", dev_str);

            cli_printf("%d\r\n", report->rssi);
            add_to_device_list(report);
        }
    }
}

static void on_scan_timeout(const ble_gap_evt_timeout_t* timeout_event){
}

static struct BleObserver ble_scanner = {
    &on_advertising_report,
    &on_scan_timeout,
};

static void scan_timeout_handler(void *p_context) {
    cli_printf("Scan completed\r\n");
    scan_in_progress = false;
}

static void do_scan(const nrf_cli_t *p_cli, size_t argc, char **argv)
{
    static bool is_observer_added = false;
    if (!standard_check(p_cli, argc, 1, argv, NULL, 0)) {
        return;
    }

    if (!is_observer_added) {
        add_observer(&ble_scanner);
        is_observer_added = true;

        ret_code_t err_code = app_timer_create(
            &m_scan_timer, APP_TIMER_MODE_SINGLE_SHOT, scan_timeout_handler);
        APP_ERROR_CHECK(err_code);
    }

    if (!scan_in_progress) {
        nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT,
                        "Start scanning for nearby devices for 30 seconds\r\n");
        memset(device_list, 0, sizeof(device_list));
        device_count = 0;
        ret_code_t err_code =
            app_timer_start(m_scan_timer, APP_TIMER_TICKS(30000), NULL);
        APP_ERROR_CHECK(err_code);
        scan_in_progress = true;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "Scan already in progress\r\n");
    }
}

static void do_ble(const nrf_cli_t *p_cli, size_t argc, char **argv)
{
    if (!standard_check(p_cli, argc, 2, argv, NULL, 0)) {
        return;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\r\n",
                    argv[0], argv[1]);
}

NRF_CLI_CREATE_STATIC_SUBCMD_SET(sub_ble){
    NRF_CLI_CMD(scan, NULL, "Scan BLE devices (Maximum 250 devices)", do_scan),
    NRF_CLI_SUBCMD_SET_END};

NRF_CLI_CMD_REGISTER(blectl, &sub_ble, "BLE Tools collection", do_ble);