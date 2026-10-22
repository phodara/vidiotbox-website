// Browser adapter around the unchanged C5 host decoder. Keep inference/rules in host.cpp.
#define main haiku_host_main
#include "host.cpp"
#undef main
#include <emscripten.h>

static float theme_history[HAIKU_N_SEASONS][G_ENTRY_COUNT];
// Recent complete lines, kept per theme so a seasonal poem never pushes a Space line out
// of the exact-line repeat guard (the device only remembers Space lines).
static std::deque<std::string> theme_lines[HAIKU_N_SEASONS];

extern "C" EMSCRIPTEN_KEEPALIVE int haiku_generate(int theme, int case_number) {
  if (theme < 0 || theme >= HAIKU_N_SEASONS || case_number < 0 || case_number > 1000000000) return 2;
  static const char *names[] = {"spring", "summer", "autumn", "winter", "space"};
  // The CLI normally initializes these once per process. Each browser call is a
  // new CLI invocation, while each theme retains its own recent-word history.
  TEMPERATURE=HAIKU_TEMPERATURE; NATURE_BOOST=HAIKU_NATURE_BOOST;
  NATURE_SCORE=HAIKU_NATURE_SCORE; FRESHNESS=0.f; free_percent=0;
  memcpy(haiku_recent, theme_history[theme], sizeof haiku_recent);
  recent_lines.swap(theme_lines[theme]);
  char case_text[16]; snprintf(case_text, sizeof case_text, "%d", case_number);
  setenv("HAIKU_START_CASE", case_text, 1);
  const char *args[]={"decoder", "/model.bin", "grammar", "1", names[theme]};
  int result=haiku_host_main(5, const_cast<char **>(args));
  memcpy(theme_history[theme], haiku_recent, sizeof haiku_recent);
  recent_lines.swap(theme_lines[theme]);
  // main's model blob is scoped to one invocation. Release all scratch buffers
  // as well, keeping only the bounded theme history between poems.
  free(s.x); free(s.h); free(s.qkv); free(s.att); free(s.g1); free(s.g2);
  free(s.ple); free(s.tmpP); free(s.trow); free(s.logits); free(s.scores);
  free(s.kcache); free(s.vcache); s={};
  return result;
}
