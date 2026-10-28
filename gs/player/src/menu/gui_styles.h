#ifndef GUI_STYLES_H
#define GUI_STYLES_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char * name;
    lv_color_t color;
    lv_color_t glow;
} theme_color_t;

extern const theme_color_t g_theme_colors[];
extern const int g_theme_colors_count;
extern int g_current_theme_idx;

/* Common styles */
extern lv_style_t style_topbar;
extern lv_style_t style_top_tab;
extern lv_style_t style_top_tab_focused;
extern lv_style_t style_top_tab_active;

extern lv_style_t style_submenu_panel;
extern lv_style_t style_item_row;
extern lv_style_t style_item_row_focused;

extern lv_style_t style_title_label;
extern lv_style_t style_desc_label;
extern lv_style_t style_val_arrow;
extern lv_style_t style_val_text;

extern lv_style_t style_btn_action;
extern lv_style_t style_btn_action_focused;

extern lv_style_t style_switch;
extern lv_style_t style_switch_checked;
extern lv_style_t style_switch_knob;
extern lv_style_t style_slider;
extern lv_style_t style_slider_knob;

/* Init and theme change */
void gui_styles_init(void);
void gui_styles_set_theme(int theme_idx);
lv_color_t gui_get_accent_color(void);
lv_color_t gui_get_accent_glow(void);

#ifdef __cplusplus
}
#endif

#endif /* GUI_STYLES_H */
