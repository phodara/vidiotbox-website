#ifndef HAIKU_GRAMMAR_H
#define HAIKU_GRAMMAR_H
#include <stdint.h>
#include <string.h>

enum { G_NOUN=1, G_ADJ, G_VERB, G_TRANSITIVE, G_COPULA, G_DET, G_PREP, G_ADV, G_CONJ, G_LIKE };
enum { G_AIR=1, G_LIGHT=2, G_WATER=4, G_PLANT=8, G_ANIMAL=16, G_GROUND=32, G_SPACE=64, G_ABSTRACT=128, G_BODY=256, G_CELESTIAL=512, G_CRAFT=1024, G_SIGNAL=2048, G_OBSERVER=4096, G_TERRAIN=8192, G_CLOCK=16384, G_MUSIC=32768, G_HUMAN=65536, G_INNER=131072, G_HAND=262144, G_ME=524288, G_WE=1048576, G_ALL=2097151,
       G_NO_PLACE=2097152,     // outside G_ALL: set only on a verb that takes no place phrase
       G_STILL=4194304,        // outside G_ALL: a person verb without motion takes no path (past...)
       G_GAZE=8388608,         // outside G_ALL: a person verb whose place phrase is "at" ("I blink at stars")
       G_WET=16777216,         // outside G_ALL: a verb whose place is water ("ducks swim in the pond")
       G_NOT_SOLID=33554432,   // preposition object bit: "in" and "through" take no solid noun
       G_NOT_AIRY=67108864,    // preposition object bit: "on" and "upon" take no airy noun
       G_NOT_SURFACE=134217728, // preposition object bit: "in" takes no surface noun ("the path")
       G_ALOFT=268435456,      // verb object bit: flying takes no "on" ("crows fly over the lake")
       G_NO_POOL=536870912,    // PP flag: flying "in"/"through" takes no water but rain, fog, mist
       G_LOOSE=1073741824,     // verb object bit: falls/drifts/floats, which a pond or lake cannot
       G_SKYWARD=1073741824,   // preposition object bit: over/across/through, where the sky's bodies go
       G_ONLY_AIRY=1073741824, // PP state bit: the sky's bodies are "in" only fog, mist, rain, air, light
       G_LIT_POOL=268435456,   // PP state bit: light goes "on" a lake, not "through" it
       G_DOWNWARD=33554432,    // verb object bit: falling lands "on" plants, not "in" them
       G_NOT_PLANT=8388608,    // PP state bit: after "falls in", or a plant "in", no plant
       G_BELOW=16777216,       // preposition object bit: under, beneath
       G_ADRIFT=67108864,      // verb object bit: floating is "on" water only
       G_NOT_WATER=2097152,    // PP state bit: wind goes "over" water, not in, through or under it
       G_ONLY_POOL=4194304,    // PP state bit: "floats on" a pond, lake, stream or water
       G_GROUNDED=134217728,   // verb object bit: crawling is not in or through water
       G_NO_POOL_NOUN=16777216 }; // PP state bit: no pond, lake, puddle, stream, brook or water
// Noun object-field marks (a noun's object field is otherwise unused). Kept in GState.tail
// (8, 16) while the clause runs; the lookahead memo compares tail in full.
#define G_POOL_NOUN 2097152u   // pond, lake, puddle, stream, brook: they stay put
#define G_ORB_NOUN 4194304u    // sun, moon, star in the earth seasons: placed over or through things
#define G_ROOTED_NOUN 8388608u // tree, branch, grass: they do not fall or drift (tail 32)
#define G_SINGLE_FOREST 1073741824u // singular tree/pine/vine/branch is rejected after "through"
#define G_BEE_NOUN 536870912u // bees are placed over a pond, not on it
#define G_SINGLE_MOON 268435456u // singular moon similes are rejected
#define G_SPACE_ONLY_NOUN 16777216u   // wave, waves: space only, not the earth seasons
#define G_EARTH_PHRASE 0x80000000u    // noun: a multi-word noun the seasons may use, "waves on the pond"
#define G_MOON_NOUN 33554432u   // moon, moons: all that "full" describes (review round 7)
#define G_STONE_NOUN 67108864u  // stone, rock: no bird sings on one (review round 7)
#define G_LONE_NOUN 134217728u   // flash: not one of two subjects, "density and a flash shift"
#define G_MOON_ONLY 0x80000000u // adjective object bit: "full"
#define G_WET_ADJ 0x40000000u   // adjective object bit: "wet", "damp"
#define G_THAW 0x80000000u      // intransitive verb object bit: "melt", which a wave cannot
#define G_STILL_ADJ 0x20000000u // adjective object bit: "still" (motionless), only after is/are, no adverb after it
// A wave on water as subject: marked in GState.subject with a bit no verb, adjective or adverb
// carries (G_SURFACE describes only place nouns), so it changes no match.
#define G_WAVE_SUBJECT G_SURFACE
// Noun limits kept in GState.adjectives above G_ALL until the next noun (the lookahead
// memo compares adjectives in full).
#define G_LIMIT_MOON 0x80000000u   // after "full": the moon only, not "full stars"
#define G_LIMIT_TREE 0x40000000u   // a bird "in" a plant is in a tree or reeds, not "a soft leaf"
#define G_LIMIT_STONE 0x20000000u  // a bird "on" no stone or rock
#define G_LIMIT_WET 0x04000000u    // after "wet", "damp": no pond, stream, water or wave on water
#define G_LIMIT_WEATHER 0x01000000u // after "across": no fog, mist, rain, dew, air, wind or light
#define G_LIMIT_NOAIR 0x02000000u  // after "soft": not the word "air", nor a wall, stone or hill (2026-10-31)
#define G_LIMIT_FIELD 0x00800000u  // after water "in"/"on": no field, meadow or garden
#define G_LIMIT_NOTINY 0x00400000u // after "small"/"tiny": not a worm
#define G_LIMIT_DARK 0x10000000u   // after "dark": no sunlight, moonlight, starlight or light (shares G_IN_A_FLASH's bit, never live together)
#define G_LIMIT_ABOVE 0x00200000u  // after flying "under"/"beneath": no ground (a bridge is fine)
#define G_OLD_ADJ 0x10000000u      // adjective object bit: "old", which no stone or rock needs
#define G_LIMITS (G_LIMIT_MOON | G_LIMIT_TREE | G_LIMIT_STONE | G_LIMIT_WET | G_LIMIT_WEATHER | G_LIMIT_NOAIR | G_LIMIT_FIELD | G_LIMIT_NOTINY | G_LIMIT_ABOVE | G_LIMIT_DARK)
// Subject adjectives bit from "in a flash" (bare 7) to the verb, which must be intransitive.
#define G_IN_A_FLASH 0x10000000u
#define G_LONE_SUBJECT 0x08000000u   // the subject is a flash: no "and" after it
#define G_LIMIT_INWATER 0x08000000u  // after a ground object ("coats a field"): no "in water"; shares G_LONE_SUBJECT's bit, which ends at the verb
// Intransitive verb object bit: a change that can happen "in a flash" (shifts, vanishes), not
// "drifts" or "floats". G_NO_POOL shares the bit, but only after a preposition.
#define G_SUDDEN 536870912u
#define G_ACT_FLY (1u << 28)                    // animal action bits (generate_grammar_tables.py)
#define G_ACT_HUM_CRAWL ((1u << 26) | (1u << 30))   // bees and beetles fly but are not birds
// PP state bit 31 (outside the enum): the sky's bodies go "through" only weather or
// foliage, not "through mud" or "through streams" (season review round 5).
#define G_SKY_THROUGH 0x80000000u
// PP state flag (2026-11-01): after crawling or burrowing "in" or "through", soil and earth are not solid. It shares
// G_HUMAN's bit, which a place noun never has: a person is only ever a subject.
#define G_IN_SOIL G_HUMAN
// PP state flags (2026-11-01) on the bits of "I" and "we", which are never places either. Under or beneath, for an animal
// that does not crawl: no grass, moss or ivy ("frogs call under grass"). By, near, beside, along or around: not earth or
// soil, which are everywhere ("an oak grows near earth").
#define G_UNDER_LOW G_ME
#define G_BESIDE G_WE
// After a ground object, in, on or through: no tree, branch, pine, oak or vine ("rain coats wet stones in a pine").
#define G_NOT_TREE G_INNER
// The space theme's classes: the seasons strip them from every word, and every space word has one (haiku_theme_entry).
#define G_SPACE_CLASSES (G_SPACE | G_ABSTRACT | G_BODY | G_CELESTIAL | G_CRAFT | G_SIGNAL | G_OBSERVER | G_TERRAIN | G_CLOCK | \
                         G_MUSIC | G_HUMAN | G_INNER | G_HAND | G_ME | G_WE)
