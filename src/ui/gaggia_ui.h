/**
 * @file demo.h
 *
 */

#ifndef DEMO_H
#define DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>

enum ui_element_types {
    BREWTEMP_SPIN,
    BREWTEMP_PLUS_BUTTON,
    BREWTEMP_MIN_BUTTON,
    STEAMTEMP_SPIN,
    STEAMTEMP_PLUS_BUTTON,
    STEAMTEMP_MIN_BUTTON,
    STOP_BUTTON,
    BREW_TEMP_OBJ,
    STEAM_TEMP_OBJ,
    BREW_TEMP_LABEL,
    STEAM_TEMP_LABEL,
    PROCESS_MESSAGE_CONTAINER,
    PROCESS_MESSAGE_BUTTON,
    PROCESS_MESSAGE_LABEL,
    PROCESS_MESSAGE_BUTTON_LABEL,
    PROCESS_MESSAGE_TITLE,
    PROCESS_SELECT_MATRIX,
    TAB_VIEW,
    TIMER_LABEL,
    TIMER_BOX,
#if defined (GUI_IO)
    HEAT_STATUS_SSR,
    VALVE_STATUS_SSR,
    PUMP_STATUS_SSR,
    BREW_BUT_STATUS,
    STEAM_BUT_STATUS,
#endif
    _LAST_ITEM_STUB
};

enum ui_event {
    EV_CLICK,
    EV_CHANGE,
    _NA
};

typedef void (*gaggia_ui_event)(enum ui_element_types label, enum ui_event event);

/**
 * Create a Gaggia UI application
 */
void gaggia_ui_create_ui(void);

/**
 * functions to interact with the UI
 */
void gaggia_ui_set_text_hint(enum ui_element_types label, const char* value, size_t sizeHint);
void gaggia_ui_set_text(enum ui_element_types label, const char* value);
char* gaggia_ui_set_text_buffer(enum ui_element_types label);
void gaggia_ui_set_visibility(enum ui_element_types label, bool en);
void gaggia_ui_add_event_cb(enum ui_element_types label, gaggia_ui_event);
void gaggia_ui_set_led(enum ui_element_types label, bool status);
void gaggia_ui_set_led_bright(enum ui_element_types label, uint8_t status);
void gaggia_ui_spin_set_range(enum ui_element_types label, int32_t min, int32_t max);
void gaggia_ui_spin_set_value(enum ui_element_types label, int32_t value);
int32_t gaggia_ui_spin_get_value(enum ui_element_types label);
void gaggia_ui_btn_map(enum ui_element_types label,  const char* btnm_map[]);
uint16_t gaggia_ui_btn_map_active(enum ui_element_types label);
void gaggia_ui_change_screen(uint8_t screen);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
