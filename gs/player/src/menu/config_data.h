#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LADDER_RUNGS 16
#define MAX_CHANNELS 16

/* Link ladder rung structure */
typedef struct {
    int mcs;                /* 0 .. 7 */
    int bw;                 /* 20 or 40 MHz */
    float overhead_base;    /* e.g. 0.5 = 50% */
    float overhead_enh;     /* e.g. 0.25 = 25% */
} link_ladder_rung_t;

/* --- Ground Station Configuration (maburgs.default.toml) --- */
typedef struct {
    /* [radio] */
    int channels[MAX_CHANNELS];
    int num_channels;
    int channel_mode;       /* 0 = "auto", 1 = 40, 2 = 64, 3 = 112, 4 = 144 */
    int width;              /* 20 or 40 */
    int tx_card;            /* -1 = auto */

    /* [radio.scan] */
    int scan_dwell_ms;
    int scan_settle_ms;
    int scan_min_rounds;
    int scan_search_ms;
    int scan_op_window_ms;
    int scan_search_after_ms;
    int scan_pick_margin;
    int scan_one_card_ms;
    int scan_max_ms;
    int scan_busy_dbm;
    int scan_blocked_pct;

    /* [fec] */
    int fec_symbol_size;
    int fec_seq_horizon;

    /* [link] */
    int link_feedback_ms;
    int link_rcf_slot_hold_ms;
    int link_arrival_guard_syms;
    int link_beacon_keepalive_ms;
    int link_static_mcs;        /* -1 = adaptive */
    int link_static_bw;         /* 20 or 40 */
    int link_max_mcs;           /* 0 .. 7 */
    char link_key_file[64];
    float link_down_util;
    float link_up_util;
    int link_confirm_ms;
    int link_clean_ms;
    int link_probation_ms;
    int link_penalty_base_ms;
    int link_penalty_max_ms;
    int link_hold_after_down_ms;
    int link_min_between_changes_ms;
    int link_feedback_timeout_ms;
    int link_starved_confirm_ms;
    bool link_s3_demote;
    float link_s3_down_util;
    int link_s3_min_syms;
    int link_s3_settle_ms;
    float link_static_overhead_base;
    float link_static_overhead_enh;

    /* [[link.ladder]] */
    link_ladder_rung_t ladder[MAX_LADDER_RUNGS];
    int num_ladders;

    /* [link.probe] */
    bool probe_enable;
    int probe_rung_offset;
    float probe_max_util;
    int probe_clean_bodies;
    int probe_min_syms;
    int probe_silence_ms;
    int probe_pin_mcs;

    /* [link.fade] */
    bool fade_cascade;
    bool fade_predict;
    int fade_hold_ms;
    int fade_confirm_ms;
    float fade_rssi_db;
    float fade_snr_db;
    int fade_trigger_ms;
    int fade_min_rung;

    /* [link.nack] */
    bool nack_enable;
    int nack_lookback;
    int nack_repeat_ms;
    int nack_max_tries;
    int nack_min_lead_ms;

    /* [link.rung_stats] */
    int rung_stats_half_life_samples;

    /* [video] */
    int video_frame_gap_timeout_ms;
    int video_frame_gap_timeout_max_ms;
    int video_frame_lookahead;

    /* [msp] & [msp.out] */
    bool msp_enable;
    int msp_symbol_size;
    int msp_window;
    char msp_out_host[32];
    int msp_out_port;

    /* [stats] */
    bool stats_enable;
    int stats_interval_ms;

    /* [au_ring] */
    bool au_ring_enable;
    char au_ring_path[64];
    char au_ring_socket[64];
    int au_ring_slot_kb;
    int au_ring_slot_count;

    /* [debug_log] */
    bool debug_log_enable;
    char debug_log_dir[64];
    int debug_log_ctl_period_ms;
    int debug_log_rung_period_s;

    /* [hop] */
    int hop_window_ms;
    int hop_persist;
    int hop_dwell_observe_ms;
    int hop_dwell_period_ms;
    int hop_relay_burst_period_ms;
    int hop_rank_visits;
    int hop_rank_max_age_ms;
    int hop_confirm_ms;
    int hop_confirm_extend_ms;
    int hop_verify_ms;
    int hop_cooldown_ms;
    int hop_max_hops_per_min;
    int hop_backoff_ms;
    int hop_one_card_repeats;

    /* [hop.verdict] */
    float verdict_loss_pct;
    float verdict_recovered_x;
    int verdict_recovered_min;
    int verdict_weak_rssi_dbm;
    int verdict_weak_snr_db;
    int verdict_fading_drop_db;
    int verdict_foreign_pps;
    int verdict_fa_pps;
    float verdict_starved_frac;

    /* Ground DVR */
    bool dvr_enable;
    int dvr_target;         /* 0 = "goggles", 1 = "vtx", 2 = "both" */
    char dvr_dir[64];
    int dvr_min_free_mb;
    int dvr_format;         /* 0 = "MP4", 1 = "MKV", 2 = "TS" */
} gs_config_t;

