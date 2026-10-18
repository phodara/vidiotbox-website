#ifndef HAIKU_GRAMMAR_H
#define HAIKU_GRAMMAR_H
#include <stdint.h>
#include <string.h>

enum { G_NOUN=1, G_ADJ, G_VERB, G_TRANSITIVE, G_COPULA, G_DET, G_PREP, G_ADV, G_CONJ, G_LIKE };
enum { G_AIR=1, G_LIGHT=2, G_WATER=4, G_PLANT=8, G_ANIMAL=16, G_GROUND=32, G_SPACE=64, G_ABSTRACT=128, G_BODY=256, G_CELESTIAL=512, G_CRAFT=1024, G_SIGNAL=2048, G_OBSERVER=4096, G_TERRAIN=8192, G_CLOCK=16384, G_MUSIC=32768, G_HUMAN=65536, G_INNER=131072, G_HAND=262144, G_ME=524288, G_WE=1048576, G_ALL=2097151,
       G_NO_PLACE=2097152,     // outside G_ALL: set only on a verb that takes no place phrase
       G_STILL=4194304 };      // outside G_ALL: a person verb without motion takes no path (past...)
enum { G_SUBJECT, G_SUBJECT_NOUN, G_FINITE, G_DONE, G_OBJECT, G_OBJECT_NOUN,
       G_PREDICATE, G_PP, G_PP_NOUN };
typedef struct {
  uint16_t token;
  uint8_t tag, number;
  uint32_t semantic, object;
  uint8_t syl, vowel, bare;
} GEntry;
typedef struct {
  uint32_t subject, object, adjectives;
  uint8_t phase, number, determiner, article, nadj, tail, conj;
} GState;

static inline GState g_start(void) {
  GState s = {0, G_ALL, G_ALL, G_SUBJECT, 0, 0, 0, 0, 0, 0};
  return s;
}
static inline int g_complete(GState s) { return s.phase == G_DONE; }

// A deliberately limited clause grammar, not a general English parser.
static inline int g_advance(GState s, const GEntry *e, GState *out) {
  int subject = s.phase == G_SUBJECT || s.phase == G_SUBJECT_NOUN;
  int np = subject || s.phase == G_OBJECT || s.phase == G_OBJECT_NOUN ||
           s.phase == G_PP || s.phase == G_PP_NOUN;
  if (np) {
    // One adverb may open a clause: "Now time and space shift apart". number 3 marks it
    // used until the subject noun sets the real number.
    if (s.phase == G_SUBJECT && e->tag == G_ADV && e->bare == 3 && s.number == 0 && !s.conj &&
        !s.determiner && !s.nadj) {
      s.number = 3;
      *out = s;
      return 1;
    }
    if (s.article && ((s.article == 2) != !!e->vowel)) return 0;
    if (e->tag == G_DET && !s.determiner && !s.nadj) {
      s.determiner = e->number; // 1=the, 2=a, 3=an, 4=some, 5=my
      s.article = e->number == 2 ? 1 : e->number == 3 ? 2 : 0;
      s.phase = subject ? G_SUBJECT_NOUN : s.phase == G_OBJECT ? G_OBJECT_NOUN : G_PP_NOUN;
    } else if (e->tag == G_ADJ && !s.nadj) {
      if (e->bare == 2) return 0;   // predicate-only: "are one", never "one time"
      s.adjectives &= e->semantic;
      if (!s.adjectives) return 0;
      s.nadj = 1;
      s.article = 0;
    } else if (e->tag == G_NOUN) {
      if (e->semantic & G_HUMAN) {
        if (!subject || s.determiner || s.nadj || s.conj) return 0;
      }
      if ((e->semantic & (G_INNER | G_HAND)) && s.determiner != 5) return 0;
      if (s.determiner == 5 && !(e->semantic & (G_INNER | G_HAND))) return 0;
      if ((e->semantic & G_MUSIC) && (subject || s.object != G_MUSIC)) return 0;
      if (!(s.adjectives & e->semantic)) return 0;
      if (!s.determiner && !e->bare) return 0;
      if ((s.determiner == 2 || s.determiner == 3) && (e->number != 1 || e->bare)) return 0;
      if (s.determiner == 4 && !e->bare) return 0;
      if (!subject && !(s.object & e->semantic)) return 0;
      if (subject) {
        // "Space and time bend": a second subject noun makes the clause plural.
        s.number = s.conj ? 2 : e->number;
        s.subject = s.conj ? (s.subject | e->semantic) : e->semantic;
        s.phase = G_FINITE;
      }
      else {
        // An object ends the verb phrase: no "down". After a simile object no
        // adverb at all, which would read as "like sounds far".
        s.tail |= s.object == G_MUSIC ? 5 : 4;
        s.phase = G_DONE;
      }
      s.article = s.determiner = s.nadj = 0;
      s.adjectives = G_ALL;
    } else return 0;
  } else if (s.phase == G_FINITE) {
    // "now" goes before the verb: "Light now burns in space". It is once per clause and
    // does not use the clause's adverb slot. article 3 marks it until the verb.
    if (e->tag == G_ADV && e->bare == 3) {
      if (s.article == 3) return 0;
      s.article = 3;
      *out = s;
      return 1;
    }
    if (e->tag == G_CONJ) {
      if (s.conj || s.article == 3 || (s.subject & (G_HUMAN | G_INNER | G_HAND))) return 0;
      s.conj = 1;
      s.phase = G_SUBJECT;
      *out = s;
      return 1;
    }
    if (e->number != ((s.number==3 && e->tag!=G_COPULA) ? 2 : s.number) || !(e->semantic & s.subject) ||
        (s.conj && (s.subject & (uint32_t)~e->semantic))) return 0;
    s.article = 0;
    if (e->tag == G_VERB) { s.phase = G_DONE; s.object = e->object; } // adverbs it accepts
    else if (e->tag == G_TRANSITIVE) { s.phase = G_OBJECT; s.object = e->object; }
    else if (e->tag == G_COPULA) s.phase = G_PREDICATE;
    else return 0;
  } else if (s.phase == G_PREDICATE) {
    if (e->tag == G_LIKE && (e->semantic & s.subject)) {
      s.phase = G_OBJECT; s.object = G_MUSIC;
      *out = s; return 1;
    }
    if (e->tag != G_ADJ || !(e->semantic & s.subject)) return 0;
    s.phase = G_DONE;
    s.tail |= 4; // after a predicate adjective, no directional adverb
  } else if (s.phase == G_DONE) {
    // Adverb number 1 is directional ("down") and 2 manner/distance ("far"); both
    // only follow an intransitive verb, never "is dark far" or "pulls light down".
    // "far" and "fast" also need a verb of motion or change: no "the quasar glows far".
    if (e->tag == G_ADV && e->bare != 3 && !(e->bare == 4 && !(s.tail & 2)) &&
        (e->semantic & s.subject) && (e->object & s.object) && !(s.tail & 1) &&
        !(e->number && (s.tail & 4))) s.tail |= 1;
    else if (e->tag == G_PREP && !(s.tail & 2) && !(s.object & G_NO_PLACE) &&
             !(e->number == 1 && (s.object & G_STILL))) {
      s.tail |= 2;
      s.phase = G_PP;
      s.object = G_PLANT | G_GROUND | G_WATER |
        (e->object & (G_SPACE | G_CELESTIAL | G_BODY | G_TERRAIN));
      // A person is placed by bodies, not in substances: "I tremble by moons",
      // never "we wait in dust" or "I wait in soft waves".
      if (s.subject & (G_HUMAN | G_INNER | G_HAND)) s.object &= e->object & (G_CELESTIAL | G_BODY);
    } else return 0;
  } else return 0;
  *out = s;
  return 1;
}

