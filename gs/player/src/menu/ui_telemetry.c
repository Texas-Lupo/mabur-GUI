#include "gui_app.h"
#include "config_data.h"
#include "gui_styles.h"
#include <stdio.h>

static setting_row_t * s_row_msp_enable;
static setting_row_t * s_row_msp_serial;
static setting_row_t * s_row_msp_baud;
static setting_row_t * s_row_msp_rate;

static const int s_baud_options[] = { 9600, 57600, 115200, 230400, 460800, 921600 };
static const int s_baud_count = 6;

static void on_msp_enable(void * ud, int dir) {
    (void)ud; (void)dir;
    g_drone_cfg.msp_enable = !g_drone_cfg.msp_enable;
    g_gs_cfg.msp_enable = g_drone_cfg.msp_enable;
    gui_update_toggle_val(s_row_msp_enable, g_drone_cfg.msp_enable);
}

static void on_msp_serial(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.msp_serial_idx += dir;
    if(g_drone_cfg.msp_serial_idx < 0) g_drone_cfg.msp_serial_idx = g_serial_count - 1;
    if(g_drone_cfg.msp_serial_idx >= g_serial_count) g_drone_cfg.msp_serial_idx = 0;
    gui_update_row_val(s_row_msp_serial, g_serial_options[g_drone_cfg.msp_serial_idx]);
}

static void on_msp_baud(void * ud, int dir) {
    (void)ud;
    int cur_idx = 2;
    for(int i = 0; i < s_baud_count; i++) {
        if(s_baud_options[i] == g_drone_cfg.msp_baud) { cur_idx = i; break; }
    }
    cur_idx += dir;
    if(cur_idx < 0) cur_idx = s_baud_count - 1;
    if(cur_idx >= s_baud_count) cur_idx = 0;
    g_drone_cfg.msp_baud = s_baud_options[cur_idx];

    char buf[32];
    snprintf(buf, sizeof(buf), "%d baud", g_drone_cfg.msp_baud);
    gui_update_row_val(s_row_msp_baud, buf);
}

static void on_msp_rate(void * ud, int dir) {
    (void)ud;
    g_drone_cfg.msp_update_rate_hz += dir;
    if(g_drone_cfg.msp_update_rate_hz < 1) g_drone_cfg.msp_update_rate_hz = 1;
    if(g_drone_cfg.msp_update_rate_hz > 20) g_drone_cfg.msp_update_rate_hz = 20;

    char buf[32];
    snprintf(buf, sizeof(buf), "%d Hz", g_drone_cfg.msp_update_rate_hz);
    gui_update_row_val(s_row_msp_rate, buf);
}

void ui_build_telemetry(lv_obj_t * parent) {
    lv_obj_t * title = lv_label_create(parent);
    lv_label_set_text(title, "Flight Controller & MSP OSD");
    lv_obj_add_style(title, &style_title_label, 0);

    lv_obj_t * desc = lv_label_create(parent);
    lv_label_set_text(desc, "UART connection to Betaflight / ArduPilot / INAV.");
    lv_obj_add_style(desc, &style_desc_label, 0);

    /* MSP Enable */
    s_row_msp_enable = gui_create_toggle_row(parent, "MSP Protocol Link",
                                             g_drone_cfg.msp_enable,
                                             on_msp_enable, NULL);

    /* Serial Port Selection: /dev/ttyS0 to /dev/ttyS3 */
    s_row_msp_serial = gui_create_selector_row(parent, "Flight Controller Serial UART",
                                               g_serial_options[g_drone_cfg.msp_serial_idx],
                                               on_msp_serial, NULL);

    /* Baud Rate */
    char baud_buf[32];
    snprintf(baud_buf, sizeof(baud_buf), "%d baud", g_drone_cfg.msp_baud);
    s_row_msp_baud = gui_create_selector_row(parent, "Baud Rate", baud_buf, on_msp_baud, NULL);

    /* Update Rate */
    char rate_buf[32];
    snprintf(rate_buf, sizeof(rate_buf), "%d Hz", g_drone_cfg.msp_update_rate_hz);
    s_row_msp_rate = gui_create_selector_row(parent, "OSD Update Rate", rate_buf, on_msp_rate, NULL);
}
