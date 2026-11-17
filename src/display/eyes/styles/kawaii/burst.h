#pragma once

#include <cstdint>

#include "display/eyes/styles/kawaii/face.h"

namespace kawaii {

// Stars that burst out of the outer sides of the eyes on the bell's ding and
// swirl off-screen, trailing dots that shorten as the ring fades. `ring` is
// the bell's loudness (0..1); at 0 the burst is over. Appends to
// face.sparks, using the eyes' positions in `face`.
void addStarBurst(Face& face, uint32_t sinceDingMs, float ring);

// Where stars launch from: an eye's center, half-width, and side (+1 for the
// screen-left eye), for looks with their own face type.
struct SparkAnchor {
  float cx;
  float cy;
  float rx;
  int8_t side;
};

// Appends the burst to sparks (up to kMaxSparks; `count` is updated).
void addStarBurst(
  Spark* sparks, uint8_t& count, const SparkAnchor& left, const SparkAnchor& right,
  uint32_t sinceDingMs, float ring
);

}  // namespace kawaii
