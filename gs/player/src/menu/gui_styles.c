#include "gui_styles.h"

const theme_color_t g_theme_colors[] = {
    { "Xbox Green",   { .red = 0x10, .green = 0x7c, .blue = 0x10 }, { .red = 0x10, .green = 0x7c, .blue = 0x10 } },
    { "Azure Blue",   { .red = 0x00, .green = 0x78, .blue = 0xd7 }, { .red = 0x00, .green = 0x78, .blue = 0xd7 } },
    { "Crimson Red",  { .red = 0xe8, .green = 0x11, .blue = 0x23 }, { .red = 0xe8, .green = 0x11, .blue = 0x23 } },
    { "Amethyst",     { .red = 0x88, .green = 0x17, .blue = 0x98 }, { .red = 0x88, .green = 0x17, .blue = 0x98 } },
    { "Gold",         { .red = 0xff, .green = 0xb9, .blue = 0x00 }, { .red = 0xff, .green = 0xb9, .blue = 0x00 } },
    { "Cyber Cyan",   { .red = 0x00, .green = 0xe5, .blue = 0xff }, { .red = 0x00, .green = 0xe5, .blue = 0xff } }
};
const int g_theme_colors_count = sizeof(g_theme_colors) / sizeof(g_theme_colors[0]);
int g_current_theme_idx = 0;

lv_style_t style_topbar;
lv_style_t style_top_tab;
lv_style_t style_top_tab_focused;
lv_style_t style_top_tab_active;

lv_style_t style_submenu_panel;
lv_style_t style_item_row;
lv_style_t style_item_row_focused;

lv_style_t style_title_label;
lv_style_t style_desc_label;
lv_style_t style_val_arrow;
lv_style_t style_val_text;

lv_style_t style_btn_action;
lv_style_t style_btn_action_focused;

lv_style_t style_switch;
lv_style_t style_switch_checked;
lv_style_t style_switch_knob;
lv_style_t style_slider;
lv_style_t style_slider_knob;

static bool s_inited = false;

lv_color_t gui_get_accent_color(void) {
    return g_theme_colors[g_current_theme_idx].color;
}

lv_color_t gui_get_accent_glow(void) {
    return g_theme_colors[g_current_theme_idx].glow;
}