// Semantic bits outside G_ALL (the object field uses these values for G_NO_PLACE and
// G_STILL): a solid thing ("stone", "fence") nothing goes into, and a thing with no
// surface to be on: airy ("fog", "wind", "light") or an area one is in ("field").
#define G_SOLID 2097152u
#define G_AIRY 4194304u
#define G_SURFACE 8388608u   // a surface one goes along or through, not "in": path, shore
enum { G_SUBJECT, G_SUBJECT_NOUN, G_FINITE, G_DONE, G_OBJECT, G_OBJECT_NOUN,
       G_PREDICATE, G_PP, G_PP_NOUN };
typedef struct {
  uint16_t token;
  uint8_t tag, number;
  uint32_t semantic, object;
  uint8_t syl, vowel, bare;
  uint8_t extra;   // GX_* word flags (below), read by tag; fills what was padding, so an entry stays 20 bytes
  uint32_t marks;  // G_ORBIT_RANK, G_PAIR_MARKS, G_MARK_DRY, G_ORBIT_VERB (below)
} GEntry;
// GEntry.extra, V8.17 (review 2026-10-30). The bits mean different things for each tag.
#define GX_THE_ONLY 1     // noun: cosmos, universe take "the" only, never "a cosmos"
#define GX_NO_DET 2       // noun: space takes no article: "in space", never "in the space"
#define GX_BIG_PLACE 4    // noun: hills, shores, fields, trees, walls...: too big a place for a worm
#define GX_SIZELESS 8     // noun: dew, rain, mist, mud, light, dust...: no "small dew", "huge stardust"
#define GX_DROP 16        // noun: raindrop(s), snowflake(s): a plural subject holds raindrops, not "the raindrop" (2026-10-31);
                          // a place for light only (2026-11-01)
#define GX_WATERY 32      // noun: dew, rain, raindrops, water: wet already, so not "wet" or "damp" (2026-10-31)
#define GX_SMALL_PART 64  // noun: petal(s), twig(s), leaf, flower, bud: nothing goes "through" one (2026-10-31)
#define GX_BUILT 128      // noun: walls, fences, gates, bridges: waves and standing water do not wash them (2026-10-31)
#define GX_SIZE 1         // adjective: small, tiny, huge, giant, large, tall (never after is/are)
#define GX_WEAR 2         // adjective: worn, weathered, cracked; after is/are, for G_MARK_IMMOVABLE subjects
#define GX_HUSH 4         // adjective: quiet, calm: never a plant, stone, wall or hill, "quiet trees", "a rock is quiet" (2026-10-31)
#define GX_BARE 8         // adjective: bare: its subject does not bloom, "bare trees bloom" (2026-10-31)
#define GX_WARM 16        // adjective: warm: light and the sun only in autumn and winter (haiku_season_entry, 2026-11-01)
#define GX_TEMP 32        // adjective: hot, warm, cold, cool: never after is/are for a wall, stone or hill (2026-11-01)
#define GX_STILL_VERB 1   // verb: crumble, lean, remain, stand; G_MARK_IMMOVABLE subjects only
#define GX_BURN 2         // verb: burn, burns (not "moonlight burns" on earth)
#define GX_SOUND 4        // verb: sing, chirp, hum, howl, whisper, rustle: not after a quiet or calm subject (2026-10-31)
#define GX_BEE_VERB 8     // verb: buzz, buzzes, swarm, favoured after a bee (haiku_bee_verb_bias)
#define GX_SHINE 16       // verb: shine, shines: not after trees, branches, pines, oaks, vines or roots (2026-10-31)
#define GX_GROW 32        // verb: grow, grows: not "softly" or "gently" in the seasons (2026-11-01)
#define GX_DRIFT 64       // verb: drift, drifts: birds instead glide or fly (review 2026-11-02)
#define GX_GLIDE 128      // verb: glide, glides: birds only, with flying's place restrictions
#define GX_GENTLE 1       // adverb: softly, gently (not "a bridge stands gently")
#define GX_DEIXIS 2       // adverb: here, there (the sun, moon and stars are never "here")
typedef struct {
  uint32_t subject, object, adjectives;
  uint8_t phase, number, determiner, article, nadj, tail, conj;
  uint32_t marks;  // the subject's rank and pair marks until the verb; then the rank (orbit verbs only), dry, G_ORBIT_VERB
} GState;
// Review 2026-10-26: "planets orbit moons". A body orbits only a larger one: moon 2 < planet,
// asteroid, comet, meteor 3 < star, sun, pulsar, sphere of gas or light 4 < galaxy, quasar 5.
// A ranked subject needs a higher-ranked object; an unranked one (halos) may circle anything.
// Planets and the other rank-3 bodies orbit stars and the sun only, not a galaxy or quasar.
// Rank 7 (nebula, supernova) neither orbits nor is orbited.
#define G_ORBIT_VERB 0x80
#define G_ORBIT_RANK 0x07
// Review 2026-10-26: subjects and verbs that share a pair mark do not go together. A subject
// noun carries its marks to the verb, which is refused if it has one of them too.
#define G_MARK_AIR 0x08      // air does not blow or howl; wind, gusts and breezes do
#define G_MARK_FROZEN 0x10   // snow, frost, ice, fog, mist, dew do not flow or ripple
#define G_MARK_FALLING 0x20  // snow, rain, dew, frost, ice do not rise or ripple
#define G_MARK_GROWN 0x20    // state after the verb: grow or grows (FALLING's bit: a pair mark a non-copula verb drops)
#define G_MARK_STILLWATER 0x100  // pond, lake, puddle stand still: they do not flow or wash; "a pond is soft" is out too
#define G_MARK_POOLISH 0x200     // pond, lake, puddle, stream, brook, water: none covers a garden
#define G_PAIR_MARKS (G_MARK_AIR | G_MARK_FROZEN | G_MARK_FALLING | G_MARK_STILLWATER | G_MARK_POOLISH | G_MARK_NOBLOOM | G_MARK_TINY | G_MARK_BRIGHT | G_MARK_COLD | G_MARK_IMMOVABLE | G_MARK_NOPULL | G_MARK_NOMERGE | G_MARK_ALL | G_MARK_FOREVER)
// Verb marks carried to the place phrase (review 2026-10-27): drift and float refuse "a dry meadow";
// a moss or ivy subject (rank 1) is a creeper, the only plant that grows "on" a stone.
#define G_MARK_DRIFTY 0x400
#define G_MARK_CREEPER 0x800
#define G_MARK_HUSHED 0x800   // a quiet or calm subject, until its verb: no word carries the bit, and the verb rebuilds the marks
#define G_MARK_NOBLOOM 0x1000      // petals, leaves, roots and twigs do not bloom
#define G_MARK_EARTH_WORD 0x2000    // a noun the tokenizer splits (hillside, ridge) that the seasons may still use
#define G_MARK_WATERWORD 0x10000 // the word "water": not "in" a field, meadow or garden (rain and mist are)
#define G_MARK_JUMPY 0x20000     // jump, jumps: not over a tree
#define G_MARK_OVERPASS 0x80000   // bridge, bridges: the one ground noun a bird flies under
#define G_MARK_WARMED 0x80000     // state: "warm" before its noun: not the moon, a star, fog, mist, dew, snow or ice (shares OVERPASS's noun-only bit; 2026-11-01)
#define G_MARK_NEEDS_PLACE 0x2000  // stay, stays: "a universe stays" is unfinished without a place (shares EARTH_WORD's bit; nouns only carry that one)
#define G_MARK_NOPULL 0x10000000   // vacuum, moon, moons: they pull nothing
#define G_MARK_ALL 0x4000          // universe, cosmos, stay, stays: everything cannot stay in anything (shares G_MARK_TREE's state-only bit)
#define G_MARK_FOREVER 0x8000      // photon, photons, faint, fade, fades: a photon never fades or grows faint (shares G_MARK_STILLNOW's state-only bit)
#define G_MARK_NOMERGE 0x20000000  // fusion, hydrogen, helium, deuterium: atoms and particles merge, these do not
#define G_MARK_NOGRASP 0x40000000  // neutrino, proton, photon...: nothing subatomic is grasped
#define G_MARK_STONY 0x40000000    // state: a stone or rock subject, never by, near or in stones or rocks (shares NOGRASP's noun-only bit; 2026-11-01)
#define G_MARK_GRASPY 0x80000000u  // grasp, grasps, hold, holds (carried to the object; after a plant's hold, no place phrase)
#define G_MARK_BIRD 0x80000000u    // subject until the verb: bird, crow, gull, duck; rebuilt at the verb before GRASPY is used
#define G_MARK_ICE 0x4000000      // ice: not on or in grass, moss or ivy (frost is)
#define G_MARK_IMMOVABLE 0x8000000 // still, and bridges, walls, stones, hills...: "a bridge is still" says nothing
#define G_MARK_DEW 0x1000000      // dew: not "in" a pond, lake or stream
#define G_MARK_ROOT 0x2000000     // root, roots: no bird sits on or in them
#define G_MARK_FLYER 0x800000     // bird, crow, gull, duck, bee: never under or beneath the ground
#define G_MARK_COLD 0x400000      // moon, moonlight, star, starlight: never "warm" anything
#define G_MARK_DUCK 0x100000      // duck, ducks: not on or in a tree
#define G_MARK_BRIGHT 0x200000    // dark, sunlight, moonlight, starlight, light: not "dark sunlight"
#define G_MARK_TINY 0x40000      // worm, worms, small, tiny: a worm is small already
#define G_MARK_TREE 0x4000      // state: the subject is a tree, branch or vine
#define G_MARK_STILLNOW 0x8000  // state: the predicate is "still" (motionless): nothing goes through, over or across
#define G_LOW_PLANT_BIT 1048576u // noun object bit: set on every noun, cleared on grass, moss and ivy
#define G_KEEP_MARKS (G_MARK_DRY | G_MARK_DRIFTY | G_MARK_CREEPER | G_MARK_TREE)
// On the adjective "dry": a dry subject takes no water and is not wet ("a dry pine holds rain").
#define G_MARK_DRY 0x40
// Review 2026-10-27: "Still reeds shade a stone in fog". Trees, branches, pines, oaks and vines carry rank 6
// and the verb "shade" needs it; shade also takes no fog, mist, dew or rain in its clause.
// The verb's state is rank 6 without G_ORBIT_VERB.
#define G_SHADE_RANK 6
#define G_PETAL_RANK 7   // petal, petals share the nebula's rank 7; in the seasons petals do not shine (2026-11-01)

