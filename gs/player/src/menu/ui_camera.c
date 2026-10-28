#include "gui_app.h"
#include "config_data.h"
#include "gui_styles.h"
#include <stdio.h>

static setting_row_t * s_row_size;
static setting_row_t * s_row_fps;
static setting_row_t * s_row_roi_enable;
static setting_row_t * s_row_roi_thresh;
static setting_row_t * s_row_roi_qp_low;
static setting_row_t * s_row_roi_qp_norm;
static setting_row_t * s_row_roi_steps;
static setting_row_t * s_row_roi_center;

static const int s_fps_options[] = { 30, 60, 90, 100, 120 };
static const int s_fps_count = 5;

static void on_size_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.venc_size_idx += dir;
    if(g_drone_cfg.venc_size_idx < 0) g_drone_cfg.venc_size_idx = g_resolution_count - 1;
    if(g_drone_cfg.venc_size_idx >= g_resolution_count) g_drone_cfg.venc_size_idx = 0;
    gui_update_row_val(s_row_size, g_resolution_options[g_drone_cfg.venc_size_idx]);
}

static void on_fps_change(void * ud, int dir) {
    (void)ud;
    int cur_idx = 1;
    for(int i = 0; i < s_fps_count; i++) {
        if(s_fps_options[i] == g_drone_cfg.venc_fps) { cur_idx = i; break; }
    }
    cur_idx += dir;
    if(cur_idx < 0) cur_idx = s_fps_count - 1;
    if(cur_idx >= s_fps_count) cur_idx = 0;
    g_drone_cfg.venc_fps = s_fps_options[cur_idx];

    char buf[32];
    snprintf(buf, sizeof(buf), "%d FPS", g_drone_cfg.venc_fps);
    gui_update_row_val(s_row_fps, buf);
}

static void on_roi_enable_change(void * ud, int dir) {
    (void)ud; (void)dir;
    g_drone_cfg.venc_roi_enabled = !g_drone_cfg.venc_roi_enabled;
    gui_update_toggle_val(s_row_roi_enable, g_drone_cfg.venc_roi_enabled);
}

static void on_roi_thresh_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.encoder_roi_threshold_kbps += dir * 500;
    if(g_drone_cfg.encoder_roi_threshold_kbps < 500) g_drone_cfg.encoder_roi_threshold_kbps = 500;
    if(g_drone_cfg.encoder_roi_threshold_kbps > 15000) g_drone_cfg.encoder_roi_threshold_kbps = 15000;

    char buf[32];
    snprintf(buf, sizeof(buf), "%d kbps", g_drone_cfg.encoder_roi_threshold_kbps);
    gui_update_row_val(s_row_roi_thresh, buf);
}

static void on_roi_qp_low_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.encoder_roi_qp_low += dir;
    if(g_drone_cfg.encoder_roi_qp_low < -35) g_drone_cfg.encoder_roi_qp_low = -35;
    if(g_drone_cfg.encoder_roi_qp_low > 0) g_drone_cfg.encoder_roi_qp_low = 0;

    char buf[32];
    snprintf(buf, sizeof(buf), "%d", g_drone_cfg.encoder_roi_qp_low);
    gui_update_row_val(s_row_roi_qp_low, buf);
}

static void on_roi_qp_norm_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.encoder_roi_qp_normal += dir;
    if(g_drone_cfg.encoder_roi_qp_normal < -15) g_drone_cfg.encoder_roi_qp_normal = -15;
    if(g_drone_cfg.encoder_roi_qp_normal > 15) g_drone_cfg.encoder_roi_qp_normal = 15;

    char buf[32];
    snprintf(buf, sizeof(buf), "%d", g_drone_cfg.encoder_roi_qp_normal);
    gui_update_row_val(s_row_roi_qp_norm, buf);
}

static void on_roi_steps_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.venc_roi_steps += dir;
    if(g_drone_cfg.venc_roi_steps < 1) g_drone_cfg.venc_roi_steps = 1;
    if(g_drone_cfg.venc_roi_steps > 8) g_drone_cfg.venc_roi_steps = 8;

    char buf[32];
    snprintf(buf, sizeof(buf), "%d", g_drone_cfg.venc_roi_steps);
    gui_update_row_val(s_row_roi_steps, buf);
}

static void on_roi_center_change(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.venc_roi_center += (float)dir * 0.05f;
    if(g_drone_cfg.venc_roi_center < 0.1f) g_drone_cfg.venc_roi_center = 0.1f;
    if(g_drone_cfg.venc_roi_center > 1.0f) g_drone_cfg.venc_roi_center = 1.0f;

    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f", g_drone_cfg.venc_roi_center);
    gui_update_row_val(s_row_roi_center, buf);
}

void ui_build_camera(lv_obj_t * parent) {
    lv_obj_t * title = lv_label_create(parent);
    lv_label_set_text(title, "Camera & Video Settings");
    lv_obj_add_style(title, &style_title_label, 0);

    lv_obj_t * desc = lv_label_create(parent);
    lv_label_set_text(desc, "Resolution, framerate and Region of Interest tuning.");
    lv_obj_add_style(desc, &style_desc_label, 0);

    /* Resolution row */
    s_row_size = gui_create_selector_row(parent, "Resolution",
                                          g_resolution_options[g_drone_cfg.venc_size_idx],
                                          on_size_change, NULL);

    /* FPS row */
    char fps_buf[32];
    snprintf(fps_buf, sizeof(fps_buf), "%d FPS", g_drone_cfg.venc_fps);
    s_row_fps = gui_create_selector_row(parent, "Framerate", fps_buf, on_fps_change, NULL);

    /* ROI Enable */
    s_row_roi_enable = gui_create_toggle_row(parent, "ROI (Region of Interest)",
                                             g_drone_cfg.venc_roi_enabled,
                                             on_roi_enable_change, NULL);

    /* ROI Threshold */
    char thresh_buf[32];
    snprintf(thresh_buf, sizeof(thresh_buf), "%d kbps", g_drone_cfg.encoder_roi_threshold_kbps);
    s_row_roi_thresh = gui_create_selector_row(parent, "ROI Threshold", thresh_buf, on_roi_thresh_change, NULL);

    /* ROI QP Low */
    char qp_low_buf[32];
    snprintf(qp_low_buf, sizeof(qp_low_buf), "%d", g_drone_cfg.encoder_roi_qp_low);
    s_row_roi_qp_low = gui_create_selector_row(parent, "ROI QP Low", qp_low_buf, on_roi_qp_low_change, NULL);

    /* ROI QP Normal */
    char qp_norm_buf[32];
    snprintf(qp_norm_buf, sizeof(qp_norm_buf), "%d", g_drone_cfg.encoder_roi_qp_normal);
    s_row_roi_qp_norm = gui_create_selector_row(parent, "ROI QP Normal", qp_norm_buf, on_roi_qp_norm_change, NULL);

    /* ROI Steps */
    char steps_buf[32];
    snprintf(steps_buf, sizeof(steps_buf), "%d", g_drone_cfg.venc_roi_steps);
    s_row_roi_steps = gui_create_selector_row(parent, "ROI Steps", steps_buf, on_roi_steps_change, NULL);

    /* ROI Center */
    char center_buf[32];
    snprintf(center_buf, sizeof(center_buf), "%.2f", g_drone_cfg.venc_roi_center);
    s_row_roi_center = gui_create_selector_row(parent, "ROI Center", center_buf, on_roi_center_change, NULL);
}
