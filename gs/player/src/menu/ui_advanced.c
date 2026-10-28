#include "gui_app.h"
#include "config_data.h"
#include "gui_styles.h"
#include <stdio.h>

typedef enum {
    ADV_CAT_GS_SCAN = 0,
    ADV_CAT_GS_LINK,
    ADV_CAT_GS_HOP,
    ADV_CAT_GS_SYS,
    ADV_CAT_DRONE_RADIO,
    ADV_CAT_DRONE_VENC,
    ADV_CAT_DRONE_POWER,
    ADV_CAT_COUNT
} adv_cat_t;

static adv_cat_t s_current_cat = ADV_CAT_GS_SCAN;
static setting_row_t * s_row_cat = NULL;

static const char * s_cat_names[] = {
    "1. GS Radio & Scanner",
    "2. GS Link Adaptation",
    "3. GS Hop & Fade Verdict",
    "4. GS System & Shm Ring",
    "5. Drone Radio & FEC",
    "6. Drone Video Encoder",
    "7. Drone Low Power & AMPDU"
};

static void on_cat_change(void * ud, int dir) {
    (void)ud;
    int next = (int)s_current_cat + dir;
    if(next < 0) next = ADV_CAT_COUNT - 1;
    if(next >= ADV_CAT_COUNT) next = 0;
    s_current_cat = (adv_cat_t)next;
    gui_rebuild_submenu();
}

/* Category 1: GS Scan Callbacks */
static void on_dwell_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.scan_dwell_ms += dir * 25;
    if(g_gs_cfg.scan_dwell_ms < 50) g_gs_cfg.scan_dwell_ms = 50;
    if(g_gs_cfg.scan_dwell_ms > 1000) g_gs_cfg.scan_dwell_ms = 1000;
    char buf[32]; snprintf(buf, sizeof(buf), "%d ms", g_gs_cfg.scan_dwell_ms);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_search_ms_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.scan_search_ms += dir * 20;
    if(g_gs_cfg.scan_search_ms < 20) g_gs_cfg.scan_search_ms = 20;
    if(g_gs_cfg.scan_search_ms > 500) g_gs_cfg.scan_search_ms = 500;
    char buf[32]; snprintf(buf, sizeof(buf), "%d ms", g_gs_cfg.scan_search_ms);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_busy_dbm_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.scan_busy_dbm += dir;
    if(g_gs_cfg.scan_busy_dbm < -100) g_gs_cfg.scan_busy_dbm = -100;
    if(g_gs_cfg.scan_busy_dbm > -60) g_gs_cfg.scan_busy_dbm = -60;
    char buf[32]; snprintf(buf, sizeof(buf), "%d dBm", g_gs_cfg.scan_busy_dbm);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_blocked_pct_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.scan_blocked_pct += dir * 5;
    if(g_gs_cfg.scan_blocked_pct < 10) g_gs_cfg.scan_blocked_pct = 10;
    if(g_gs_cfg.scan_blocked_pct > 90) g_gs_cfg.scan_blocked_pct = 90;
    char buf[32]; snprintf(buf, sizeof(buf), "%d %%", g_gs_cfg.scan_blocked_pct);
    gui_update_row_val((setting_row_t*)ud, buf);
}

