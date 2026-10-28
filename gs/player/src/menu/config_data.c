#include "config_data.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

gs_config_t g_gs_cfg;
drone_config_t g_drone_cfg;

const char * const g_resolution_options[] = {
    "1920x1080",
    "1728x972",
    "1728x816",
    "1280x720"
};
const int g_resolution_count = 4;

const char * const g_serial_options[] = {
    "/dev/ttyS0",
    "/dev/ttyS1",
    "/dev/ttyS2",
    "/dev/ttyS3"
};
const int g_serial_count = 4;

void config_data_init_defaults(void) {
    /* Initialize Ground Station Configuration from maburgs.default.toml */
    memset(&g_gs_cfg, 0, sizeof(g_gs_cfg));
    
    g_gs_cfg.channels[0] = 40;
    g_gs_cfg.channels[1] = 64;
    g_gs_cfg.channels[2] = 112;
    g_gs_cfg.channels[3] = 144;
    g_gs_cfg.num_channels = 4;
    g_gs_cfg.channel_mode = 0; /* "auto" */
    g_gs_cfg.width = 40;
    g_gs_cfg.tx_card = -1;

    g_gs_cfg.scan_dwell_ms = 250;
    g_gs_cfg.scan_settle_ms = 30;
    g_gs_cfg.scan_min_rounds = 3;
    g_gs_cfg.scan_search_ms = 100;
    g_gs_cfg.scan_op_window_ms = 300;
    g_gs_cfg.scan_search_after_ms = 5000;
    g_gs_cfg.scan_pick_margin = 20;
    g_gs_cfg.scan_one_card_ms = 5000;
    g_gs_cfg.scan_max_ms = 30000;
    g_gs_cfg.scan_busy_dbm = -83;
    g_gs_cfg.scan_blocked_pct = 50;

    g_gs_cfg.fec_symbol_size = 332;
    g_gs_cfg.fec_seq_horizon = 512;

    g_gs_cfg.link_feedback_ms = 50;
    g_gs_cfg.link_rcf_slot_hold_ms = 30;
    g_gs_cfg.link_arrival_guard_syms = 192;
    g_gs_cfg.link_beacon_keepalive_ms = 1000;
    g_gs_cfg.link_static_mcs = -1;
    g_gs_cfg.link_static_bw = 20;
    g_gs_cfg.link_max_mcs = 5;
    strncpy(g_gs_cfg.link_key_file, "/etc/mabur.key", sizeof(g_gs_cfg.link_key_file) - 1);
    g_gs_cfg.link_down_util = 0.35f;
    g_gs_cfg.link_up_util = 0.15f;
    g_gs_cfg.link_confirm_ms = 250;
    g_gs_cfg.link_clean_ms = 1500;
    g_gs_cfg.link_probation_ms = 3000;
    g_gs_cfg.link_penalty_base_ms = 5000;
    g_gs_cfg.link_penalty_max_ms = 10000;
    g_gs_cfg.link_hold_after_down_ms = 0;
    g_gs_cfg.link_min_between_changes_ms = 150;
    g_gs_cfg.link_feedback_timeout_ms = 1000;
    g_gs_cfg.link_starved_confirm_ms = 300;
    g_gs_cfg.link_s3_demote = true;
    g_gs_cfg.link_s3_down_util = 0.15f;
    g_gs_cfg.link_s3_min_syms = 50;
    g_gs_cfg.link_s3_settle_ms = 300;
    g_gs_cfg.link_static_overhead_base = 0.5f;
    g_gs_cfg.link_static_overhead_enh = 0.5f;

    /* [[link.ladder]] - 5 rungs from maburgs.default.toml */
    g_gs_cfg.num_ladders = 5;
    for(int i = 0; i < 5; i++) {
        g_gs_cfg.ladder[i].mcs = i;
        g_gs_cfg.ladder[i].bw = 40;
        g_gs_cfg.ladder[i].overhead_base = 0.5f;
        g_gs_cfg.ladder[i].overhead_enh = 0.25f;
    }

    g_gs_cfg.probe_enable = true;
    g_gs_cfg.probe_rung_offset = 1;
    g_gs_cfg.probe_max_util = 0.05f;
    g_gs_cfg.probe_clean_bodies = 90;
    g_gs_cfg.probe_min_syms = 16;
    g_gs_cfg.probe_silence_ms = 500;
    g_gs_cfg.probe_pin_mcs = -1;

    g_gs_cfg.fade_cascade = true;
    g_gs_cfg.fade_predict = true;
    g_gs_cfg.fade_hold_ms = 2500;
    g_gs_cfg.fade_confirm_ms = 100;
    g_gs_cfg.fade_rssi_db = 14.0f;
    g_gs_cfg.fade_snr_db = 6.0f;
    g_gs_cfg.fade_trigger_ms = 300;
    g_gs_cfg.fade_min_rung = 2;

    g_gs_cfg.nack_enable = true;
    g_gs_cfg.nack_lookback = 256;
    g_gs_cfg.nack_repeat_ms = 16;
    g_gs_cfg.nack_max_tries = 2;
    g_gs_cfg.nack_min_lead_ms = 12;

    g_gs_cfg.rung_stats_half_life_samples = 600;

    g_gs_cfg.video_frame_gap_timeout_ms = 50;
    g_gs_cfg.video_frame_gap_timeout_max_ms = 100;
    g_gs_cfg.video_frame_lookahead = 8;

    g_gs_cfg.msp_enable = true;
    g_gs_cfg.msp_symbol_size = 1312;
    g_gs_cfg.msp_window = 16;
    strncpy(g_gs_cfg.msp_out_host, "127.0.0.1", sizeof(g_gs_cfg.msp_out_host) - 1);
    g_gs_cfg.msp_out_port = 14560;

    g_gs_cfg.stats_enable = true;
    g_gs_cfg.stats_interval_ms = 200;

    g_gs_cfg.au_ring_enable = true;
    strncpy(g_gs_cfg.au_ring_path, "/dev/shm/mabur-au", sizeof(g_gs_cfg.au_ring_path) - 1);
    strncpy(g_gs_cfg.au_ring_socket, "/run/mabur-au.sock", sizeof(g_gs_cfg.au_ring_socket) - 1);
    g_gs_cfg.au_ring_slot_kb = 512;
    g_gs_cfg.au_ring_slot_count = 16;

    g_gs_cfg.debug_log_enable = false;
    strncpy(g_gs_cfg.debug_log_dir, "/media/dvr/log", sizeof(g_gs_cfg.debug_log_dir) - 1);
    g_gs_cfg.debug_log_ctl_period_ms = 50;
    g_gs_cfg.debug_log_rung_period_s = 10;

    g_gs_cfg.hop_window_ms = 150;
    g_gs_cfg.hop_persist = 2;
    g_gs_cfg.hop_dwell_observe_ms = 5;
    g_gs_cfg.hop_dwell_period_ms = 333;
    g_gs_cfg.hop_relay_burst_period_ms = 1000;
    g_gs_cfg.hop_rank_visits = 5;
    g_gs_cfg.hop_rank_max_age_ms = 10000;
    g_gs_cfg.hop_confirm_ms = 1000;
    g_gs_cfg.hop_confirm_extend_ms = 3000;
    g_gs_cfg.hop_verify_ms = 1000;
    g_gs_cfg.hop_cooldown_ms = 2000;
    g_gs_cfg.hop_max_hops_per_min = 4;
    g_gs_cfg.hop_backoff_ms = 30000;
    g_gs_cfg.hop_one_card_repeats = 5;

    g_gs_cfg.verdict_loss_pct = 3.0f;
    g_gs_cfg.verdict_recovered_x = 3.0f;
    g_gs_cfg.verdict_recovered_min = 8;
    g_gs_cfg.verdict_weak_rssi_dbm = -78;
    g_gs_cfg.verdict_weak_snr_db = 12;
    g_gs_cfg.verdict_fading_drop_db = 6;
    g_gs_cfg.verdict_foreign_pps = 50;
    g_gs_cfg.verdict_fa_pps = 100;
    g_gs_cfg.verdict_starved_frac = 0.25f;

    /* Ground DVR defaults */
    g_gs_cfg.dvr_enable = true;
    g_gs_cfg.dvr_target = 0; /* "goggles" */
    strncpy(g_gs_cfg.dvr_dir, "/media/dvr", sizeof(g_gs_cfg.dvr_dir) - 1);
    g_gs_cfg.dvr_min_free_mb = 1024;
    g_gs_cfg.dvr_format = 0; /* "MP4" */

    /* Initialize Drone Configuration from mabur.default.toml */
    memset(&g_drone_cfg, 0, sizeof(g_drone_cfg));

    g_drone_cfg.usb_vid = 3034;
    g_drone_cfg.usb_pid = 0;
    g_drone_cfg.channels[0] = 40;
    g_drone_cfg.channels[1] = 64;
    g_drone_cfg.channels[2] = 112;
    g_drone_cfg.channels[3] = 144;
    g_drone_cfg.num_channels = 4;
    g_drone_cfg.width = 40;
    g_drone_cfg.power_mode = 0; /* "none" */
    g_drone_cfg.tx_threads = 4;
    g_drone_cfg.ldpc = true;

    int default_walls[8] = {63, 63, 62, 45, 22, 6, 9, 5};
    memcpy(g_drone_cfg.rate_walls_rel, default_walls, sizeof(default_walls));
    g_drone_cfg.legacy_wall_rel = 63;
    g_drone_cfg.wall_margin_db = 1.0f;

    g_drone_cfg.fec_symbol_size = 332;
    g_drone_cfg.fec_window = 32;
    g_drone_cfg.fec_blocks_base = 4;
    g_drone_cfg.fec_blocks_enh = 4;
    g_drone_cfg.fec_base_overhead = 0.5f;
    g_drone_cfg.fec_feed_batch = 1;
    g_drone_cfg.fec_flush_ms = 25;

    g_drone_cfg.encoder_bitrate_min_kbps = 1000;
    g_drone_cfg.encoder_bitrate_max_kbps = 24000;
    g_drone_cfg.encoder_airtime_budget = 0.65f;
    g_drone_cfg.encoder_roi_threshold_kbps = 3000;
    g_drone_cfg.encoder_roi_qp_low = -24;
    g_drone_cfg.encoder_roi_qp_normal = 0;

    strncpy(g_drone_cfg.venc_sensor_bin, "/etc/sensors/imx415_greg_fpvXIX_colortrans.bin", sizeof(g_drone_cfg.venc_sensor_bin) - 1);
    g_drone_cfg.venc_size_idx = 0; /* "1920x1080" */
    g_drone_cfg.venc_fps = 60;
    g_drone_cfg.venc_gop_s = 0;
    g_drone_cfg.venc_qp_delta = 4;
    g_drone_cfg.venc_max_ipprop = 2;
    g_drone_cfg.venc_min_iqp = 34;
    g_drone_cfg.venc_intra_refresh_frames = 0;
    g_drone_cfg.venc_intra_refresh_qp = 36;
    g_drone_cfg.venc_ref_base = 1;
    g_drone_cfg.venc_ref_enhance = 1;
    g_drone_cfg.venc_ref_pred = true;
    g_drone_cfg.venc_slices = 4;
    g_drone_cfg.venc_ae_fps = 15;
    g_drone_cfg.venc_awb_fps = 15;
    g_drone_cfg.venc_snapshot_quality = 80;
    g_drone_cfg.venc_debug_port = 8301;
    g_drone_cfg.venc_superframe_p_pct = 200;

    g_drone_cfg.venc_roi_enabled = false;
    g_drone_cfg.venc_roi_steps = 2;
    g_drone_cfg.venc_roi_center = 0.4f;

    g_drone_cfg.link_failsafe_ms = 3000;
    g_drone_cfg.link_rc_drain_ms = 5;
    g_drone_cfg.link_rendezvous_ms = 30000;
    g_drone_cfg.link_move_confirm_ms = 2000;
    g_drone_cfg.link_tick_ms = 100;
    strncpy(g_drone_cfg.link_key_file, "/etc/mabur.key", sizeof(g_drone_cfg.link_key_file) - 1);

    g_drone_cfg.msp_enable = true;
    g_drone_cfg.msp_serial_idx = 2; /* "/dev/ttyS2" */
    g_drone_cfg.msp_baud = 115200;
    g_drone_cfg.msp_update_rate_hz = 3;
    g_drone_cfg.msp_symbol_size = 1312;
    g_drone_cfg.msp_window = 16;
    g_drone_cfg.msp_overhead = 1.0f;

    g_drone_cfg.low_power_enable = false;
    g_drone_cfg.low_power_bitrate_kbps = 1000;
    g_drone_cfg.low_power_fps = 30;
    g_drone_cfg.low_power_stale_ms = 2000;

    g_drone_cfg.ampdu_max_num = 6;
    g_drone_cfg.ampdu_max_time = 32;
    g_drone_cfg.ampdu_min_mcs_20 = 4;
    g_drone_cfg.ampdu_min_mcs_40 = 2;

    g_drone_cfg.air_clock_shed_ms = 25;
    float eff20[8] = {0.93f, 0.88f, 0.84f, 0.80f, 0.76f, 0.76f, 0.78f, 0.76f};
    float eff40[8] = {0.77f, 0.75f, 0.75f, 0.75f, 0.74f, 0.77f, 0.76f, 0.75f};
    memcpy(g_drone_cfg.air_clock_eff_20, eff20, sizeof(eff20));
    memcpy(g_drone_cfg.air_clock_eff_40, eff40, sizeof(eff40));
    g_drone_cfg.air_clock_body_us = 0;

    /* [record] - Air recording */
    g_drone_cfg.record_enable = true;
    strncpy(g_drone_cfg.record_dir, "/mnt/mmcblk0p1", sizeof(g_drone_cfg.record_dir) - 1);
    g_drone_cfg.record_bitrate_kbps = 40000;
    g_drone_cfg.record_fps = 60;
    g_drone_cfg.record_min_free_mb = 512;
    g_drone_cfg.record_size_idx = 0; /* "1920x1080" */

    /* [nack] */
    g_drone_cfg.nack_ring_ms = 150;
    g_drone_cfg.nack_air_pct = 5;
    g_drone_cfg.nack_burst_ms = 20;
}

