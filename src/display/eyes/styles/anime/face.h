#pragma once

#include <cstddef>
#include <cstdint>

#include "display/eyes/styles/kawaii/face.h"

// Anime face renderer: almond eyes with a heavy lash line, a screentoned iris
// with a sharp highlight, expressive brows, a mouth with teeth and fangs, and
// manga symbols (anger vein, sweat drop, gloom lines, blush hatching).
//
// Drawn like manga on a lit screen: ink is lit, paper is dark. The lash
// line, brows, iris tone, and mouth are lit; the whites of the eyes, the
// highlights, and the teeth are dark. Pure C++ for native tests.

namespace anime {

using kawaii::EyeKind;
using kawaii::kFrameBytes;
using kawaii::kHeight;
using kawaii::kMaxSparks;
using kawaii::kWidth;
using kawaii::Spark;

// EyeKind for anime: Round is the normal eye, Chevron squeezes into "> <",
// Heart puts a heart in the iris, and Cross (kawaii's "x x") is a dizzy
// spiral.
struct Eye {
  EyeKind kind;
  int8_t side;       // +1 screen-left eye (inner corner at +x), -1 screen-right
  float cx;          // center, px
  float cy;
  float rx;          // half width, px
  float ry;          // half height when fully open, px
  float open;        // 0 closed .. 1 open (blinks, kind swaps)
  float happy;       // 0..1 cheeks push the lower lid up; high values close into "^"
  float lid;         // 0..1 upper lid lowered
  float tilt;        // + lowers the inner corner (sharp, angry), - the outer (droopy)
  float iris;        // iris radius, px (0 hides it)
  float pupil;       // pupil radius, px
  float gazeX;       // -1..1
  float gazeY;
  float pupilShake;  // px sideways (the bell's shake)
  float shine;       // highlight size multiplier; 0 is a dull, highlight-less eye
  float browLift;    // -1 lowered .. 1 raised (this eye's brow)
  float browTilt;    // + inner end down (angry V), - inner end up (worried)
};

struct Face {
  Eye left;
  Eye right;
  kawaii::Mouth mouth;
  float teeth;   // 0..1 top teeth in an open mouth
  float fang;    // 0..1 fangs hanging from the teeth
  float vein;    // 0..1 anger vein mark
  float sweat;   // 0..1 sweat drop
  float gloom;   // 0..1 gloom lines above the eyes
  float blush;   // 0..1 hatching under the eyes
  float glint;   // twinkle stars beside the eyes, as kawaii::Face::glint
  Spark sparks[kMaxSparks];
  uint8_t sparkCount;
};

// Draws `face` into `frame` (cleared first). False if the buffer is too small.
bool render(const Face& face, uint8_t* frame, size_t frameBytes);

}  // namespace anime