/* Category 2: GS Link Adaptation Callbacks */
static void on_down_util_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.link_down_util += (float)dir * 0.05f;
    if(g_gs_cfg.link_down_util < 0.10f) g_gs_cfg.link_down_util = 0.10f;
    if(g_gs_cfg.link_down_util > 0.80f) g_gs_cfg.link_down_util = 0.80f;
    char buf[32]; snprintf(buf, sizeof(buf), "%.0f %%", g_gs_cfg.link_down_util * 100.0f);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_up_util_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.link_up_util += (float)dir * 0.05f;
    if(g_gs_cfg.link_up_util < 0.05f) g_gs_cfg.link_up_util = 0.05f;
    if(g_gs_cfg.link_up_util > 0.50f) g_gs_cfg.link_up_util = 0.50f;
    char buf[32]; snprintf(buf, sizeof(buf), "%.0f %%", g_gs_cfg.link_up_util * 100.0f);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_confirm_ms_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.link_confirm_ms += dir * 25;
    if(g_gs_cfg.link_confirm_ms < 50) g_gs_cfg.link_confirm_ms = 50;
    if(g_gs_cfg.link_confirm_ms > 2000) g_gs_cfg.link_confirm_ms = 2000;
    char buf[32]; snprintf(buf, sizeof(buf), "%d ms", g_gs_cfg.link_confirm_ms);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_clean_ms_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.link_clean_ms += dir * 100;
    if(g_gs_cfg.link_clean_ms < 200) g_gs_cfg.link_clean_ms = 200;
    if(g_gs_cfg.link_clean_ms > 5000) g_gs_cfg.link_clean_ms = 5000;
    char buf[32]; snprintf(buf, sizeof(buf), "%d ms", g_gs_cfg.link_clean_ms);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_s3_demote_change(void * ud, int dir) {
    (void)dir;
    g_gs_cfg.link_s3_demote = !g_gs_cfg.link_s3_demote;
    gui_update_toggle_val((setting_row_t*)ud, g_gs_cfg.link_s3_demote);
}

/* Category 3: GS Hop & Fade Callbacks */
static void on_fade_predict_change(void * ud, int dir) {
    (void)dir;
    g_gs_cfg.fade_predict = !g_gs_cfg.fade_predict;
    gui_update_toggle_val((setting_row_t*)ud, g_gs_cfg.fade_predict);
}

static void on_fade_rssi_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.fade_rssi_db += (float)dir * 1.0f;
    if(g_gs_cfg.fade_rssi_db < 4.0f) g_gs_cfg.fade_rssi_db = 4.0f;
    if(g_gs_cfg.fade_rssi_db > 30.0f) g_gs_cfg.fade_rssi_db = 30.0f;
    char buf[32]; snprintf(buf, sizeof(buf), "%.1f dB", g_gs_cfg.fade_rssi_db);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_hop_cooldown_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.hop_cooldown_ms += dir * 500;
    if(g_gs_cfg.hop_cooldown_ms < 500) g_gs_cfg.hop_cooldown_ms = 500;
    if(g_gs_cfg.hop_cooldown_ms > 10000) g_gs_cfg.hop_cooldown_ms = 10000;
    char buf[32]; snprintf(buf, sizeof(buf), "%d ms", g_gs_cfg.hop_cooldown_ms);
    gui_update_row_val((setting_row_t*)ud, buf);
}

/* Category 4: GS System Callbacks */
static void on_au_slot_kb_change(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.au_ring_slot_kb += dir * 128;
    if(g_gs_cfg.au_ring_slot_kb < 128) g_gs_cfg.au_ring_slot_kb = 128;
    if(g_gs_cfg.au_ring_slot_kb > 2048) g_gs_cfg.au_ring_slot_kb = 2048;
    char buf[32]; snprintf(buf, sizeof(buf), "%d KB", g_gs_cfg.au_ring_slot_kb);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_debug_log_change(void * ud, int dir) {
    (void)dir;
    g_gs_cfg.debug_log_enable = !g_gs_cfg.debug_log_enable;
    gui_update_toggle_val((setting_row_t*)ud, g_gs_cfg.debug_log_enable);
}

/* Category 5: Drone Radio & FEC Callbacks */
static void on_power_mode_change(void * ud, int dir) {
    (void)dir;
    g_drone_cfg.power_mode = (g_drone_cfg.power_mode == 0) ? 1 : 0;
    gui_update_row_val((setting_row_t*)ud, (g_drone_cfg.power_mode == 0) ? "Card Default" : "Offset Walls");
}

static void on_tx_threads_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.tx_threads += dir;
    if(g_drone_cfg.tx_threads < 1) g_drone_cfg.tx_threads = 1;
    if(g_drone_cfg.tx_threads > 8) g_drone_cfg.tx_threads = 8;
    char buf[32]; snprintf(buf, sizeof(buf), "%d threads", g_drone_cfg.tx_threads);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_ldpc_change(void * ud, int dir) {
    (void)dir;
    g_drone_cfg.ldpc = !g_drone_cfg.ldpc;
    gui_update_toggle_val((setting_row_t*)ud, g_drone_cfg.ldpc);
}

static void on_fec_window_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.fec_window += dir * 4;
    if(g_drone_cfg.fec_window < 8) g_drone_cfg.fec_window = 8;
    if(g_drone_cfg.fec_window > 64) g_drone_cfg.fec_window = 64;
    char buf[32]; snprintf(buf, sizeof(buf), "%d symbols", g_drone_cfg.fec_window);
    gui_update_row_val((setting_row_t*)ud, buf);
}

