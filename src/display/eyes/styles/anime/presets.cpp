#include "display/eyes/styles/anime/presets.h"

namespace anime {

namespace {

using kawaii::Expression;
using kawaii::EyeKind;
using kawaii::Mood;

// A cool, slightly lidded resting face.
Expression neutral() {
  Expression e = {};
  e.kind = EyeKind::Round;
  e.lid = 0.15f;
  e.pupil = 1.0f;
  e.sparkle = 1.0f;
  e.scale = 1.0f;
  e.followGlance = 1.0f;
  e.mouthWidth = 7.0f;
  e.mouthCurve = 0.5f;
  return e;
}

void lineMouth(Expression& e, float width, float curve, float shift = 0.0f) {
  e.mouthWidth = width;
  e.mouthCurve = curve;
  e.mouthShift = shift;
}

// An open mouth, optionally showing teeth and fangs.
void openMouth(Expression& e, float width, float curve, float up, float down, float teeth,
               float fang) {
  e.mouthWidth = width;
  e.mouthCurve = curve;
  e.mouthOpenUp = up;
  e.mouthOpenDown = down;
  e.teeth = teeth;
  e.fang = fang;
}

void oMouth(Expression& e, float width, float open) {
  e.mouthWidth = width;
  e.mouthCurve = 0.0f;
  e.mouthOpenUp = open;
  e.mouthOpenDown = open;
  e.mouthHollow = 1.0f;
}

void brows(Expression& e, float lift, float tilt, float asym = 0.0f) {
  e.browLift = lift;
  e.browTilt = tilt;
  e.browAsym = asym;
}

void wink(Expression& e, bool left) {
  e.happyLeft = left ? 1.0f : 0.2f;
  e.happyRight = left ? 0.2f : 1.0f;
  brows(e, 0.2f, 0.0f, left ? 0.6f : -0.6f);  // the open eye's brow goes up
  openMouth(e, 9.0f, 2.0f, 0.0f, 1.8f, 0.8f, 1.0f);  // a fanged smirk
  e.mouthShift = left ? -2.0f : 2.0f;
  e.glint = 0.7f;
}

}  // namespace

Expression expressionFor(Mood mood) {
  Expression e = neutral();

  switch (mood) {
    case Mood::Content:
      e.lid = 0.2f;
      lineMouth(e, 7.0f, 1.2f);
      break;
    case Mood::Smile:
      e.happyLeft = e.happyRight = 0.35f;
      brows(e, 0.2f, 0.0f);
      lineMouth(e, 10.0f, 2.5f);
      break;
    case Mood::Grin:
      e.happyLeft = e.happyRight = 1.0f;  // ^ ^
      brows(e, 0.3f, 0.0f);
      openMouth(e, 13.0f, 2.5f, 0.0f, 3.5f, 1.0f, 0.6f);
      e.blush = 0.6f;
      break;
    case Mood::WinkLeft:
    case Mood::WinkRight:
      wink(e, mood == Mood::WinkLeft);
      break;
    case Mood::Excited:
      e.lid = 0.0f;
      e.scale = 1.1f;
      e.pupil = 1.1f;
      e.sparkle = 1.4f;
      brows(e, 0.8f, -0.2f);
      openMouth(e, 14.0f, 2.0f, 1.0f, 5.0f, 1.0f, 0.5f);
      e.blush = 0.7f;
      e.glint = 1.0f;
      e.bounce = 1.5f;
      break;
    case Mood::Love:
      e.kind = EyeKind::Heart;
      e.lid = 0.05f;
      brows(e, 0.5f, -0.3f);
      openMouth(e, 10.0f, 2.5f, 0.0f, 2.0f, 0.6f, 0.0f);
      e.blush = 1.0f;
      e.glint = 0.6f;
      break;
    case Mood::Focused:
      e.lid = 0.4f;  // narrowed, locked on
      e.pupil = 0.9f;
      brows(e, -0.3f, 0.35f);
      lineMouth(e, 6.0f, 0.0f);
      e.followGlance = 0.0f;  // the code reader leads
      break;
    case Mood::Reading:
      e.lid = 0.35f;
      brows(e, -0.1f, 0.0f);
      lineMouth(e, 6.0f, 0.5f);
      break;
    case Mood::Thinking:
      e.lid = 0.25f;
      e.gazeY = -0.3f;
      brows(e, 0.1f, 0.1f, 0.8f);  // one brow up
      lineMouth(e, 6.0f, -0.5f, 3.0f);
      e.mouthWave = 0.5f;
      e.followGlance = 0.5f;
      break;
    case Mood::Surprised:
      e.lid = 0.0f;
      e.scale = 1.1f;
      e.pupil = 0.45f;  // shrunken pupils
      e.sparkle = 0.6f;
      brows(e, 1.0f, -0.3f);
      oMouth(e, 7.0f, 3.5f);
      break;
    case Mood::Curious:
      e.lid = 0.1f;
      e.pupil = 0.9f;
      brows(e, 0.3f, 0.0f, -0.7f);
      oMouth(e, 5.0f, 1.5f);
      break;
    case Mood::Angry:
      e.lid = 0.35f;
      e.tilt = 0.6f;  // sharp, upturned eyes
      e.pupil = 0.6f;
      e.sparkle = 0.5f;
      brows(e, -0.6f, 1.0f);  // a hard V
      openMouth(e, 12.0f, -1.5f, 1.0f, 2.0f, 1.0f, 1.0f);  // a fanged snarl
      e.vein = 1.0f;
      break;
    case Mood::Panic:
      e.lid = 0.0f;
      e.pupil = 0.35f;  // pinprick pupils
      e.sparkle = 0.4f;
      brows(e, 0.6f, -1.0f);
      lineMouth(e, 12.0f, 0.0f);  // a wobbling line
      e.mouthWave = 1.3f;
      e.sweat = 1.0f;
      break;
    case Mood::Sleepy:
      e.lid = 0.6f;
      e.sparkle = 0.0f;
      brows(e, -0.2f, -0.2f);
      oMouth(e, 6.0f, 3.0f);  // a yawn
      break;
    case Mood::Sad:
      e.lid = 0.25f;
      e.tilt = -0.4f;  // droopy outer corners
      e.pupil = 1.1f;
      e.sparkle = 1.4f;  // glassy, about to cry
      brows(e, 0.3f, -0.9f);
      lineMouth(e, 8.0f, -2.0f);
      e.gloom = 0.8f;
      break;
    case Mood::Dizzy:
      e.kind = EyeKind::Cross;  // spirals
      e.lid = 0.0f;
      brows(e, 0.2f, -0.6f);
      lineMouth(e, 12.0f, -0.5f);
      e.mouthWave = 1.0f;
      e.sweat = 0.6f;
      break;
    case Mood::Talking:
      lineMouth(e, 9.0f, 1.0f);
      e.mouthOpenDown = 0.6f;
      e.teeth = 0.6f;
      break;
    case Mood::Asleep:
      brows(e, -0.2f, 0.0f);
      lineMouth(e, 6.0f, 1.0f);
      break;
    case Mood::Pleased:
      e.lid = 0.3f;  // smug
      e.happyLeft = e.happyRight = 0.45f;
      brows(e, 0.1f, 0.0f);
      lineMouth(e, 9.0f, 2.5f, 2.0f);
      e.blush = 0.5f;
      break;
    case Mood::Aha:
      e.lid = 0.0f;
      e.scale = 1.08f;
      e.pupil = 0.8f;
      e.sparkle = 1.4f;
      e.gazeY = -0.4f;
      brows(e, 1.0f, -0.2f);
      openMouth(e, 10.0f, 2.0f, 0.0f, 3.0f, 1.0f, 0.0f);
      e.glint = 1.0f;
      e.bounce = 1.0f;
      break;
    case Mood::Suspense:
      // A deep breath with tension: risen, wide, pupils tight, brows set.
      e.lid = 0.1f;
      e.rise = 3.0f;
      e.scale = 1.08f;
      e.pupil = 0.6f;
      brows(e, 0.6f, 0.3f);
      oMouth(e, 9.0f, 4.0f);
      e.mouthOpenUp = 3.0f;
      e.blush = 0.5f;
      break;
    case Mood::Yell:
      e.kind = EyeKind::Chevron;  // squeezed shut, shouting
      e.scale = 1.1f;
      brows(e, 0.8f, -0.3f);
      openMouth(e, 18.0f, 2.0f, 2.0f, 7.0f, 1.0f, 1.0f);
      e.glint = 1.0f;
      e.bounce = 1.5f;
      break;
    case Mood::Bliss:
      e.lid = 0.0f;
      e.happyLeft = e.happyRight = 0.3f;
      e.pupil = 1.1f;
      e.sparkle = 1.5f;
      brows(e, 0.5f, -0.2f);
      openMouth(e, 12.0f, 2.5f, 0.0f, 3.5f, 0.8f, 0.0f);
      e.blush = 1.0f;
      e.glint = 1.0f;
      e.bounce = 0.6f;
      break;
    case Mood::Afterglow:
      e.lid = 0.3f;
      e.happyLeft = e.happyRight = 0.55f;
      e.gazeY = -0.25f;
      brows(e, 0.2f, -0.2f);
      lineMouth(e, 9.0f, 2.0f);
      e.blush = 0.9f;
      break;
    case Mood::Count:
      break;
  }

  return e;
}

}  // namespace anime
