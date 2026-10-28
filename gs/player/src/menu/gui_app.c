#include "gui_app.h"
#include "config_data.h"
#include "gui_styles.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* Top tabs configuration */
static const char * s_tab_titles[TAB_COUNT] = {
    "Camera",
    "Link",
    "Recordings",
    "Telemetry",
    "Custom",
    "Calibrate",
    "Advanced"
};

/* State */
static nav_zone_t s_active_zone = ZONE_TOP_BAR;
static int s_active_tab_idx = 0;
static int s_active_row_idx = 0;
static submenu_side_t s_submenu_side = SIDE_LEFT;
static bg_mode_t s_bg_mode = BG_MODE_TRANSPARENT;
static bool s_advanced_unlocked = false;

/* Konami Code tracker */
static const nav_key_t s_konami_code[8] = {
    NAV_UP, NAV_UP, NAV_DOWN, NAV_DOWN, NAV_LEFT, NAV_RIGHT, NAV_LEFT, NAV_RIGHT
};
static int s_konami_step = 0;

/* UI Widgets */
static lv_obj_t * s_scr_root = NULL;
static lv_obj_t * s_bg_layer = NULL;
static lv_obj_t * s_splash_img = NULL;
static lv_obj_t * s_top_bar = NULL;
static lv_obj_t * s_top_tabs_cont = NULL;
static lv_obj_t * s_tab_btns[TAB_COUNT] = { NULL };
static lv_obj_t * s_submenu_panel = NULL;
static lv_obj_t * s_modal_box = NULL;
static lv_obj_t * s_modal_title = NULL;
static lv_obj_t * s_modal_msg = NULL;
static lv_timer_t * s_modal_timer = NULL;

/* Splash asset descriptor */
static void * s_splash_map = NULL;
static size_t s_splash_map_len = 0;
static lv_image_dsc_t s_splash_dsc;

/* Linked list of rows in current submenu */
static setting_row_t * s_rows_head = NULL;
static int s_rows_count = 0;

/* Internal prototypes */
static void build_current_submenu(void);
static void refresh_topbar_styles(void);
static void refresh_submenu_focus(void);
static void update_submenu_layout(void);
static void create_background_layer(void);
static void update_background_mode(void);
static bool load_splash_asset(void);

static gui_close_cb_t s_close_req_cb = NULL;
void gui_set_close_request_cb(gui_close_cb_t cb) { s_close_req_cb = cb; }

void gui_app_reset_focus(void) {
    s_active_zone = ZONE_TOP_BAR;
    s_active_row_idx = 0;
    refresh_topbar_styles();
    build_current_submenu();
}

static int s_menu_scale = 100;

int gui_get_menu_scale(void) { return s_menu_scale; }

__attribute__((weak)) void vrx_menu_clear_canvas(void) {}

void gui_set_menu_scale(int scale_percent) {
    if(scale_percent < 10) scale_percent = 10;
    if(scale_percent > 500) scale_percent = 500;
    s_menu_scale = scale_percent;

    int32_t val = (int32_t)((256 * s_menu_scale) / 100);

    /* Dynamically scale top bar height so scaled tabs fit without clipping */
    int top_h = (56 * s_menu_scale) / 100;
    if(top_h < 40) top_h = 40;
    if(s_top_bar) {
        lv_obj_set_height(s_top_bar, top_h);
    }

    if(s_top_tabs_cont) {
        lv_obj_set_style_transform_pivot_x(s_top_tabs_cont, lv_pct(50), 0);
        lv_obj_set_style_transform_pivot_y(s_top_tabs_cont, lv_pct(50), 0);
        lv_obj_set_style_transform_scale(s_top_tabs_cont, val, 0);
    }

    if(s_submenu_panel) {
        update_submenu_layout();
    }

    if(s_modal_box) {
        lv_obj_set_style_transform_pivot_x(s_modal_box, 240, 0);
        lv_obj_set_style_transform_pivot_y(s_modal_box, 110, 0);
        lv_obj_set_style_transform_scale(s_modal_box, val, 0);
    }

    /* Keep top bar and modal in front of submenu panel */
    if(s_top_bar) lv_obj_move_foreground(s_top_bar);
    if(s_modal_box) lv_obj_move_foreground(s_modal_box);

    /* Invalidate and clear backing canvas */
    vrx_menu_clear_canvas();
    if(s_scr_root) {
        lv_obj_update_layout(s_scr_root);
        lv_obj_invalidate(s_scr_root);
    }
}

