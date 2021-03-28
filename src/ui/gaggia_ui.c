/**
 * @file demo.c
 *
 */

#include "gaggia_ui.h"
//#include <esp_log.h>
#include "lv_theme_gaggia.h"
#include <stdio.h>
#include <stdlib.h>

/**********************
 *  STATIC VARIABLES
 **********************/

LV_IMG_DECLARE(coffee)
//LV_IMG_DECLARE(airplane)

static lv_style_t style_lbl_large;
static lv_style_t style_default;
static lv_style_t style_tab;
static lv_style_t style_box_opa00;
static lv_style_t style_btn_stop;
static lv_style_t style_bg_test;

#define OFFSET PAD_DEF

/**********************
 *   EVENT FUNCTIONS
 **********************/

typedef struct {
    lv_obj_t* element;
    gaggia_ui_event event_cb;
} ui_element;

typedef struct {
    char* text;
    size_t length;
} text_element;

// Doing it like this uses perhaps a bit to much memory, but we don't have a lot of textable items anyways...
static ui_element ui_elements[_LAST_ITEM_STUB];
static text_element text_elements[_LAST_ITEM_STUB];

static void generic_event_handler(lv_obj_t* obj, lv_event_t event) {
    uint32_t id = lv_obj_get_user_data(obj);

    if (event == LV_EVENT_CLICKED) {
        if (ui_elements[id].event_cb == NULL) {
            return;
        }

        ui_elements[id].event_cb(id, EV_CLICK);
    } else if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {

        // Used internall for the spinners
        if (id == STEAMTEMP_PLUS_BUTTON) {
            lv_spinbox_increment(ui_elements[STEAMTEMP_SPIN].element);
        } else if (id == STEAMTEMP_MIN_BUTTON) {
            lv_spinbox_decrement(ui_elements[STEAMTEMP_SPIN].element);
        } else if (id == BREWTEMP_PLUS_BUTTON) {
            lv_spinbox_increment(ui_elements[BREWTEMP_SPIN].element);
        } else if (id == BREWTEMP_MIN_BUTTON) {
            lv_spinbox_decrement(ui_elements[BREWTEMP_SPIN].element);
        }

    } else if (event == LV_EVENT_VALUE_CHANGED) {
        if (ui_elements[id].event_cb == NULL) {
            return;
        }


        // if (id == PROCESS_SELECT_MATRIX) {
        //     const char* txt = lv_btnmatrix_get_active_btn_text(obj);
        //     uint16_t id = lv_btnmatrix_get_active_btn_text(obj);
        //     printf("%s was pressed\n", txt);
        // }
    }

    // Call generic handler
    if (event == LV_EVENT_CLICKED) {
        if (ui_elements[GENERIC_UI_INTERACTION].event_cb != NULL) {
            ui_elements[GENERIC_UI_INTERACTION].event_cb(GENERIC_UI_INTERACTION, _NA);
        }
    }

}



/**
 * Create Gagia application
 */
static void gaggia_style() {

    lv_theme_t* th = lv_theme_gaggia_init(
                         LV_COLOR_ORANGE,
                         LV_COLOR_BLACK,
                         LV_THEME_GAGGIA_FLAG_DARK,
                         &lv_font_montserrat_18, &lv_font_montserrat_18,
                         &lv_font_montserrat_18, &lv_font_montserrat_18);
    lv_theme_set_act(th);

    // STyle TAB
    lv_style_init(&style_tab);
    lv_style_set_bg_opa(&style_tab, LV_STATE_DEFAULT, LV_OPA_50);

    lv_style_init(&style_bg_test);

    // Style BG
    static lv_style_t style;
    lv_style_init(&style);

    // Style box
    lv_style_init(&style_box_opa00);
    lv_style_set_bg_opa(&style_box_opa00, LV_STATE_DEFAULT, LV_OPA_0);

    // Normal Font
    lv_style_init(&style_default);

    // Large Font
    lv_style_init(&style_lbl_large);
    lv_style_set_text_font(&style_lbl_large, LV_STATE_DEFAULT, &lv_font_montserrat_42);

    // Stop button
    lv_style_init(&style_btn_stop);
    lv_style_set_bg_color(&style_btn_stop, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xc4, 0x30, 0x20));
    //lv_style_set_border_pad(&style_btn_stop, LV_STATE_DEFAULT, 2);
    lv_style_set_border_width(&style_btn_stop, LV_STATE_DEFAULT, 2);
    lv_style_set_border_color(&style_btn_stop, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xc4, 0x30, 0x20));
}

