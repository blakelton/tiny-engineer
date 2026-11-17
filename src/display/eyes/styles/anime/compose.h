#pragma once

#include <cstdint>

#include "display/eyes/styles/anime/face.h"
#include "display/eyes/styles/kawaii/animator.h"
#include "display/eyes/styles/kawaii/compose.h"

namespace anime {

constexpr float kEyeHalfWidth = 12.0f;   // px at scale 1
constexpr float kEyeHalfHeight = 7.0f;
constexpr float kIrisRadius = 5.5f;
constexpr float kPupilRadius = 2.2f;
constexpr float kMouthDrop = 8.0f;       // mouth below the eye centers, px

// Builds the anime face from the shared pose and the eased expression.
void composeFace(const kawaii::Pose& pose, const kawaii::Animator& animator, uint32_t now,
                 Face& out);

}  // namespace anime
