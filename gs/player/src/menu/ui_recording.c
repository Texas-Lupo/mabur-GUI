#include "gui_app.h"
#include "config_data.h"
#include "gui_styles.h"
#include <stdio.h>

static setting_row_t * s_row_air_enable;
static setting_row_t * s_row_air_bitrate;
static setting_row_t * s_row_air_fps;
static setting_row_t * s_row_air_res;
static setting_row_t * s_row_air_minfree;

static setting_row_t * s_row_gnd_enable;
static setting_row_t * s_row_gnd_target;
static setting_row_t * s_row_gnd_format;
static setting_row_t * s_row_gnd_minfree;

static const char * s_target_names[] = { "Goggles Only", "VTX Only", "Both Air & Gnd" };
static const int s_target_count = 3;

static const char * s_format_names[] = { "MP4 (H.265)", "MKV (Matroska)", "TS (MPEG-TS)" };
static const int s_format_count = 3;

static void on_air_enable(void * ud, int dir) {
    (void)ud; (void)dir;
    g_drone_cfg.record_enable = !g_drone_cfg.record_enable;
    gui_update_toggle_val(s_row_air_enable, g_drone_cfg.record_enable);
}

static void on_air_bitrate(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.record_bitrate_kbps += dir * 5000;
    if(g_drone_cfg.record_bitrate_kbps < 5000) g_drone_cfg.record_bitrate_kbps = 5000;
    if(g_drone_cfg.record_bitrate_kbps > 100000) g_drone_cfg.record_bitrate_kbps = 100000;

    char buf[32];
    snprintf(buf, sizeof(buf), "%d kbps", g_drone_cfg.record_bitrate_kbps);
    gui_update_row_val(s_row_air_bitrate, buf);
}

static void on_air_fps(void * ud, int dir) {
    (void)ud; (void)dir;
    g_drone_cfg.record_fps = (g_drone_cfg.record_fps == 60) ? 30 : 60;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d FPS", g_drone_cfg.record_fps);
    gui_update_row_val(s_row_air_fps, buf);
}

static void on_air_res(void * ud, int dir) {
    (void)ud; (void)dir;
    g_drone_cfg.record_size_idx = (g_drone_cfg.record_size_idx == 0) ? 1 : 0;
    const char * opts[] = { "1920x1080", "1280x720" };
    gui_update_row_val(s_row_air_res, opts[g_drone_cfg.record_size_idx]);
}

static void on_air_minfree(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.record_min_free_mb += dir * 128;
    if(g_drone_cfg.record_min_free_mb < 128) g_drone_cfg.record_min_free_mb = 128;
    if(g_drone_cfg.record_min_free_mb > 10240) g_drone_cfg.record_min_free_mb = 10240;

    char buf[32];
    snprintf(buf, sizeof(buf), "%d MB", g_drone_cfg.record_min_free_mb);
    gui_update_row_val(s_row_air_minfree, buf);
}

static void on_gnd_enable(void * ud, int dir) {
    (void)ud; (void)dir;
    g_gs_cfg.dvr_enable = !g_gs_cfg.dvr_enable;
    gui_update_toggle_val(s_row_gnd_enable, g_gs_cfg.dvr_enable);
}

static void on_gnd_target(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.dvr_target += dir;
    if(g_gs_cfg.dvr_target < 0) g_gs_cfg.dvr_target = s_target_count - 1;
    if(g_gs_cfg.dvr_target >= s_target_count) g_gs_cfg.dvr_target = 0;
    gui_update_row_val(s_row_gnd_target, s_target_names[g_gs_cfg.dvr_target]);
}

static void on_gnd_format(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.dvr_format += dir;
    if(g_gs_cfg.dvr_format < 0) g_gs_cfg.dvr_format = s_format_count - 1;
    if(g_gs_cfg.dvr_format >= s_format_count) g_gs_cfg.dvr_format = 0;
    gui_update_row_val(s_row_gnd_format, s_format_names[g_gs_cfg.dvr_format]);
}

static void on_gnd_minfree(void * ud, int dir) {
    (void)ud;
    g_gs_cfg.dvr_min_free_mb += dir * 256;
    if(g_gs_cfg.dvr_min_free_mb < 256) g_gs_cfg.dvr_min_free_mb = 256;
    if(g_gs_cfg.dvr_min_free_mb > 20480) g_gs_cfg.dvr_min_free_mb = 20480;

    char buf[32];
    snprintf(buf, sizeof(buf), "%d MB", g_gs_cfg.dvr_min_free_mb);
    gui_update_row_val(s_row_gnd_minfree, buf);
}

