#include <app_timer.h>

#include "drivers/controls.h"
#include "drivers/display.h"

#include "application.h"
#include "gfx_effect.h"

#include "images/external/snake_pattern_scale_bitmap.h"
#include "images/external/snake_splash_10_bitmap.h"
#include "images/external/snake_splash_11_bitmap.h"
#include "images/external/snake_splash_12_bitmap.h"
#include "images/external/snake_splash_13_bitmap.h"
#include "images/external/snake_splash_14_bitmap.h"
#include "images/external/snake_splash_15_bitmap.h"
#include "images/external/snake_splash_1_bitmap.h"
#include "images/external/snake_splash_2_bitmap.h"
#include "images/external/snake_splash_3_bitmap.h"
#include "images/external/snake_splash_4_bitmap.h"
#include "images/external/snake_splash_5_bitmap.h"
#include "images/external/snake_splash_6_bitmap.h"
#include "images/external/snake_splash_7_bitmap.h"
#include "images/external/snake_splash_8_bitmap.h"
#include "images/external/snake_splash_9_bitmap.h"
#include "images/external/snake_splash_bitmap.h"

#define SNAKE_BUTTON_NONE 255

#define SNAKE_CONTENTS_EMPTY -1

#define SNAKE_GOING_LEFT (p_state->position.dx == -1)
#define SNAKE_GOING_RIGHT (p_state->position.dx == 1)
#define SNAKE_GOING_UP (p_state->position.dy == -1)
#define SNAKE_GOING_DOWN (p_state->position.dy == 1)

#define SNAKE_GO_LEFT (p_state->position.dx = -1, p_state->position.dy = 0)
#define SNAKE_GO_RIGHT (p_state->position.dx = 1, p_state->position.dy = 0)
#define SNAKE_GO_UP (p_state->position.dx = 0, p_state->position.dy = -1)
#define SNAKE_GO_DOWN (p_state->position.dx = 0, p_state->position.dy = 1)

#define SNAKE_GRID_CELL(x, y) (y * SNAKE_GRID_WIDTH + x)
#define SNAKE_GRID_CELL_X(z) (z % SNAKE_GRID_WIDTH)
#define SNAKE_GRID_CELL_Y(z) (z / SNAKE_GRID_WIDTH)
#define SNAKE_GRID_HEIGHT 13
#define SNAKE_GRID_PX_BORDER 1
#define SNAKE_GRID_PX_CELL 5
#define SNAKE_GRID_PX_OFFSET_X 1
#define SNAKE_GRID_PX_OFFSET_Y 1
#define SNAKE_GRID_WIDTH 24

#define SNAKE_PATTERN_EMPTY 0
#define SNAKE_PATTERN_SCALE 1

APP_TIMER_DEF(snake_game_timer);

typedef struct SnakeGamePositionState {
    int8_t dx;
    int8_t dy;
    uint8_t growth;
    uint8_t head_x;
    uint8_t head_y;
    uint8_t prev_x;
    uint8_t prev_y;
    uint8_t shrink_delay;
    uint8_t tail_x;
    uint8_t tail_y;
} SnakeGamePositionState;

typedef struct SnakeGameState {
    SnakeGamePositionState position;

    bool delay;
    int16_t grid[SNAKE_GRID_WIDTH * SNAKE_GRID_HEIGHT];
} SnakeGameState;

static const struct bitmap_ext *splash_animation[] = {
    &snake_splash_1_bitmap,  &snake_splash_2_bitmap,  &snake_splash_3_bitmap,
    &snake_splash_4_bitmap,  &snake_splash_5_bitmap,  &snake_splash_6_bitmap,
    &snake_splash_7_bitmap,  &snake_splash_8_bitmap,  &snake_splash_9_bitmap,
    &snake_splash_10_bitmap, &snake_splash_11_bitmap, &snake_splash_12_bitmap,
    &snake_splash_13_bitmap, &snake_splash_14_bitmap, &snake_splash_15_bitmap,
};

