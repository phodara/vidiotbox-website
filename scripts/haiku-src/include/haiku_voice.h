#pragma once
#include "haiku_grammar.h"
#include <string.h>
#ifndef HAIKU_SPACE_CANNED_PERCENT
#define HAIKU_SPACE_CANNED_PERCENT 0
#endif
#ifndef HAIKU_SPACE_PERSON_PERCENT
#define HAIKU_SPACE_PERSON_PERCENT 50
#endif
// Share of first-person space poems whose line three also starts with a person
// ("my heart is silent"), framing the poem. A line that cannot finish so falls back.
#ifndef HAIKU_SPACE_PERSON_END_PERCENT
#define HAIKU_SPACE_PERSON_END_PERCENT 50
#endif
// Per-poem palette (haiku_thin_palette): the share of adjectives, adverbs and person
// verbs a space poem may use. 100 keeps them all. Chosen on 4,000 host poems, 2026-10-15.
#ifndef HAIKU_SPACE_ADJ_PERCENT
#define HAIKU_SPACE_ADJ_PERCENT 70
#endif
#ifndef HAIKU_SPACE_ADV_PERCENT
#define HAIKU_SPACE_ADV_PERCENT 70
#endif
#ifndef HAIKU_SPACE_PERSON_VERB_PERCENT
#define HAIKU_SPACE_PERSON_VERB_PERCENT 50
#endif
static_assert(HAIKU_SPACE_CANNED_PERCENT>=0 && HAIKU_SPACE_CANNED_PERCENT<=100, "invalid canned percentage");
static_assert(HAIKU_SPACE_PERSON_PERCENT>=0 && HAIKU_SPACE_PERSON_PERCENT<=100, "invalid person percentage");
// The pronoun "I" is lowercase in the vocabulary; capitalise it as a whole word.
static inline void haiku_capitalize_i(char *line) {
  for (char *p=line; *p; p++)
    if (*p=='i' && (p==line || *(p-1)==' ') && (!p[1] || p[1]==' ' || p[1]==',' || p[1]=='.')) *p='I';
}
// Whose voice a line is in: 1 for I or my, 2 for we, 0 for none. A leading "now" is skipped.
static inline int haiku_voice_of(const char *line) {
  while (*line == ' ') line++;
  if ((line[0]=='N' || line[0]=='n') && !strncmp(line + 1, "ow ", 3)) line += 4;
  char a = line[0] | 32, b = line[1] | 32;
  if (a == 'i' && (line[1] == ' ' || !line[1])) return 1;
  if (a == 'm' && b == 'y' && line[2] == ' ') return 1;
  if (a == 'w' && b == 'e' && (line[2] == ' ' || !line[2])) return 2;
  return 0;
}
// Line three in a given voice (haiku_voice_of): 1 allows I and my, 2 allows only we.
static inline GState haiku_voice_start_as(int voice) {
  GState state=g_start();
  state.adjectives=voice==1 ? G_ME | G_INNER | G_HAND : voice==2 ? G_WE : G_HUMAN | G_INNER | G_HAND;
  return state;
}
static inline GState haiku_voice_start(bool person) {
  GState state=g_start();
  state.adjectives=person ? G_HUMAN | G_INNER | G_HAND : 65535;
  return state;
}
