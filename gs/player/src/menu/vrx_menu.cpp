#include "vrx_menu.h"
#include "gui_app.h"
#include "rec_button.h"

#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <algorithm>

#include "lvgl/lvgl.h"

namespace maburplay {

static VrxMenu* s_active_vrx_menu = nullptr;

extern "C" {
static void VrxMenuFlushCallback(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  VrxMenu* self = static_cast<VrxMenu*>(lv_display_get_user_data(disp));
  if (self) {
    self->on_flush(area->x1, area->y1, area->x2, area->y2,
                   reinterpret_cast<const uint32_t*>(px_map));
  }
  lv_display_flush_ready(disp);
}

static uint32_t VrxMenuTickGet(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<uint32_t>(static_cast<uint64_t>(ts.tv_sec) * 1000u +
                               static_cast<uint64_t>(ts.tv_nsec) / 1000000u);
}

static void OnGuiCloseRequest(void) {
  if (s_active_vrx_menu) {
    s_active_vrx_menu->close();
  }
}
}

VrxMenu::VrxMenu() {
  btn_center_.name = "Center";
  btn_left_.name = "Left";
  btn_right_.name = "Right";
  btn_up_.name = "Up";
  btn_down_.name = "Down";
  btn_rec_.name = "Rec";
}

VrxMenu::~VrxMenu() {
  if (s_active_vrx_menu == this) s_active_vrx_menu = nullptr;
  auto close_btn = [](ButtonState& b) {
    if (b.line_fd >= 0) {
      ::close(b.line_fd);
      b.line_fd = -1;
    }
  };
  close_btn(btn_center_);
  close_btn(btn_left_);
  close_btn(btn_right_);
  close_btn(btn_up_);
  close_btn(btn_down_);
  close_btn(btn_rec_);
}

bool VrxMenu::open_button_line(ButtonState& btn, int pin, bool active_low,
                               const std::string& bias, std::string* err) {
  btn.pin = pin;
  btn.active_low = active_low;

  std::string chip_path;
  unsigned offset = 0;
  if (!RecButton::resolve_pin(pin, &chip_path, &offset, err)) {
    return false;
  }

  const int chip_fd = ::open(chip_path.c_str(), O_RDONLY | O_CLOEXEC);
  if (chip_fd < 0) {
    if (err) *err = chip_path + ": " + std::strerror(errno);
    return false;
  }

  struct gpio_v2_line_request req {};
  req.offsets[0] = offset;
  req.num_lines = 1;
  req.config.flags = GPIO_V2_LINE_FLAG_INPUT;
  if (active_low) req.config.flags |= GPIO_V2_LINE_FLAG_ACTIVE_LOW;
  if (bias == "pull-up") {
    req.config.flags |= GPIO_V2_LINE_FLAG_BIAS_PULL_UP;
  } else if (bias == "pull-down") {
    req.config.flags |= GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN;
  } else if (bias == "disable" || bias == "disabled") {
    req.config.flags |= GPIO_V2_LINE_FLAG_BIAS_DISABLED;
  }
  // If bias is "none" or "as-is", leave hardware / DT bias flags unchanged
  std::snprintf(req.consumer, sizeof(req.consumer), "mabur_menu_%d", pin);

  if (::ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &req) != 0 || req.fd < 0) {
    const int e = errno;
    struct gpio_v2_line_info li {};
    li.offset = offset;
    if (e == EBUSY && ::ioctl(chip_fd, GPIO_V2_GET_LINEINFO_IOCTL, &li) == 0 &&
        li.consumer[0] != '\0') {
      if (err) *err = chip_path + " line " + std::to_string(offset) + " is held by \"" + li.consumer + "\"";
    } else {
      if (err) *err = chip_path + " line " + std::to_string(offset) + ": " + std::strerror(e);
    }
    ::close(chip_fd);
    return false;
  }
  ::close(chip_fd);
  btn.line_fd = req.fd;
  return true;
}