static uint8_t snake_button_read_value = SNAKE_BUTTON_NONE;

static void snake_buttons_handle(button_t button)
{
    switch (button) {
    case BUTTON_BACK:
    case BUTTON_DOWN:
    case BUTTON_ENTER:
    case BUTTON_UP:
        snake_button_read_value = button;
        break;

    default:
        break;
    }
}

static uint8_t snake_buttons_read()
{
    if (snake_button_read_value != SNAKE_BUTTON_NONE) {
        uint8_t value = snake_button_read_value;
        snake_button_read_value = SNAKE_BUTTON_NONE;

        return value;
    }

    return SNAKE_BUTTON_NONE;
}

static void snake_render_pattern(uint8_t x, uint8_t y, uint8_t pattern)
{
    uint8_t origin_x = x * (SNAKE_GRID_PX_CELL + SNAKE_GRID_PX_BORDER) +
                       SNAKE_GRID_PX_OFFSET_X + SNAKE_GRID_PX_BORDER;

    uint8_t origin_y = y * (SNAKE_GRID_PX_CELL + SNAKE_GRID_PX_BORDER) +
                       SNAKE_GRID_PX_OFFSET_Y + SNAKE_GRID_PX_BORDER;

    switch (pattern) {
    case SNAKE_PATTERN_EMPTY:
        gfx_fill_rect(origin_x, origin_y, SNAKE_GRID_PX_CELL,
                      SNAKE_GRID_PX_CELL, DISPLAY_BLACK);
        break;

    case SNAKE_PATTERN_SCALE:
        display_draw_16bit_ext_bitmap(origin_x, origin_y,
                                      &snake_pattern_scale_bitmap, 0);
        break;
    }
}

static void snake_game_engine_register_position(SnakeGameState *p_state)
{
    SnakeGamePositionState *p_pos = &((*p_state).position);

    p_state->grid[SNAKE_GRID_CELL(p_pos->prev_x, p_pos->prev_y)] =
        SNAKE_GRID_CELL(p_pos->head_x, p_pos->head_y);

    snake_render_pattern(p_pos->head_x, p_pos->head_y, SNAKE_PATTERN_SCALE);

    if (p_pos->shrink_delay > 0) {
        p_pos->growth++;
        p_pos->shrink_delay--;
    } else {
        int16_t next_tail =
            p_state->grid[SNAKE_GRID_CELL(p_pos->tail_x, p_pos->tail_y)];

        p_state->grid[SNAKE_GRID_CELL(p_pos->tail_x, p_pos->tail_y)] =
            SNAKE_CONTENTS_EMPTY;

        snake_render_pattern(p_pos->tail_x, p_pos->tail_y, SNAKE_PATTERN_EMPTY);

        p_pos->tail_x = SNAKE_GRID_CELL_X(next_tail);
        p_pos->tail_y = SNAKE_GRID_CELL_Y(next_tail);
    }
}

static void snake_game_engine_update_position(SnakeGameState *p_state)
{
    switch (snake_buttons_read()) {
    case BUTTON_BACK:
        if (SNAKE_GOING_DOWN || SNAKE_GOING_UP) {
            SNAKE_GO_LEFT;
        }
        break;

    case BUTTON_DOWN:
        if (SNAKE_GOING_LEFT || SNAKE_GOING_RIGHT) {
            SNAKE_GO_DOWN;
        }
        break;

    case BUTTON_ENTER:
        if (SNAKE_GOING_DOWN || SNAKE_GOING_UP) {
            SNAKE_GO_RIGHT;
        }
        break;

    case BUTTON_UP:
        if (SNAKE_GOING_LEFT || SNAKE_GOING_RIGHT) {
            SNAKE_GO_UP;
        }
        break;
    }

    p_state->position.prev_x = p_state->position.head_x;
    p_state->position.prev_y = p_state->position.head_y;

    p_state->position.head_x += p_state->position.dx;
    p_state->position.head_y += p_state->position.dy;

    if (p_state->position.head_x > SNAKE_GRID_WIDTH) {
        p_state->position.head_x = SNAKE_GRID_WIDTH - 1;
    } else if (p_state->position.head_x == SNAKE_GRID_WIDTH) {
        p_state->position.head_x = 0;
    }

    if (p_state->position.head_y > SNAKE_GRID_HEIGHT) {
        p_state->position.head_y = SNAKE_GRID_HEIGHT - 1;
    } else if (p_state->position.head_y == SNAKE_GRID_HEIGHT) {
        p_state->position.head_y = 0;
    }
}

