#include "display/eyes/styles/anime/compose.h"
#include "display/eyes/styles/anime/face.h"
#include "display/eyes/styles/anime/presets.h"
#include "display/eyes/styles/eye_style.h"
#include "display/eyes/styles/face_player.h"
#include "display/eyes/styles/kawaii/burst.h"

// The anime eye style: the expressive face engine (kawaii/) with the anime
// look (lash lines, screentoned irises, brows, teeth, manga symbols). See
// anime/README.md.

namespace {

kawaii::SparkAnchor anchorOf(const anime::Eye& eye) {
  return {eye.cx, eye.cy, eye.rx, eye.side};
}

bool drawAnimeFrame(const kawaii::Pose& pose, const kawaii::Animator& animator, uint32_t now,
                    const FaceExtras& extras, uint8_t* frame, size_t frameBytes) {
  anime::Face face = {};
  anime::composeFace(pose, animator, now, face);
  if (extras.burst) {
    kawaii::addStarBurst(face.sparks, face.sparkCount, anchorOf(face.left), anchorOf(face.right),
                         extras.sinceDingMs, extras.ring);
  }
  return anime::render(face, frame, frameBytes);
}

const FaceLook kAnimeLook = {anime::expressionFor, drawAnimeFrame};

void drawAnime(EyeMode mode, AnimationId animation, uint32_t modeStartedMs, uint32_t now,
               EyeStyleSleepPhase sleepPhase) {
  drawExpressiveFace(kAnimeLook, mode, animation, modeStartedMs, now, sleepPhase);
}

}  // namespace

extern const EyeStyleRenderer kAnimeEyeStyle = {
  "anime",
  true,
  true,
  drawAnime,
};
