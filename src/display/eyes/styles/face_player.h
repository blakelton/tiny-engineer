#pragma once

#include <cstddef>
#include <cstdint>

#include "display/eyes/styles/eye_style.h"
#include "display/eyes/styles/kawaii/animator.h"
#include "display/eyes/styles/kawaii/compose.h"
#include "display/eyes/styles/kawaii/expression.h"

// Firmware driver shared by the expressive face styles (kawaii, anime). It
// runs the engine in kawaii/: mood and personality per animation, speech and
// lip sync, the bell celebration, the typing gaze, and easing. A look only
// supplies its faces (presets) and how it draws one frame.

// Extras a look may draw on a frame: the bell's star burst.
struct FaceExtras {
  bool burst;
  uint32_t sinceDingMs;
  float ring;  // 0..1 the bell's loudness
};

struct FaceLook {
  kawaii::Presets presets;
  // Draws one frame into `frame` (128×32, 1 bit). False to skip the frame.
  bool (*draw)(const kawaii::Pose& pose, const kawaii::Animator& animator, uint32_t now,
               const FaceExtras& extras, uint8_t* frame, size_t frameBytes);
};

// One eyes-controller frame for `look`. Switching looks starts fresh.
void drawExpressiveFace(const FaceLook& look, EyeMode mode, AnimationId animation,
                        uint32_t modeStartedMs, uint32_t now, EyeStyleSleepPhase sleepPhase);
