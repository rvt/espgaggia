/**
 * @file lv_theme_gaggia_theme.h
 *
 */

#ifndef LV_THEME_GAGGIA_THEME_H
#define LV_THEME_GAGGIA_THEME_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>


/*********************
 *      DEFINES
 *********************/
/*Colors*/
#define LV_THEME_GAGGIA_WHITE           lv_color_hex(0xffffff)
#define LV_THEME_GAGGIA_LIGHT           lv_color_hex(0xf3f8fe)
#define LV_THEME_GAGGIA_GRAY            lv_color_hex(0x8a8a8a)
#define LV_THEME_GAGGIA_LIGHT_GRAY      lv_color_hex(0xc4c4c4)
#define LV_THEME_GAGGIA_BLUE            lv_color_hex(0x2f3243) //006fb6
#define LV_THEME_GAGGIA_GREEN           lv_color_hex(0x4cb242)
#define LV_THEME_GAGGIA_RED             lv_color_hex(0xd51732)


/*********************
 *      DEFINES
 *********************/
typedef enum {
    LV_THEME_GAGGIA_FLAG_DARK =  0x01,
    LV_THEME_GAGGIA_FLAG_LIGHT = 0x02,
    LV_THEME_GAGGIA_FLAG_NO_TRANSITION  = 0x10,
    LV_THEME_GAGGIA_FLAG_NO_FOCUS  = 0x20,
} lv_THEME_GAGGIA__flag_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

#define PAD_DEF (lv_disp_get_size_category(NULL) <= LV_DISP_SIZE_MEDIUM ? LV_DPX(4) : (LV_DPX(8)))


/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_theme_gaggia_init(lv_color_t color_primary, lv_color_t color_secondary, uint32_t flags,
                                 const lv_font_t* font_small, const lv_font_t* font_normal, const lv_font_t* font_subtitle,
                                 const lv_font_t* font_title);
/**********************
 *      MACROS
 **********************/



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

