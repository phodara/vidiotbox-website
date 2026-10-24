#ifndef HAIKU_GRAMMAR_DECODE_H
#define HAIKU_GRAMMAR_DECODE_H
#include <stdio.h>
#include "haiku_grammar.h"
#include "generated/grammar_tables.h"

static inline const GEntry *haiku_grammar_entry(int token, const GEntry *entries = G_ENTRIES,
                                         int count = G_ENTRY_COUNT) {
  int a = 0, b = count;
  while (a < b) {
    int m = (a + b) / 2;
    if (entries[m].token < token) a = m + 1; else b = m;
  }
  return a < count && entries[a].token == token ? entries + a : nullptr;
}

// The other inflection of a grammar word (light/lights, glow/glows), or -1.
// Marking both as used makes no-repeat cover "some lights and light".
static inline int haiku_word_sibling(int token) {
  const GEntry *entry = haiku_grammar_entry(token);
  return entry && G_SIBLING[entry - G_ENTRIES] != UINT16_MAX ? G_SIBLING[entry - G_ENTRIES] : -1;
}

// A look-alike word (neutron/neutrino, proton/photon), or -1. Its inflection is the
// sibling of the returned token, so marking both covers "neutrinos".
static inline int haiku_word_kin(int token) {
  const GEntry *entry = haiku_grammar_entry(token);
  return entry && G_KIN[entry - G_ENTRIES] != UINT16_MAX ? G_KIN[entry - G_ENTRIES] : -1;
}

// Freshness: content words chosen in recent poems score lower, so the story
// model's few favourites (light, star, shine) give way to the rest of the
// lexicon. Inflections share a count. Each new poem decays every count.
#ifndef HAIKU_FRESH_DECAY
#define HAIKU_FRESH_DECAY 0.9f   // share of each count kept per poem; was 0.7; 0.9 cut the top opening from 140 to 84 per 1,000
#endif
static float haiku_recent[G_ENTRY_COUNT];
static inline int haiku_fresh_index(int token) {
  const GEntry *entry = haiku_grammar_entry(token);
  // Function words are exempt: penalising "in" gives "glows by space".
  if (!entry || entry->tag == G_DET || entry->tag == G_COPULA || entry->tag == G_CONJ ||
      entry->tag == G_LIKE || entry->tag == G_PREP) return -1;
  return (int)(entry - G_ENTRIES);
}
static inline float haiku_freshness_penalty(int token, float weight) {
  int i = weight > 0 ? haiku_fresh_index(token) : -1;
  return i < 0 ? 0.f : weight * haiku_recent[i];
}
static inline void haiku_note_word(int token) {
  int i = haiku_fresh_index(token);
  if (i < 0) return;
  haiku_recent[i] += 1.f;
  int sibling = haiku_fresh_index(haiku_word_sibling(token));
  if (sibling >= 0) haiku_recent[sibling] += 1.f;
}
static inline void haiku_decay_recent(float decay = HAIKU_FRESH_DECAY) {
  for (int i = 0; i < G_ENTRY_COUNT; i++) haiku_recent[i] *= decay;
}

// The space theme uses only entries with a SPACE, ABSTRACT or BODY class. Bodies
// (sun, moon, stars) and abstract nouns (time) are never prepositional objects. The earth seasons skip
// entries that are only space classes and lose those bits on shared ones, so
// their grammar is exactly what it was before the theme existed.
static inline bool haiku_theme_entry(const GEntry &in, bool space_theme, GEntry *out) {
  const uint32_t space_bits = G_SPACE | G_ABSTRACT | G_BODY | G_CELESTIAL | G_CRAFT | G_SIGNAL | G_OBSERVER | G_TERRAIN | G_CLOCK | G_MUSIC | G_HUMAN | G_INNER | G_HAND | G_ME | G_WE;
  *out = in;
  // "Not on fog" is an earth-season rule (2026-10-22); space keeps "shines on light".
  if (space_theme) {
    out->semantic &= ~G_AIRY;
    // Space places its stars and moons freely. Nouns only: verbs use this bit as G_STILL.
    if (out->tag == G_NOUN) out->object &= ~G_ORB_NOUN;
    if (out->tag == G_VERB) out->object &= ~G_ADRIFT;   // halos float on anything in space
    return in.semantic & space_bits;
  }
  // On earth here/there/nearby need no place phrase, and may not follow one: not
  // "through a tree there" (season review 2026-10-22).
  if (out->tag == G_ADV && out->number == 0 && (out->bare == 4 || out->bare == 0)) out->bare = 5;
  if (!(in.semantic & (uint32_t)~space_bits)) return false;
  if (in.tag == G_NOUN && (in.object & G_SPACE_ONLY_NOUN)) return false;   // no "cold wave" on earth
  out->semantic &= (uint32_t)~space_bits;
  return true;
}

