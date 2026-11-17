#include "display/eyes/styles/eye_style.h"
#include "display/eyes/styles/face_player.h"
#include "display/eyes/styles/kawaii/burst.h"
#include "display/eyes/styles/kawaii/compose.h"
#include "display/eyes/styles/kawaii/face.h"

// The kawaii eye style: the expressive face engine with the kawaii look
// (round sparkly eyes, a little mouth, blush). See kawaii/README.md.

namespace {

bool drawKawaiiFrame(const kawaii::Pose& pose, const kawaii::Animator& animator, uint32_t now,
                     const FaceExtras& extras, uint8_t* frame, size_t frameBytes) {
  kawaii::Face face = {};
  kawaii::composeFace(pose, animator, now, face);
  if (extras.burst) {
    kawaii::addStarBurst(face, extras.sinceDingMs, extras.ring);
  }
  return kawaii::render(face, frame, frameBytes);
}

const FaceLook kKawaiiLook = {kawaii::expressionFor, drawKawaiiFrame};

void drawKawaii(EyeMode mode, AnimationId animation, uint32_t modeStartedMs, uint32_t now,
                EyeStyleSleepPhase sleepPhase) {
  drawExpressiveFace(kKawaiiLook, mode, animation, modeStartedMs, now, sleepPhase);
}

}  // namespace

extern const EyeStyleRenderer kKawaiiEyeStyle = {
  "kawaii",
  true,
  true,
  drawKawaii,
};
