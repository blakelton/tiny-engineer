#include <cstring>
#include <unity.h>

#include "display/eyes/styles/anime/compose.h"
#include "display/eyes/styles/anime/face.h"
#include "display/eyes/styles/anime/presets.h"
#include "display/eyes/styles/kawaii/burst.h"

using kawaii::EyeKind;
using kawaii::Mood;

namespace {

uint8_t g_frame[anime::kFrameBytes];

bool lit(int x, int y) {
  return kawaii::pixelAt(g_frame, x, y);
}

int litCount(int x0, int y0, int x1, int y1) {
  int n = 0;
  for (int y = y0; y <= y1; y++) {
    for (int x = x0; x <= x1; x++) {
      n += lit(x, y) ? 1 : 0;
    }
  }
  return n;
}

anime::Eye plainEye(float cx, int8_t side) {
  anime::Eye e = {};
  e.kind = EyeKind::Round;
  e.side = side;
  e.cx = cx;
  e.cy = 16.0f;
  e.rx = anime::kEyeHalfWidth;
  e.ry = anime::kEyeHalfHeight;
  e.open = 1.0f;
  e.iris = anime::kIrisRadius;
  e.pupil = anime::kPupilRadius;
  e.shine = 1.0f;
  return e;
}

anime::Face plainFace() {
  anime::Face f = {};
  f.left = plainEye(32.0f, 1);
  f.right = plainEye(96.0f, -1);
  f.mouth.cx = 64.0f;
  f.mouth.cy = 24.0f;
  return f;
}

// First lit row from the top in column x, within rows 0..maxRow (-1 if none).
int firstLitRow(int x, int maxRow) {
  for (int y = 0; y <= maxRow; y++) {
    if (lit(x, y)) {
      return y;
    }
  }
  return -1;
}

}  // namespace

void setUp() {
  memset(g_frame, 0, sizeof(g_frame));
}

void tearDown() {}

void test_render_rejects_small_buffer() {
  const anime::Face f = plainFace();
  TEST_ASSERT_FALSE(anime::render(f, g_frame, anime::kFrameBytes - 1));
  TEST_ASSERT_FALSE(anime::render(f, nullptr, anime::kFrameBytes));
  TEST_ASSERT_TRUE(anime::render(f, g_frame, anime::kFrameBytes));
}

void test_open_eye_has_lash_iris_pupil_and_highlight() {
  anime::render(plainFace(), g_frame, anime::kFrameBytes);
  TEST_ASSERT_TRUE(litCount(20, 6, 44, 10) > 15);  // the lash line across the top
  TEST_ASSERT_TRUE(lit(32, 17));                   // pupil
  TEST_ASSERT_TRUE(lit(37, 17));                   // iris rim
  TEST_ASSERT_FALSE(lit(30, 15));                  // highlight: paper inside the iris
  TEST_ASSERT_FALSE(lit(23, 15));                  // white of the eye
}

void test_brows_carry_the_emotion() {
  anime::Face angry = plainFace();
  angry.left.browTilt = 1.0f;  // a V: inner end down
  anime::render(angry, g_frame, anime::kFrameBytes);
  const int angryInner = firstLitRow(35, 8);
  const int angryOuter = firstLitRow(20, 8);

  anime::Face worried = plainFace();
  worried.left.browTilt = -1.0f;  // inner end up
  memset(g_frame, 0, sizeof(g_frame));
  anime::render(worried, g_frame, anime::kFrameBytes);
  const int worriedInner = firstLitRow(35, 8);
  const int worriedOuter = firstLitRow(20, 8);

  TEST_ASSERT_TRUE(angryInner >= 0 && angryOuter >= 0);
  TEST_ASSERT_TRUE(angryInner > angryOuter);
  TEST_ASSERT_TRUE(worriedInner < worriedOuter);

  anime::Face raised = plainFace();
  raised.left.browLift = 1.0f;
  memset(g_frame, 0, sizeof(g_frame));
  anime::render(raised, g_frame, anime::kFrameBytes);
  TEST_ASSERT_TRUE(firstLitRow(28, 8) < firstLitRow(92, 8));  // only the raised one moved up
}

void test_full_smile_closes_into_an_arch() {
  anime::Face f = plainFace();
  f.left.happy = 1.0f;
  anime::render(f, g_frame, anime::kFrameBytes);
  TEST_ASSERT_TRUE(litCount(26, 10, 38, 15) > 5);   // the "^"
  TEST_ASSERT_EQUAL_INT(0, litCount(26, 17, 38, 22));  // no iris below it
}

// A closing eye is a curve near its center line, never a stroke up the side.
void test_closing_eye_stays_a_curve() {
  for (float open = 0.0f; open <= 0.3f; open += 0.02f) {
    for (float cx = 31.0f; cx < 33.0f; cx += 0.13f) {
      anime::Face f = plainFace();
      f.left.cx = cx;
      f.left.open = open;
      f.right.rx = 0.0f;
      memset(g_frame, 0, sizeof(g_frame));
      anime::render(f, g_frame, anime::kFrameBytes);
      TEST_ASSERT_EQUAL_INT(0, litCount(14, 10, 50, 11));  // between the brow and the lid line
    }
  }
}

