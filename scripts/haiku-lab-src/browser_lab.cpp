// Browser adapter around the lab decoder (lab_host.cpp), the same way browser.cpp wraps host.cpp.
// Space only. Each style keeps its own recent-word and recent-line history.
#define main haiku_lab_main
#include "lab_host.cpp"
#undef main
#include <emscripten.h>

static float style_history[3][G_ENTRY_COUNT];
static std::deque<std::string> style_lines[3];

extern "C" EMSCRIPTEN_KEEPALIVE int haiku_lab_generate(int style, int concept, int case_number) {
  if (style < 1 || style > 2 || concept < -1 || concept >= LAB_N_CONCEPTS ||
      case_number < 0 || case_number > 1000000000) return 2;
  TEMPERATURE=HAIKU_TEMPERATURE; NATURE_BOOST=HAIKU_NATURE_BOOST;
  NATURE_SCORE=HAIKU_NATURE_SCORE; FRESHNESS=0.f; free_percent=0; season_features=false;
  memcpy(haiku_recent, style_history[style], sizeof haiku_recent);
  recent_lines.swap(style_lines[style]);
  char text[16];
  snprintf(text, sizeof text, "%d", case_number); setenv("HAIKU_START_CASE", text, 1);
  snprintf(text, sizeof text, "%d", style); setenv("LAB_STYLE", text, 1);
  snprintf(text, sizeof text, "%d", concept); setenv("LAB_CONCEPT", text, 1);
  const char *args[]={"decoder", "/model.bin", "grammar", "1", "space"};
  int result=haiku_lab_main(5, const_cast<char **>(args));
  memcpy(style_history[style], haiku_recent, sizeof haiku_recent);
  recent_lines.swap(style_lines[style]);
  free(s.x); free(s.h); free(s.qkv); free(s.att); free(s.g1); free(s.g2);
  free(s.ple); free(s.tmpP); free(s.trow); free(s.logits); free(s.scores);
  free(s.kcache); free(s.vcache); s={};
  return result;
}