void gui_styles_init(void) {
    if(s_inited) return;

    /* Top bar: Sleek dark translucent bar across the top */
    lv_style_init(&style_topbar);
    lv_style_set_bg_color(&style_topbar, lv_color_hex(0x121213));
    lv_style_set_bg_opa(&style_topbar, LV_OPA_90);
    lv_style_set_border_width(&style_topbar, 0);
    lv_style_set_border_color(&style_topbar, lv_color_hex(0x333333));
    lv_style_set_border_side(&style_topbar, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_radius(&style_topbar, 0);
    lv_style_set_pad_all(&style_topbar, 6);

    /* Top tab default */
    lv_style_init(&style_top_tab);
    lv_style_set_bg_color(&style_top_tab, lv_color_hex(0x1e1e1f));
    lv_style_set_bg_opa(&style_top_tab, LV_OPA_0);
    lv_style_set_text_color(&style_top_tab, lv_color_hex(0xaaaaaa));
    lv_style_set_border_width(&style_top_tab, 2);
    lv_style_set_border_color(&style_top_tab, lv_color_hex(0x000000));
    lv_style_set_border_opa(&style_top_tab, LV_OPA_0);
    lv_style_set_radius(&style_top_tab, 4);
    lv_style_set_pad_hor(&style_top_tab, 12);
    lv_style_set_pad_ver(&style_top_tab, 8);

    /* Top tab active */
    lv_style_init(&style_top_tab_active);
    lv_style_set_bg_color(&style_top_tab_active, lv_color_hex(0x2d2d30));
    lv_style_set_bg_opa(&style_top_tab_active, LV_OPA_60);
    lv_style_set_text_color(&style_top_tab_active, lv_color_white());
    lv_style_set_border_side(&style_top_tab_active, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_width(&style_top_tab_active, 3);
    lv_style_set_border_opa(&style_top_tab_active, LV_OPA_COVER);
    lv_style_set_border_color(&style_top_tab_active, gui_get_accent_color());

    /* Top tab focused */
    lv_style_init(&style_top_tab_focused);
    lv_style_set_bg_color(&style_top_tab_focused, lv_color_hex(0x353538));
    lv_style_set_bg_opa(&style_top_tab_focused, LV_OPA_90);
    lv_style_set_text_color(&style_top_tab_focused, lv_color_white());
    lv_style_set_border_width(&style_top_tab_focused, 2);
    lv_style_set_border_side(&style_top_tab_focused, LV_BORDER_SIDE_FULL);
    lv_style_set_border_color(&style_top_tab_focused, gui_get_accent_color());
    lv_style_set_shadow_width(&style_top_tab_focused, 14);
    lv_style_set_shadow_color(&style_top_tab_focused, gui_get_accent_glow());
    lv_style_set_shadow_opa(&style_top_tab_focused, LV_OPA_70);

    /* Submenu panel (Side menu): Solid/Translucent dark panel */
    lv_style_init(&style_submenu_panel);
    lv_style_set_bg_color(&style_submenu_panel, lv_color_hex(0x1a1a1c));
    lv_style_set_bg_opa(&style_submenu_panel, LV_OPA_90);
    lv_style_set_border_width(&style_submenu_panel, 1);
    lv_style_set_border_color(&style_submenu_panel, lv_color_hex(0x38383a));
    lv_style_set_radius(&style_submenu_panel, 8);
    lv_style_set_pad_all(&style_submenu_panel, 14);
    lv_style_set_shadow_width(&style_submenu_panel, 20);
    lv_style_set_shadow_color(&style_submenu_panel, lv_color_black());
    lv_style_set_shadow_opa(&style_submenu_panel, LV_OPA_70);

    /* Interactive setting item row */
    lv_style_init(&style_item_row);
    lv_style_set_bg_color(&style_item_row, lv_color_hex(0x2a2a2c));
    lv_style_set_bg_opa(&style_item_row, LV_OPA_80);
    lv_style_set_border_width(&style_item_row, 2);
    lv_style_set_border_color(&style_item_row, lv_color_hex(0x38383a));
    lv_style_set_radius(&style_item_row, 6);
    lv_style_set_pad_hor(&style_item_row, 12);
    lv_style_set_pad_ver(&style_item_row, 8);
    lv_style_set_text_color(&style_item_row, lv_color_white());

    /* Item row focused with Xbox glow - background is same grey color as when item is not selected */
    lv_style_init(&style_item_row_focused);
    lv_style_set_bg_color(&style_item_row_focused, lv_color_hex(0x2a2a2c));
    lv_style_set_bg_opa(&style_item_row_focused, LV_OPA_80);
    lv_style_set_border_width(&style_item_row_focused, 2);
    lv_style_set_border_color(&style_item_row_focused, gui_get_accent_color());
    lv_style_set_shadow_width(&style_item_row_focused, 16);
    lv_style_set_shadow_color(&style_item_row_focused, gui_get_accent_glow());
    lv_style_set_shadow_opa(&style_item_row_focused, LV_OPA_80);

    /* Text styles */
    lv_style_init(&style_title_label);
    lv_style_set_text_color(&style_title_label, lv_color_white());
    lv_style_set_text_font(&style_title_label, &lv_font_montserrat_18);

    lv_style_init(&style_desc_label);
    lv_style_set_text_color(&style_desc_label, lv_color_hex(0x999999));
    lv_style_set_text_font(&style_desc_label, &lv_font_montserrat_12);

    lv_style_init(&style_val_arrow);
    lv_style_set_text_color(&style_val_arrow, gui_get_accent_color());
    lv_style_set_text_font(&style_val_arrow, &lv_font_montserrat_14);

    lv_style_init(&style_val_text);
    lv_style_set_text_color(&style_val_text, lv_color_white());
    lv_style_set_text_font(&style_val_text, &lv_font_montserrat_14);

    /* Buttons: same color as Accent color */
    lv_style_init(&style_btn_action);
    lv_style_set_bg_color(&style_btn_action, gui_get_accent_color());
    lv_style_set_bg_opa(&style_btn_action, LV_OPA_COVER);
    lv_style_set_radius(&style_btn_action, 6);
    lv_style_set_border_width(&style_btn_action, 0);
    lv_style_set_text_color(&style_btn_action, lv_color_white());
    lv_style_set_pad_hor(&style_btn_action, 14);
    lv_style_set_pad_ver(&style_btn_action, 8);

    lv_style_init(&style_btn_action_focused);
    lv_style_set_bg_color(&style_btn_action_focused, gui_get_accent_color());
    lv_style_set_border_width(&style_btn_action_focused, 2);
    lv_style_set_border_color(&style_btn_action_focused, lv_color_white());
    lv_style_set_shadow_width(&style_btn_action_focused, 16);
    lv_style_set_shadow_color(&style_btn_action_focused, gui_get_accent_glow());
    lv_style_set_shadow_opa(&style_btn_action_focused, LV_OPA_90);

    /* Toggle switch / slider default (OFF state: grey track like when cursor is on item) */
    lv_style_init(&style_switch);
    lv_style_set_bg_color(&style_switch, lv_color_hex(0x38383a));
    lv_style_set_bg_opa(&style_switch, LV_OPA_COVER);
    lv_style_set_radius(&style_switch, LV_RADIUS_CIRCLE);

    /* Toggle switch / slider ON state: Accent color */
    lv_style_init(&style_switch_checked);
    lv_style_set_bg_color(&style_switch_checked, gui_get_accent_color());
    lv_style_set_bg_opa(&style_switch_checked, LV_OPA_COVER);
    lv_style_set_radius(&style_switch_checked, LV_RADIUS_CIRCLE);

    /* Toggle switch knob */
    lv_style_init(&style_switch_knob);
    lv_style_set_bg_color(&style_switch_knob, lv_color_white());
    lv_style_set_bg_opa(&style_switch_knob, LV_OPA_COVER);
    lv_style_set_radius(&style_switch_knob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&style_switch_knob, -1);

    s_inited = true;
}

void gui_styles_set_theme(int theme_idx) {
    if(theme_idx < 0 || theme_idx >= g_theme_colors_count) return;
    g_current_theme_idx = theme_idx;

    lv_color_t acc = gui_get_accent_color();
    lv_color_t glow = gui_get_accent_glow();

    lv_style_set_border_color(&style_top_tab_active, acc);
    lv_style_set_border_color(&style_top_tab_focused, acc);
    lv_style_set_shadow_color(&style_top_tab_focused, glow);

    lv_style_set_border_color(&style_item_row_focused, acc);
    lv_style_set_shadow_color(&style_item_row_focused, glow);

    lv_style_set_text_color(&style_val_arrow, acc);

    lv_style_set_bg_color(&style_btn_action, acc);
    lv_style_set_bg_color(&style_btn_action_focused, acc);
    lv_style_set_shadow_color(&style_btn_action_focused, glow);

    lv_style_set_bg_color(&style_switch_checked, acc);
}
