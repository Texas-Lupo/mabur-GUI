#include "gui_app.h"
#include "config_data.h"
#include "gui_styles.h"
#include <stdio.h>

static setting_row_t * s_row_color;
static setting_row_t * s_row_scale;
static setting_row_t * s_row_side;
static setting_row_t * s_row_bg;

static const char * s_bg_mode_names[] = {
    "Transparency",
    "Splashscreen"
};

static void on_scale_change(void * ud, int dir) {
    (void)ud;
    int cur = gui_get_menu_scale();
    cur += dir * 10;
    if(cur < 10) cur = 10;
    if(cur > 500) cur = 500;
    gui_set_menu_scale(cur);
    gui_save_menu_settings();
    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", cur);
    gui_update_row_val(s_row_scale, buf);
}

static void on_color_change(void * ud, int dir) {
    (void)ud;
    int next_idx = g_current_theme_idx + dir;
    if(next_idx < 0) next_idx = g_theme_colors_count - 1;
    if(next_idx >= g_theme_colors_count) next_idx = 0;
    gui_styles_set_theme(next_idx);
    gui_save_menu_settings();
    gui_update_row_val(s_row_color, g_theme_colors[next_idx].name);
}

static void on_side_change(void * ud, int dir) {
    (void)ud; (void)dir;
    submenu_side_t cur = gui_get_submenu_side();
    gui_set_submenu_side(cur == SIDE_LEFT ? SIDE_RIGHT : SIDE_LEFT);
    gui_save_menu_settings();
    gui_update_row_val(s_row_side, (gui_get_submenu_side() == SIDE_LEFT) ? "Left Side" : "Right Side");
}

static void on_bg_change(void * ud, int dir) {
    (void)ud;
    bg_mode_t cur = gui_get_bg_mode();
    int next = (int)cur + dir;
    if(next < 0) next = 1;
    if(next > 1) next = 0;
    gui_set_bg_mode((bg_mode_t)next);
    gui_save_menu_settings();
    gui_update_row_val(s_row_bg, s_bg_mode_names[next]);
}

static void on_save_clicked(void * ud) {
    (void)ud;
    gui_save_menu_settings();
    if(config_data_save_all()) {
        gui_show_toast("Settings Saved", "Configuration saved & maburgs service restarted!", 3000);
    } else {
        gui_show_toast("Save Failed", "Could not write settings file.", 2500);
    }
}

void ui_build_customization(lv_obj_t * parent) {
    lv_obj_t * title = lv_label_create(parent);
    lv_label_set_text(title, "GUI Customization");
    lv_obj_add_style(title, &style_title_label, 0);

    lv_obj_t * desc = lv_label_create(parent);
    lv_label_set_text(desc, "Theme accent color, scale, menu side alignment & background mode.");
    lv_obj_add_style(desc, &style_desc_label, 0);

    /* Theme color */
    s_row_color = gui_create_selector_row(parent, "Accent Glow Color",
                                          g_theme_colors[g_current_theme_idx].name,
                                          on_color_change, NULL);

    /* Scale */
    char scale_buf[16];
    snprintf(scale_buf, sizeof(scale_buf), "%d%%", gui_get_menu_scale());
    s_row_scale = gui_create_selector_row(parent, "Scale",
                                          scale_buf,
                                          on_scale_change, NULL);

    /* Submenu position (Left / Right) */
    s_row_side = gui_create_selector_row(parent, "Submenu Position",
                                         (gui_get_submenu_side() == SIDE_LEFT) ? "Left Side" : "Right Side",
                                         on_side_change, NULL);

    /* Video Background Mode */
    s_row_bg = gui_create_selector_row(parent, "Transparent Background",
                                       s_bg_mode_names[gui_get_bg_mode()],
                                       on_bg_change, NULL);

    /* Save button */
    gui_create_action_row(parent, "Persist Settings", "Save Config", on_save_clicked, NULL);
}