void gui_save_menu_settings(void) {
    FILE * f = fopen("/etc/mabur_menu.conf", "w");
    if(f) {
        fprintf(f, "scale=%d\n", s_menu_scale);
        fprintf(f, "side=%d\n", (int)s_submenu_side);
        fprintf(f, "bg_mode=%d\n", (int)s_bg_mode);
        fprintf(f, "theme=%d\n", g_current_theme_idx);
        fprintf(f, "advanced=%d\n", s_advanced_unlocked ? 1 : 0);
        fflush(f);
        fclose(f);
        sync();
    }
}

void gui_on_exit_menu(void) {
    gui_save_menu_settings();
    config_data_save_all();
}

static void load_menu_settings(void) {
    FILE * f = fopen("/etc/mabur_menu.conf", "r");
    if(f) {
        char line[64];
        while(fgets(line, sizeof(line), f)) {
            int val = 0;
            if(sscanf(line, "scale=%d", &val) == 1) {
                if(val >= 10 && val <= 500) s_menu_scale = val;
            } else if(sscanf(line, "side=%d", &val) == 1) {
                if(val >= 0 && val <= 1) s_submenu_side = (submenu_side_t)val;
            } else if(sscanf(line, "bg_mode=%d", &val) == 1) {
                if(val >= 0 && val <= 1) s_bg_mode = (bg_mode_t)val;
            } else if(sscanf(line, "theme=%d", &val) == 1) {
                if(val >= 0 && val < g_theme_colors_count) {
                    gui_styles_set_theme(val);
                }
            } else if(sscanf(line, "advanced=%d", &val) == 1) {
                if(val == 1) s_advanced_unlocked = true;
            }
        }
        fclose(f);
    }
}

submenu_side_t gui_get_submenu_side(void) { return s_submenu_side; }
void gui_set_submenu_side(submenu_side_t side) {
    s_submenu_side = side;
    update_submenu_layout();
}

bg_mode_t gui_get_bg_mode(void) { return s_bg_mode; }
void gui_set_bg_mode(bg_mode_t mode) {
    s_bg_mode = mode;
    update_background_mode();
    vrx_menu_clear_canvas();
    if(s_scr_root) {
        lv_obj_invalidate(s_scr_root);
    }
}

bool gui_is_advanced_unlocked(void) { return s_advanced_unlocked; }

void gui_unlock_advanced(void) {
    if(s_advanced_unlocked) return;
    s_advanced_unlocked = true;

    /* Make the Advanced tab button visible */
    if(s_tab_btns[TAB_ADVANCED]) {
        lv_obj_set_hidden(s_tab_btns[TAB_ADVANCED], false);
    }

    /* Switch directly to advanced tab */
    s_active_tab_idx = TAB_ADVANCED;
    s_active_zone = ZONE_TOP_BAR;
    s_active_row_idx = 0;

    refresh_topbar_styles();
    build_current_submenu();

    gui_show_toast("ADVANCED MENU UNLOCKED",
                   "Konami code recognized!\nFull engineering tuning parameters are now available.",
                   4000);
}

/* Modal toast dismiss callback */
static void modal_auto_close_cb(lv_timer_t * t) {
    (void)t;
    if(s_modal_box) {
        lv_obj_set_hidden(s_modal_box, true);
    }
    if(s_modal_timer) {
        lv_timer_delete(s_modal_timer);
        s_modal_timer = NULL;
    }
    s_active_zone = (s_rows_count > 0) ? ZONE_SUBMENU : ZONE_TOP_BAR;
}