void test_open_mouth_shows_teeth_as_gaps() {
  anime::Face f = plainFace();
  f.mouth.width = 13.0f;
  f.mouth.curve = 2.5f;
  f.mouth.openDown = 3.5f;
  f.teeth = 1.0f;
  anime::render(f, g_frame, anime::kFrameBytes);
  TEST_ASSERT_TRUE(lit(64, 26));   // upper lip
  TEST_ASSERT_FALSE(lit(64, 27));  // teeth
  TEST_ASSERT_TRUE(lit(64, 29));   // inside of the mouth

  f.teeth = 0.0f;
  anime::render(f, g_frame, anime::kFrameBytes);
  TEST_ASSERT_TRUE(lit(64, 27));
}

void test_symbols_appear_past_half() {
  anime::Face f = plainFace();
  f.vein = 0.4f;
  f.sweat = 0.4f;
  anime::render(f, g_frame, anime::kFrameBytes);
  const int corner = litCount(0, 0, 12, 7);
  const int temple = litCount(110, 6, 127, 18);

  f.vein = 1.0f;
  f.sweat = 1.0f;
  anime::render(f, g_frame, anime::kFrameBytes);
  TEST_ASSERT_TRUE(litCount(0, 0, 12, 7) > corner);   // anger vein
  TEST_ASSERT_TRUE(litCount(110, 6, 127, 18) > temple);  // sweat drop
}

void test_dizzy_eyes_are_spirals() {
  anime::Face f = plainFace();
  f.left.kind = EyeKind::Cross;
  anime::render(f, g_frame, anime::kFrameBytes);
  TEST_ASSERT_TRUE(lit(32, 16));  // the spiral starts at the center
  TEST_ASSERT_TRUE(litCount(24, 12, 40, 21) > 25);
}

void test_pupil_shake_moves_the_iris() {
  anime::Face f = plainFace();
  anime::render(f, g_frame, anime::kFrameBytes);
  TEST_ASSERT_FALSE(lit(30, 15));  // the highlight
  TEST_ASSERT_TRUE(lit(32, 15));   // the pupil
  f.left.pupilShake = 2.0f;
  anime::render(f, g_frame, anime::kFrameBytes);
  TEST_ASSERT_FALSE(lit(32, 15));  // the highlight moved two pixels right with the iris
}

void test_every_mood_draws_on_the_panel() {
  for (int i = 0; i < (int)Mood::Count; i++) {
    const Mood m = (Mood)i;
    kawaii::Animator a;
    a.snap(anime::expressionFor(m));
    const kawaii::Pose p = {{32, 16, 1}, {96, 16, 1}, 1.0f, 0.0f, 0.0f};
    anime::Face f = {};
    anime::composeFace(p, a, 200, f);
    f.right.cx = 124.0f;  // half off the panel: drawing must clip
    TEST_ASSERT_TRUE(anime::render(f, g_frame, anime::kFrameBytes));
    TEST_ASSERT_TRUE_MESSAGE(litCount(0, 0, 127, 31) > 20, kawaii::moodName(m));
  }
}

void test_presets_are_expressive() {
  const kawaii::Expression angry = anime::expressionFor(Mood::Angry);
  TEST_ASSERT_TRUE(angry.browTilt > 0.5f && angry.vein > 0.5f && angry.fang > 0.5f);
  TEST_ASSERT_TRUE(anime::expressionFor(Mood::Sad).browTilt < -0.5f);
  TEST_ASSERT_TRUE(anime::expressionFor(Mood::Surprised).pupil < 0.6f);  // shrunken pupils
  TEST_ASSERT_TRUE(anime::expressionFor(Mood::Panic).sweat > 0.5f);
  TEST_ASSERT_TRUE(anime::expressionFor(Mood::Dizzy).kind == EyeKind::Cross);
  TEST_ASSERT_TRUE(anime::expressionFor(Mood::Thinking).browAsym != 0.0f);  // one brow up
  TEST_ASSERT_EQUAL_FLOAT(0.0f, anime::expressionFor(Mood::Focused).followGlance);
}

void test_stars_burst_from_anime_eyes() {
  anime::Face f = plainFace();
  const kawaii::SparkAnchor left = {f.left.cx, f.left.cy, f.left.rx, f.left.side};
  const kawaii::SparkAnchor right = {f.right.cx, f.right.cy, f.right.rx, f.right.side};
  kawaii::addStarBurst(f.sparks, f.sparkCount, left, right, 250, 0.5f);
  TEST_ASSERT_TRUE(f.sparkCount > 4);
  for (uint8_t i = 0; i < f.sparkCount; i++) {
    TEST_ASSERT_TRUE(f.sparks[i].x < 32 - 12 || f.sparks[i].x > 96 + 12);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_render_rejects_small_buffer);
  RUN_TEST(test_open_eye_has_lash_iris_pupil_and_highlight);
  RUN_TEST(test_brows_carry_the_emotion);
  RUN_TEST(test_full_smile_closes_into_an_arch);
  RUN_TEST(test_closing_eye_stays_a_curve);
  RUN_TEST(test_open_mouth_shows_teeth_as_gaps);
  RUN_TEST(test_symbols_appear_past_half);
  RUN_TEST(test_dizzy_eyes_are_spirals);
  RUN_TEST(test_pupil_shake_moves_the_iris);
  RUN_TEST(test_every_mood_draws_on_the_panel);
  RUN_TEST(test_presets_are_expressive);
  RUN_TEST(test_stars_burst_from_anime_eyes);
  return UNITY_END();
}
