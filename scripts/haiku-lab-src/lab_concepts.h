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
