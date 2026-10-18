#pragma once
#include "generated/haiku_tables.h"

static bool haiku_season_allows(int token, int season) {
  if (token < 0 || token >= HAIKU_VOCAB || season < 0 || season >= HAIKU_N_SEASONS) return false;
  int a = 0, b = HAIKU_N_SEASON_WORDS;
  while (a < b) {
    int m = (a + b) / 2;
    if (HAIKU_SEASON_WORDS[m].token < token) a = m + 1; else b = m;
  }
  return a == HAIKU_N_SEASON_WORDS || HAIKU_SEASON_WORDS[a].token != token ||
         (HAIKU_SEASON_WORDS[a].mask & (1u << season));
}