void gui_show_toast(const char * title, const char * msg, uint32_t duration_ms) {
    if(!s_modal_box) return;

    lv_label_set_text(s_modal_title, title);
    lv_label_set_text(s_modal_msg, msg);
    lv_obj_set_hidden(s_modal_box, false);
    s_active_zone = ZONE_MODAL;

    if(s_modal_timer) {
        lv_timer_delete(s_modal_timer);
        s_modal_timer = NULL;
    }

    if(duration_ms > 0) {
        s_modal_timer = lv_timer_create(modal_auto_close_cb, duration_ms, NULL);
        lv_timer_set_repeat_count(s_modal_timer, 1);
    }
}

/* Helper to build standard interactive setting row */
setting_row_t * gui_create_selector_row(lv_obj_t * parent,
                                       const char * label_text,
                                       const char * initial_val,
                                       row_change_cb_t change_cb,
                                       void * user_data) {
    setting_row_t * row = (setting_row_t *)lv_malloc(sizeof(setting_row_t));
    memset(row, 0, sizeof(setting_row_t));

    row->type = ROW_SELECTOR;
    row->change_cb = change_cb;
    row->user_data = user_data;

    /* Container */
    row->container = lv_obj_create(parent);
    lv_obj_set_size(row->container, LV_PCT(100), 52);
    lv_obj_add_style(row->container, &style_item_row, 0);
    lv_obj_set_flex_flow(row->container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row->container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollable(row->container, false);

    /* Left Label */
    row->label_title = lv_label_create(row->container);
    lv_label_set_text(row->label_title, label_text);
    lv_obj_set_style_text_color(row->label_title, lv_color_hex(0xdddddd), 0);
    lv_obj_set_style_text_font(row->label_title, &lv_font_montserrat_14, 0);

    /* Right Selector: < Value > */
    lv_obj_t * sel_box = lv_obj_create(row->container);
    lv_obj_set_size(sel_box, LV_SIZE_CONTENT, LV_PCT(100));
    lv_obj_set_style_bg_opa(sel_box, LV_OPA_0, 0);
    lv_obj_set_style_border_width(sel_box, 0, 0);
    lv_obj_set_style_pad_all(sel_box, 0, 0);
    lv_obj_set_flex_flow(sel_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sel_box, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollable(sel_box, false);

    lv_obj_t * arrow_left = lv_label_create(sel_box);
    lv_label_set_text(arrow_left, "< ");
    lv_obj_add_style(arrow_left, &style_val_arrow, 0);

    row->label_val = lv_label_create(sel_box);
    lv_label_set_text(row->label_val, initial_val);
    lv_obj_add_style(row->label_val, &style_val_text, 0);

    lv_obj_t * arrow_right = lv_label_create(sel_box);
    lv_label_set_text(arrow_right, " >");
    lv_obj_add_style(arrow_right, &style_val_arrow, 0);

    /* Link into row list */
    if(!s_rows_head) {
        s_rows_head = row;
    } else {
        setting_row_t * p = s_rows_head;
        while(p->next) p = p->next;
        p->next = row;
    }
    s_rows_count++;

    return row;
}

setting_row_t * gui_create_toggle_row(lv_obj_t * parent,
                                     const char * label_text,
                                     bool initial_state,
                                     row_change_cb_t change_cb,
                                     void * user_data) {
    setting_row_t * row = (setting_row_t *)lv_malloc(sizeof(setting_row_t));
    memset(row, 0, sizeof(setting_row_t));

    row->type = ROW_TOGGLE;
    row->change_cb = change_cb;
    row->user_data = user_data;

    row->container = lv_obj_create(parent);
    lv_obj_set_size(row->container, LV_PCT(100), 52);
    lv_obj_add_style(row->container, &style_item_row, 0);
    lv_obj_set_flex_flow(row->container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row->container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollable(row->container, false);

    row->label_title = lv_label_create(row->container);
    lv_label_set_text(row->label_title, label_text);
    lv_obj_set_style_text_color(row->label_title, lv_color_hex(0xdddddd), 0);
    lv_obj_set_style_text_font(row->label_title, &lv_font_montserrat_14, 0);

    row->label_val = lv_switch_create(row->container);
    lv_obj_add_style(row->label_val, &style_switch, 0);
    lv_obj_add_style(row->label_val, &style_switch_checked, LV_STATE_CHECKED);
    lv_obj_add_style(row->label_val, &style_switch_checked, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_style(row->label_val, &style_switch, LV_PART_INDICATOR);
    lv_obj_add_style(row->label_val, &style_switch_knob, LV_PART_KNOB);
    if(initial_state) {
        lv_obj_add_state(row->label_val, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(row->label_val, LV_STATE_CHECKED);
    }

    if(!s_rows_head) {
        s_rows_head = row;
    } else {
        setting_row_t * p = s_rows_head;
        while(p->next) p = p->next;
        p->next = row;
    }
    s_rows_count++;

    return row;
}

void gui_update_toggle_val(setting_row_t * row, bool state) {
    if(row && row->label_val) {
        if(state) {
            lv_obj_add_state(row->label_val, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(row->label_val, LV_STATE_CHECKED);
        }
    }
}

setting_row_t * gui_create_action_row(lv_obj_t * parent,
                                     const char * label_text,
                                     const char * btn_text,
                                     row_click_cb_t click_cb,
                                     void * user_data) {
    setting_row_t * row = (setting_row_t *)lv_malloc(sizeof(setting_row_t));
    memset(row, 0, sizeof(setting_row_t));

    row->type = ROW_ACTION;
    row->click_cb = click_cb;
    row->user_data = user_data;

    row->container = lv_obj_create(parent);
    lv_obj_set_size(row->container, LV_PCT(100), 52);
    lv_obj_add_style(row->container, &style_item_row, 0);
    lv_obj_set_flex_flow(row->container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row->container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollable(row->container, false);

    row->label_title = lv_label_create(row->container);
    lv_label_set_text(row->label_title, label_text);
    lv_obj_set_style_text_color(row->label_title, lv_color_hex(0xdddddd), 0);
    lv_obj_set_style_text_font(row->label_title, &lv_font_montserrat_14, 0);

    row->label_val = lv_btn_create(row->container);
    lv_obj_add_style(row->label_val, &style_btn_action, 0);
    lv_obj_t * blbl = lv_label_create(row->label_val);
    lv_label_set_text(blbl, btn_text);
    lv_obj_center(blbl);

    if(!s_rows_head) {
        s_rows_head = row;
    } else {
        setting_row_t * p = s_rows_head;
        while(p->next) p = p->next;
        p->next = row;
    }
    s_rows_count++;

    return row;
}

void gui_update_row_val(setting_row_t * row, const char * new_val) {
    if(row && row->label_val) {
        lv_label_set_text(row->label_val, new_val);
    }
}

/* Background creation and mode switching */
static bool load_splash_asset(void) {
    if(s_splash_map) return true;

    const char * paths[] = {
        "/usr/local/share/mabur/splash.bin",
        "gs/player/bundle/splash.bin",
        "../gs/player/bundle/splash.bin",
        "../../gs/player/bundle/splash.bin",
        "/tmp/splash.bin",
        NULL
    };

    int fd = -1;
    for(int i = 0; paths[i] != NULL; i++) {
        fd = open(paths[i], O_RDONLY);
        if(fd >= 0) break;
    }
    if(fd < 0) return false;

    struct stat st;
    if(fstat(fd, &st) != 0 || st.st_size < 16 + 1920 * 1080 * 4) {
        close(fd);
        return false;
    }

    s_splash_map_len = (size_t)st.st_size;
    s_splash_map = mmap(NULL, s_splash_map_len, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if(s_splash_map == MAP_FAILED) {
        s_splash_map = NULL;
        return false;
    }

    const uint8_t * p = (const uint8_t *)s_splash_map;
    if(memcmp(p, "MSPL", 4) != 0) {
        munmap(s_splash_map, s_splash_map_len);
        s_splash_map = NULL;
        return false;
    }

    uint32_t w = *(const uint32_t *)(p + 4);
    uint32_t h = *(const uint32_t *)(p + 8);

    s_splash_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    s_splash_dsc.header.cf = LV_COLOR_FORMAT_ARGB8888;
    s_splash_dsc.header.flags = 0;
    s_splash_dsc.header.w = w;
    s_splash_dsc.header.h = h;
    s_splash_dsc.header.stride = w * 4;
    s_splash_dsc.data_size = w * h * 4;
    s_splash_dsc.data = p + 16;

    return true;
}

static void create_background_layer(void) {
    s_bg_layer = lv_obj_create(s_scr_root);
    lv_obj_set_size(s_bg_layer, 1920, 1080);
    lv_obj_align(s_bg_layer, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_border_width(s_bg_layer, 0, 0);
    lv_obj_set_style_pad_all(s_bg_layer, 0, 0);
    lv_obj_set_style_radius(s_bg_layer, 0, 0);
    lv_obj_set_scrollable(s_bg_layer, false);

    if(load_splash_asset()) {
        s_splash_img = lv_image_create(s_bg_layer);
        lv_image_set_src(s_splash_img, &s_splash_dsc);
        lv_obj_align(s_splash_img, LV_ALIGN_TOP_LEFT, 0, 0);
    }

    update_background_mode();
}

static void update_background_mode(void) {
    if(!s_bg_layer) return;

    if(s_bg_mode == BG_MODE_TRANSPARENT) {
        lv_obj_set_style_bg_opa(s_bg_layer, LV_OPA_0, 0);
        if(s_splash_img) {
            lv_obj_set_hidden(s_splash_img, true);
        }
    } else { /* BG_MODE_SPLASH */
        if(s_splash_img) {
            lv_obj_set_style_bg_opa(s_bg_layer, LV_OPA_0, 0);
            lv_obj_set_hidden(s_splash_img, false);
        } else {
            /* Fallback solid dark theme if splash binary not present */
            lv_obj_set_style_bg_opa(s_bg_layer, LV_OPA_COVER, 0);
            lv_obj_set_style_bg_color(s_bg_layer, lv_color_hex(0x101420), 0);
        }
    }
}

/* Layout and Submenu creation */
static void update_submenu_layout(void) {
    if(!s_submenu_panel) return;

    int32_t val = (int32_t)((256 * s_menu_scale) / 100);
    int top_h = (56 * s_menu_scale) / 100;
    if(top_h < 40) top_h = 40;
    int sub_y = top_h + (12 * s_menu_scale) / 100;

    if(s_submenu_side == SIDE_LEFT) {
        lv_obj_align(s_submenu_panel, LV_ALIGN_TOP_LEFT, 16, sub_y);
        lv_obj_set_style_transform_pivot_x(s_submenu_panel, 0, 0);
        lv_obj_set_style_transform_pivot_y(s_submenu_panel, 0, 0);
    } else {
        lv_obj_align(s_submenu_panel, LV_ALIGN_TOP_RIGHT, -16, sub_y);
        lv_obj_set_style_transform_pivot_x(s_submenu_panel, 440, 0);
        lv_obj_set_style_transform_pivot_y(s_submenu_panel, 0, 0);
    }
    lv_obj_set_style_transform_scale(s_submenu_panel, val, 0);
}

static void clean_rows_list(void) {
    setting_row_t * p = s_rows_head;
    while(p) {
        setting_row_t * next = p->next;
        lv_free(p);
        p = next;
    }
    s_rows_head = NULL;
    s_rows_count = 0;
}

void gui_rebuild_submenu(void) {
    build_current_submenu();
}

static void build_current_submenu(void) {
    clean_rows_list();
    lv_obj_clean(s_submenu_panel);

    switch((menu_tab_t)s_active_tab_idx) {
        case TAB_CAMERA:
            ui_build_camera(s_submenu_panel);
            break;
        case TAB_LINK:
            ui_build_link(s_submenu_panel);
            break;
        case TAB_RECORDINGS:
            ui_build_recordings(s_submenu_panel);
            break;
        case TAB_TELEMETRY:
            ui_build_telemetry(s_submenu_panel);
            break;
        case TAB_CUSTOMIZATION:
            ui_build_customization(s_submenu_panel);
            break;
        case TAB_CALIBRATION:
            ui_build_calibration(s_submenu_panel);
            break;
        case TAB_ADVANCED:
            ui_build_advanced(s_submenu_panel);
            break;
        default:
            break;
    }

    refresh_submenu_focus();
}

static void refresh_topbar_styles(void) {
    for(int i = 0; i < TAB_COUNT; i++) {
        if(!s_tab_btns[i]) continue;
        lv_obj_remove_style(s_tab_btns[i], &style_top_tab_active, 0);
        lv_obj_remove_style(s_tab_btns[i], &style_top_tab_focused, 0);

        if(i == s_active_tab_idx) {
            if(s_active_zone == ZONE_TOP_BAR) {
                lv_obj_add_style(s_tab_btns[i], &style_top_tab_focused, 0);
            } else {
                lv_obj_add_style(s_tab_btns[i], &style_top_tab_active, 0);
            }
        }
    }
}

static void refresh_submenu_focus(void) {
    setting_row_t * cur = s_rows_head;
    int idx = 0;
    while(cur) {
        if(!cur->container) { cur = cur->next; idx++; continue; }

        if(s_active_zone == ZONE_SUBMENU && idx == s_active_row_idx) {
            lv_obj_add_style(cur->container, &style_item_row_focused, 0);
            lv_obj_scroll_to_view(cur->container, LV_ANIM_ON);

            if(cur->type == ROW_ACTION && cur->label_val) {
                lv_obj_add_style(cur->label_val, &style_btn_action_focused, 0);
            }
        } else {
            lv_obj_remove_style(cur->container, &style_item_row_focused, 0);
            if(cur->type == ROW_ACTION && cur->label_val) {
                lv_obj_remove_style(cur->label_val, &style_btn_action_focused, 0);
            }
        }
        cur = cur->next;
        idx++;
    }
}

/* 5-way physical button input dispatcher */
void gui_handle_input(nav_key_t key) {
    /* 1. Track Konami Code */
    if(key == s_konami_code[s_konami_step]) {
        s_konami_step++;
        if(s_konami_step == 8) {
            gui_unlock_advanced();
            s_konami_step = 0;
            return;
        }
    } else {
        s_konami_step = (key == s_konami_code[0]) ? 1 : 0;
    }

    /* 2. Modal Box active: Dismiss with Enter or Back */
    if(s_active_zone == ZONE_MODAL) {
        if(key == NAV_ENTER || key == NAV_BACK) {
            if(s_modal_box) lv_obj_set_hidden(s_modal_box, true);
            if(s_modal_timer) {
                lv_timer_delete(s_modal_timer);
                s_modal_timer = NULL;
            }
            s_active_zone = (s_rows_count > 0) ? ZONE_SUBMENU : ZONE_TOP_BAR;
            refresh_topbar_styles();
            refresh_submenu_focus();
        }
        return;
    }

    /* 3. Global Tab switching (via bumpers/trigger if available) */
    int max_tabs = s_advanced_unlocked ? TAB_COUNT : (TAB_COUNT - 1);
    if(key == NAV_TAB_NEXT) {
        s_active_tab_idx = (s_active_tab_idx + 1) % max_tabs;
        s_active_row_idx = 0;
        refresh_topbar_styles();
        build_current_submenu();
        return;
    } else if(key == NAV_TAB_PREV) {
        s_active_tab_idx = (s_active_tab_idx > 0) ? s_active_tab_idx - 1 : max_tabs - 1;
        s_active_row_idx = 0;
        refresh_topbar_styles();
        build_current_submenu();
        return;
    }

    /* 4. Top Category Bar Navigation */
    if(s_active_zone == ZONE_TOP_BAR) {
        switch(key) {
            case NAV_LEFT:
                s_active_tab_idx = (s_active_tab_idx > 0) ? s_active_tab_idx - 1 : max_tabs - 1;
                s_active_row_idx = 0;
                refresh_topbar_styles();
                build_current_submenu();
                break;
            case NAV_RIGHT:
                s_active_tab_idx = (s_active_tab_idx < max_tabs - 1) ? s_active_tab_idx + 1 : 0;
                s_active_row_idx = 0;
                refresh_topbar_styles();
                build_current_submenu();
                break;
            case NAV_DOWN:
            case NAV_ENTER:
                if(s_rows_count > 0) {
                    s_active_zone = ZONE_SUBMENU;
                    s_active_row_idx = 0;
                    refresh_topbar_styles();
                    refresh_submenu_focus();
                }
                break;
            case NAV_BACK:
                if(s_close_req_cb) s_close_req_cb();
                break;
            default:
                break;
        }
        return;
    }

    /* 5. Submenu Zone Navigation (5-button operation) */
    if(s_active_zone == ZONE_SUBMENU) {
        setting_row_t * cur_row = s_rows_head;
        for(int i = 0; i < s_active_row_idx && cur_row; i++) {
            cur_row = cur_row->next;
        }

        switch(key) {
            case NAV_UP:
                if(s_active_row_idx > 0) {
                    s_active_row_idx--;
                    refresh_submenu_focus();
                } else {
                    /* Seamless transition back to top category bar */
                    s_active_zone = ZONE_TOP_BAR;
                    refresh_topbar_styles();
                    refresh_submenu_focus();
                }
                break;
            case NAV_DOWN:
                if(s_active_row_idx < s_rows_count - 1) {
                    s_active_row_idx++;
                    refresh_submenu_focus();
                }
                break;
            case NAV_LEFT:
                if(cur_row && cur_row->change_cb) {
                    cur_row->change_cb(cur_row->user_data, -1);
                }
                break;
            case NAV_RIGHT:
                if(cur_row && cur_row->change_cb) {
                    cur_row->change_cb(cur_row->user_data, +1);
                }
                break;
            case NAV_ENTER:
                if(cur_row) {
                    if(cur_row->type == ROW_ACTION && cur_row->click_cb) {
                        cur_row->click_cb(cur_row->user_data);
                    } else if(cur_row->type == ROW_TOGGLE && cur_row->change_cb) {
                        cur_row->change_cb(cur_row->user_data, 0);
                    } else if(cur_row->change_cb) {
                        cur_row->change_cb(cur_row->user_data, +1);
                    }
                }
                break;
            case NAV_BACK:
                s_active_zone = ZONE_TOP_BAR;
                refresh_topbar_styles();
                refresh_submenu_focus();
                break;
            default:
                break;
        }
    }
}

static void on_top_tab_click(lv_event_t * e) {
    int idx = (int)(uintptr_t)lv_event_get_user_data(e);
    s_active_tab_idx = idx;
    s_active_zone = ZONE_TOP_BAR;
    s_active_row_idx = 0;
    refresh_topbar_styles();
    build_current_submenu();
}

/* GUI Initialization */
void gui_app_init(void) {
    /* Initialize styles & configuration */
    gui_styles_init();
    config_data_load_all();
    load_menu_settings();

    /* Screen root setup with transparency */
    s_scr_root = lv_screen_active();
    lv_obj_set_style_bg_opa(s_scr_root, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollable(s_scr_root, false);

    /* Background simulation / splash layer (underneath everything) */
    create_background_layer();

    /* Top Bar: spans full 1920 display width */
    s_top_bar = lv_obj_create(s_scr_root);
    lv_obj_set_size(s_top_bar, LV_PCT(100), 56);
    lv_obj_align(s_top_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_style(s_top_bar, &style_topbar, 0);
    lv_obj_set_flex_flow(s_top_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s_top_bar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollable(s_top_bar, false);

    /* Container for tab buttons: centered horizontally so scaling doesn't push edges out */
    s_top_tabs_cont = lv_obj_create(s_top_bar);
    lv_obj_set_size(s_top_tabs_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(s_top_tabs_cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(s_top_tabs_cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(s_top_tabs_cont, 0, 0);
    lv_obj_set_style_pad_all(s_top_tabs_cont, 0, 0);
    lv_obj_set_flex_flow(s_top_tabs_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s_top_tabs_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollable(s_top_tabs_cont, false);

    for(int i = 0; i < TAB_COUNT; i++) {
        lv_obj_t * btn = lv_btn_create(s_top_tabs_cont);
        s_tab_btns[i] = btn;
        lv_obj_add_style(btn, &style_top_tab, 0);

        lv_obj_t * lbl = lv_label_create(btn);
        lv_label_set_text(lbl, s_tab_titles[i]);
        lv_obj_center(lbl);

        lv_obj_add_event_cb(btn, on_top_tab_click, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        /* Hide Advanced tab by default until Konami code is unlocked */
        if(i == TAB_ADVANCED) {
            lv_obj_set_hidden(btn, true);
        }
    }

    /* Submenu Side Panel (Width: 440px, Height: 420px) */
    s_submenu_panel = lv_obj_create(s_scr_root);
    lv_obj_set_size(s_submenu_panel, 440, 420);
    lv_obj_add_style(s_submenu_panel, &style_submenu_panel, 0);
    lv_obj_set_flex_flow(s_submenu_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(s_submenu_panel, 6, 0);
    lv_obj_set_scrollbar_mode(s_submenu_panel, LV_SCROLLBAR_MODE_AUTO);

    update_submenu_layout();

    /* Modal / Toast Box */
    s_modal_box = lv_obj_create(s_scr_root);
    lv_obj_set_size(s_modal_box, 480, 220);
    lv_obj_center(s_modal_box);
    lv_obj_set_style_bg_color(s_modal_box, lv_color_hex(0x222224), 0);
    lv_obj_set_style_bg_opa(s_modal_box, LV_OPA_90, 0);
    lv_obj_set_style_border_width(s_modal_box, 2, 0);
    lv_obj_set_style_border_color(s_modal_box, gui_get_accent_color(), 0);
    lv_obj_set_style_shadow_width(s_modal_box, 30, 0);
    lv_obj_set_style_shadow_color(s_modal_box, lv_color_black(), 0);
    lv_obj_set_style_radius(s_modal_box, 10, 0);
    lv_obj_set_flex_flow(s_modal_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(s_modal_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollable(s_modal_box, false);

    s_modal_title = lv_label_create(s_modal_box);
    lv_obj_set_style_text_color(s_modal_title, gui_get_accent_color(), 0);
    lv_obj_set_style_text_font(s_modal_title, &lv_font_montserrat_18, 0);

    s_modal_msg = lv_label_create(s_modal_box);
    lv_obj_set_style_text_color(s_modal_msg, lv_color_white(), 0);
    lv_obj_set_style_text_align(s_modal_msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_ver(s_modal_msg, 12, 0);

    lv_obj_t * ok_lbl = lv_label_create(s_modal_box);
    lv_label_set_text(ok_lbl, "Press [ENTER] or [A] to Dismiss");
    lv_obj_set_style_text_color(ok_lbl, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(ok_lbl, &lv_font_montserrat_12, 0);

    lv_obj_set_hidden(s_modal_box, true);

    if(s_top_bar) lv_obj_move_foreground(s_top_bar);
    if(s_modal_box) lv_obj_move_foreground(s_modal_box);

    /* If advanced was unlocked in saved settings, show the tab button */
    if(s_advanced_unlocked && s_tab_btns[TAB_ADVANCED]) {
        lv_obj_set_hidden(s_tab_btns[TAB_ADVANCED], false);
    }

    /* Initial render */
    refresh_topbar_styles();
    build_current_submenu();
    gui_set_menu_scale(s_menu_scale);
}