static inline GState g_start(void) {
  GState s = {0, G_ALL, G_ALL, G_SUBJECT, 0, 0, 0, 0, 0, 0, 0};
  return s;
}
static inline int g_complete(GState s) {
  return s.phase == G_DONE && !((s.marks & G_MARK_NEEDS_PLACE) && !(s.tail & 2));   // "stays" needs a place phrase
}

// A deliberately limited clause grammar, not a general English parser.
static inline int g_advance(GState s, const GEntry *e, GState *out) {
  int subject = s.phase == G_SUBJECT || s.phase == G_SUBJECT_NOUN;
  // Review 2026-10-30: "a full moon fades there", "a pale sun glows here". The seasons only:
  // space clears G_ORB_NOUN, so s.tail bit 16 is never set there.
  if ((s.tail & 16) && !subject && e->tag == G_ADV && (e->extra & GX_DEIXIS)) return 0;
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
      s.adjectives &= e->semantic | G_LIMITS;
      if (e->extra & GX_HUSH) s.adjectives &= ~(uint32_t)G_PLANT;   // a tree makes no sound to be quiet about
      if (!(s.adjectives & G_ALL)) return 0;
      if (subject && (e->extra & GX_BARE)) s.marks |= G_MARK_NOBLOOM;
      if (subject && (e->extra & GX_HUSH)) s.marks |= G_MARK_HUSHED;
      if (e->object & G_MOON_ONLY) s.adjectives |= G_LIMIT_MOON;
      if (e->object & G_WET_ADJ) s.adjectives |= G_LIMIT_WET;
      if (subject && (e->marks & G_MARK_DRY)) s.marks |= G_MARK_DRY;
      if (subject && (e->marks & G_MARK_FOREVER)) s.marks |= G_MARK_FOREVER;   // "faint": not before "photon"
      if (e->marks & G_MARK_COLD) s.marks |= G_MARK_WARMED;   // "warm", in any noun phrase
      if (!subject && (s.marks & G_MARK_DRIFTY) && (e->marks & G_MARK_DRY)) return 0;   // not "twigs drift through a dry meadow"
      if (e->object & G_OLD_ADJ) s.adjectives |= G_LIMIT_STONE;   // all stones are old: not "an old stone"
      if (e->marks & G_MARK_AIR) s.adjectives |= G_LIMIT_NOAIR;    // soft air is always wrong
      if ((e->marks & G_MARK_TINY) || (e->extra & GX_SIZE)) s.adjectives |= G_LIMIT_NOTINY;  // a worm is small already; dew has no size
      if (e->marks & G_MARK_BRIGHT) s.adjectives |= G_LIMIT_DARK;  // not "dark sunlight"
      s.nadj = 1;
      s.article = 0;
    } else if (e->tag == G_NOUN) {
      // Review 2026-10-30 (V8.17).
      if ((e->extra & GX_THE_ONLY) && s.determiner != 1) return 0;   // "the cosmos", never "a cosmos"
      if ((e->extra & GX_NO_DET) && s.determiner) return 0;          // "space", never "the space"
      // Review 2026-10-31: "branches hold the dark raindrop". Many things hold raindrops, not one.
      if ((s.phase == G_OBJECT || s.phase == G_OBJECT_NOUN) && s.number == 2 && (e->extra & GX_DROP) && e->number == 1) return 0;
      // Review 2026-10-31: "branches shade a rock in fields". After "in" (G_NOT_SOLID with G_NOT_SURFACE), a bare plural
      // place needs a plural thing in it: "a rock in a field", "a rock in the fields", "rocks in fields", and mass
      // things fill one: "grass grows in fields" (number 0). s.number is the subject's until an object noun replaces it
      // (below); after the verb only these two rules read it.
      // 2026-11-01: "a tree blooms on hills" should be "trees bloom on a hill"; "on" and "upon" (G_NOT_AIRY) too. A wall
      // or stone is never on anything, and carries G_NOT_AIRY for "by" and "along" ("a wall stands by rocks"). The seasons
      // only: every space noun keeps a space class (haiku_theme_entry), and space poems stay as they were.
      if ((s.phase == G_PP || s.phase == G_PP_NOUN) &&
          (((s.object & G_NOT_SOLID) && (s.object & G_NOT_SURFACE)) ||
           ((s.object & G_NOT_AIRY) && !(s.marks & G_MARK_IMMOVABLE) && !(e->semantic & G_SPACE_CLASSES))) &&
          !s.determiner && e->number == 2 && s.number == 1) return 0;
      if (!subject && (s.tail & 2)) {
        if ((s.object & G_UNDER_LOW) && (e->semantic & G_PLANT) && !(e->object & G_LOW_PLANT_BIT)) return 0;   // not "frogs call under grass"
        if ((s.object & G_NOT_TREE) && (e->marks & G_ORBIT_RANK) == G_SHADE_RANK) return 0;   // not "wet stones in a pine" (2026-11-01)
        if ((s.object & G_BESIDE) && (e->semantic & G_SOLID) && (e->extra & GX_SIZELESS)) return 0;   // not "an oak grows near earth"
        // Nothing sings, rests or grows "in", "under" or "through" ice.
        if ((s.subject & (G_ANIMAL | G_PLANT)) && (s.object & (G_NOT_SURFACE | G_NOT_SOLID)) && (e->marks & G_MARK_ICE)) return 0;
        // A worm keeps to small places: soil, mud, a stone or leaf, grass, a path; not hills, shores or trees.
        if ((s.marks & G_MARK_TINY) && (e->extra & GX_BIG_PLACE)) return 0;
        // Worms and roots stay off water: not "worms crawl on the puddles", "roots grow on the lake" (rain is fine).
        if ((s.marks & (G_MARK_TINY | G_MARK_ROOT)) && (e->semantic & G_WATER) && !(e->semantic & G_AIRY)) return 0;
        // A wall or stone is in rain, mist or fog, not "in the dew"; and by things, not "by frost" or "by soil".
        if ((s.marks & G_MARK_IMMOVABLE) && ((e->marks & G_MARK_DEW) || (!(s.object & G_ONLY_AIRY) && (e->extra & GX_SIZELESS)))) return 0;
      }
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
      // Classes only: rule bits above G_ALL (G_NOT_SOLID...) must not match animal-action bits.
      if (!subject && !(s.object & e->semantic & G_ALL)) return 0;
      // Not "through a stone". Crawling and burrowing go in or through soil and earth, though (G_IN_SOIL, 2026-11-01:
      // "cicadas burrow in the soil"); stones stay solid, and nothing else is in the soil ("the cicada leaps in earth").
      if ((s.object & G_NOT_SOLID) && (e->semantic & G_SOLID) && !((s.object & G_IN_SOIL) && (e->extra & GX_SIZELESS))) return 0;
      if ((s.object & G_NOT_AIRY) && (e->semantic & G_AIRY)) return 0;     // not "fades on fog"
      if ((s.object & G_NOT_SURFACE) && (e->semantic & G_SURFACE)) return 0; // not "shines in the path"
      if ((s.object & G_NO_POOL) && (e->semantic & G_WATER) && !(e->semantic & G_AIRY)) return 0; // not "fly in the pond"
      // Weather only, not "in waves" or "in a field" (fields are G_AIRY for "not on").
      if ((s.object & G_ONLY_AIRY) && (!(e->semantic & G_AIRY) || (e->semantic & G_GROUND))) return 0;
      if ((s.object & G_LIT_POOL) && (e->object & (G_POOL_NOUN | G_EARTH_PHRASE))) return 0;   // not "light through a lake"
      if ((s.object & G_NOT_PLANT) && (e->semantic & G_PLANT)) return 0;     // not "falls in leaves"
      if ((s.object & G_NOT_WATER) && (e->semantic & G_WATER) && !(e->semantic & G_AIRY)) return 0;
      if ((s.object & G_ONLY_POOL) && !(e->object & G_POOL_NOUN)) return 0;  // not "float on ice"
      // Not "bees hum in the lake", nor (2026-10-25) "roots hold a wave on the stream".
      if ((s.object & G_NO_POOL_NOUN) && (e->object & (G_POOL_NOUN | G_EARTH_PHRASE))) return 0;
      if ((s.object & G_SKY_THROUGH) && (e->object & G_SINGLE_FOREST)) return 0; // use "through the trees/pines/vines/branches"
      // Review 2026-10-31: "twigs drift through a green petal". Nothing goes through a single petal, twig, leaf, flower
      // or bud; "through the leaves" stays.
      // The air rule above turns "through" into "in" for the word "air"; air is in no twig either.
      if (!subject && (s.object & G_NOT_SOLID) && (!(s.object & G_NOT_SURFACE) || ((s.marks & G_MARK_AIR) && !(s.subject & G_LIGHT))) &&
          (e->extra & GX_SMALL_PART)) return 0;
      // Review 2026-10-31: anything goes through branches, trees, pines, vines and oaks, not one ("sunlight glows through
      // the bare branch", "the wind rises through a branch", "crows fly through the oak"). "through" is the one
      // preposition with G_NOT_SOLID and no G_NOT_SURFACE; the air rule turns it into "in" for the word "air".
      if (!subject && (s.object & G_NOT_SOLID) && (!(s.object & G_NOT_SURFACE) || ((s.marks & G_MARK_AIR) && !(s.subject & G_LIGHT))) &&
          (e->object & G_SINGLE_FOREST)) return 0;
      // Review 2026-10-31 (autumn batch 2). "in" is G_NOT_SOLID with G_NOT_SURFACE.
      if (!subject && (s.object & G_NOT_SOLID) && (s.object & G_NOT_SURFACE)) {
        // "crickets chirp in a dark leaf": nothing is in a single leaf, petal, twig, flower or bud (on it is fine).
        if (e->extra & GX_SMALL_PART) return 0;
      }
      // "branches sway in a raindrop": nothing but light is in a raindrop or snowflake, nor (2026-11-01) through, on, under
      // or by one ("frogs swim through a warm raindrop", "vines sway on a dark raindrop"). Light shines through one.
      if (!subject && (s.tail & 2) && (e->extra & GX_DROP) && !(s.subject & G_LIGHT)) return 0;
      // "crickets sing beneath the hill", "the water flows under hills": nothing is under a hill, hillside, hilltop, ridge,
      // shore or cliff (the immovable nouns with G_MARK_NOPULL). "under" sets G_NOT_SURFACE without G_NOT_SOLID; so does
      // the air rule's "over" for the word "air", which this leaves alone.
      if (!subject && (s.object & G_NOT_SURFACE) && !(s.object & G_NOT_SOLID) && !((s.marks & G_MARK_AIR) && !(s.subject & G_LIGHT)) &&
          (e->marks & G_MARK_IMMOVABLE) && (e->marks & G_MARK_NOPULL)) return 0;
      // "the wind rises through a path": wind and air go over or along a path or shore, not through it.
      if (!subject && (s.subject & G_AIR) && (s.object & G_NOT_SOLID) && !(s.object & G_NOT_SURFACE) && (e->semantic & G_SURFACE)) return 0;
      // "the wind blows on soil": wind, breezes and air are not "on" soil, earth or mud (G_NOT_AIRY marks "on").
      if (!subject && (s.subject & G_AIR) && (s.object & G_NOT_AIRY) && (e->extra & GX_SIZELESS) && (e->semantic & G_GROUND)) return 0;
      // ...nor in or through them: "air rises slowly through mud" (2026-11-01).
      if (!subject && (s.subject & G_AIR) && (s.object & G_NOT_SOLID) && (e->extra & GX_SIZELESS) && (e->semantic & G_GROUND)) return 0;
      if ((s.tail & 64) && (s.object & G_ONLY_POOL) && (e->object & G_POOL_NOUN)) return 0; // bees float over ponds
      if ((s.adjectives & G_LIMIT_MOON) && !(e->object & G_MOON_NOUN)) return 0;   // a full moon only
      if ((s.adjectives & G_LIMIT_TREE) && (e->semantic & G_PLANT) && !(e->object & G_ROOTED_NOUN)) return 0;
      // ...nor in grass, moss or ivy: "crows are still in grass" is better "in trees" (2026-11-01); on the grass is fine.
      if ((s.adjectives & G_LIMIT_TREE) && (e->semantic & G_PLANT) && !(e->object & G_LOW_PLANT_BIT)) return 0;
      if ((s.adjectives & G_LIMIT_STONE) && (e->object & G_STONE_NOUN)) return 0;
      // Review 2026-10-31: "an old bee buzzes". Bees, crickets, beetles and worms (the hum and crawl actions) are not old.
      if ((s.adjectives & G_LIMIT_STONE) && (e->semantic & G_ACT_HUM_CRAWL)) return 0;
      if ((s.adjectives & G_LIMIT_WET) && ((e->object & (G_POOL_NOUN | G_EARTH_PHRASE)) || (e->extra & GX_WATERY))) return 0; // not "wet waves", "damp dew"
      if ((s.object & G_SKY_THROUGH) && !(((e->semantic & G_AIRY) && !(e->semantic & G_GROUND)) ||
                                          (e->semantic & G_PLANT))) return 0;   // not "a sun fades through mud"
      const int rank = (int)(e->marks & G_ORBIT_RANK);
      if (!subject && (s.marks & G_ORBIT_VERB) &&
          (rank <= (int)(s.marks & G_ORBIT_RANK) || rank == (int)G_ORBIT_RANK ||
           rank == G_SHADE_RANK || ((int)(s.marks & G_ORBIT_RANK) == 3 && rank == 5))) return 0;   // not "planets orbit moons", nor the quasar: stars, the sun
      if (!subject && (s.marks & G_MARK_DRY) && (e->semantic & G_WATER)) return 0;   // not "a dry leaf holds dew"
      if (!subject && (s.marks & (G_ORBIT_VERB | G_ORBIT_RANK)) == G_SHADE_RANK &&
          (e->semantic & G_WATER) && (e->semantic & G_AIRY)) return 0;   // not "pines shade a stone in fog"
      if (!subject && (s.marks & (G_ORBIT_VERB | G_ORBIT_RANK)) == G_SHADE_RANK && rank == G_SHADE_RANK) return 0;   // not "vines shade a pine"
      if ((s.adjectives & G_LIMIT_WEATHER) && (e->semantic & G_AIRY) && !(e->semantic & G_GROUND)) return 0;   // not "grass grows across rain"
      if ((s.adjectives & G_LIMIT_NOAIR) && (e->marks & (G_MARK_AIR | G_MARK_IMMOVABLE))) return 0;      // not "soft air", "a soft wall"
      if ((s.adjectives & G_LIMIT_NOTINY) && (e->marks & G_MARK_TINY)) return 0;    // not "a small worm"
      if ((s.adjectives & G_LIMIT_NOTINY) && (e->extra & GX_SIZELESS)) return 0;    // not "small dew", "huge stardust"
      if ((s.adjectives & G_LIMIT_DARK) && (e->marks & G_MARK_BRIGHT)) return 0;     // not "dark sunlight"
      if ((s.marks & G_MARK_WARMED) && (e->marks & (G_MARK_COLD | G_MARK_FROZEN))) return 0;   // not "warm moonlight", "warm stars", "warm fog" (2026-11-01)
      s.marks &= ~(uint32_t)G_MARK_WARMED;
      if (!subject && (s.marks & G_MARK_ICE) && (s.object & (G_NOT_AIRY | G_NOT_SURFACE)) && (e->semantic & G_PLANT) && !(e->object & G_LOW_PLANT_BIT)) return 0;   // not "ice shines on the grass"
      if (!subject && (s.marks & G_MARK_FLYER) && (e->marks & G_MARK_ROOT)) return 0;   // not "birds sing on roots", "the birds float across a root" (2026-10-31)
      if (!subject && (s.marks & G_MARK_DUCK) && rank == G_SHADE_RANK && (s.object & (G_NOT_AIRY | G_NOT_SURFACE))) return 0;   // not "ducks rest on a pine"
      if ((s.adjectives & G_LIMIT_ABOVE) && (e->semantic & G_GROUND) && !(e->marks & G_MARK_OVERPASS)) return 0;   // not "a crow flies beneath stones"
      // 2026-11-01: nor under grass, moss or ivy, which are too low for a bird or bee ("gulls perch under moss").
      if ((s.adjectives & G_LIMIT_ABOVE) && (s.marks & G_MARK_FLYER) && (e->semantic & G_PLANT) && !(e->object & G_LOW_PLANT_BIT)) return 0;
      if (!subject && (s.marks & G_MARK_STONY) && (e->object & G_STONE_NOUN)) return 0;   // not "a stone stands by rocks" (2026-11-01)
      if ((s.adjectives & G_LIMIT_FIELD) && (e->semantic & G_AIRY) && (e->semantic & G_GROUND)) return 0;   // not "water shines in a field"
      if (!subject && (s.marks & G_MARK_JUMPY) && rank == G_SHADE_RANK) return 0;   // not "frogs jump over a tree"
      if (!subject && (s.marks & G_MARK_GRASPY) && (e->marks & G_MARK_NOGRASP)) return 0;   // not "I grasp the neutrino"
      if ((s.tail & 16) && (s.object & G_SKY_THROUGH) && (e->semantic & G_PLANT) && !(e->object & G_LOW_PLANT_BIT)) return 0;   // not "a star fades through grass"
      if (subject) {
        if (s.conj && (e->object & G_LONE_NOUN)) return 0;   // 2026-10-24: "density in a flash shifts"
        // Two lights are one subject named twice: not "sunlight and light shine", "starlight and a sun".
        // Light itself is AIRY on earth and a SIGNAL in space (the space theme strips AIRY);
        // two signals one of which is light are light twice too: "sunlight and the sunbeam".
        if (s.conj && (((s.subject & e->semantic & G_LIGHT) && ((s.subject | e->semantic) & (G_AIRY | G_SIGNAL))) ||
                       ((s.subject & e->semantic & G_SIGNAL) && ((s.subject | e->semantic) & G_LIGHT)))) return 0;
        if (rank > (int)(s.marks & G_ORBIT_RANK)) s.marks = (uint32_t)((s.marks & ~G_ORBIT_RANK) | (uint32_t)rank);   // "planets and comets": the larger
        if (s.marks & e->marks & G_MARK_FOREVER) return 0;   // not "a faint photon"
        s.marks &= ~(uint32_t)G_MARK_FOREVER;   // the adjective's mark ends with its noun: "faint light fades" is fine
        s.marks |= e->marks & (G_PAIR_MARKS | G_MARK_WATERWORD | G_MARK_DUCK | G_MARK_FLYER | G_MARK_DEW | G_MARK_ICE | G_MARK_ROOT | G_MARK_BIRD);
        if (e->object & G_STONE_NOUN) s.marks |= G_MARK_STONY;
        // "Space and time bend": a second subject noun makes the clause plural. A mass noun (singular, no article
        // needed: grass, rain, snow) is number 0 until the verb, which agrees with it as singular (2026-10-31).
        s.number = s.conj ? 2 : (e->number == 1 && e->bare) ? 0 : e->number;
        s.subject = s.conj ? (s.subject | e->semantic) : e->semantic;
        s.phase = G_FINITE;
        if (e->object & G_POOL_NOUN) s.tail |= 8;    // season review 2026-10-22
        if (e->object & G_ORB_NOUN) s.tail |= 16;
        if (e->object & G_ROOTED_NOUN) s.tail |= 32;
        if (e->object & G_BEE_NOUN) s.tail |= 64;
        if (e->object & G_SINGLE_MOON) s.tail |= 128;
        if (e->object & G_EARTH_PHRASE) s.subject |= G_WAVE_SUBJECT;
      }
      else {
        // An object ends the verb phrase: no "down". After a simile object no
        // adverb at all, which would read as "like sounds far".
        // Review 2026-10-31: "calm waves on the stream wash gates". Waves and standing water wash shores and stones.
        if ((e->extra & GX_BUILT) && ((s.subject & G_WAVE_SUBJECT) || (s.tail & 8))) return 0;
        s.tail |= s.object == G_MUSIC ? 5 : 4;
        s.phase = G_DONE;
        s.number = (e->number == 1 && e->bare) ? 0 : e->number;   // the object's from here on: "a rock in fields" (2026-10-31)
        s.object &= ~G_NO_POOL_NOUN;   // "holds" is done with it; G_WET shares the bit
        s.marks = (uint32_t)((s.marks & G_KEEP_MARKS) | ((s.subject & G_PLANT) ? (s.marks & G_MARK_GRASPY) : 0) |   // a plant's hold (2026-10-31)
                             ((s.marks & (G_ORBIT_VERB | G_ORBIT_RANK)) == G_SHADE_RANK ? G_SHADE_RANK : 0));
      }
      s.article = s.determiner = s.nadj = 0;
      const int ground_object = s.phase == G_DONE && s.object != G_MUSIC && !(s.tail & 2) && (e->semantic & G_GROUND);
      s.adjectives = subject && (e->object & G_LONE_NOUN) ? G_ALL | G_LONE_SUBJECT : G_ALL;
      if (ground_object) s.adjectives |= G_LIMIT_INWATER;   // not "snow coats a field in water"
    } else return 0;
  } else if (s.phase == G_FINITE) {
    // "now" goes before the verb: "Light now burns in space". It is once per clause and
    // does not use the clause's adverb slot. article 3 marks it until the verb.
    if (e->tag == G_ADV && e->bare == 3) {
      // Not after I or we: "I now blink" reads as stilted; "Now I blink" still opens a clause.
      if (s.article == 3 || (s.adjectives & G_IN_A_FLASH) || (s.subject & G_HUMAN)) return 0;
      s.article = 3;
      *out = s;
      return 1;
    }
    // A person's manner adverb may also go before the verb: "I silently wander", "my heart softly wonders".
    // It is the clause's adverb (tail 1, which nothing else sets before the verb), so none follows the verb,
    // and a verb must come next, never "I softly am". "slowly" (bare 10) only follows its verb.
    if (e->tag == G_ADV && e->bare == 9) {
      if ((s.tail & 1) || s.article == 3 || (s.adjectives & G_IN_A_FLASH) || s.conj || !(s.subject & (G_HUMAN | G_INNER | G_HAND))) return 0;
      s.tail |= 1;
      *out = s;
      return 1;
    }
    // "in a flash" takes the same place, with a following verb that stands alone:
    // "density in a flash shifts", not "a void in a flash is cold" (2026-10-24).
    if (e->tag == G_ADV && e->bare == 7) {
      if (s.article == 3 || (s.adjectives & G_IN_A_FLASH) || s.conj || !(e->semantic & s.subject)) return 0;
      s.adjectives |= G_IN_A_FLASH;
      *out = s;
      return 1;
    }
    if (e->tag == G_CONJ) {
      if (s.conj || s.article == 3 || (s.adjectives & (G_IN_A_FLASH | G_LONE_SUBJECT)) || (s.subject & (G_HUMAN | G_INNER | G_HAND))) return 0;
      s.conj = 1;
      s.phase = G_SUBJECT;
      *out = s;
      return 1;
    }
    if (e->number != ((s.number==3 && e->tag!=G_COPULA) ? 2 : s.number==0 ? 1 : s.number) || !(e->semantic & s.subject) ||
        (s.conj && (s.subject & G_ALL & (uint32_t)~e->semantic))) return 0;
    if ((s.tail & 1) && e->tag != G_VERB && e->tag != G_TRANSITIVE) return 0;   // after "I softly", a verb
    s.article = 0;
    if ((s.adjectives & G_IN_A_FLASH) && (e->tag != G_VERB || !(e->object & G_SUDDEN))) return 0;
    if (s.adjectives & G_IN_A_FLASH) s.tail |= 1;   // it is the clause's adverb: no "shifts fast"
    s.adjectives &= ~(G_IN_A_FLASH | G_LONE_SUBJECT);
    if ((s.tail & (8 | 32)) && e->tag == G_VERB && (e->object & G_LOOSE)) return 0;   // not "a lake falls", "a tree falls"
    if ((s.subject & G_WAVE_SUBJECT) && e->tag == G_VERB && (e->object & G_THAW)) return 0;   // not "waves on the brook melt"
    if ((s.subject & G_WAVE_SUBJECT) && e->tag == G_VERB && (e->object & G_DOWNWARD)) return 0;   // not "a wave on the pond falls"
    if (s.marks & e->marks & G_PAIR_MARKS) return 0;   // not "air blows", "snow flows", "snow rises", "dew ripples"
    if (e->tag == G_VERB && (e->extra & GX_STILL_VERB) && !(s.marks & G_MARK_IMMOVABLE)) return 0;
    // Review 2026-10-31: "crickets float over the fence". Crawlers (crickets, beetles, cicadas: the crawl action, bit 30)
    // do not float or drift on the air; bees, fireflies and leaves may. Birds float, but glide or fly instead of drift.
    if (e->tag == G_VERB && (e->marks & G_MARK_DRIFTY) && (s.subject & (1u << 30))) return 0;
    if (e->tag == G_VERB && (e->extra & GX_DRIFT) && (s.marks & G_MARK_BIRD)) return 0;
    if (e->tag == G_VERB && (e->extra & GX_GLIDE) && !(s.marks & G_MARK_BIRD)) return 0;
    if ((e->tag == G_VERB || e->tag == G_TRANSITIVE) && (e->extra & GX_SOUND) && (s.marks & G_MARK_HUSHED)) return 0;   // not "quiet birds sing"
    if ((e->tag == G_VERB || e->tag == G_TRANSITIVE) && (e->extra & GX_SOUND) && (s.marks & G_MARK_ROOT)) return 0;   // roots are underground: not "roots rustle" (2026-11-01)   // walls, gates, stones... crumble, lean, remain, stand
    // Moonlight and starlight are G_AIRY on earth only: "starlight burns through space" stays.
    if (e->tag == G_VERB && (e->extra & GX_BURN) && (s.marks & G_MARK_COLD) && (s.subject & G_AIRY)) return 0;   // not "moonlight burns"
    const int shade = (e->marks & (G_ORBIT_VERB | G_ORBIT_RANK)) == G_SHADE_RANK;
    if (shade && (s.marks & G_ORBIT_RANK) != G_SHADE_RANK) return 0;   // not "reeds shade a stone"
    const int creeper = (s.marks & G_ORBIT_RANK) == 1;   // moss, ivy
    const int tree = (s.marks & G_ORBIT_RANK) == G_SHADE_RANK;   // trees, branches, vines
    // Review 2026-10-31: "branches shine on the dark path". Wood and roots do not shine; leaves may (petals not since 2026-11-01, below).
    if ((e->extra & GX_SHINE) && (tree || (s.marks & G_MARK_ROOT))) return 0;
    // 2026-11-01: "petals shine over a tree" should be "petals float over a tree": petals do not shine either (nebulae, rank 7 too, do).
    if ((e->extra & GX_SHINE) && (s.marks & G_ORBIT_RANK) == G_PETAL_RANK && !(s.subject & G_SPACE_CLASSES)) return 0;
    s.marks = (uint32_t)((s.marks & G_MARK_DRY) | (shade ? G_SHADE_RANK : 0) | (e->marks & (G_MARK_DRIFTY | G_MARK_JUMPY | G_MARK_GRASPY | G_MARK_NOBLOOM | G_MARK_NEEDS_PLACE)) |
                         (s.marks & (G_MARK_WATERWORD | G_MARK_AIR | G_MARK_DUCK | G_MARK_FLYER | G_MARK_DEW | G_MARK_ICE)) |
                         (s.marks & (G_MARK_TINY | G_MARK_ROOT | G_MARK_IMMOVABLE | G_MARK_STONY)) |   // where a worm, a root, a wall or a stone may be (V8.17)
                         (s.marks & G_MARK_STILLWATER) |   // a pond, lake or puddle goes nowhere through; a stream may (2026-11-01)
                         ((e->tag == G_VERB && (e->extra & GX_GROW)) ? G_MARK_GROWN : 0) |   // "roots grow slowly", not "softly"
                         // A hill, cliff, ridge or shore (immovable and G_MARK_NOPULL) keeps that mark for its place phrase (2026-10-31).
                         ((s.marks & G_MARK_IMMOVABLE) ? (s.marks & G_MARK_NOPULL) : 0) |
                         (creeper ? G_MARK_CREEPER : 0) | (tree ? G_MARK_TREE : 0) |
                         (e->tag == G_COPULA ? (s.marks & G_PAIR_MARKS) : 0) |   // "a pond is soft"
                         ((e->marks & G_ORBIT_VERB) ? (s.marks & G_ORBIT_RANK) | G_ORBIT_VERB : 0));
    // Review 2026-10-25: light bends. "Bend" and "curve" (bare 8) take an object after a
    // physical cause ("mass bends light") and stand alone after light or a signal ("light
    // waves bend", "starlight curves past the sun"), with a place but no adverb.
    if (e->tag == G_TRANSITIVE && e->bare == 8 && !(s.subject & G_CRAFT)) {
      s.phase = G_DONE; s.object = G_ALL; s.tail |= 1;
    }
    else if (e->tag == G_VERB) { s.phase = G_DONE; s.object = e->object & ~G_THAW; } // adverbs it accepts; bit 31 is G_SKY_THROUGH after a preposition
    else if (e->tag == G_TRANSITIVE) { s.phase = G_OBJECT; s.object = e->object; }
    else if (e->tag == G_COPULA) {
      if (s.marks & G_MARK_TINY) return 0;   // a worm is never described ("a worm is tall"): it crawls or rests
      s.phase = G_PREDICATE;
    }
    else return 0;
  } else if (s.phase == G_PREDICATE) {
    if (e->tag == G_LIKE && (e->semantic & s.subject)) {
      if (s.tail & 128) return 0; // use "the moons are like songs"
      s.phase = G_OBJECT; s.object = G_MUSIC;
      *out = s; return 1;
    }
    // bare 6: attributive only ("winter wind", never "leaves are winter").
    if (e->tag != G_ADJ || e->bare == 6 || !(e->semantic & s.subject)) return 0;
    if (e->extra & GX_SIZE) return 0;   // "an old wall is small" measures, it makes no image; "a small stone" is fine
    if ((e->extra & GX_WEAR) && !(s.marks & G_MARK_IMMOVABLE)) return 0;   // "an old wall is worn", not "a pond is worn"
    if ((s.marks & G_MARK_DRY) && (e->object & G_WET_ADJ)) return 0;   // not "dry fields are wet"
    if (s.marks & e->marks & G_PAIR_MARKS) return 0;   // not "a pond is soft"
    if ((e->extra & GX_HUSH) && ((s.subject & G_PLANT) || (s.marks & G_MARK_IMMOVABLE))) return 0;   // a rock is always quiet (2026-10-31)
    if ((e->extra & GX_TEMP) && (s.marks & G_MARK_IMMOVABLE)) return 0;   // "an old bridge is hot" measures it; "a warm stone" is fine (2026-11-01)
    if ((e->marks & G_MARK_AIR) && (s.marks & G_MARK_IMMOVABLE)) return 0;   // "soft" (the one adjective with this mark): not "the wall is soft"
    s.phase = G_DONE;
    s.tail |= 4; // after a predicate adjective, no directional adverb
    if (e->object & G_STILL_ADJ) { s.tail |= 1; s.marks |= G_MARK_STILLNOW; }   // not "an oak is still there": it reads as "continues"
  } else if (s.phase == G_DONE) {
    if (e->tag == G_ADV && (e->extra & GX_GENTLE) && (s.marks & G_MARK_IMMOVABLE)) return 0;   // not "a bridge stands gently"
    // 2026-11-01: "roots grow softly on the hill" should be "roots grow slowly in the hills". The seasons only.
    if (e->tag == G_ADV && (e->extra & GX_GENTLE) && (s.marks & G_MARK_GROWN) && !(s.subject & G_SPACE_CLASSES)) return 0;
    // Adverb number 1 is directional ("down") and 2 manner/distance ("far"); both
    // only follow an intransitive verb, never "is dark far" or "pulls light down".
    // "far" and "fast" also need a verb of motion or change: no "the quasar glows far".
    if (e->tag == G_ADV && e->bare != 3 && e->bare != 7 && !(e->bare == 4 && !(s.tail & 2)) && !(e->bare == 5 && (s.tail & 2)) &&
        (e->semantic & s.subject) && (e->object & s.object) && !(s.tail & 1) &&
        !(e->number && (s.tail & 4))) s.tail |= 1;
    else if (e->tag == G_PREP && !(s.tail & 2) && !(s.object & G_NO_PLACE) &&
             !(e->number == 1 && (s.object & G_STILL)) &&
             // "at" (number 2) only, and only, after a person's gaze verb: "I blink at stars".
             (e->number == 2) == ((s.object & G_GAZE) && (s.subject & (G_HUMAN | G_INNER | G_HAND))) &&
             !((s.object & G_ALOFT) && (e->object & G_NOT_AIRY)) &&   // on/upon carry G_NOT_AIRY
             !((s.object & G_ADRIFT) && (e->object & G_BELOW)) &&   // nothing floats under a field
             !((s.subject & G_AIR) && (s.object & G_LOOSE) && !(e->object & G_SKYWARD)) &&   // air rises over or through, never "in" or "on" a pine or shore
             !((s.marks & G_MARK_STILLNOW) && (e->object & G_SKYWARD)) &&   // not "birds are still through mist"
             !((s.object & G_STILL) && (s.subject & G_ANIMAL) && (e->object & G_SKYWARD)) &&   // not "a worm rests through grass"
             !((s.object & G_DOWNWARD) && (e->object & G_BELOW)) &&   // nor falls under a rock (2026-10-26)
             !((s.marks & G_MARK_ROOT) && (e->object & G_NOT_AIRY)) &&   // roots are in the ground, never "on the hill" (2026-11-01)
             // Round 5: floating is "on" water, or over or across things; not through or by.
             !((s.object & G_ADRIFT) && !(e->object & G_NOT_AIRY) &&
               !((e->object & G_SKYWARD) && !(e->object & G_NOT_SOLID))) &&
             // A pond or lake goes nowhere "through"; the sun, moon and stars go over or through. A stream or brook does
             // (2026-11-01, V8.28 site: "streams sparkle through a field"); a puddle and the word "water" do not.
             !((s.tail & 8) && (s.marks & (G_MARK_STILLWATER | G_MARK_WATERWORD)) && (e->object & G_NOT_SOLID) && !(e->object & G_NOT_SURFACE)) &&
             // ...nor "on" anything: it lies in the ground (2026-10-31: "the pond sparkles on a rock"). Water itself may.
             !((s.tail & 8) && !(s.marks & G_MARK_WATERWORD) && (e->object & G_NOT_AIRY)) &&
             // Review 2026-10-31 (autumn batch 1): "the puddles shine over fields". Standing water is not over, across,
             // beyond or past things either; "through" is refused above, except for a stream or brook.
             !((s.tail & 8) && !(s.marks & G_MARK_WATERWORD) && (e->object & G_SKYWARD) &&
               !(!(s.marks & G_MARK_STILLWATER) && (e->object & G_NOT_SOLID) && !(e->object & G_NOT_SURFACE))) &&
             // "fog coats red trees across rocks": no "across" (number 3) after an object. tail 4 is set by an object noun
             // and by a predicate adjective alike, and "air is cold over leaves" stays, so only "across".
             !((s.tail & 4) && e->number == 3) &&
             // "the oak holds cool mist on gates": what a plant holds has no place phrase (hold carries G_MARK_GRASPY;
             // grasp's subjects are people, never plants).
             !((s.tail & 4) && (s.marks & G_MARK_GRASPY) && (s.subject & G_PLANT)) &&
             // "branches grow through a cold field": trees, branches, pines, oaks and vines go through nothing.
             !((s.marks & G_MARK_TREE) && (e->object & G_SKYWARD) && (e->object & G_NOT_SOLID)) &&
             // ...or "in" weather: "a star fades in fog" (G_NOT_SURFACE marks "in").
             !((s.tail & 16) && !(e->object & (G_SKYWARD | G_NOT_SURFACE)))) {
      uint32_t verb_object = s.object;
      s.tail |= 2;
      // Walls, gates, stones, hills... are "in" weather or by, near, beside, along or around things;
      // never on, under, over or through them ("an old cliff stands on a leaf").
      const int still_thing = (s.marks & G_MARK_IMMOVABLE) != 0;
      if (still_thing && (e->object & (G_NOT_AIRY | G_BELOW | G_SKYWARD))) return 0;
      // Review 2026-10-31: "hills remain by a soft branch". A hill, cliff, ridge or shore is too big to be by, near, beside,
      // along or around a thing; it is only "in" weather ("hills remain in the cold rain"). Walls and stones keep "by".
      if (still_thing && (s.marks & G_MARK_NOPULL) && !(e->object & G_NOT_SURFACE)) return 0;
      s.phase = G_PP;
      s.object = G_PLANT | G_GROUND | G_WATER |
        (e->object & (G_SPACE | G_CELESTIAL | G_BODY | G_TERRAIN));
      if (verb_object & G_WET) s.object &= G_WATER;   // not "ducks swim in the fence"
      if ((verb_object & G_WET) && (e->object & G_BELOW)) { s.object |= G_GROUND; s.adjectives |= G_LIMIT_ABOVE; }
      if ((verb_object & G_GROUNDED) && (e->object & G_NOT_SOLID)) s.object |= G_IN_SOIL;   // crawl or burrow in the soil
      if ((s.subject & G_ANIMAL) && !(s.subject & (1u << 30)) && (e->object & G_BELOW)) s.object |= G_UNDER_LOW;   // only crawlers under grass
      if (e->number == 0 && !(e->object & (G_NOT_SOLID | G_NOT_AIRY | G_NOT_SURFACE | G_BELOW | G_SKYWARD))) s.object |= G_BESIDE;   // by, near...
      // A burrow (crawling and still: G_GROUNDED with G_STILL) is in or under the ground only: not "worms burrow on a
      // wet branch" or "in a branch" (2026-11-01).
      if ((verb_object & G_GROUNDED) && (verb_object & G_STILL)) {
        if (!(e->object & (G_NOT_SURFACE | G_BELOW))) return 0;
        s.object &= ~(uint32_t)(G_PLANT | G_WATER);
      }   // "ducks swim under a bridge": water below it; not under a stone
      // A person is placed by bodies, not in substances: "I tremble by moons",
      // never "we wait in dust" or "I wait in soft waves".
      if (s.subject & (G_HUMAN | G_INNER | G_HAND)) s.object &= e->object & (G_CELESTIAL | G_BODY);
      // The preposition's own limits ride in the object mask, which the lookahead memo compares.
      s.object |= e->object & (G_NOT_SOLID | G_NOT_AIRY | G_NOT_SURFACE);
      if (e->object & G_BELOW) s.object |= G_NOT_SURFACE;   // nothing is under a path or shore ("a tree blooms under a path")
      if (e->object & G_BELOW) s.object |= G_NOT_WATER;     // nor under or beneath a lake, pond or water (V8.17)
      if (still_thing) s.object |= (e->object & G_NOT_SURFACE) ? G_ONLY_AIRY : G_NOT_AIRY;   // "in the rain"; "by the path", not "along cool fog"
      if ((s.adjectives & G_LIMIT_INWATER) && (e->object & G_NOT_SURFACE)) s.object |= G_NOT_WATER;   // "in": not "a field in water"
      if ((s.adjectives & G_LIMIT_INWATER) && (e->object & (G_NOT_SOLID | G_NOT_AIRY))) s.object |= G_NOT_TREE;   // "near a pine"
      s.adjectives &= ~G_LIMIT_INWATER;
      if ((verb_object & (G_ALOFT | G_GROUNDED)) && (e->object & G_NOT_SOLID)) s.object |= G_NO_POOL;   // in/through
      if ((s.tail & 16) && !(e->object & G_SKYWARD)) s.object |= G_ONLY_AIRY;   // a star "in" fog only
      if ((s.tail & 16) && (e->object & G_NOT_SOLID) && !(e->object & G_NOT_SURFACE)) s.object |= G_SKY_THROUGH;
      // Round 5: flying "in" is weather ("crows fly in the rain", not "in the dry garden").
      if ((verb_object & G_ALOFT) && (e->object & G_NOT_SURFACE)) s.object |= G_ONLY_AIRY;
      // Round 5: "tall grass sways on leaves". Rooted plants are not on other plants.
      if ((s.tail & 32) && (e->object & G_NOT_AIRY)) s.object |= G_NOT_PLANT;   // on/upon
      // Season review 2026-10-22: sunlight shines on a lake, not in or through it.
      if ((s.subject & G_LIGHT) && (e->object & (G_NOT_SOLID | G_NOT_SURFACE)))
        s.object |= G_LIT_POOL;   // "in"/"through" after light
      // Review 2026-10-26: nothing goes "through" a pond or water ("dry grass shines through the
      // lake", "moss grows through the pond"); "in", "on", "over" and "across" stay.
      if ((e->object & G_NOT_SOLID) && !(e->object & G_NOT_SURFACE)) s.object |= G_LIT_POOL;
      // Season review 2026-10-22: "cold rain falls in leaves" should be "on leaves".
      if ((verb_object & G_DOWNWARD) && (e->object & G_NOT_SURFACE)) s.object |= G_NOT_PLANT;   // "in"
      // 2026-11-01: "dry leaves fall through grass" should be "on grass": nothing falls through grass, moss or ivy.
      if ((verb_object & G_DOWNWARD) && (e->object & G_NOT_SOLID) && !(e->object & G_NOT_SURFACE)) s.object |= G_UNDER_LOW;
      // Season review round 3: "tall grass sways in leaves", "wind rises through waves",
      // "leaves float on cold stones".
      // Light rests "on" a branch, not "in" it (season review 2026-10-22).
      if ((s.subject & (G_PLANT | G_LIGHT)) && (e->object & G_NOT_SURFACE)) s.object |= G_NOT_PLANT;
      if ((s.subject & G_AIR) && (e->object & (G_NOT_SOLID | G_BELOW))) s.object |= G_NOT_WATER;
      if ((verb_object & G_ADRIFT) && (e->object & G_NOT_AIRY)) s.object |= G_ONLY_POOL;   // on/upon
      if ((s.marks & G_MARK_DUCK) && (s.marks & G_MARK_DRIFTY)) s.object |= G_ONLY_POOL;   // a duck floats on water, not over a garden (2026-11-01)
      // Round 4: "bees hum in the lake". Bit 27 is the "swim" action (ducks, frogs, gulls).
      if ((s.subject & G_ANIMAL) && !(s.subject & (1u << 27)) && (e->object & G_NOT_SURFACE))
        s.object |= G_NO_POOL_NOUN;   // "in"
      // Review round 7: "the lake sparkles on a pond". No water on or in water.
      if ((s.tail & 8) && (e->object & (G_NOT_AIRY | G_NOT_SURFACE))) s.object |= G_NO_POOL_NOUN | G_NOT_PLANT;   // "streams sparkle on a tree"
      if (e->number == 3) s.adjectives |= G_LIMIT_WEATHER;   // across
      if ((s.marks & G_MARK_TREE) && (e->object & (G_NOT_SURFACE | G_NOT_AIRY))) s.object |= G_NO_POOL_NOUN;   // not "pines sway in the pond"
      if ((s.subject & G_LIGHT) && (e->object & G_NOT_AIRY)) s.adjectives |= G_LIMIT_STONE;   // not "light glows on a rock"
      if ((verb_object & G_ALOFT) && (e->object & G_BELOW)) s.adjectives |= G_LIMIT_ABOVE;   // flying under the ground
      if ((s.marks & G_MARK_NOBLOOM) && (e->object & (G_BELOW | G_NOT_SURFACE))) s.object |= G_NOT_WATER;   // not "vines bloom under the water"; rain and dew are fine
      if ((s.marks & G_MARK_DEW) && (e->object & G_NOT_SURFACE)) s.object |= G_NO_POOL_NOUN;   // not "dew shines in a lake"
      if ((s.marks & G_MARK_FLYER) && (e->object & G_BELOW)) s.adjectives |= G_LIMIT_ABOVE;   // birds sing under a bridge or branches, not beneath stones
      // Review 2026-10-31: "branches bloom under a field", "under a rock". A plant is not under the ground (a bridge is fine).
      if ((s.subject & G_PLANT) && (e->object & G_BELOW)) s.adjectives |= G_LIMIT_ABOVE;
      // 2026-11-01 (V8.28 site): "streams sparkle under a field" should be "through a field". Water is not under the
      // ground either (a stream may be under the trees or a bridge), and a stream goes through a field or the grass, not
      // through fog or rain (the limit "across" uses).
      if ((s.tail & 8) && (e->object & G_BELOW)) s.adjectives |= G_LIMIT_ABOVE;
      if ((s.tail & 8) && (e->object & G_NOT_SOLID) && !(e->object & G_NOT_SURFACE)) s.adjectives |= G_LIMIT_WEATHER;
      // 2026-11-01: a firefly (an animal and a light) floats or drifts over water, never on it.
      if ((s.subject & G_ANIMAL) && (s.subject & G_LIGHT) && (e->object & G_NOT_AIRY)) s.object |= G_NOT_WATER;
      if ((s.marks & G_MARK_DRIFTY) && (e->object & G_NOT_AIRY)) s.adjectives |= G_LIMIT_STONE;   // drift over stones, not "on" them
      if ((s.marks & G_MARK_WATERWORD) && (e->object & (G_NOT_SURFACE | G_NOT_AIRY))) s.adjectives |= G_LIMIT_FIELD;   // water is not "in" a field
      if ((s.marks & G_MARK_AIR) && (e->object & G_NOT_SURFACE)) s.object |= G_NOT_PLANT;   // air is over leaves, not "in" them
      // Nor "air rises through a small path" (2026-10-31). The word "air" only: light, the sun, moon and stars share
      // G_MARK_AIR, and "the moon rises over the shore" stays (V8.18 refused it by mistake).
      if ((s.marks & G_MARK_AIR) && !(s.subject & G_LIGHT) && (e->object & G_SKYWARD)) s.object |= G_NOT_SURFACE;
      if ((s.tail & 32) && (e->object & G_NOT_AIRY) && !(s.marks & G_MARK_CREEPER)) s.adjectives |= G_LIMIT_STONE;   // not "grass sways on stones"; moss does grow on them
      if ((s.tail & 64) && (e->object & G_NOT_AIRY)) s.object |= G_NO_POOL_NOUN;   // "bees hum on the lake"
      // Review round 7: "the bird sings in a soft leaf", "birds sing on the stone".
      if ((s.subject & G_ACT_FLY) && !(s.subject & G_ACT_HUM_CRAWL)) {
        if (e->object & G_NOT_SURFACE) s.adjectives |= G_LIMIT_TREE;   // "in"
        if (e->object & G_NOT_AIRY) s.adjectives |= G_LIMIT_STONE;     // on, upon
      }
    } else return 0;
  } else return 0;
  *out = s;
  return 1;
}

