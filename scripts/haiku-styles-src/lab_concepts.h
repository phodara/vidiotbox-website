// Ten conceptual images for the haiku lab (2026-10-25). Each concept names the space words
// that carry it and, for two-part poems, an earthly image from one season for lines 2-3,
// set against line one across the cut. Words must already be in the grammar tables;
// lab_check_concepts() reports any that are missing or outside their theme.
#pragma once

struct LabConcept {
  const char *name;
  const char *space;    // one-thing: every line; two-part: line one
  int season;           // two-part: theme of lines 2-3 (0 spring, 1 summer, 2 autumn, 3 winter)
  const char *earth;    // two-part: lines 2-3
};

static const LabConcept LAB_CONCEPTS[] = {
  {"ancient light", "starlight light photon photons beam beams sunbeam ancient distant older eyes time crosses cross carries carry",
   3, "stone stones rock rocks moss frost old"},
  {"scale", "galaxies galaxy universe cosmos vast infinite huge giant cosmic dust stardust tiny hand palm",
   1, "bee bees beetle beetles cricket crickets worm worms small grass"},
  {"awe", "eyes heart gasp gasps tremble breathe breathes watch watches wonder wonders silent vast stars",
   2, "lake pond still quiet calm mist reeds reed"},
  {"made of stars", "supernova stardust atoms atom hydrogen helium fusion burn burns hands palm palms heart",
   0, "blossom blossoms petals petal bloom blooms"},
  {"edge of knowing", "horizon horizons beyond past void distant infinite faint vacuum mind seek seeks grasp grasps wonder",
   2, "fog mist path paths field fields fade fades"},
  {"black hole", "gravity void black mass bend bends curve curves pull pulls slow slows vanish vanishes density time",
   3, "freeze freezes ice frost pond still cold"},
  {"the beginning", "primordial universe cosmos energy hydrogen expand expands unfold unfolds spark sparks plasma",
   0, "bud buds grow grows root roots soil"},
  {"impermanence", "supernova fade fades vanish vanishes burn burns starlight ancient distant older",
   2, "leaves leaf fall falls dry red"},
  {"silence", "silent void vacuum vast space quiet still echo echoes dark",
   3, "snow snowflakes snowflake fall falls softly quietly still lake"},
  {"order and beauty", "orbit orbits circle circles spin spins galaxy galaxies planets halo halos rhythm rhythms harmony symphony",
   1, "bees bee hum flowers flower petals garden"},
};
static constexpr int LAB_N_CONCEPTS = sizeof LAB_CONCEPTS / sizeof LAB_CONCEPTS[0];

// Season styles (2026-10-31, winter first). A season poem keeps its written opener, so the opener picks
// the concept: one-thing stays with that image in every line; two-part sets it against the partner
// concept's image in lines 2-3, across the cut. Concepts with no openers serve only as partners.
// A season none of whose concepts names an opener (spring) draws the concept for each poem instead, and the
// model writes line one, which must name one of the concept's names: spring's words are mostly shared with
// the other seasons, and a free line one with no concept named a spring thing in only 58 of 200 poems.
struct SeasonConcept {
  const char *name;
  int season;           // 0 spring, 1 summer, 2 autumn, 3 winter
  const char *openers;  // the season openers (seed texts) that start this image, separated by |
  const char *words;
  int partner;          // two-part: the concept for lines 2-3 (index into SEASON_CONCEPTS)
  const char *names=nullptr;   // drawn concepts: the nouns line one must name, ones that read as spring
};