// Virtual word IDs are never fed to the model. Each expands atomically into
// a bounded sequence; grammar and syllable counts advance once per word.
#ifdef G_MULTI_COUNT
static inline int haiku_word_length(int word) {
  if (word < G_TOKENIZER_VOCAB) return 1;
  int i = word - G_TOKENIZER_VOCAB;
  return i >= 0 && i < G_MULTI_COUNT ? G_MULTI_LENGTH[i] : 0;
}
static inline int haiku_word_piece(int word, int piece) {
  return word < G_TOKENIZER_VOCAB ? word : G_MULTI_IDS[word - G_TOKENIZER_VOCAB][piece];
}
#endif

// The token or virtual word ID of a grammar word, or -1.
static inline int haiku_word_token(const char *word) {
  for (int i = 0; i < G_ENTRY_COUNT; i++)
    if (!strcmp(G_WORDS[i], word)) return G_ENTRIES[i].token == UINT16_MAX ? -1 : G_ENTRIES[i].token;
  return -1;
}

// Counter the story model's preference for its familiar generic nouns.
// This is a sampling bias, not a learned probability or a quality guarantee.
static inline float haiku_space_vocabulary_bonus(const GEntry &entry, bool space) {
  return space && entry.tag==G_NOUN &&
    (entry.semantic & (G_CELESTIAL | G_CRAFT | G_SIGNAL | G_OBSERVER | G_TERRAIN)) ? 2.0f : 0.f;
}

// Craters belong to moons: "crater" and "craters" are offered only once the poem
// has used a moon word. `used` is the decoder's per-poem used-word bitmap.
static inline bool haiku_moonless_crater(int token, const uint32_t *used) {
  static int ids[6], ready;
  if (!ready) {
    const char *names[6] = {"crater", "craters", "moon", "moons", "moonlight", "lunar"};
    for (int i = 0; i < 6; i++) ids[i] = haiku_word_token(names[i]);
    ready = 1;
  }
  if (token < 0 || (token != ids[0] && token != ids[1])) return false;
  for (int i = 2; i < 6; i++)
    if (ids[i] >= 0 && (used[ids[i] >> 5] & (1u << (ids[i] & 31)))) return false;
  return true;
}

// Space poems draw their place nouns (cloud, wave, dust...) and their intransitive
// verbs (falls, floats...) from random subsets that change every poem, so no one
// word can fill line two of most poems. Bodies, abstract nouns and `protect` (the
// token of "space") always stay. A zero seed keeps everything. Returns the new
// entry count.
static inline int haiku_thin_places(GEntry *entries, int count, uint32_t seed,
                                    int noun_percent, int verb_percent, int protect) {
  if (!seed) return count;
  uint32_t x = seed;
  int kept = 0;
  for (int i = 0; i < count; i++) {
    const GEntry &e = entries[i];
    int percent = 100;
    if (e.tag == G_NOUN && (e.semantic & G_SPACE) && !(e.semantic & (G_BODY | G_ABSTRACT)) &&
        e.token != protect) percent = noun_percent;
    else if (e.tag == G_VERB && (e.semantic & (G_SPACE | G_BODY))) percent = verb_percent;
    if (percent < 100) {
      x ^= x << 13; x ^= x >> 17; x ^= x << 5;
      if ((int)(x % 100) >= percent) continue;
    }
    entries[kept++] = e;
  }
  return kept;
}

// The earth seasons' version: each poem keeps a random share of the nature nouns
// (water, plants, animals, ground, air, light and the sky's bodies) and of the
// intransitive verbs, so "birds float by waves" cannot fill most poems.
// A zero seed keeps everything. Returns the new entry count.
static inline int haiku_thin_season(GEntry *entries, int count, uint32_t seed,
                                    int noun_percent, int verb_percent) {
  if (!seed) return count;
  // Bits 24-31 are animal actions (sing, chirp, swim...): those verbs carry no G_ANIMAL.
  const uint32_t nature = G_AIR | G_LIGHT | G_WATER | G_PLANT | G_ANIMAL | G_GROUND | G_BODY | 0xff000000u;
  uint32_t x = seed;
  int kept = 0;
  for (int i = 0; i < count; i++) {
    const GEntry &e = entries[i];
    int percent = e.tag == G_NOUN && (e.semantic & nature) ? noun_percent :
                  e.tag == G_VERB && (e.semantic & nature) ? verb_percent : 100;
    if (percent < 100) {
      x ^= x << 13; x ^= x >> 17; x ^= x << 5;
      if ((int)(x % 100) >= percent) continue;
    }
    entries[kept++] = e;
  }
  return kept;
}