bool VrxMenu::sample_button(const char* name, ButtonState& btn, uint64_t now_ms, bool* just_pressed, bool* just_released) {
  *just_pressed = false;
  *just_released = false;
  if (btn.line_fd < 0) return false;

  struct gpio_v2_line_values v {};
  v.mask = 1;
  if (::ioctl(btn.line_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &v) != 0) {
    return btn.debounced_level;
  }
  bool raw = (v.bits & 1u) != 0;

  if (raw != btn.cand_val) {
    btn.cand_val = raw;
    btn.cand_ms = now_ms;
  }

  if (now_ms - btn.cand_ms >= 30) {
    if (btn.debounced_level != btn.cand_val) {
      btn.debounced_level = btn.cand_val;
      if (btn.debounced_level) {
        *just_pressed = true;
        std::fprintf(stderr, "maburplay: button %s PRESSED (pin %d)\n", name, btn.pin);
      } else {
        *just_released = true;
        std::fprintf(stderr, "maburplay: button %s RELEASED (pin %d)\n", name, btn.pin);
      }
    }
  }
  return btn.debounced_level;
}

bool VrxMenu::init(int width, int height, const Config& cfg, std::string* err) {
  width_ = width;
  height_ = height;
  canvas_.assign(width_ * height_, 0);

  // Open button GPIO lines:
  // RunCam WiFiLink VRX pinout:
  // Center: PIN_11 (gpiochip3 line 1)
  // Left: PIN_13 (gpiochip3 line 2)
  // Right: PIN_38 (gpiochip3 line 6)
  // Up: PIN_16 (gpiochip3 line 9)
  // Down: PIN_18 (gpiochip3 line 10)
  // Rec: PIN_32 (gpiochip3 line 18)
  std::string berr;
  if (open_button_line(btn_center_, 11, false, "as-is", &berr)) {
    std::fprintf(stderr, "maburplay: menu button: Center -> PIN_11 OK\n");
  } else {
    std::fprintf(stderr, "maburplay: menu button Center: %s\n", berr.c_str());
  }

  if (open_button_line(btn_left_, 13, false, "as-is", &berr)) {
    std::fprintf(stderr, "maburplay: menu button: Left -> PIN_13 OK\n");
  } else {
    std::fprintf(stderr, "maburplay: menu button Left: %s\n", berr.c_str());
  }

  if (open_button_line(btn_right_, 38, false, "as-is", &berr)) {
    std::fprintf(stderr, "maburplay: menu button: Right -> PIN_38 OK\n");
  } else {
    std::fprintf(stderr, "maburplay: menu button Right: %s\n", berr.c_str());
  }

  if (open_button_line(btn_up_, 16, false, "as-is", &berr)) {
    std::fprintf(stderr, "maburplay: menu button: Up -> PIN_16 OK\n");
  } else {
    std::fprintf(stderr, "maburplay: menu button Up: %s\n", berr.c_str());
  }

  if (open_button_line(btn_down_, 18, false, "as-is", &berr)) {
    std::fprintf(stderr, "maburplay: menu button: Down -> PIN_18 OK\n");
  } else {
    std::fprintf(stderr, "maburplay: menu button Down: %s\n", berr.c_str());
  }

  int rec_pin = cfg.input.rec.configured ? cfg.input.rec.pin : 32;
  bool rec_low = cfg.input.rec.configured ? cfg.input.rec.active_low : false;
  std::string rec_bias = cfg.input.rec.configured ? cfg.input.rec.bias : "as-is";
  // On RunCam WiFiLink VRX, pin 32 is active-high (idles at 0). If user config had active_low=true, override it:
  if (rec_pin == 32) {
    rec_low = false;
    rec_bias = "as-is";
  }
  if (open_button_line(btn_rec_, rec_pin, rec_low, rec_bias, &berr)) {
    std::fprintf(stderr, "maburplay: menu button: Rec -> PIN_%d OK\n", rec_pin);
  } else {
    std::fprintf(stderr, "maburplay: menu button Rec: %s\n", berr.c_str());
  }

  // Initialize LVGL
  lv_init();
  lv_tick_set_cb(VrxMenuTickGet);

  lv_display_t* disp = lv_display_create(width_, height_);
  if (!disp) {
    if (err) *err = "lv_display_create failed";
    return false;
  }
  lv_disp_ = disp;
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_ARGB8888);
  lv_display_set_user_data(disp, this);

  // Allocate 200 lines partial draw buffer (~1.5 MB)
  draw_buf_.resize(width_ * 200 * sizeof(uint32_t));
  lv_display_set_buffers(disp, draw_buf_.data(), nullptr, draw_buf_.size(), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp, VrxMenuFlushCallback);

  s_active_vrx_menu = this;
  gui_app_init();
  gui_set_close_request_cb(OnGuiCloseRequest);

  return true;
}

