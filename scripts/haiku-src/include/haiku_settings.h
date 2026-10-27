#pragma once
// Generation defaults shared by the firmware and host simulator.
#define HAIKU_TOP_K 20
// Finished poems the board writes ahead while nobody presses a button (firmware only).
// One is written after each poem shown; more follow while idle, up to this many, then it rests.
#define HAIKU_PREFETCH_MAX 5
#define HAIKU_TEMPERATURE 0.7f
#define HAIKU_TRIES 5
// Weight of "the model would punctuate here" against mean word log-probability.
#define HAIKU_END_WEIGHT 1.0f
#define HAIKU_NATURE_BOOST 1.5f
#define HAIKU_NATURE_SCORE 0.4f
// Space samples hotter with a smaller nature boost: with the seasonal settings
// "The sun falls on a bright wave" opened a third of the poems. Measured on 60
// host poems, distinct line twos rose from 33 to 56. Seasons keep the values above.
#define HAIKU_SPACE_TEMPERATURE 1.2f
#define HAIKU_SPACE_NATURE_BOOST 0.5f
#define HAIKU_SPACE_NATURE_SCORE 0.2f
#define HAIKU_SPACE_FRESHNESS 5.0f     // logit penalty per recent use of a word (0 = off)
#define HAIKU_SPACE_PLACE_PERCENT 45   // share of place nouns a space poem may use
#define HAIKU_SPACE_VERB_PERCENT 55    // share of intransitive verbs a space poem may use
// Host and V8 firmware reject an exact line already used in recent space poems.
// The fixed-size hash ring keeps this decoder-only diversity guard small on-device.
#define HAIKU_SPACE_LINE_HISTORY 96
// Earth seasons (spring to winter) with the same variety features as space: word
// freshness, a per-poem palette, recent-line memory and model-written first lines.
// HAIKU_SEASON_FEATURES 0 restores the original seasonal decoder.
#define HAIKU_SEASON_FEATURES 1
#define HAIKU_SEASON_FRESHNESS 5.0f     // logit penalty per recent use of a word
#define HAIKU_SEASON_NOUN_PERCENT 50    // share of nature nouns a seasonal poem may use
#define HAIKU_SEASON_VERB_PERCENT 55    // share of intransitive verbs a seasonal poem may use
#define HAIKU_SEASON_ADJ_PERCENT 70
#define HAIKU_SEASON_ADV_PERCENT 70
// Review round 7: "a small rock is dry" ended almost half the seasonal poems. Line three
// may use is/are in this share of seasonal poems (it comes back if the line cannot finish).
#define HAIKU_SEASON_COPULA_PERCENT 50
// Model-written first lines lost the season ("Starlight burns through grass" opened spring,
// summer and autumn), so seasonal poems keep their openers; the other features gave the gain.
#define HAIKU_SEASON_FREE_PERCENT 0     // share of seasonal poems whose line one the model writes
// Review 2026-10-25: a wave on water "falls" in about a third of its lines. Logit penalty on
// "fall"/"falls" after one; waves still fall, less often than they ripple, drift or wash.
#define HAIKU_WAVE_FALL_PENALTY 1.0f
// Review 2026-10-27: too many stars, suns and moons in the seasons (about 45% of poems named one).
// Logit penalty on sun, moon and star nouns in the four seasons; space is unchanged.
#define HAIKU_SEASON_SKY_PENALTY 5.0f
