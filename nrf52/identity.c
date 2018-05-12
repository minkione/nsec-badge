//  Copyright (c) 2017
//  Benjamin Vanheuverzwijn <bvanheu@gmail.com>
//  Marc-Etienne M. Leveille <marc.etienne.ml@gmail.com>
//
//  License: MIT (see LICENSE for details)

#include "identity.h"
#include <stdint.h>
#include <stdio.h>
#include <nrf52.h>
#include <app_timer.h>
#include "ble/nsec_ble.h"
#include "ssd1306.h"
#include "nsec_nearby_badges.h"
#include "app_glue.h"
#include "3d.h"

#include "ble/ble_device.h"
#include "ble/service_characteristic.h"
#include "ble/vendor_service.h"
#include "images/default_avatar_bitmap.c"
#include "images/nsec_logo_tiny_bitmap.c"
#include "images/star_bitmap.c"
#include "logs.h"
#include "ble/gap_configuration.h"


#define AVATAR_SIZE \
    ((NSEC_IDENTITY_AVATAR_WIDTH * NSEC_IDENTITY_AVATAR_HEIGHT + 1) / 8)

#define M_PI            3.14159265358979323846
#define UNLOCK_PASSWORD_SIZE 4

static uint16_t on_name_write(CharacteristicWriteEvent* event);
static uint16_t on_avatar_write(CharacteristicWriteEvent* event);
static uint16_t on_unlock_password_write(CharacteristicWriteEvent* event);

static void configure_service();


typedef struct {
    char name[16];
    uint8_t unlocked;
    uint8_t avatar[AVATAR_SIZE];
} BadgeIdentity;

static BadgeIdentity identity;
static VendorService identity_ble_service;
static ServiceCharacteristic name_characteristic;
static ServiceCharacteristic unlock_password_characteristic;
static ServiceCharacteristic unlocked_characteristic;
static ServiceCharacteristic avatar_characteristic;


#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifdef NSEC_SHOW_FLAG_ON_SUCCESSFUL_NAME_CHANGE
static char flag[] = "flag{ble_all_the_things}";
#endif

APP_TIMER_DEF(nsec_render_timer);


static void nsec_render_3d_mesh(void * context) {
    if(is_at_main_menu) {
        static float current_angle = 0.0f;
        gfx_fillRect(0, 10, 40, 42, SSD1306_BLACK);
        nsec_draw_rotated_mesh(nsec_cube, (int[2]) {20, 32}, 11,
                               (float[3]) {current_angle,
                                           current_angle + 1.0f,
                                           current_angle + 2.0f});
        current_angle += 0.03f;
        if(current_angle >= 2 * M_PI) {
            current_angle -= 2 * M_PI;
        }
        gfx_update();
    }
}

void init_identity_service() {
    memset(identity.name, 0, sizeof(identity.name));

    app_timer_create(&nsec_render_timer, APP_TIMER_MODE_REPEATED, nsec_render_3d_mesh);
    app_timer_start(nsec_render_timer, APP_TIMER_TICKS(40), NULL);

#if defined(NSEC_HARDCODED_BADGE_IDENTITY_NAME)
#define NSEC_STRINGIFY_(...) #__VA_ARGS__
#define NSEC_STRINGIFY(...) NSEC_STRINGIFY_(__VA_ARGS__)
    snprintf(identity.name, sizeof(identity.name), NSEC_STRINGIFY(NSEC_HARDCODED_BADGE_IDENTITY_NAME));
#else
    snprintf(identity.name, sizeof(identity.name), "Comrade #%05ld", (NRF_FICR->DEVICEID[0] & 0xFFFF));
#endif
    memcpy(identity.avatar, default_avatar_bitmap, sizeof(identity.avatar));
    identity.unlocked = 0;
    configure_service();
    set_default_advertised_service(&identity_ble_service);
}

static void nsec_draw_empty_progress_bar(uint16_t x, uint16_t y, uint16_t w) {
    gfx_drawRect(x, y, w, 3, SSD1306_BLACK);
    gfx_drawFastHLine(x + 1, y    , w - 2, SSD1306_WHITE);
    gfx_drawFastHLine(x + 1, y + 2, w - 2, SSD1306_WHITE);
    gfx_drawFastVLine(x,         y + 1, 1, SSD1306_WHITE);
    gfx_drawFastVLine(x + w - 1, y + 1, 1, SSD1306_WHITE);
}