static const SeasonConcept SEASON_CONCEPTS[] = {
  {"falling snow", 3, "Snow falls|Snowflakes fall", "snow snowflake snowflakes fall falls softly quietly still quiet field fields hill lake cover covers", 4},
  {"ice and frost", 3, "Ice shines|Frost sparkles|Mist freezes", "ice frost freeze freezes cold sparkle sparkles shine shines pond stone stones", 6},
  {"winter wind", 3, "Cold wind howls|Bare trees sway|Pines sway", "wind winds gust gusts howl howls blow blows sway bare branch branches pine pines", 5},
  {"fog", 3, "Fog drifts", "fog mist drift fade fades pale path paths field fields", 4},
  {"crows", 3, "Crows rest", "crow crows branch branches oak perch rest rests bare", 0},
  {"stillness", 3, "", "pond lake water still calm quiet stone stones", 2},
  {"winter light", 3, "", "sun sunlight moonlight moon light glow glows pale warm warms", 1},
  // Autumn (2026-10-31, first draft).
  {"falling leaves", 2, "Red leaves drift|Dry leaves fall|Leaves rustle", "leaves leaf fall falls drift red dry rustle earth path", 12},
  {"autumn wind", 2, "Oaks sway|The wind blows", "wind winds gust gusts blow blows sway oak branch branches bare", 12},
  {"cold rain and mist", 2, "Cold rain falls|Mist rises", "rain cold mist fog rise rises fall falls field fields hill hills", 10},
  {"crows", 2, "Crows fly", "crow crows fly flies field fields branch branches oak", 11},
  {"fading", 2, "Moonlight fades|Crickets chirp", "moonlight moon fade fades pale cricket crickets chirp chirps", 7},
  {"a still pond", 2, "", "pond lake water still calm quiet stone stones", 9},
  // Summer (2026-11-01, first draft).
  {"summer wind", 1, "Hot winds blow|Tall grass sways", "wind winds breeze breezes gust gusts blow blows sway hot tall grass meadow meadows field fields", 18},
  {"summer light", 1, "Sunlight burns|Dew shines", "sun sunlight light burn burns shine shines glisten sparkle sparkles glow glows bright hot warm warms dew", 18},
  {"night song", 1, "Crickets sing|Frogs sing", "cricket crickets frog frogs sing sings chirp chirps reed reeds pond moon moonlight stars dark", 13},
  {"bees and beetles", 1, "Bees hum|Beetles crawl", "bee bees hum buzz buzzes swarm beetle beetles crawl crawls flower flowers petal petals garden gardens bloom blooms", 14},
  {"water birds", 1, "Ducks swim|Gulls perch", "duck ducks gulls swim swims float floats perch lake pond shore shores water ripple ripples", 13},
  {"a cool shade", 1, "", "shade shades cool tree trees oak branch branches leaves rest rests still quiet", 14},
  // V8.27 (2026-11-01): the five added summer openers, summer's own insects in the Japanese almanac.
  {"cicadas and fireflies", 1, "A cicada sings|A firefly glows|A cicada calls|A cicada molts|A firefly floats",
   "cicada cicadas firefly fireflies sing sings call calls glow glows float floats drift pine pines reed reeds moonlight stars dark", 13},
  // Spring (2026-11-01, first draft): no openers, so the concept is drawn and the model writes line one, which names
  // one of the concept's names (never moss, leaves, roots, stones or fog). The words also give lines 2-3 a setting.
  {"blossoms", 0, "", "blossom blossoms petal petals flower flowers bloom blooms drift fall falls soft warm sway branch branches tree trees garden gardens", 24,
   "blossom blossoms petal petals flower flowers"},
  {"new growth", 0, "", "bud buds grass vine vines green grow grows tall soft sway rise rises meadow meadows field fields root roots soil", 22,
   "bud buds grass vine vines"},
  {"spring rain", 0, "", "rain raindrop raindrops puddle puddles soft wet warm fall falls wash washes coat coats mud soil path paths", 20,
   "rain raindrop raindrops puddle puddles"},
  {"the stream", 0, "", "stream streams flow flows ripple ripples rise rises glisten sparkle sparkles water cool shore shores reed reeds field fields grass", 21,
   "stream streams"},   // spring has no bare "brook", only "waves on the brook"; since V8.29 no "bank", and streams run through fields
  {"birdsong", 0, "", "bird birds sing sings chirp chirps call calls fly flies perch rest rests branch branches tree trees pine pines oak", 23,
   "bird birds"},
};
static constexpr int SEASON_N_CONCEPTS = sizeof SEASON_CONCEPTS / sizeof SEASON_CONCEPTS[0];
