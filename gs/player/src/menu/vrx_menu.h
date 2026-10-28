#ifndef MABUR_PLAYER_VRX_MENU_H_
#define MABUR_PLAYER_VRX_MENU_H_

#include <cstdint>
#include <string>
#include <vector>

#include "osd_raster.h"  // Surface
#include "player_config.h"

namespace maburplay {

class VrxMenu {
 public:
  VrxMenu();
  ~VrxMenu();
  VrxMenu(const VrxMenu&) = delete;
  VrxMenu& operator=(const VrxMenu&) = delete;

  // Resolves GPIO pins, opens them, and initializes LVGL.
  // Resolution: width, height (e.g. 1920, 1080).
  bool init(int width, int height, const Config& cfg, std::string* err);

  // Polls hardware button lines.
  // Handles hold (>= 800ms) on center button to toggle open/close.
  // Handles navigation when open, and returns true if DVR rec button was
  // pressed while menu is closed.
  bool poll(uint64_t now_ms);

  bool is_open() const { return is_open_; }
  bool is_armed_locked() const { return armed_locked_; }
  void set_armed_locked(bool locked);
  void open();
  void close();
  void toggle();
  void inject_cmd(const std::string& cmd);

  // Call while is_open() and back buffer Surface is available.
  // Executes LVGL timer, copies dirty canvas to surf, and returns true if surf changed.
  bool render(const Surface& surf, uint64_t now_ms);

  // When menu closes, returns true for 2 frames so caller can clear both DRM buffers.
  bool needs_clear();
  void clear_canvas();

  // Internal callback from LVGL flush
  void on_flush(int x1, int y1, int x2, int y2, const uint32_t* px);

 private:
  struct ButtonState {
    int pin = 0;
    std::string name;
    int line_fd = -1;
    bool active_low = false;
    bool debounced_level = false;
    uint64_t cand_ms = 0;
    bool cand_val = false;
    uint64_t press_start_ms = 0;
    uint64_t last_repeat_ms = 0;
    bool hold_fired = false;
  };

  bool open_button_line(ButtonState& btn, int pin, bool active_low, const std::string& bias, std::string* err);
  bool sample_button(const char* name, ButtonState& btn, uint64_t now_ms, bool* just_pressed, bool* just_released);

  int width_ = 1920;
  int height_ = 1080;
  bool is_open_ = false;
  bool armed_locked_ = false;
  int clear_flips_ = 0;
  int dirty_flips_ = 0;

  ButtonState btn_center_;
  ButtonState btn_left_;
  ButtonState btn_right_;
  ButtonState btn_up_;
  ButtonState btn_down_;
  ButtonState btn_rec_;

  std::vector<uint32_t> canvas_;
  std::vector<uint8_t> draw_buf_;

  void* lv_disp_ = nullptr;
};

}  // namespace maburplay

#endif  // MABUR_PLAYER_VRX_MENU_H_