static bool update_toml_key_in_file(const char * filepath, const char * section, const char * key, const char * val_str) {
    FILE * in = fopen(filepath, "r");
    if(!in) return false;
    char tmp_path[256];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", filepath);
    FILE * out = fopen(tmp_path, "w");
    if(!out) {
        fclose(in);
        return false;
    }

    char line[512];
    char cur_section[128] = "";
    bool key_updated = false;

    while(fgets(line, sizeof(line), in)) {
        char trimmed[512];
        strncpy(trimmed, line, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        char * p = trimmed;
        while(*p == ' ' || *p == '\t') p++;

        if(*p == '[') {
            char * end = strchr(p, ']');
            if(end && *(p + 1) != '[') {
                int len = end - (p + 1);
                if(len > 0 && len < (int)sizeof(cur_section)) {
                    strncpy(cur_section, p + 1, len);
                    cur_section[len] = '\0';
                }
            }
        }

        if(section && strcmp(cur_section, section) == 0 && !key_updated) {
            int klen = strlen(key);
            if(strncmp(p, key, klen) == 0) {
                char next_c = p[klen];
                if(next_c == ' ' || next_c == '\t' || next_c == '=') {
                    char * eq = strchr(p, '=');
                    if(eq) {
                        char * comment = strchr(eq, '#');
                        if(comment) {
                            fprintf(out, "%-16s = %s   %s", key, val_str, comment);
                        } else {
                            fprintf(out, "%-16s = %s\n", key, val_str);
                        }
                        key_updated = true;
                        continue;
                    }
                }
            }
        }

        fputs(line, out);
    }

    fclose(in);
    fclose(out);

    if(rename(tmp_path, filepath) != 0) {
        remove(tmp_path);
        return false;
    }
    return true;
}

static bool update_ladder_bw_in_file(const char * filepath, int bw) {
    if(!filepath) return false;
    FILE * in = fopen(filepath, "r");
    if(!in) return false;

    char tmp_path[512];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", filepath);
    FILE * out = fopen(tmp_path, "w");
    if(!out) {
        fclose(in);
        return false;
    }

    char line[512];
    bool in_ladder = false;

    while(fgets(line, sizeof(line), in)) {
        char * p = line;
        while(*p == ' ' || *p == '\t') p++;

        if(strncmp(p, "[[link.ladder]]", 15) == 0) {
            in_ladder = true;
            fputs(line, out);
            continue;
        } else if(*p == '[' && strncmp(p, "[[link.ladder]]", 15) != 0) {
            in_ladder = false;
        }

        if(in_ladder) {
            if(strncmp(p, "bw", 2) == 0 && (p[2] == ' ' || p[2] == '\t' || p[2] == '=')) {
                char * eq = strchr(p, '=');
                if(eq) {
                    char * comment = strchr(eq, '#');
                    if(comment) {
                        fprintf(out, "bw  = %d   %s", bw, comment);
                    } else {
                        fprintf(out, "bw  = %d\n", bw);
                    }
                    continue;
                }
            }
        }

        fputs(line, out);
    }

    fclose(in);
    fclose(out);

    if(rename(tmp_path, filepath) != 0) {
        remove(tmp_path);
        return false;
    }
    return true;
}

void config_data_load_all(void) {
    config_data_init_defaults();

    FILE * f = fopen("/etc/maburgs.toml", "r");
    if(f) {
        char line[512];
        char cur_section[128] = "";
        while(fgets(line, sizeof(line), f)) {
            char * p = line;
            while(*p == ' ' || *p == '\t') p++;
            if(*p == '#' || *p == '\0' || *p == '\n') continue;
            if(*p == '[') {
                char * end = strchr(p, ']');
                if(end && *(p + 1) != '[') {
                    int len = end - (p + 1);
                    if(len > 0 && len < (int)sizeof(cur_section)) {
                        strncpy(cur_section, p + 1, len);
                        cur_section[len] = '\0';
                    }
                }
                continue;
            }

            char * eq = strchr(p, '=');
            if(!eq) continue;
            *eq = '\0';
            char * k = p;
            char * v = eq + 1;
            char * k_end = k + strlen(k) - 1;
            while(k_end > k && (*k_end == ' ' || *k_end == '\t')) *k_end-- = '\0';
            while(*v == ' ' || *v == '\t') v++;
            char * comm = strchr(v, '#');
            if(comm) *comm = '\0';
            char * v_end = v + strlen(v) - 1;
            while(v_end >= v && (*v_end == ' ' || *v_end == '\t' || *v_end == '\r' || *v_end == '\n')) *v_end-- = '\0';

            if(strcmp(cur_section, "radio") == 0) {
                if(strcmp(k, "channel") == 0) {
                    if(strstr(v, "auto")) g_gs_cfg.channel_mode = 0;
                    else {
                        int ch = atoi(v);
                        if(ch == 40) g_gs_cfg.channel_mode = 1;
                        else if(ch == 64) g_gs_cfg.channel_mode = 2;
                        else if(ch == 112) g_gs_cfg.channel_mode = 3;
                        else if(ch == 144) g_gs_cfg.channel_mode = 4;
                    }
                } else if(strcmp(k, "width") == 0) {
                    g_gs_cfg.width = atoi(v);
                }
            } else if(strcmp(cur_section, "link.nack") == 0) {
                if(strcmp(k, "enable") == 0) {
                    g_gs_cfg.nack_enable = (strstr(v, "true") != NULL);
                }
            }
        }
        fclose(f);
    }

    f = fopen("/etc/maburplay.toml", "r");
    if(f) {
        char line[512];
        char cur_section[128] = "";
        while(fgets(line, sizeof(line), f)) {
            char * p = line;
            while(*p == ' ' || *p == '\t') p++;
            if(*p == '#' || *p == '\0' || *p == '\n') continue;
            if(*p == '[') {
                char * end = strchr(p, ']');
                if(end && *(p + 1) != '[') {
                    int len = end - (p + 1);
                    if(len > 0 && len < (int)sizeof(cur_section)) {
                        strncpy(cur_section, p + 1, len);
                        cur_section[len] = '\0';
                    }
                }
                continue;
            }

            char * eq = strchr(p, '=');
            if(!eq) continue;
            *eq = '\0';
            char * k = p;
            char * v = eq + 1;
            char * k_end = k + strlen(k) - 1;
            while(k_end > k && (*k_end == ' ' || *k_end == '\t')) *k_end-- = '\0';
            while(*v == ' ' || *v == '\t') v++;
            char * comm = strchr(v, '#');
            if(comm) *comm = '\0';
            char * v_end = v + strlen(v) - 1;
            while(v_end >= v && (*v_end == ' ' || *v_end == '\t' || *v_end == '\r' || *v_end == '\n')) *v_end-- = '\0';

            if(strcmp(cur_section, "dvr") == 0) {
                if(strcmp(k, "autostart") == 0) {
                    g_gs_cfg.dvr_enable = (strstr(v, "true") != NULL);
                } else if(strcmp(k, "target") == 0) {
                    if(strstr(v, "gs")) g_gs_cfg.dvr_target = 0;
                    else if(strstr(v, "vtx")) g_gs_cfg.dvr_target = 1;
                    else if(strstr(v, "both")) g_gs_cfg.dvr_target = 2;
                } else if(strcmp(k, "mode") == 0) {
                    if(strstr(v, "burned")) g_gs_cfg.dvr_format = 0;
                    else if(strstr(v, "raw")) g_gs_cfg.dvr_format = 1;
                }
            } else if(strcmp(cur_section, "osd") == 0) {
                if(strcmp(k, "enable") == 0) {
                    g_gs_cfg.msp_enable = (strstr(v, "true") != NULL);
                }
            }
        }
        fclose(f);
    }

    /* Load persistent system & drone configuration */
    FILE * sf = fopen("/etc/mabur_system.conf", "r");
    if(sf) {
        char line[128];
        while(fgets(line, sizeof(line), sf)) {
            char k[64] = "";
            char v[64] = "";
            if(sscanf(line, "%63[^=]=%63s", k, v) == 2) {
                int ival = atoi(v);
                float fval = (float)atof(v);
                if(strcmp(k, "gs.channel_mode") == 0) {
                    if(ival >= 0 && ival < 5) g_gs_cfg.channel_mode = ival;
                } else if(strcmp(k, "gs.width") == 0) {
                    if(ival == 20 || ival == 40) g_gs_cfg.width = ival;
                } else if(strcmp(k, "gs.nack_enable") == 0) {
                    g_gs_cfg.nack_enable = (ival != 0);
                } else if(strcmp(k, "gs.dvr_enable") == 0) {
                    g_gs_cfg.dvr_enable = (ival != 0);
                } else if(strcmp(k, "gs.dvr_target") == 0) {
                    if(ival >= 0 && ival < 3) g_gs_cfg.dvr_target = ival;
                } else if(strcmp(k, "gs.dvr_format") == 0) {
                    if(ival >= 0 && ival < 3) g_gs_cfg.dvr_format = ival;
                } else if(strcmp(k, "gs.msp_enable") == 0) {
                    g_gs_cfg.msp_enable = (ival != 0);
                } else if(strcmp(k, "drone.venc_fps") == 0) {
                    if(ival >= 30 && ival <= 120) g_drone_cfg.venc_fps = ival;
                } else if(strcmp(k, "drone.venc_size_idx") == 0) {
                    if(ival >= 0 && ival < g_resolution_count) g_drone_cfg.venc_size_idx = ival;
                } else if(strcmp(k, "drone.venc_roi_enabled") == 0) {
                    g_drone_cfg.venc_roi_enabled = (ival != 0);
                } else if(strcmp(k, "drone.encoder_roi_threshold_kbps") == 0) {
                    g_drone_cfg.encoder_roi_threshold_kbps = ival;
                } else if(strcmp(k, "drone.encoder_roi_qp_low") == 0) {
                    g_drone_cfg.encoder_roi_qp_low = ival;
                } else if(strcmp(k, "drone.encoder_roi_qp_normal") == 0) {
                    g_drone_cfg.encoder_roi_qp_normal = ival;
                } else if(strcmp(k, "drone.venc_roi_steps") == 0) {
                    g_drone_cfg.venc_roi_steps = ival;
                } else if(strcmp(k, "drone.venc_roi_center") == 0) {
                    g_drone_cfg.venc_roi_center = fval;
                } else if(strcmp(k, "drone.msp_serial_idx") == 0) {
                    if(ival >= 0 && ival < g_serial_count) g_drone_cfg.msp_serial_idx = ival;
                } else if(strcmp(k, "drone.msp_baud") == 0) {
                    g_drone_cfg.msp_baud = ival;
                } else if(strcmp(k, "drone.record_enable") == 0) {
                    g_drone_cfg.record_enable = (ival != 0);
                } else if(strcmp(k, "drone.record_bitrate_kbps") == 0) {
                    g_drone_cfg.record_bitrate_kbps = ival;
                } else if(strcmp(k, "drone.record_fps") == 0) {
                    g_drone_cfg.record_fps = ival;
                } else if(strcmp(k, "drone.low_power_enable") == 0) {
                    g_drone_cfg.low_power_enable = (ival != 0);
                } else if(strcmp(k, "drone.low_power_bitrate_kbps") == 0) {
                    g_drone_cfg.low_power_bitrate_kbps = ival;
                } else if(strcmp(k, "drone.encoder_bitrate_min_kbps") == 0) {
                    g_drone_cfg.encoder_bitrate_min_kbps = ival;
                } else if(strcmp(k, "drone.encoder_bitrate_max_kbps") == 0) {
                    g_drone_cfg.encoder_bitrate_max_kbps = ival;
                } else if(strcmp(k, "drone.encoder_airtime_budget") == 0) {
                    g_drone_cfg.encoder_airtime_budget = fval;
                } else if(strcmp(k, "drone.ampdu_min_mcs_20") == 0) {
                    g_drone_cfg.ampdu_min_mcs_20 = ival;
                } else if(strcmp(k, "drone.ampdu_min_mcs_40") == 0) {
                    g_drone_cfg.ampdu_min_mcs_40 = ival;
                } else if(strcmp(k, "drone.ampdu_max_num") == 0) {
                    g_drone_cfg.ampdu_max_num = ival;
                } else if(strcmp(k, "drone.air_clock_shed_ms") == 0) {
                    g_drone_cfg.air_clock_shed_ms = ival;
                } else if(strcmp(k, "drone.fec_symbol_size") == 0) {
                    g_drone_cfg.fec_symbol_size = ival;
                } else if(strcmp(k, "drone.fec_window") == 0) {
                    g_drone_cfg.fec_window = ival;
                } else if(strcmp(k, "drone.fec_base_overhead") == 0) {
                    g_drone_cfg.fec_base_overhead = fval;
                } else if(strcmp(k, "drone.nack_ring_ms") == 0) {
                    g_drone_cfg.nack_ring_ms = ival;
                }
            }
        }
        fclose(sf);
    }
}

bool config_data_save_all(void) {
    const char * const chan_vals[] = { "\"auto\"", "40", "64", "112", "144" };
    if(g_gs_cfg.channel_mode >= 0 && g_gs_cfg.channel_mode < 5) {
        update_toml_key_in_file("/etc/maburgs.toml", "radio", "channel", chan_vals[g_gs_cfg.channel_mode]);
    }
    char wbuf[16];
    snprintf(wbuf, sizeof(wbuf), "%d", g_gs_cfg.width);
    update_toml_key_in_file("/etc/maburgs.toml", "radio", "width", wbuf);
    update_toml_key_in_file("/etc/maburgs.toml", "link", "static_bw", wbuf);
    update_ladder_bw_in_file("/etc/maburgs.toml", g_gs_cfg.width);
    for(int i = 0; i < g_gs_cfg.num_ladders; i++) {
        g_gs_cfg.ladder[i].bw = g_gs_cfg.width;
    }
    update_toml_key_in_file("/etc/maburgs.toml", "link.nack", "enable", g_gs_cfg.nack_enable ? "true" : "false");

    update_toml_key_in_file("/etc/maburplay.toml", "dvr", "autostart", g_gs_cfg.dvr_enable ? "true" : "false");
    const char * const target_vals[] = { "\"gs\"", "\"vtx\"", "\"both\"" };
    if(g_gs_cfg.dvr_target >= 0 && g_gs_cfg.dvr_target < 3) {
        update_toml_key_in_file("/etc/maburplay.toml", "dvr", "target", target_vals[g_gs_cfg.dvr_target]);
    }
    const char * const dvr_modes[] = { "\"burned\"", "\"raw\"", "\"raw\"" };
    if(g_gs_cfg.dvr_format >= 0 && g_gs_cfg.dvr_format < 3) {
        update_toml_key_in_file("/etc/maburplay.toml", "dvr", "mode", dvr_modes[g_gs_cfg.dvr_format]);
    }
    update_toml_key_in_file("/etc/maburplay.toml", "osd", "enable", g_gs_cfg.msp_enable ? "true" : "false");

    /* Save persistent configuration to flash (/etc) */
    FILE * f = fopen("/etc/mabur_system.conf", "w");
    if(f) {
        fprintf(f, "# Mabur Persistent System Configuration\n");
        fprintf(f, "gs.channel_mode=%d\n", g_gs_cfg.channel_mode);
        fprintf(f, "gs.width=%d\n", g_gs_cfg.width);
        fprintf(f, "gs.nack_enable=%d\n", g_gs_cfg.nack_enable ? 1 : 0);
        fprintf(f, "gs.dvr_enable=%d\n", g_gs_cfg.dvr_enable ? 1 : 0);
        fprintf(f, "gs.dvr_target=%d\n", g_gs_cfg.dvr_target);
        fprintf(f, "gs.dvr_format=%d\n", g_gs_cfg.dvr_format);
        fprintf(f, "gs.msp_enable=%d\n", g_gs_cfg.msp_enable ? 1 : 0);

        fprintf(f, "drone.venc_fps=%d\n", g_drone_cfg.venc_fps);
        fprintf(f, "drone.venc_size_idx=%d\n", g_drone_cfg.venc_size_idx);
        fprintf(f, "drone.venc_roi_enabled=%d\n", g_drone_cfg.venc_roi_enabled ? 1 : 0);
        fprintf(f, "drone.encoder_roi_threshold_kbps=%d\n", g_drone_cfg.encoder_roi_threshold_kbps);
        fprintf(f, "drone.encoder_roi_qp_low=%d\n", g_drone_cfg.encoder_roi_qp_low);
        fprintf(f, "drone.encoder_roi_qp_normal=%d\n", g_drone_cfg.encoder_roi_qp_normal);
        fprintf(f, "drone.venc_roi_steps=%d\n", g_drone_cfg.venc_roi_steps);
        fprintf(f, "drone.venc_roi_center=%f\n", (double)g_drone_cfg.venc_roi_center);
        fprintf(f, "drone.msp_serial_idx=%d\n", g_drone_cfg.msp_serial_idx);
        fprintf(f, "drone.msp_baud=%d\n", g_drone_cfg.msp_baud);
        fprintf(f, "drone.record_enable=%d\n", g_drone_cfg.record_enable ? 1 : 0);
        fprintf(f, "drone.record_bitrate_kbps=%d\n", g_drone_cfg.record_bitrate_kbps);
        fprintf(f, "drone.record_fps=%d\n", g_drone_cfg.record_fps);
        fprintf(f, "drone.low_power_enable=%d\n", g_drone_cfg.low_power_enable ? 1 : 0);
        fprintf(f, "drone.low_power_bitrate_kbps=%d\n", g_drone_cfg.low_power_bitrate_kbps);
        fprintf(f, "drone.encoder_bitrate_min_kbps=%d\n", g_drone_cfg.encoder_bitrate_min_kbps);
        fprintf(f, "drone.encoder_bitrate_max_kbps=%d\n", g_drone_cfg.encoder_bitrate_max_kbps);
        fprintf(f, "drone.encoder_airtime_budget=%f\n", (double)g_drone_cfg.encoder_airtime_budget);
        fprintf(f, "drone.ampdu_min_mcs_20=%d\n", g_drone_cfg.ampdu_min_mcs_20);
        fprintf(f, "drone.ampdu_min_mcs_40=%d\n", g_drone_cfg.ampdu_min_mcs_40);
        fprintf(f, "drone.ampdu_max_num=%d\n", g_drone_cfg.ampdu_max_num);
        fprintf(f, "drone.air_clock_shed_ms=%d\n", g_drone_cfg.air_clock_shed_ms);
        fprintf(f, "drone.fec_symbol_size=%d\n", g_drone_cfg.fec_symbol_size);
        fprintf(f, "drone.fec_window=%d\n", g_drone_cfg.fec_window);
        fprintf(f, "drone.fec_base_overhead=%f\n", (double)g_drone_cfg.fec_base_overhead);
        fprintf(f, "drone.nack_ring_ms=%d\n", g_drone_cfg.nack_ring_ms);
        fflush(f);
        fclose(f);
    }

    /* Also write /tmp copy for tools checking tmpfs */
    FILE * tf = fopen("/tmp/mabur_gui_saved.txt", "w");
    if(tf) {
        fprintf(tf, "# Saved GUI configuration\n");
        fprintf(tf, "gs.channel_mode=%d\n", g_gs_cfg.channel_mode);
        fprintf(tf, "gs.width=%d\n", g_gs_cfg.width);
        fprintf(tf, "gs.nack_enable=%d\n", g_gs_cfg.nack_enable ? 1 : 0);
        fprintf(tf, "drone.venc_fps=%d\n", g_drone_cfg.venc_fps);
        fprintf(tf, "drone.venc_size=%s\n", g_resolution_options[g_drone_cfg.venc_size_idx]);
        fprintf(tf, "drone.msp_serial=%s\n", g_serial_options[g_drone_cfg.msp_serial_idx]);
        fprintf(tf, "drone.roi_enabled=%d\n", g_drone_cfg.venc_roi_enabled ? 1 : 0);
        fprintf(tf, "drone.record_enable=%d\n", g_drone_cfg.record_enable ? 1 : 0);
        fflush(tf);
        fclose(tf);
    }

    sync();

    system("/etc/init.d/S96maburgs restart >/dev/null 2>&1 &");
    return true;
}