// A per-poem palette for space: each poem keeps a random share of adjectives, adverbs
// and person verbs (wonder, wait, watch...), so one word cannot win across poems. A word
// and its sibling ("wait", "waits") are kept or dropped together. "now" always stays,
// and if no person verb survives they all stay, so a person's line one stays possible.
// A zero seed or 100 percent keeps everything. Returns the new entry count.
static inline int haiku_thin_palette(GEntry *entries, int count, uint32_t seed,
                                     int adj_percent, int adv_percent, int person_percent) {
  if (!seed) return count;
  auto person_verb = [](const GEntry &e) {
    return (e.tag == G_VERB || e.tag == G_TRANSITIVE) && (e.semantic & (G_HUMAN | G_INNER | G_HAND));
  };
  auto kept = [&](const GEntry &e, bool persons) {
    int percent = e.tag == G_ADJ ? adj_percent : e.tag == G_ADV && e.bare != 3 ? adv_percent :
                  persons && person_verb(e) ? person_percent : 100;
    if (percent >= 100) return true;
    int sibling = haiku_word_sibling(e.token);
    uint32_t x = seed ^ ((uint32_t)(sibling >= 0 && sibling < e.token ? sibling : e.token) * 2654435761u);
    x ^= x >> 16; x *= 0x45d9f3bu; x ^= x >> 16;
    return (int)(x % 100) < percent;
  };
  bool persons = false;
  for (int i = 0; i < count && !persons; i++) persons = person_verb(entries[i]) && kept(entries[i], true);
  int n = 0;
  for (int i = 0; i < count; i++)
    if (kept(entries[i], persons)) entries[n++] = entries[i];
  return n;
}

// "X is bright on Y" wins almost every line because it fits the meter easily.
// A theme can allow it once per poem: after a line uses is/are, drop the
// copulas from the entry set. The caller must clear the lookahead memo, which
// is keyed without the entry set. Returns the new entry count.
static inline bool haiku_word_is_copula(int token, const GEntry *entries, int count) {
  const GEntry *entry = haiku_grammar_entry(token, entries, count);
  return entry && entry->tag == G_COPULA;
}
// Whether a seasonal poem's line three may use is/are, chosen from the poem's place seed
// so the board, the simulator and the website agree (HAIKU_SEASON_COPULA_PERCENT).
static inline bool haiku_keeps_copula(uint32_t seed, int percent) {
  return (int)(((seed * 2246822519u) >> 16) % 100u) < percent;
}

static inline int haiku_drop_copulas(GEntry *entries, int count) {
  int kept = 0;
  for (int i = 0; i < count; i++)
    if (entries[i].tag != G_COPULA) entries[kept++] = entries[i];
  return kept;
}

static inline void haiku_mark_prefix_words(const char *text, void (*mark)(int)) {
  char copy[96];
  snprintf(copy, sizeof copy, "%s", text);
  for (char *p=copy; *p; p++) if (*p>='A' && *p<='Z') *p += 'a'-'A';
  for (char *word=strtok(copy, " .,!"); word; word=strtok(nullptr, " .,!")) {
    int id=haiku_word_token(word);
    if (id>=0) mark(id);
  }
}

static inline bool haiku_grammar_prefix(const char *text, GState *state) {
  *state = g_start();
  char copy[96];
  if (strlen(text) >= sizeof copy) return false;
  snprintf(copy, sizeof copy, "%s", text);
  for (char *p = copy; *p; p++) if (*p >= 'A' && *p <= 'Z') *p += 'a' - 'A';
  for (char *word = strtok(copy, " .,"); word; word = strtok(nullptr, " .,")) {
    const GEntry *entry = nullptr;
    for (int i = 0; i < G_ENTRY_COUNT; i++)
      if (!strcmp(word, G_WORDS[i])) { entry = G_ENTRIES + i; break; }
    GState next;
    if (!entry || !g_advance(*state, entry, &next)) return false;
    *state = next;
  }
  return true;
}

static inline bool haiku_grammar_take(GState *state, int word,
                               const GEntry *entries = G_ENTRIES, int count = G_ENTRY_COUNT) {
  const GEntry *entry = haiku_grammar_entry(word, entries, count);
  GState next;
  if (!entry || !g_advance(*state, entry, &next)) return false;
  *state = next;
  return true;
}

static inline bool haiku_grammar_allows(GWorkspace *workspace, GState state,
                                 int word, int cost, int remaining,
                                 const GEntry *entries = G_ENTRIES, int count = G_ENTRY_COUNT) {
  const GEntry *entry = haiku_grammar_entry(word, entries, count);
  GState next;
  return entry && entry->syl == cost && g_advance(state, entry, &next) &&
    g_can_finish(workspace, next, remaining - cost, entries, count);
}
#endif
