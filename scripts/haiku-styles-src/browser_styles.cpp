// Browser adapter around the styles decoder (styles_host.cpp), the same way browser.cpp wraps host.cpp.
// Space, or a season with concepts (winter). Each style and theme keeps its own recent-word and recent-line
// history; in space the concept is picked per case, in a season by the poem's opener.
#define main haiku_styles_main
#include "styles_host.cpp"
#undef main
#include <emscripten.h>

static float style_history[3][HAIKU_N_SEASONS][G_ENTRY_COUNT];
static std::deque<std::string> style_lines[3][HAIKU_N_SEASONS];

extern "C" EMSCRIPTEN_KEEPALIVE int haiku_styles_generate(int style, int theme, int case_number) {
  if (style < 1 || style > 2 || case_number < 0 || case_number > 1000000000) return 2;
  if (theme != HAIKU_SEASON_SPACE && (theme < 0 || theme >= HAIKU_N_SEASONS || !season_styled(theme))) return 2;
  static const char *names[] = {"spring", "summer", "autumn", "winter", "space"};
  TEMPERATURE=HAIKU_TEMPERATURE; NATURE_BOOST=HAIKU_NATURE_BOOST;
  NATURE_SCORE=HAIKU_NATURE_SCORE; FRESHNESS=0.f; free_percent=0; season_features=false;
  memcpy(haiku_recent, style_history[style][theme], sizeof haiku_recent);
  recent_lines.swap(style_lines[style][theme]);
  char text[16];
  snprintf(text, sizeof text, "%d", case_number); setenv("HAIKU_START_CASE", text, 1);
  snprintf(text, sizeof text, "%d", style); setenv("LAB_STYLE", text, 1);
  setenv("LAB_CONCEPT", "-1", 1);
  const char *args[]={"decoder", "/model.bin", "grammar", "1", names[theme]};
  int result=haiku_styles_main(5, const_cast<char **>(args));
  memcpy(style_history[style][theme], haiku_recent, sizeof haiku_recent);
  recent_lines.swap(style_lines[style][theme]);
  free(s.x); free(s.h); free(s.qkv); free(s.att); free(s.g1); free(s.g2);
  free(s.ple); free(s.tmpP); free(s.trow); free(s.logits); free(s.scores);
  free(s.kcache); free(s.vcache); s={};
  return result;
}
