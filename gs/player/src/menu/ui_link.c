#include "gui_app.h"
#include "config_data.h"
#include "gui_styles.h"
#include <stdio.h>

static setting_row_t * s_row_channel;
static setting_row_t * s_row_width;
static setting_row_t * s_row_nack;
static setting_row_t * s_row_active_rung;
static setting_row_t * s_row_rung_mcs;
static setting_row_t * s_row_rung_bw;
static setting_row_t * s_row_rung_base;
static setting_row_t * s_row_rung_enh;

static int s_selected_rung_idx = 0;

static const char * s_chan_names[] = {
    "Auto (Adaptive)",
    "Ch 40 (5200 MHz)",
    "Ch 64 (5320 MHz)",
    "Ch 112 (5560 MHz)",
    "Ch 144 (5720 MHz)"
};
static const int s_chan_count = 5;

static void update_rung_display(void);

static void on_save_clicked(void * ud) {
    (void)ud;
    if(config_data_save_all()) {
        gui_show_toast("Link Settings Saved", "Applied to /etc/maburgs.toml & restarted maburgs!", 3000);
    } else {
        gui_show_toast("Save Failed", "Could not write /etc/maburgs.toml", 3000);
    }
}

static void on_channel_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.channel_mode += dir;
    if(g_gs_cfg.channel_mode < 0) g_gs_cfg.channel_mode = s_chan_count - 1;
    if(g_gs_cfg.channel_mode >= s_chan_count) g_gs_cfg.channel_mode = 0;
    gui_update_row_val(s_row_channel, s_chan_names[g_gs_cfg.channel_mode]);
}

static void on_width_change(void * ud, int dir) {
    (void)ud; (void)dir;
    g_gs_cfg.width = (g_gs_cfg.width == 40) ? 20 : 40;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d MHz", g_gs_cfg.width);
    gui_update_row_val(s_row_width, buf);
}

static void on_nack_change(void * ud, int dir) {
    (void)ud; (void)dir;
    g_gs_cfg.nack_enable = !g_gs_cfg.nack_enable;
    gui_update_toggle_val(s_row_nack, g_gs_cfg.nack_enable);
}

static void on_active_rung_change(void * ud, int dir) {
    (void)ud;
    s_selected_rung_idx += dir;
    if(s_selected_rung_idx < 0) s_selected_rung_idx = g_gs_cfg.num_ladders - 1;
    if(s_selected_rung_idx >= g_gs_cfg.num_ladders) s_selected_rung_idx = 0;
    update_rung_display();
}

static void on_rung_mcs_change(void * ud, int dir) {
    (void)ud;
    link_ladder_rung_t * r = &g_gs_cfg.ladder[s_selected_rung_idx];
    r->mcs += dir;
    if(r->mcs < 0) r->mcs = 0;
    if(r->mcs > 7) r->mcs = 7;
    update_rung_display();
}

static void on_rung_bw_change(void * ud, int dir) {
    (void)ud; (void)dir;
    link_ladder_rung_t * r = &g_gs_cfg.ladder[s_selected_rung_idx];
    r->bw = (r->bw == 40) ? 20 : 40;
    update_rung_display();
}

static void on_rung_base_change(void * ud, int dir) {
    (void)ud;
    link_ladder_rung_t * r = &g_gs_cfg.ladder[s_selected_rung_idx];
    r->overhead_base += (float)dir * 0.05f;
    if(r->overhead_base < 0.10f) r->overhead_base = 0.10f;
    if(r->overhead_base > 1.50f) r->overhead_base = 1.50f;
    update_rung_display();
}

static void on_rung_enh_change(void * ud, int dir) {
    (void)ud;
    link_ladder_rung_t * r = &g_gs_cfg.ladder[s_selected_rung_idx];
    r->overhead_enh += (float)dir * 0.05f;
    if(r->overhead_enh < 0.05f) r->overhead_enh = 0.05f;
    if(r->overhead_enh > 1.50f) r->overhead_enh = 1.50f;
    update_rung_display();
}

static void update_rung_display(void) {
    char buf[64];
    snprintf(buf, sizeof(buf), "Rung %d of %d", s_selected_rung_idx + 1, g_gs_cfg.num_ladders);
    gui_update_row_val(s_row_active_rung, buf);

    link_ladder_rung_t * r = &g_gs_cfg.ladder[s_selected_rung_idx];

    snprintf(buf, sizeof(buf), "MCS %d", r->mcs);
    gui_update_row_val(s_row_rung_mcs, buf);

    snprintf(buf, sizeof(buf), "%d MHz", r->bw);
    gui_update_row_val(s_row_rung_bw, buf);

    snprintf(buf, sizeof(buf), "+%.0f %%", r->overhead_base * 100.0f);
    gui_update_row_val(s_row_rung_base, buf);

    snprintf(buf, sizeof(buf), "+%.0f %%", r->overhead_enh * 100.0f);
    gui_update_row_val(s_row_rung_enh, buf);
}

void ui_build_link(lv_obj_t * parent) {
    lv_obj_t * title = lv_label_create(parent);
    lv_label_set_text(title, "Link Configuration");
    lv_obj_add_style(title, &style_title_label, 0);

    lv_obj_t * desc = lv_label_create(parent);
    lv_label_set_text(desc, "Radio channels, bandwidth, ladders & retransmission.");
    lv_obj_add_style(desc, &style_desc_label, 0);

    /* Channel */
    s_row_channel = gui_create_selector_row(parent, "Radio Channel",
                                            s_chan_names[g_gs_cfg.channel_mode],
                                            on_channel_change, NULL);

    /* Width */
    char bw_buf[32];
    snprintf(bw_buf, sizeof(bw_buf), "%d MHz", g_gs_cfg.width);
    s_row_width = gui_create_selector_row(parent, "Channel Width", bw_buf, on_width_change, NULL);

    /* NACK enable */
    s_row_nack = gui_create_toggle_row(parent, "Link NACK Retransmit",
                                       g_gs_cfg.nack_enable,
                                       on_nack_change, NULL);

    /* Section divider for Link Ladders */
    lv_obj_t * ladder_lbl = lv_label_create(parent);
    lv_label_set_text(ladder_lbl, "-- Adaptive Link Ladders --");
    lv_obj_set_style_text_color(ladder_lbl, gui_get_accent_color(), 0);
    lv_obj_set_style_pad_top(ladder_lbl, 6, 0);

    /* Rung Selector */
    s_row_active_rung = gui_create_selector_row(parent, "Selected Ladder Rung", "Rung 1 of 5", on_active_rung_change, NULL);

    /* Ladder MCS */
    s_row_rung_mcs = gui_create_selector_row(parent, "  -> Rung MCS", "MCS 0", on_rung_mcs_change, NULL);

    /* Ladder BW */
    s_row_rung_bw = gui_create_selector_row(parent, "  -> Rung Bandwidth", "40 MHz", on_rung_bw_change, NULL);

    /* Ladder Base Overhead */
    s_row_rung_base = gui_create_selector_row(parent, "  -> Base Overhead", "+50 %", on_rung_base_change, NULL);

    /* Ladder Enh Overhead */
    s_row_rung_enh = gui_create_selector_row(parent, "  -> Enh Overhead", "+25 %", on_rung_enh_change, NULL);

    update_rung_display();

    /* Save & apply action */
    gui_create_action_row(parent, "Save & Apply Changes", "Save & Restart GS", on_save_clicked, NULL);
}