/* Category 6: Drone Venc Callbacks */
static void on_max_bitrate_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.encoder_bitrate_max_kbps += dir * 1000;
    if(g_drone_cfg.encoder_bitrate_max_kbps < 5000) g_drone_cfg.encoder_bitrate_max_kbps = 5000;
    if(g_drone_cfg.encoder_bitrate_max_kbps > 50000) g_drone_cfg.encoder_bitrate_max_kbps = 50000;
    char buf[32]; snprintf(buf, sizeof(buf), "%d kbps", g_drone_cfg.encoder_bitrate_max_kbps);
    gui_update_row_val((setting_row_t*)ud, buf);
}

static void on_slices_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.venc_slices += dir;
    if(g_drone_cfg.venc_slices < 1) g_drone_cfg.venc_slices = 1;
    if(g_drone_cfg.venc_slices > 16) g_drone_cfg.venc_slices = 16;
    char buf[32]; snprintf(buf, sizeof(buf), "%d slices", g_drone_cfg.venc_slices);
    gui_update_row_val((setting_row_t*)ud, buf);
}

/* Category 7: Drone Low Power & AMPDU Callbacks */
static void on_low_power_change(void * ud, int dir) {
    (void)dir;
    g_drone_cfg.low_power_enable = !g_drone_cfg.low_power_enable;
    gui_update_toggle_val((setting_row_t*)ud, g_drone_cfg.low_power_enable);
}

static void on_ampdu_num_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.ampdu_max_num += dir;
    if(g_drone_cfg.ampdu_max_num < 0) g_drone_cfg.ampdu_max_num = 0;
    if(g_drone_cfg.ampdu_max_num > 16) g_drone_cfg.ampdu_max_num = 16;
    char buf[32]; snprintf(buf, sizeof(buf), "%d frames", g_drone_cfg.ampdu_max_num);
    gui_update_row_val((setting_row_t*)ud, buf);
}

