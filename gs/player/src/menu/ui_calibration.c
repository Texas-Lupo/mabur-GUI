#include "gui_app.h"
#include "config_data.h"
#include "gui_styles.h"
#include <stdio.h>

static void on_start_imu_cal(void * ud) {
    (void)ud;
    gui_show_toast("IMU Calibration", "Place device on a flat surface.\nCalibrating accelerometer & gyro...\nDone! Offsets stored.", 3500);
}

static void on_start_rf_cal(void * ud) {
    (void)ud;
    gui_show_toast("Radio Power Calibration", "Running maburcal sweep...\nMeasuring rate walls on active channels...\nCalibrated walls written to offset mode.", 4000);
}

void ui_build_calibration(lv_obj_t * parent) {
    lv_obj_t * title = lv_label_create(parent);
    lv_label_set_text(title, "Hardware Calibration");
    lv_obj_add_style(title, &style_title_label, 0);

    lv_obj_t * desc = lv_label_create(parent);
    lv_label_set_text(desc, "Calibrate flight sensor gyros and RTL8812 radio transmission walls.");
    lv_obj_add_style(desc, &style_desc_label, 0);

    /* IMU */
    lv_obj_t * imu_lbl = lv_label_create(parent);
    lv_label_set_text(imu_lbl, "-- Inertial Measurement Unit --");
    lv_obj_set_style_text_color(imu_lbl, gui_get_accent_color(), 0);
    lv_obj_set_style_pad_top(imu_lbl, 4, 0);

    gui_create_action_row(parent, "Flight Controller Gyro", "Start Calibration", on_start_imu_cal, NULL);

    /* RF */
    lv_obj_t * rf_lbl = lv_label_create(parent);
    lv_label_set_text(rf_lbl, "-- RTL8812 Power Walls (maburcal) --");
    lv_obj_set_style_text_color(rf_lbl, gui_get_accent_color(), 0);
    lv_obj_set_style_pad_top(rf_lbl, 6, 0);

    gui_create_action_row(parent, "Radio Power Sweep", "Calibrate RF Walls", on_start_rf_cal, NULL);
}