void VrxMenu::set_armed_locked(bool locked) {
  if (armed_locked_ != locked) {
    armed_locked_ = locked;
    if (armed_locked_) {
      std::fprintf(stderr, "maburplay: VRX menu locked: drone is ARMED\n");
      if (is_open_) {
        close();
      }
    } else {
      std::fprintf(stderr, "maburplay: VRX menu unlocked: drone is disarmed/disconnected\n");
    }
  }
}

void VrxMenu::open() {
  if (armed_locked_) {
    std::fprintf(stderr, "maburplay: VRX menu open rejected -- drone is ARMED\n");
    return;
  }
  if (!is_open_) {
    is_open_ = true;
    dirty_flips_ = 2;
    gui_app_reset_focus();
    if (lv_disp_) {
      lv_obj_invalidate(lv_screen_active());
    }
    std::fprintf(stderr, "maburplay: VRX menu OPENED\n");
  }
}

void VrxMenu::close() {
  if (is_open_) {
    is_open_ = false;
    gui_on_exit_menu();
    clear_canvas();
    clear_flips_ = 0;
    dirty_flips_ = 0;
    std::fprintf(stderr, "maburplay: VRX menu CLOSED (all changes applied)\n");
  }
}

void VrxMenu::clear_canvas() {
  std::fill(canvas_.begin(), canvas_.end(), 0);
  dirty_flips_ = 2;
}

extern "C" void vrx_menu_clear_canvas(void) {
  if (s_active_vrx_menu) {
    s_active_vrx_menu->clear_canvas();
  }
}

void VrxMenu::toggle() {
  if (is_open_) close();
  else open();
}

void VrxMenu::inject_cmd(const std::string& cmd) {
  if (cmd == "toggle") {
    toggle();
  } else if (cmd == "open") {
    open();
  } else if (cmd == "close") {
    close();
  } else if (cmd == "up") {
    gui_handle_input(NAV_UP);
  } else if (cmd == "down") {
    gui_handle_input(NAV_DOWN);
  } else if (cmd == "left") {
    gui_handle_input(NAV_LEFT);
  } else if (cmd == "right") {
    gui_handle_input(NAV_RIGHT);
  } else if (cmd == "enter") {
    gui_handle_input(NAV_ENTER);
  } else if (cmd == "back") {
    gui_handle_input(NAV_BACK);
  } else if (cmd == "tab_next") {
    gui_handle_input(NAV_TAB_NEXT);
  } else if (cmd == "tab_prev") {
    gui_handle_input(NAV_TAB_PREV);
  } else if (cmd == "konami") {
    gui_unlock_advanced();
  }
}

bool VrxMenu::needs_clear() {
  return false;
}