void ui_build_advanced(lv_obj_t * parent) {
    lv_obj_t * title = lv_label_create(parent);
    lv_label_set_text(title, "ADVANCED TUNING MENU");
    lv_obj_add_style(title, &style_title_label, 0);

    lv_obj_t * desc = lv_label_create(parent);
    lv_label_set_text(desc, "Complete maburgs & mabur protocol parameters.");
    lv_obj_add_style(desc, &style_desc_label, 0);

    /* Category switcher row (Row 0) */
    s_row_cat = gui_create_selector_row(parent, "Subsystem Category",
                                        s_cat_names[s_current_cat],
                                        on_cat_change, NULL);

    char buf[64];
    setting_row_t * row = NULL;

    switch(s_current_cat) {
        case ADV_CAT_GS_SCAN: {
            snprintf(buf, sizeof(buf), "%d ms", g_gs_cfg.scan_dwell_ms);
            row = gui_create_selector_row(parent, "Scan Dwell Time", buf, on_dwell_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%d ms", g_gs_cfg.scan_search_ms);
            row = gui_create_selector_row(parent, "DISC Search Time", buf, on_search_ms_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%d dBm", g_gs_cfg.scan_busy_dbm);
            row = gui_create_selector_row(parent, "Busy Channel Threshold", buf, on_busy_dbm_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%d %%", g_gs_cfg.scan_blocked_pct);
            row = gui_create_selector_row(parent, "Blocked Air Margin", buf, on_blocked_pct_change, NULL);
            row->user_data = row;
            break;
        }
        case ADV_CAT_GS_LINK: {
            snprintf(buf, sizeof(buf), "%.0f %%", g_gs_cfg.link_down_util * 100.0f);
            row = gui_create_selector_row(parent, "Step-Down Loss Util", buf, on_down_util_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%.0f %%", g_gs_cfg.link_up_util * 100.0f);
            row = gui_create_selector_row(parent, "Step-Up Clean Util", buf, on_up_util_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%d ms", g_gs_cfg.link_confirm_ms);
            row = gui_create_selector_row(parent, "Loss Confirm Window", buf, on_confirm_ms_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%d ms", g_gs_cfg.link_clean_ms);
            row = gui_create_selector_row(parent, "Clean Confirm Window", buf, on_clean_ms_change, NULL);
            row->user_data = row;

            row = gui_create_toggle_row(parent, "S3 Enhance Layer Demote", g_gs_cfg.link_s3_demote, on_s3_demote_change, NULL);
            row->user_data = row;
            break;
        }
        case ADV_CAT_GS_HOP: {
            row = gui_create_toggle_row(parent, "Fade Prediction Downstep", g_gs_cfg.fade_predict, on_fade_predict_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%.1f dB", g_gs_cfg.fade_rssi_db);
            row = gui_create_selector_row(parent, "Fade Trigger RSSI Drop", buf, on_fade_rssi_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%d ms", g_gs_cfg.hop_cooldown_ms);
            row = gui_create_selector_row(parent, "In-Flight Hop Cooldown", buf, on_hop_cooldown_change, NULL);
            row->user_data = row;
            break;
        }
        case ADV_CAT_GS_SYS: {
            snprintf(buf, sizeof(buf), "%d KB", g_gs_cfg.au_ring_slot_kb);
            row = gui_create_selector_row(parent, "AU Shared Mem Ring Slot", buf, on_au_slot_kb_change, NULL);
            row->user_data = row;

            row = gui_create_toggle_row(parent, "Link Debug Log to Disk", g_gs_cfg.debug_log_enable, on_debug_log_change, NULL);
            row->user_data = row;
            break;
        }
        case ADV_CAT_DRONE_RADIO: {
            row = gui_create_selector_row(parent, "TX Power Mode",
                                          (g_drone_cfg.power_mode == 0) ? "Card Default" : "Offset Walls",
                                          on_power_mode_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%d threads", g_drone_cfg.tx_threads);
            row = gui_create_selector_row(parent, "Parallel USB TX Threads", buf, on_tx_threads_change, NULL);
            row->user_data = row;

            row = gui_create_toggle_row(parent, "LDPC Frame Coding", g_drone_cfg.ldpc, on_ldpc_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%d symbols", g_drone_cfg.fec_window);
            row = gui_create_selector_row(parent, "FEC Repair Window", buf, on_fec_window_change, NULL);
            row->user_data = row;
            break;
        }
        case ADV_CAT_DRONE_VENC: {
            snprintf(buf, sizeof(buf), "%d kbps", g_drone_cfg.encoder_bitrate_max_kbps);
            row = gui_create_selector_row(parent, "Max Encoder Bitrate", buf, on_max_bitrate_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%d slices", g_drone_cfg.venc_slices);
            row = gui_create_selector_row(parent, "H.265 Row Slices", buf, on_slices_change, NULL);
            row->user_data = row;
            break;
        }
        case ADV_CAT_DRONE_POWER: {
            row = gui_create_toggle_row(parent, "Disarmed Low Power Mode", g_drone_cfg.low_power_enable, on_low_power_change, NULL);
            row->user_data = row;

            snprintf(buf, sizeof(buf), "%d frames", g_drone_cfg.ampdu_max_num);
            row = gui_create_selector_row(parent, "A-MPDU Aggregation Max", buf, on_ampdu_num_change, NULL);
            row->user_data = row;
            break;
        }
        default:
            break;
    }
}
