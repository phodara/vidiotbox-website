#pragma once
#include <stdint.h>

// How long auto mode leaves a poem on screen before showing the next one.
// The next poem is written in the background meanwhile, so this can be about
// the time one takes (35 s on the board) without a spinner appearing.
#define HAIKU_AUTO_DISPLAY_MS 40000

enum class HaikuAction : uint8_t { None, Season, Generate, ToggleAuto };

class HaikuButtons {
  uint8_t raw = 0, stable = 0, gesture = 0;
  uint32_t changed[2] = {}, both_since = 0;
  bool chord = false, holding_both = false, toggled = false;

public:
  HaikuAction update(uint8_t pins, uint32_t now) {
    for (int i = 0; i < 2; i++) {
      uint8_t bit = 1u << i;
      if ((pins ^ raw) & bit) { raw ^= bit; changed[i] = now; }
      if (((raw ^ stable) & bit) && now - changed[i] >= 30) stable ^= bit;
    }
    if (stable) gesture |= stable;
    if (stable == 3) {
      chord = true;
      if (!holding_both) { holding_both = true; both_since = now; }
      if (!toggled && now - both_since >= 800) {
        toggled = true;
        return HaikuAction::ToggleAuto;
      }
    } else {
      // A partial release interrupts the hold but still consumes the chord.
      holding_both = false;
    }
    if (!stable && gesture) {
      HaikuAction action = gesture == 1 ? HaikuAction::Season :
                           gesture == 2 ? HaikuAction::Generate : HaikuAction::None;
      if (chord) action = HaikuAction::None;
      gesture = 0; chord = holding_both = toggled = false;
      return action;
    }
    return HaikuAction::None;
  }
};

class HaikuAutoMode {
  bool enabled = false, notice = false, have_attempt = false;
  uint32_t notice_since = 0, finished_at = 0;

public:
  bool on() const { return enabled; }
  void set(bool value, uint32_t now) {
    enabled = value; notice = true; notice_since = now;
  }
  HaikuAction apply(HaikuAction action, uint32_t now) {
    // Only the two-button hold changes the mode; Season and Generate keep their
    // normal effect while auto is on.
    if (action == HaikuAction::ToggleAuto) set(!enabled, now);
    return action;
  }
  bool message_visible(uint32_t now) const { return notice && now - notice_since < 3000; }
  bool expire_message(uint32_t now) {
    if (!notice || message_visible(now)) return false;
    notice = false;
    return true;
  }
  void finished(uint32_t now) { have_attempt = true; finished_at = now; }
  bool due(uint32_t now) const {
    return enabled && !message_visible(now) &&
      (!have_attempt || now - finished_at >= HAIKU_AUTO_DISPLAY_MS);
  }
};
