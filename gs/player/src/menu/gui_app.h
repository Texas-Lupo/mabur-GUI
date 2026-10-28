#ifndef GUI_APP_H
#define GUI_APP_H

#include "lvgl/lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NAV_UP = 0,
    NAV_DOWN,
    NAV_LEFT,
    NAV_RIGHT,
    NAV_ENTER,
    NAV_BACK,
    NAV_TAB_PREV,
    NAV_TAB_NEXT
} nav_key_t;

typedef enum {
    TAB_CAMERA = 0,
    TAB_LINK,
    TAB_RECORDINGS,
    TAB_TELEMETRY,
    TAB_CUSTOMIZATION,
    TAB_CALIBRATION,
    TAB_ADVANCED,
    TAB_COUNT
} menu_tab_t;

typedef enum {
    ZONE_TOP_BAR = 0,
    ZONE_SUBMENU,
    ZONE_MODAL
} nav_zone_t;

typedef enum {
    SIDE_LEFT = 0,
    SIDE_RIGHT
} submenu_side_t;

typedef enum {
    BG_MODE_TRANSPARENT = 0,
    BG_MODE_SPLASH
} bg_mode_t;

/* Public API */
typedef void (*gui_close_cb_t)(void);
void gui_set_close_request_cb(gui_close_cb_t cb);
void gui_app_init(void);
void gui_app_reset_focus(void);
void gui_handle_input(nav_key_t key);
void gui_show_toast(const char * title, const char * msg, uint32_t duration_ms);
void gui_close_modal(void);

/* Settings row helper callbacks */
typedef void (*row_change_cb_t)(void * user_data, int dir);
typedef void (*row_click_cb_t)(void * user_data);

typedef enum {
    ROW_SELECTOR = 0,
    ROW_TOGGLE,
    ROW_ACTION,
    ROW_SUBPAGE
} row_type_t;

typedef struct setting_row_t {
    lv_obj_t * container;
    lv_obj_t * label_title;
    lv_obj_t * label_val;
    row_type_t type;
    row_change_cb_t change_cb;
    row_click_cb_t click_cb;
    void * user_data;
    struct setting_row_t * next;
} setting_row_t;

/* Row builder functions */
setting_row_t * gui_create_selector_row(lv_obj_t * parent, const char * title, const char * initial_val, row_change_cb_t cb, void * user_data);
setting_row_t * gui_create_toggle_row(lv_obj_t * parent, const char * title, bool initial_state, row_change_cb_t cb, void * user_data);
setting_row_t * gui_create_action_row(lv_obj_t * parent, const char * title, const char * btn_label, row_click_cb_t cb, void * user_data);
void gui_update_row_val(setting_row_t * row, const char * val_text);
void gui_update_toggle_val(setting_row_t * row, bool state);

/* Submenu builders */
void gui_rebuild_submenu(void);
void ui_build_camera(lv_obj_t * parent);
void ui_build_link(lv_obj_t * parent);
void ui_build_recordings(lv_obj_t * parent);
void ui_build_telemetry(lv_obj_t * parent);
void ui_build_customization(lv_obj_t * parent);
void ui_build_calibration(lv_obj_t * parent);
void ui_build_advanced(lv_obj_t * parent);

/* Getters & Setters */
int gui_get_menu_scale(void);
void gui_set_menu_scale(int scale_percent);
void gui_save_menu_settings(void);
void gui_on_exit_menu(void);
submenu_side_t gui_get_submenu_side(void);
void gui_set_submenu_side(submenu_side_t side);
bg_mode_t gui_get_bg_mode(void);
void gui_set_bg_mode(bg_mode_t mode);
bool gui_is_advanced_unlocked(void);
void gui_unlock_advanced(void);

#ifdef __cplusplus
}
#endif

#endif /* GUI_APP_H */