static lv_obj_t* gaggia_gauge(lv_obj_t* parent, lv_obj_t** retValueBox, lv_obj_t** retValueLabel, lv_obj_t** retLabel) {
    lv_obj_t* box = lv_obj_create(parent, NULL);
    lv_obj_set_size(box, LV_DPX(90), LV_DPX(70));
    lv_obj_add_style(box, LV_LABEL_PART_MAIN, &style_box_opa00);

    // Label above number
    lv_obj_t* label = lv_label_create(box, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label, box, LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_obj_set_width_fit(label, lv_obj_get_width_fit(box));

    // Number
    lv_obj_t* valueLabel = lv_label_create(box, label);
    lv_obj_add_style(valueLabel, LV_LABEL_PART_MAIN, &style_lbl_large);
    lv_obj_align(valueLabel, box, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    *retValueBox = box;
    *retValueLabel = valueLabel;
    *retLabel = label;

    return box;
}

static lv_obj_t* gaggia_timer(lv_obj_t* parent) {
    ui_elements[TIMER_BOX].element = lv_obj_create(parent, NULL);
    lv_obj_set_size(ui_elements[TIMER_BOX].element, LV_DPX(140), LV_DPX(70));
    lv_obj_add_style(ui_elements[TIMER_BOX].element, LV_LABEL_PART_MAIN, &style_box_opa00);

    // Number
    ui_elements[TIMER_LABEL].element = lv_label_create(ui_elements[TIMER_BOX].element, NULL);
    lv_obj_add_style(ui_elements[TIMER_LABEL].element, LV_LABEL_PART_MAIN, &style_lbl_large);

    lv_obj_align(ui_elements[TIMER_LABEL].element, ui_elements[TIMER_BOX].element, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_align(ui_elements[TIMER_LABEL].element, LV_LABEL_ALIGN_RIGHT);
    lv_obj_set_width_fit(ui_elements[TIMER_LABEL].element, lv_obj_get_width_fit(ui_elements[TIMER_BOX].element));

#if defined (BUILD_NATIVE)
    lv_label_set_text(ui_elements[TIMER_LABEL].element, "12.6");
#else
    gaggia_ui_set_visibility(TIMER_BOX, false);
#endif

    return ui_elements[TIMER_BOX].element ;
}

static lv_obj_t* spin_create(lv_obj_t* parent,
                             lv_obj_t** retSpinner,
                             lv_obj_t** plusButton,
                             lv_obj_t** minButton,
                             char* labelTxt,
                             int min,
                             int max) {
    lv_obj_t* box = lv_obj_create(parent, NULL);
    //lv_obj_add_style(box, LV_STATE_DEFAULT, &style_box_opa00);
    lv_obj_set_width_fit(box, lv_obj_get_width_fit(parent));

    // Spinner
    lv_obj_t* spinbox = lv_spinbox_create(box, NULL);
    lv_spinbox_set_range(spinbox, min, max);
    lv_spinbox_set_digit_format(spinbox, 3, 0);
    lv_spinbox_step_prev(spinbox);
    lv_obj_set_width(spinbox, 46);
    //lv_spinbox_set_step(spinbox, 1);
    lv_coord_t h = lv_obj_get_height(spinbox);

    lv_obj_align(spinbox, box, LV_ALIGN_IN_RIGHT_MID, -40, 0);

    // + button
    lv_obj_t* plusBtn = lv_btn_create(box, NULL);
    lv_obj_set_size(plusBtn, h, h);
    lv_obj_align(plusBtn, spinbox, LV_ALIGN_OUT_RIGHT_MID, 0000, 0);
    lv_theme_apply(plusBtn, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(plusBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);

    // - button
    lv_obj_t* minBtn  = lv_btn_create(box, plusBtn);
    lv_obj_align(minBtn, spinbox, LV_ALIGN_OUT_LEFT_MID, -0000, 0);
    lv_obj_set_style_local_value_str(minBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_MINUS);

    // Label
    lv_obj_t* label = lv_label_create(box, NULL);
    lv_label_set_text(label, labelTxt);
    lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
    lv_obj_align(label, box, LV_ALIGN_IN_LEFT_MID, OFFSET, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);

    *retSpinner = spinbox;
    *minButton = minBtn;
    *plusButton = plusBtn;
    return box;
}
#if defined (BUILD_NATIVE)
const char* map[] = {"Cappuccino", "A. Coffee", ""};
#endif

lv_obj_t* gaggia_brew_options(lv_obj_t* parent) {
    // Matrix to select it
    lv_obj_t* btnm1 = lv_btnmatrix_create(parent, NULL);
    lv_obj_set_width_fit(btnm1, lv_obj_get_width(parent));
    lv_obj_add_style(btnm1, LV_LABEL_PART_MAIN, &style_box_opa00);
#if defined (BUILD_NATIVE)
    lv_btnmatrix_set_map(btnm1, map);
#endif
    lv_obj_align(btnm1, parent, LV_ALIGN_CENTER, 0, OFFSET);

    return btnm1;
}

lv_obj_t* gaggia_proccess(lv_obj_t* parent) {

    // Main Box
    lv_obj_t* mainBox = lv_obj_create(parent, NULL);
    //lv_obj_add_style(mainBox, LV_STATE_DEFAULT, &style_box_opa00);
    lv_obj_set_width_fit(mainBox, lv_obj_get_width_fit(parent));
    lv_obj_set_height(mainBox, LV_DPX(120));
    lv_coord_t fit_w = lv_obj_get_width_fit(mainBox);

    // Label on left
    lv_obj_t* message = lv_label_create(mainBox, NULL);
    lv_label_set_long_mode(message, LV_LABEL_LONG_BREAK);     /*Break the long lines*/
    lv_label_set_align(message, LV_LABEL_ALIGN_LEFT);       /*Center aligned lines*/
    lv_label_set_recolor(message, true);                      /*Enable re-coloring by commands in the text*/
    //    lv_label_set_text(message, "#ff00ff Re-color# #ff00ff words# #ff0000 of a# label "
    //                      "and  wrap long text automatically.");
    lv_obj_set_width_fit(message, fit_w - LV_DPX(60));
    lv_obj_align(message, mainBox, LV_ALIGN_IN_LEFT_MID, OFFSET, 0);

    // Label on left
    lv_obj_t* title = lv_label_create(mainBox, NULL);
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);       /*Center aligned lines*/
    lv_obj_align(title, mainBox, LV_ALIGN_IN_TOP_MID, OFFSET, 0);

    // Ok on the right
    lv_obj_t* btn2  = lv_btn_create(mainBox, NULL);
    lv_obj_set_size(btn2, LV_DPX(60), LV_DPX(60));

#if defined (BUILD_NATIVE)
    lv_label_set_text(title, "Cappucchino");
    lv_label_set_text(message, "Brew brew to start");
#endif

#if ! defined (BUILD_NATIVE)
    lv_obj_set_hidden(btn2, true);
#endif

    lv_obj_t* label2 = lv_label_create(btn2, NULL);
    lv_label_set_text(label2, "Ok");
    lv_obj_align(btn2, mainBox, LV_ALIGN_IN_RIGHT_MID, -OFFSET, 0);

    ui_elements[PROCESS_MESSAGE_CONTAINER].element = mainBox;
    ui_elements[PROCESS_MESSAGE_BUTTON].element = btn2;
    ui_elements[PROCESS_MESSAGE_BUTTON_LABEL].element = label2;
    ui_elements[PROCESS_MESSAGE_LABEL].element = message;
    ui_elements[PROCESS_MESSAGE_TITLE].element = title;
    return mainBox;
}


lv_obj_t* gaggia_cancel(lv_obj_t* parent) {
    // Cancel on the CENTER
    ui_elements[STOP_BUTTON].element = lv_btn_create(parent, NULL);
    lv_obj_set_size(ui_elements[STOP_BUTTON].element, 70, 70);
    lv_obj_t* label1 = lv_label_create(ui_elements[STOP_BUTTON].element, NULL);
    lv_label_set_text(label1, "Stop");
    lv_obj_add_style(ui_elements[STOP_BUTTON].element, LV_LABEL_PART_MAIN, &style_btn_stop);
    return ui_elements[STOP_BUTTON].element;
}


static void gaggia_brew_screen(lv_obj_t* parent) {
    lv_obj_t* label1, * label2;
    lv_obj_t* gp;
    lv_obj_t* stop;
    lv_obj_t* obj;

    // Place for Steam Temperature
    gp = gaggia_gauge(parent, &ui_elements[STEAM_TEMP_OBJ].element, &ui_elements[STEAM_TEMP_LABEL].element, &label1);
#if defined (BUILD_NATIVE)
    gaggia_ui_set_text(STEAM_TEMP_LABEL, "145");
#endif
    lv_label_set_text(label1, "Steam");
    lv_obj_realign(label1);
    lv_obj_align(gp, parent,  LV_ALIGN_IN_BOTTOM_LEFT, OFFSET, -OFFSET);

    // Place for brew Temperaturex
    gp = gaggia_gauge(parent,  &ui_elements[BREW_TEMP_OBJ].element, &ui_elements[BREW_TEMP_LABEL].element, &label2);
#if defined (BUILD_NATIVE)
    gaggia_ui_set_text(BREW_TEMP_LABEL, "90");
#endif
    lv_label_set_text(label2, "brew");
    lv_obj_realign(label2);
    lv_obj_align(gp, parent, LV_ALIGN_IN_BOTTOM_RIGHT, -OFFSET, -OFFSET);

    // Place for to show current process
    gp = gaggia_proccess(parent);
    lv_obj_align(gp, parent,  LV_ALIGN_IN_TOP_MID, 0, OFFSET);

    stop = gaggia_cancel(parent);
    lv_obj_align(stop, parent,  LV_ALIGN_IN_BOTTOM_MID, 0, -PAD_DEF);

    obj = gaggia_timer(parent);
    lv_obj_align(obj, stop,  LV_ALIGN_OUT_TOP_MID, 0, -PAD_DEF);

}

static void gaggia_select_screen(lv_obj_t* parent) {
    ui_elements[PROCESS_SELECT_MATRIX].element = gaggia_brew_options(parent);
    lv_obj_align(ui_elements[PROCESS_SELECT_MATRIX].element, parent, LV_ALIGN_IN_TOP_MID, 0, OFFSET);
}

static void gaggia_setting_screen(lv_obj_t* parent) {
    lv_obj_t* box;
    box = spin_create(parent,
                      &ui_elements[BREWTEMP_SPIN].element,
                      &ui_elements[BREWTEMP_PLUS_BUTTON].element,
                      &ui_elements[BREWTEMP_MIN_BUTTON].element, "Brew tmp.", 60, 115);
    lv_obj_align(box, parent, LV_ALIGN_IN_TOP_MID, 0, OFFSET);

    box = spin_create(parent,
                      &ui_elements[STEAMTEMP_SPIN].element,
                      &ui_elements[STEAMTEMP_PLUS_BUTTON].element,
                      &ui_elements[STEAMTEMP_MIN_BUTTON].element, "Steam tmp.", 130, 170);
    lv_obj_align(box, parent, LV_ALIGN_IN_TOP_MID, 0, 46 + OFFSET);
}

void gaggia_ui_create_ui(void) {
    gaggia_style();

    for (int i = 0; i < _LAST_ITEM_STUB; i++) {
        ui_elements[i].element = NULL;
    }

    lv_coord_t hres = lv_disp_get_hor_res(NULL);
    // lv_coord_t vres = lv_disp_get_ver_res(NULL);

    // Background
    lv_obj_t* wp = lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_width(wp, hres);
    lv_img_set_src(wp, &coffee);

    // Tabs
    ui_elements[TAB_VIEW].element  = lv_tabview_create(lv_scr_act(), NULL);
    lv_obj_t* tab1 = lv_tabview_add_tab(ui_elements[TAB_VIEW].element, "Brew");
    lv_obj_t* tab2 = lv_tabview_add_tab(ui_elements[TAB_VIEW].element, "Select");
    lv_obj_t* tab3 = lv_tabview_add_tab(ui_elements[TAB_VIEW].element, "Setting");
    lv_obj_add_style(ui_elements[TAB_VIEW].element, LV_TABVIEW_PART_BG, &style_tab);


    /*Create a LED for status during programming */
#if defined (GUI_IO)
    ui_elements[HEAT_STATUS_SSR].element = lv_led_create(tab1, NULL);
    lv_obj_align(ui_elements[HEAT_STATUS_SSR].element, NULL, LV_ALIGN_CENTER, -40, 30);
    lv_led_on(ui_elements[HEAT_STATUS_SSR].element);

    /*Copy the previous LED and set a brightness*/
    ui_elements[VALVE_STATUS_SSR].element  = lv_led_create(tab1, ui_elements[HEAT_STATUS_SSR].element);
    lv_obj_align(ui_elements[VALVE_STATUS_SSR].element, NULL, LV_ALIGN_CENTER, 0, 30);

    /*Copy the previous LED and switch it ON*/
    ui_elements[PUMP_STATUS_SSR].element  = lv_led_create(tab1, ui_elements[HEAT_STATUS_SSR].element);
    lv_obj_align(ui_elements[PUMP_STATUS_SSR].element, NULL, LV_ALIGN_CENTER, 40, 30);


    /*Copy the previous LED and switch it ON*/
    ui_elements[BREW_BUT_STATUS].element  = lv_led_create(tab1, NULL);
    lv_obj_align(ui_elements[BREW_BUT_STATUS].element, NULL, LV_ALIGN_CENTER, 80, 60);

    /*Copy the previous LED and switch it ON*/
    ui_elements[STEAM_BUT_STATUS].element  = lv_led_create(tab1, NULL);
    lv_obj_align(ui_elements[STEAM_BUT_STATUS].element, NULL, LV_ALIGN_CENTER, -80, 60);
#endif


    gaggia_brew_screen(tab1);
    gaggia_select_screen(tab2);
    gaggia_setting_screen(tab3);

    // Set element ENUM in ID
    for (int i = 0; i < _LAST_ITEM_STUB; i++) {
        if (ui_elements[i].element != NULL) {
            lv_obj_set_user_data(ui_elements[i].element, i);
        } else {
            printf("Not matched %i\n", i);
        }

        text_elements[i].text = NULL;
        text_elements[i].length = 0;
    }

    lv_obj_set_event_cb(ui_elements[BREWTEMP_PLUS_BUTTON].element, generic_event_handler);
    lv_obj_set_event_cb(ui_elements[BREWTEMP_MIN_BUTTON].element, generic_event_handler);
    lv_obj_set_event_cb(ui_elements[STEAMTEMP_PLUS_BUTTON].element, generic_event_handler);
    lv_obj_set_event_cb(ui_elements[STEAMTEMP_MIN_BUTTON].element, generic_event_handler);
}

void gaggia_ui_set_text(enum ui_element_types label, const char* value) {
    gaggia_ui_set_text_hint(label, value, value == NULL ? 0 : strlen(value));
}

/**
 * Will set a text on a label and caches the nenory only re-allocating when needed
 * When text did not change, the update function of the label is not called
 * sizeHint can be given when you have an idea of maximum size of text you need to avoid allocations later in program lifetime
 */
void gaggia_ui_set_text_hint(enum ui_element_types label, const char* value, size_t sizeHint) {
    // CHeck for refresh request
    if (value == NULL) {
        lv_label_set_text_static(ui_elements[label].element, NULL);
        lv_obj_realign(ui_elements[label].element);
        return;
    }

    const size_t strLen = strlen(value);

    // Allocate if buffer is to small or non excistend, allocate 4 extra bytes because we do
    // use numbers a lot and might prevent extra allocations (and not forget the extra 0 char)
    // if we just need a few bytes extra
    if (text_elements[label].text == NULL || text_elements[label].length <= strLen) {
        const size_t requestSize = strLen + 4;
        free(text_elements[label].text);
        text_elements[label].text = malloc(requestSize);

        if (text_elements[label].text != NULL) {
            text_elements[label].length = requestSize;
        }
    }

    if (text_elements[label].text != NULL) {
        strncpy(text_elements[label].text, value, text_elements[label].length);
        lv_label_set_text_static(ui_elements[label].element, text_elements[label].text);
        lv_obj_realign(ui_elements[label].element);
    }
}

char* gaggia_ui_set_text_buffer(enum ui_element_types label) {
    return text_elements[label].text;
}

void gaggia_ui_set_visibility(enum ui_element_types label, bool en) {
    lv_obj_set_hidden(ui_elements[label].element, !en);
}

void gaggia_ui_add_event_cb(enum ui_element_types label, gaggia_ui_event event) {
    if (ui_elements[label].element) {
        lv_obj_set_event_cb(ui_elements[label].element, generic_event_handler);
    }

    ui_elements[label].event_cb = event;
}

void gaggia_ui_set_led(enum ui_element_types label, bool status) {
    //ESP_LOGE("gaggia_ui", "Setting status %s", status?"true":"false");
    if (status) {
        lv_led_on(ui_elements[label].element);
    } else {
        lv_led_off(ui_elements[label].element);
    }
}

void gaggia_ui_set_led_bright(enum ui_element_types label, uint8_t status) {
    lv_led_set_bright(ui_elements[label].element, status);
}

void gaggia_ui_spin_set_range(enum ui_element_types label, int32_t min, int32_t max) {
    lv_spinbox_set_range(ui_elements[label].element, min, max);
}

void gaggia_ui_spin_set_value(enum ui_element_types label, int32_t value) {
    return lv_spinbox_set_value(ui_elements[label].element, value);
}

int32_t gaggia_ui_spin_get_value(enum ui_element_types label) {
    return lv_spinbox_get_value(ui_elements[label].element);
}

void gaggia_ui_btn_map(enum ui_element_types label,  const char* map[]) {
    lv_btnmatrix_set_map(ui_elements[label].element, map);
}

uint16_t gaggia_ui_btn_map_active(enum ui_element_types label) {
    return lv_btnmatrix_get_active_btn(ui_elements[label].element);
}

void gaggia_ui_change_screen(uint8_t screen) {
    lv_tabview_set_tab_act(ui_elements[TAB_VIEW].element, screen, LV_ANIM_ON);

}