void nsec_identity_draw(void) {
    gfx_fillRect(8, 8, 128-16, 64-16, SSD1306_BLACK);
    gfx_drawBitmapBg(8, 16, identity.avatar, NSEC_IDENTITY_AVATAR_WIDTH, NSEC_IDENTITY_AVATAR_HEIGHT, SSD1306_WHITE, SSD1306_BLACK);
    gfx_setCursor(16+16+8, 20);
    char name_with_spaces[sizeof(identity.name)];
    snprintf(name_with_spaces, sizeof(name_with_spaces), "%-14s", identity.name);
    gfx_puts(name_with_spaces);
    gfx_drawBitmap(59, 32, star_bitmap, star_bitmap_width, star_bitmap_height, SSD1306_WHITE);
    nsec_draw_empty_progress_bar(70, 38, 30);
    gfx_drawBitmapBg(111, 47, nsec_logo_tiny_bitmap, nsec_logo_tiny_bitmap_width, nsec_logo_tiny_bitmap_height, SSD1306_WHITE, SSD1306_BLACK);
    nsec_identity_update_nearby();
}

void nsec_identity_update_nearby(void) {
    int16_t bar_width = (28 * nsec_nearby_badges_current_count()) / (NSEC_MAX_NEARBY_BADGES_COUNT);
    gfx_drawFastHLine(71, 39, 28, SSD1306_BLACK);
    gfx_drawFastHLine(71, 39, bar_width, SSD1306_WHITE);
}

void nsec_identity_get_unlock_key(char * data, size_t length) {
    snprintf(data, length, "%04lX", ((NRF_FICR->DEVICEID[1] % 0xFFFF) ^ 0xC3C3));
}

static uint16_t on_name_write(CharacteristicWriteEvent* event){
	if(identity.unlocked) {
		memset(identity.name, 0, sizeof(identity.name));
		strncpy(identity.name, (char *) event->data_buffer, MIN(event->data_length, sizeof(identity.name)));
		if(is_at_main_menu) {
			nsec_identity_draw();
			gfx_update();
		}
		event->data_length = sizeof(identity.name);
		event->data_buffer = (uint8_t*)identity.name;
		return BLE_GATT_STATUS_SUCCESS;
	}
	else{
		return BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED;
	}
}

static uint16_t on_avatar_write(CharacteristicWriteEvent* event){
	NRF_LOG_INFO("received %d bytes", event->data_length);
	if(identity.unlocked && event->data_length == AVATAR_SIZE) {
		memcpy(identity.avatar, event->data_buffer, AVATAR_SIZE);
		if(is_at_main_menu) {
				nsec_identity_draw();
				gfx_update();
		}
		return BLE_GATT_STATUS_SUCCESS;
	}
	return BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED;
}

static uint16_t on_unlock_password_write(CharacteristicWriteEvent* event){
	char unlock_password[UNLOCK_PASSWORD_SIZE];
	nsec_identity_get_unlock_key(unlock_password, UNLOCK_PASSWORD_SIZE);
	if(identity.unlocked){
		return BLE_GATT_STATUS_SUCCESS;
	}
	else if(event->data_length == UNLOCK_PASSWORD_SIZE && memcmp(unlock_password, (const char*)event->data_buffer, UNLOCK_PASSWORD_SIZE) == 0){
		identity.unlocked = 1;
		set_characteristic_value(&unlocked_characteristic, &identity.unlocked);
		return BLE_GATT_STATUS_SUCCESS;
	}
	return BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED;
}

static void configure_service(){
	create_vendor_service(&identity_ble_service);
	add_vendor_service(&identity_ble_service);

	add_characteristic_to_vendor_service(&identity_ble_service, &name_characteristic, sizeof(identity.name), AUTO_READ, REQUEST_WRITE);
	add_write_request_handler(&name_characteristic, on_name_write);
	set_characteristic_value(&name_characteristic, (uint8_t*)identity.name);

	add_characteristic_to_vendor_service(&identity_ble_service, &avatar_characteristic, sizeof(identity.avatar), AUTO_READ, REQUEST_WRITE);
	add_write_request_handler(&avatar_characteristic, on_avatar_write);
	set_characteristic_value(&avatar_characteristic, identity.avatar);

	add_characteristic_to_vendor_service(&identity_ble_service, &unlock_password_characteristic, UNLOCK_PASSWORD_SIZE, DENY_READ, REQUEST_WRITE);
	add_write_request_handler(&unlock_password_characteristic, on_unlock_password_write);

	add_characteristic_to_vendor_service(&identity_ble_service, &unlocked_characteristic, sizeof(identity.unlocked), AUTO_READ, DENY_WRITE);
	set_characteristic_value(&unlocked_characteristic, &identity.unlocked);
}
