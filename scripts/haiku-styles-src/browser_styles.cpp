// Browser adapter around the styles decoder (styles_host.cpp), the same way browser.cpp wraps host.cpp.
// Space only. Each style keeps its own recent-word and recent-line history; the concept is picked per case.
#define main haiku_styles_main
#include "styles_host.cpp"
#undef main
#include <emscripten.h>

static float style_history[3][G_ENTRY_COUNT];
static std::deque<std::string> style_lines[3];

extern "C" EMSCRIPTEN_KEEPALIVE int haiku_styles_generate(int style, int case_number) {
  if (style < 1 || style > 2 || case_number < 0 || case_number > 1000000000) return 2;
  TEMPERATURE=HAIKU_TEMPERATURE; NATURE_BOOST=HAIKU_NATURE_BOOST;
  NATURE_SCORE=HAIKU_NATURE_SCORE; FRESHNESS=0.f; free_percent=0; season_features=false;
  memcpy(haiku_recent, style_history[style], sizeof haiku_recent);
  recent_lines.swap(style_lines[style]);
  char text[16];
  snprintf(text, sizeof text, "%d", case_number); setenv("HAIKU_START_CASE", text, 1);
  snprintf(text, sizeof text, "%d", style); setenv("LAB_STYLE", text, 1);
  setenv("LAB_CONCEPT", "-1", 1);
  const char *args[]={"decoder", "/model.bin", "grammar", "1", "space"};
  int result=haiku_styles_main(5, const_cast<char **>(args));
  memcpy(style_history[style], haiku_recent, sizeof haiku_recent);
  recent_lines.swap(style_lines[style]);
  free(s.x); free(s.h); free(s.qkv); free(s.att); free(s.g1); free(s.g2);
  free(s.ple); free(s.tmpP); free(s.trow); free(s.logits); free(s.scores);
  free(s.kcache); free(s.vcache); s={};
  return result;
}