static void snake_boot_sequence(void (*service_device)())
{
    service_device();
    display_draw_16bit_ext_bitmap(0, 0, &snake_splash_bitmap, 0);

    snake_buttons_read();
    do {
        service_device();
    } while (snake_buttons_read() == SNAKE_BUTTON_NONE);

    for (uint8_t i = 0; i < 15; i++) {
        service_device();
        display_draw_16bit_ext_bitmap(40, 0, splash_animation[i], 0);
    }

    do {
        service_device();
    } while (snake_buttons_read() == SNAKE_BUTTON_NONE);

    gfx_fill_rect(0, 0, DISPLAY_HEIGHT, DISPLAY_WIDTH, DISPLAY_BLACK);
    gfx_update();
}

static void snake_initial_snake(SnakeGameState *p_state)
{
    uint8_t initial_length = 3;

    p_state->position = (SnakeGamePositionState){
        .dx = 1,
        .dy = 0,
        .growth = 0,
        .head_x = 0,
        .head_y = 0,
        .prev_x = 0,
        .prev_y = 0,
        .shrink_delay = 0,
        .tail_x = 0,
        .tail_y = 0,
    };

    for (uint16_t i = 0; i < SNAKE_GRID_WIDTH * SNAKE_GRID_HEIGHT; i++) {
        p_state->grid[i] = SNAKE_CONTENTS_EMPTY;
    }

    SnakeGamePositionState *p_pos = &((*p_state).position);

    p_pos->head_x = p_pos->tail_x = (SNAKE_GRID_WIDTH / 2) - initial_length;
    p_pos->head_y = p_pos->tail_y = SNAKE_GRID_HEIGHT / 2;

    snake_render_pattern(p_pos->head_x, p_pos->head_y, SNAKE_PATTERN_SCALE);
    for (uint8_t i = 1; i < initial_length; i++) {
        p_pos->head_x++;

        p_state->grid[SNAKE_GRID_CELL(p_pos->head_x - 1, p_pos->head_y)] =
            SNAKE_GRID_CELL(p_pos->head_x, p_pos->head_y);

        snake_render_pattern(p_pos->head_x, p_pos->head_y, SNAKE_PATTERN_SCALE);
    }
}

static void snake_game_loop(SnakeGameState *p_state)
{
    if (p_state->delay) {
        return;
    } else {
        p_state->delay = true;
    }

    snake_game_engine_update_position(p_state);
    snake_game_engine_register_position(p_state);
}

static void snake_game_timer_callback(void *p_context)
{
    SnakeGameState *p_state = (SnakeGameState *)p_context;

    p_state->delay = false;
}

void snake_application(void (*service_device)())
{
    SnakeGameState state = {.delay = true};

    nsec_controls_add_handler(snake_buttons_handle);

    snake_boot_sequence(service_device);
    snake_initial_snake(&state);

    app_timer_create(&snake_game_timer, APP_TIMER_MODE_REPEATED,
                     snake_game_timer_callback);
    app_timer_start(snake_game_timer, APP_TIMER_TICKS(150), &state);

    while (application_get() == snake_application) {
        snake_game_loop(&state);
        service_device();
    }

    ret_code_t err_code = app_timer_stop(snake_game_timer);
    APP_ERROR_CHECK(err_code);

    nsec_controls_suspend_handler(snake_buttons_handle);
}