typedef struct { uint64_t key; uint32_t subject, object, adjectives, marks; uint8_t occupied, possible, remaining, tail; } GMemo;
#define G_MEMO_SIZE 512
typedef struct { GMemo cells[G_MEMO_SIZE]; } GWorkspace;

// Low semantic bits and syntax form the hash key; full masks and meter
// are compared separately so human classes cannot alias legacy classes.
static inline uint64_t g_key(GState s, int remaining) {
  (void)remaining;
  // The marks are folded into the subject's key bits: the full subject is compared
  // separately, so equal keys with equal subjects mean equal marks.
  return (uint64_t)((s.subject ^ (uint32_t)s.marks << 8) & 65535) | (uint64_t)(s.object & 65535) << 16 | (uint64_t)(s.adjectives & 65535) << 32 |
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
        w->cells[j].adjectives == s.adjectives && w->cells[j].tail == s.tail && w->cells[j].marks == s.marks) return w->cells[j].possible;
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
    w->cells[free_slot].tail = s.tail;
    w->cells[free_slot].marks = s.marks;
    w->cells[free_slot].remaining = (uint8_t)remaining;
    w->cells[free_slot].possible = (uint8_t)possible;
    w->cells[free_slot].occupied = 1;
  }
  return possible;
}
#endif