/* --- Drone Configuration (mabur.default.toml) --- */
typedef struct {
    /* [radio] */
    int usb_vid;
    int usb_pid;
    int channels[MAX_CHANNELS];
    int num_channels;
    int width;
    int power_mode;         /* 0 = "none", 1 = "offset" */
    int tx_threads;
    bool ldpc;
    int rate_walls_rel[8];
    int legacy_wall_rel;
    float wall_margin_db;

    /* [fec] */
    int fec_symbol_size;
    int fec_window;
    int fec_blocks_base;
    int fec_blocks_enh;
    float fec_base_overhead;
    int fec_feed_batch;
    int fec_flush_ms;

    /* [encoder] */
    int encoder_bitrate_min_kbps;
    int encoder_bitrate_max_kbps;
    float encoder_airtime_budget;
    int encoder_roi_threshold_kbps;
    int encoder_roi_qp_low;
    int encoder_roi_qp_normal;

    /* [venc] */
    char venc_sensor_bin[128];
    int venc_size_idx;      /* 0: 1920x1080, 1: 1728x972, 2: 1728x816, 3: 1280x720 */
    int venc_fps;           /* 30, 60, 90, 100, 120 */
    int venc_gop_s;
    int venc_qp_delta;
    int venc_max_ipprop;
    int venc_min_iqp;
    int venc_intra_refresh_frames;
    int venc_intra_refresh_qp;
    int venc_ref_base;
    int venc_ref_enhance;
    bool venc_ref_pred;
    int venc_slices;
    int venc_ae_fps;
    int venc_awb_fps;
    int venc_snapshot_quality;
    int venc_debug_port;
    int venc_superframe_p_pct;

    /* [venc.roi] */
    bool venc_roi_enabled;
    int venc_roi_steps;
    float venc_roi_center;

    /* [link] */
    int link_failsafe_ms;
    int link_rc_drain_ms;
    int link_rendezvous_ms;
    int link_move_confirm_ms;
    int link_tick_ms;
    char link_key_file[64];

    /* [msp] */
    bool msp_enable;
    int msp_serial_idx;     /* 0: /dev/ttyS0, 1: /dev/ttyS1, 2: /dev/ttyS2, 3: /dev/ttyS3 */
    int msp_baud;           /* 115200 */
    int msp_update_rate_hz;
    int msp_symbol_size;
    int msp_window;
    float msp_overhead;

    /* [low_power] */
    bool low_power_enable;
    int low_power_bitrate_kbps;
    int low_power_fps;
    int low_power_stale_ms;

    /* [ampdu] */
    int ampdu_max_num;
    int ampdu_max_time;
    int ampdu_min_mcs_20;
    int ampdu_min_mcs_40;

    /* [air_clock] */
    int air_clock_shed_ms;
    float air_clock_eff_20[8];
    float air_clock_eff_40[8];
    int air_clock_body_us;

    /* [record] - Air recording */
    bool record_enable;
    char record_dir[64];
    int record_bitrate_kbps;
    int record_fps;
    int record_min_free_mb;
    int record_size_idx;    /* 0: 1920x1080, 1: 1280x720 */

    /* [nack] */
    int nack_ring_ms;
    int nack_air_pct;
    int nack_burst_ms;
} drone_config_t;

/* Global configuration instances */
extern gs_config_t g_gs_cfg;
extern drone_config_t g_drone_cfg;

/* Resolution string lookup */
extern const char * const g_resolution_options[];
extern const int g_resolution_count;

/* Serial port string lookup */
extern const char * const g_serial_options[];
extern const int g_serial_count;

/* Functions */
void config_data_init_defaults(void);
void config_data_load_all(void);
bool config_data_save_all(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_DATA_H */
