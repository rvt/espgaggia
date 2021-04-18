/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "lvgl.h"
#include <app_hal.h>
#include "../../src/ui/gaggia_ui.h"


int main(void) {
    lv_init();

    hal_setup();

    gaggia_ui_create_ui();

    static const char* map[] = {"Cappuccino", "A. Coffee", "\n", "Purge Cold", "Purge Hot", ""};
    gaggia_ui_btn_map(PROCESS_SELECT_MATRIX, map);

    hal_loop();
}