bool VrxMenu::poll(uint64_t now_ms) {
  static uint64_t last_cmd_check = 0;
  if (now_ms - last_cmd_check >= 100) {
    last_cmd_check = now_ms;
    FILE* f = std::fopen("/tmp/mabur_menu_cmd", "r");
    if (f) {
      char buf[64];
      if (std::fgets(buf, sizeof(buf), f)) {
        char* end = buf + std::strlen(buf) - 1;
        while (end >= buf && (*end == '\n' || *end == '\r' || *end == ' ')) *end-- = '\0';
        std::fprintf(stderr, "maburplay: received menu command: %s\n", buf);
        inject_cmd(buf);
      }
      std::fclose(f);
      std::remove("/tmp/mabur_menu_cmd");
    }
  }

  bool rec_toggle = false;

  // 1. Center button: Hold >= 450ms toggles menu. Short press = NAV_ENTER in menu.
  bool c_press = false, c_rel = false;
  bool c_held = sample_button("Center", btn_center_, now_ms, &c_press, &c_rel);
  if (c_press) {
    btn_center_.press_start_ms = now_ms;
    btn_center_.hold_fired = false;
  }
  if (c_held) {
    if (!btn_center_.hold_fired && (now_ms - btn_center_.press_start_ms >= 450)) {
      btn_center_.hold_fired = true;
      if (armed_locked_) {
        std::fprintf(stderr, "maburplay: Center button LONG PRESS ignored -- drone is ARMED\n");
      } else {
        std::fprintf(stderr, "maburplay: Center button LONG PRESS (held %llu ms) -> toggle menu\n",
                     static_cast<unsigned long long>(now_ms - btn_center_.press_start_ms));
        toggle();
      }
    }
  }
  if (c_rel) {
    if (!btn_center_.hold_fired) {
      if (is_open_) {
        gui_handle_input(NAV_ENTER);
      }
    }
  }

  // 2. Right button:
  // Short press: NAV_RIGHT (when open).
  // Long press >= 500ms: Fallback to act as Center / Toggle Menu (matching pixelpilot behavior on WiFiLink VRX).
  bool r_press = false, r_rel = false;
  bool r_held = sample_button("Right", btn_right_, now_ms, &r_press, &r_rel);
  if (r_press) {
    btn_right_.press_start_ms = now_ms;
    btn_right_.last_repeat_ms = now_ms;
    btn_right_.hold_fired = false;
    if (is_open_) {
      gui_handle_input(NAV_RIGHT);
    }
  } else if (r_held) {
    if (!btn_right_.hold_fired && (now_ms - btn_right_.press_start_ms >= 500)) {
      btn_right_.hold_fired = true;
      if (armed_locked_) {
        std::fprintf(stderr, "maburplay: Right button LONG PRESS ignored -- drone is ARMED\n");
      } else {
        std::fprintf(stderr, "maburplay: Right button LONG PRESS (acting as center)\n");
        if (!is_open_) {
          open();
        } else {
          gui_handle_input(NAV_ENTER);
        }
      }
    } else if (is_open_) {
      if (now_ms - btn_right_.press_start_ms >= 350 && now_ms - btn_right_.last_repeat_ms >= 150) {
        gui_handle_input(NAV_RIGHT);
        btn_right_.last_repeat_ms = now_ms;
      }
    }
  }

  // 3. Directional buttons (Up, Down, Left): only when open. Support key repeat.
  auto handle_dir = [&](const char* name, ButtonState& b, nav_key_t key) {
    bool p = false, r = false;
    bool is_held = sample_button(name, b, now_ms, &p, &r);
    if (!is_open_) return;
    if (p) {
      gui_handle_input(key);
      b.press_start_ms = now_ms;
      b.last_repeat_ms = now_ms;
    } else if (is_held) {
      if (now_ms - b.press_start_ms >= 350 && now_ms - b.last_repeat_ms >= 150) {
        gui_handle_input(key);
        b.last_repeat_ms = now_ms;
      }
    }
  };

  handle_dir("Up", btn_up_, NAV_UP);
  handle_dir("Down", btn_down_, NAV_DOWN);
  handle_dir("Left", btn_left_, NAV_LEFT);

  // 4. Rec button:
  // When menu is open -> NAV_BACK.
  // When menu is closed -> toggles DVR recording!
  bool rec_p = false, rec_r = false;
  sample_button("Rec", btn_rec_, now_ms, &rec_p, &rec_r);
  if (rec_p) {
    if (is_open_) {
      gui_handle_input(NAV_BACK);
    } else {
      rec_toggle = true;
    }
  }

  return rec_toggle;
}

void VrxMenu::on_flush(int x1, int y1, int x2, int y2, const uint32_t* px) {
  int w = x2 - x1 + 1;
  int h = y2 - y1 + 1;
  for (int y = 0; y < h; ++y) {
    int dst_y = y1 + y;
    if (dst_y < 0 || dst_y >= height_) continue;
    uint32_t* dst = canvas_.data() + dst_y * width_ + x1;
    const uint32_t* src = px + y * w;
    std::memcpy(dst, src, w * sizeof(uint32_t));
  }
  dirty_flips_ = 2;
}

bool VrxMenu::render(const Surface& surf, uint64_t now_ms) {
  if (!is_open_ || !surf.pixels) return false;

  lv_timer_handler();

  if (dirty_flips_ > 0) {
    int w = std::min(width_, surf.width);
    int h = std::min(height_, surf.height);
    for (int y = 0; y < h; ++y) {
      uint32_t* dst = surf.pixels + y * surf.stride_px;
      const uint32_t* src = canvas_.data() + y * width_;
      std::memcpy(dst, src, w * sizeof(uint32_t));
    }
    dirty_flips_--;
    static uint64_t last_render_log = 0;
    if (now_ms - last_render_log >= 1000) {
      last_render_log = now_ms;
      std::fprintf(stderr, "maburplay: VRX menu rendered to surface (%dx%d, stride=%d)\n",
                   w, h, surf.stride_px);
    }
    return true;
  }
  return false;
}

}  // namespace maburplay