static void on_play_clip(void * ud) {
    const char * clip = (const char *)ud;
    char msg[128];
    snprintf(msg, sizeof(msg), "Simulating playback of:\n%s\nOutput routed to maburplay.", clip);
    gui_show_toast("Media Player", msg, 3000);
}

void ui_build_recordings(lv_obj_t * parent) {
    lv_obj_t * title = lv_label_create(parent);
    lv_label_set_text(title, "Recording & DVR Settings");
    lv_obj_add_style(title, &style_title_label, 0);

    lv_obj_t * desc = lv_label_create(parent);
    lv_label_set_text(desc, "Configure Air (Onboard SD) & Ground (GS DVR) recording.");
    lv_obj_add_style(desc, &style_desc_label, 0);

    /* Section: AIR RECORDING */
    lv_obj_t * air_lbl = lv_label_create(parent);
    lv_label_set_text(air_lbl, "-- Air Recording (Onboard Drone SD) --");
    lv_obj_set_style_text_color(air_lbl, gui_get_accent_color(), 0);
    lv_obj_set_style_pad_top(air_lbl, 4, 0);

    s_row_air_enable = gui_create_toggle_row(parent, "Air Recording Enable", g_drone_cfg.record_enable, on_air_enable, NULL);

    char air_br_buf[32];
    snprintf(air_br_buf, sizeof(air_br_buf), "%d kbps", g_drone_cfg.record_bitrate_kbps);
    s_row_air_bitrate = gui_create_selector_row(parent, "Air Bitrate", air_br_buf, on_air_bitrate, NULL);

    char air_fps_buf[32];
    snprintf(air_fps_buf, sizeof(air_fps_buf), "%d FPS", g_drone_cfg.record_fps);
    s_row_air_fps = gui_create_selector_row(parent, "Air Framerate", air_fps_buf, on_air_fps, NULL);

    const char * res_opts[] = { "1920x1080", "1280x720" };
    s_row_air_res = gui_create_selector_row(parent, "Air Resolution", res_opts[g_drone_cfg.record_size_idx], on_air_res, NULL);

    char air_free_buf[32];
    snprintf(air_free_buf, sizeof(air_free_buf), "%d MB", g_drone_cfg.record_min_free_mb);
    s_row_air_minfree = gui_create_selector_row(parent, "Air Min Free Space", air_free_buf, on_air_minfree, NULL);

    /* Section: GROUND DVR */
    lv_obj_t * gnd_lbl = lv_label_create(parent);
    lv_label_set_text(gnd_lbl, "-- Ground Station DVR Recording --");
    lv_obj_set_style_text_color(gnd_lbl, gui_get_accent_color(), 0);
    lv_obj_set_style_pad_top(gnd_lbl, 6, 0);

    s_row_gnd_enable = gui_create_toggle_row(parent, "Ground DVR Enable", g_gs_cfg.dvr_enable, on_gnd_enable, NULL);
    s_row_gnd_target = gui_create_selector_row(parent, "DVR Record Target", s_target_names[g_gs_cfg.dvr_target], on_gnd_target, NULL);
    s_row_gnd_format = gui_create_selector_row(parent, "Video Container", s_format_names[g_gs_cfg.dvr_format], on_gnd_format, NULL);

    char gnd_free_buf[32];
    snprintf(gnd_free_buf, sizeof(gnd_free_buf), "%d MB", g_gs_cfg.dvr_min_free_mb);
    s_row_gnd_minfree = gui_create_selector_row(parent, "Ground Min Free Space", gnd_free_buf, on_gnd_minfree, NULL);

    /* Section: RECENT CLIPS */
    lv_obj_t * clips_lbl = lv_label_create(parent);
    lv_label_set_text(clips_lbl, "-- Recent Recorded Media --");
    lv_obj_set_style_text_color(clips_lbl, gui_get_accent_color(), 0);
    lv_obj_set_style_pad_top(clips_lbl, 6, 0);

    gui_create_action_row(parent, "Flight_042.mp4 (1.4 GB)", "Play Clip", on_play_clip, (void *)"Flight_042.mp4");
    gui_create_action_row(parent, "Flight_041.mp4 (820 MB)", "Play Clip", on_play_clip, (void *)"Flight_041.mp4");
}