typedef struct { uint64_t key; uint32_t subject, object, adjectives; uint8_t occupied, possible, remaining; } GMemo;
#define G_MEMO_SIZE 512
typedef struct { GMemo cells[G_MEMO_SIZE]; } GWorkspace;

// Low semantic bits and syntax form the hash key; full masks and meter
// are compared separately so human classes cannot alias legacy classes.
static inline uint64_t g_key(GState s, int remaining) {
  (void)remaining;
  return (uint64_t)(s.subject & 65535) | (uint64_t)(s.object & 65535) << 16 | (uint64_t)(s.adjectives & 65535) << 32 |
    (uint64_t)s.phase << 48 | (uint64_t)s.number << 52 | (uint64_t)s.determiner << 54 |
    (uint64_t)s.article << 57 | (uint64_t)s.nadj << 59 | (uint64_t)s.tail << 60 |
    (uint64_t)s.conj << 63;
}

// Lookahead uses only the grammar and meter. The decoder separately enforces
// no-repeat; a feasible grammar path can therefore still lose a decoding try.
static inline int g_can_finish(GWorkspace *w, GState s, int remaining,
                        const GEntry *entries, int count) {
  if (remaining == 0) return g_complete(s);
  if (remaining < 0 || remaining > 7) return 0;
  uint64_t key = g_key(s, remaining);
  unsigned slot = (unsigned)((key ^ (key >> 17) ^ (key >> 31)) % G_MEMO_SIZE);
  unsigned free_slot = G_MEMO_SIZE;
  for (unsigned i = 0; i < G_MEMO_SIZE; i++) {
    unsigned j = (slot + i) % G_MEMO_SIZE;
    if (!w->cells[j].occupied) { free_slot = j; break; }
    if (w->cells[j].key == key && w->cells[j].remaining == remaining &&
        w->cells[j].subject == s.subject && w->cells[j].object == s.object &&
        w->cells[j].adjectives == s.adjectives) return w->cells[j].possible;
  }
  int possible = 0;
  for (int i = 0; i < count && !possible; i++) {
    GState next;
    if (entries[i].token != UINT16_MAX && entries[i].syl <= remaining && g_advance(s, entries + i, &next))
      possible = g_can_finish(w, next, remaining - entries[i].syl, entries, count);
  }
  // Recursive calls may occupy the remembered slot; never overwrite their key.
  if (free_slot < G_MEMO_SIZE && !w->cells[free_slot].occupied) {
    w->cells[free_slot].key = key;
    w->cells[free_slot].subject = s.subject;
    w->cells[free_slot].object = s.object;
    w->cells[free_slot].adjectives = s.adjectives;
    w->cells[free_slot].remaining = (uint8_t)remaining;
    w->cells[free_slot].possible = (uint8_t)possible;
    w->cells[free_slot].occupied = 1;
  }
  return possible;
}
#endif
