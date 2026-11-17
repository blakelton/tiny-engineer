#include "display/eyes/styles/anime/compose.h"

#include <cmath>

#include "display/eyes/styles/kawaii/util.h"

namespace anime {

namespace {

constexpr uint32_t kHopPeriodMs = 360;
constexpr uint32_t kGlintPeriodMs = 560;
constexpr float kGlintBase = 1.6f;
constexpr float kGlintSwing = 1.4f;
constexpr float kHollowAbove = 0.5f;
// Shock shrinks the whole iris, not just the pupil: below pupil 1 the iris
// shrinks too, down to this share of its size at pupil 0.
constexpr float kIrisFloor = 0.4f;

float wave(uint32_t now, uint32_t periodMs) {
  return sinf(kawaii::kTwoPi * (float)(now % periodMs) / (float)periodMs);
}

Eye eyeFrom(const kawaii::EyePose& pose, int8_t side, const kawaii::Expression& e, float happy,
            float open, float lift, float shake, float gazeX, float gazeY) {
  Eye eye = {};
  eye.kind = e.kind;
  eye.side = side;
  eye.cx = pose.cx;
  eye.cy = pose.cy;
  eye.rx = kEyeHalfWidth * e.scale;
  eye.ry = kEyeHalfHeight * e.scale * pose.heightScale;
  eye.open = open;
  eye.happy = happy;
  eye.lid = e.lid;
  eye.tilt = e.tilt;
  eye.iris = kIrisRadius * e.scale * fminf(1.0f, kIrisFloor + (1.0f - kIrisFloor) * e.pupil);
  eye.pupil = kPupilRadius * e.scale * e.pupil;
  eye.gazeX = gazeX;
  eye.gazeY = gazeY;
  eye.pupilShake = shake;
  eye.shine = e.sparkle;
  eye.browLift = lift;
  eye.browTilt = e.browTilt;
  return eye;
}

}  // namespace

void composeFace(const kawaii::Pose& pose, const kawaii::Animator& animator, uint32_t now,
                 Face& out) {
  const kawaii::Expression& e = animator.current();
  const float hop = -fabsf(wave(now, kHopPeriodMs)) * e.bounce - e.rise;
  const float open = pose.open * animator.kindOpen();
  const float shake = e.quiver * sinf(animator.quiverPhase());
  const float gx = pose.gazeX + e.gazeX;
  const float gy = pose.gazeY + e.gazeY;

  kawaii::EyePose left = pose.left;
  kawaii::EyePose right = pose.right;
  left.cy += hop;
  right.cy += hop;

  // browAsym raises the screen-right brow (+) or the left (-).
  out.left = eyeFrom(left, 1, e, e.happyLeft, open, e.browLift - e.browAsym, shake, gx, gy);
  out.right = eyeFrom(right, -1, e, e.happyRight, open, e.browLift + e.browAsym, shake, gx, gy);

  kawaii::Mouth& m = out.mouth;
  m.cx = (left.cx + right.cx) * 0.5f + e.mouthShift;
  m.cy = (left.cy + right.cy) * 0.5f + kMouthDrop;
  m.width = e.mouthWidth;
  m.curve = e.mouthCurve;
  m.omega = e.mouthOmega;
  m.openUp = e.mouthOpenUp;
  m.openDown = e.mouthOpenDown;
  m.wave = e.mouthWave;
  m.hollow = e.mouthHollow >= kHollowAbove;

  out.teeth = e.teeth;
  out.fang = e.fang;
  out.vein = e.vein;
  out.sweat = e.sweat;
  out.gloom = e.gloom;
  out.blush = e.blush;
  out.glint = e.glint * (kGlintBase + kGlintSwing * wave(now, kGlintPeriodMs));
}

}  // namespace anime
