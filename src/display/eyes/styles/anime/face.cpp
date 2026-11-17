#include "display/eyes/styles/anime/face.h"

#include <cmath>
#include <cstring>
#include <initializer_list>

#include "display/eyes/styles/kawaii/util.h"

namespace anime {

namespace {

using kawaii::clamp01;
using kawaii::lerp;

constexpr int kRowBytes = kWidth / 8;

// ------------------------------------------------------------- tuning ---
// Sizes are fractions of the eye's half width (rx) or half height (ry)
// unless they end in Px.

// Eye shape: an almond whose upper lid peaks toward the nose.
constexpr float kInnerLift = 0.15f;   // upper lid higher on the inner side
constexpr float kLowerDepth = 0.8f;   // lower lid, × ry at the center
constexpr float kLidTravel = 1.5f;    // `lid` = 1 lowers the upper lid this far
constexpr float kTiltDepth = 0.8f;    // `tilt` = 1 drops one corner this far
constexpr float kCheekRise = 1.7f;    // `happy` = 1 pushes the lower lid up this far
constexpr float kClosedBelowPx = 1.5f;  // a thinner opening draws as closed
constexpr float kHappyClosedAbove = 0.5f;

// Lash line: heavy at the outer corner, lighter toward the nose, with a
// flick past the outer corner.
constexpr float kLashOuterPx = 3.0f;
constexpr float kLashInnerPx = 1.5f;
constexpr int kFlickLenPx = 3;
constexpr float kFlickRise = 0.5f;    // px up per px out
constexpr float kLowerReach = 0.35f;  // lower lid line from the outer corner to here (s)

// Iris: screentone, darker under the lid; a solid pupil; two highlights.
constexpr float kMinIrisPx = 1.5f;
constexpr float kIrisDropPx = 1.0f;   // sits a little low, tucked under the lid
constexpr float kIrisTravelX = 0.75f;  // how far it looks sideways, × rx (minus its radius)
constexpr float kIrisTravelY = 0.4f;
constexpr float kToneTop = 0.8f;      // screentone density at the top of the iris
constexpr float kToneBottom = 0.3f;
constexpr float kBayerLevels = 16.0f;
constexpr float kBigShineAt = -0.35f;   // × iris radius, up and toward the left
constexpr float kBigShine = 0.36f;
constexpr float kSmallShineX = 0.4f;
constexpr float kSmallShineY = 0.3f;
constexpr float kSmallShine = 0.13f;
constexpr float kHeartScale = 0.95f;
constexpr float kHeartLift = 0.15f;

// The other eye kinds.
constexpr float kSqueezeHeight = 0.9f;  // "> <", × ry
constexpr float kSqueezeWidth = 0.8f;
constexpr float kSqueezeHalfStrokePx = 1.0f;
constexpr float kSpiralTurns = 2.5f;
constexpr float kSpiralStep = 0.12f;  // rad; ~1 px between points at the rim
constexpr float kSpiralSize = 0.6f;   // × rx
constexpr float kSpiralSquash = 0.8f;

// Closed eyes.
constexpr float kClosedSagPx = 2.0f;
constexpr float kClosedArchPx = 3.0f;  // a smiling "^" eye

// Brows: thick at the inner end, tapering outward.
constexpr float kBrowInner = 0.3f;    // inner end, × rx toward the nose
constexpr float kBrowOuter = 1.15f;   // outer end, × rx away from it
constexpr float kBrowGapPx = 3.5f;    // above the top of the eye
constexpr float kBrowTravelPx = 3.0f;  // browLift = ±1
constexpr float kBrowTiltInnerPx = 3.0f;  // browTilt = 1: inner end down this far
constexpr float kBrowTiltOuterPx = 1.0f;  // ... and the outer end up this far
constexpr float kBrowArchPx = 1.5f;
constexpr float kBrowInnerThickPx = 2.2f;
constexpr float kBrowOuterThickPx = 1.0f;

// Mouth.
constexpr int kMaxMouthCols = 48;
constexpr float kOpenMouthPx = 1.2f;   // openUp + openDown above this fills it
constexpr float kMinTeeth = 0.3f;
constexpr float kTallMouthPx = 5.0f;   // tall enough for two rows of teeth
constexpr float kFangAt = 0.45f;       // × half width from the center
constexpr float kBigFang = 0.7f;
constexpr float kMouthWaveCycles = 3.0f;

// Symbols.
constexpr float kSymbolAbove = 0.5f;  // drawn once their amount passes this
constexpr int kGloomLenPx = 5;
constexpr int kGloomSpacingPx = 3;
constexpr int kBlushLines = 3;
constexpr float kMinGlint = 0.3f;
constexpr float kGlintOutsetPx = 4.0f;

constexpr uint8_t kBayer[4][4] = {
  {0, 8, 2, 10},
  {12, 4, 14, 6},
  {3, 11, 1, 9},
  {15, 7, 13, 5},
};

// 💢 anger vein: four corner brackets.
constexpr uint8_t kVein[7] = {
  0b0100010,
  0b1100011,
  0b0000000,
  0b0000000,
  0b0000000,
  0b1100011,
  0b0100010,
};

// Sweat drop.
constexpr uint8_t kSweat[7] = {
  0b00100,
  0b00100,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b01110,
};

// ------------------------------------------------------------ helpers ---

// Floor without the floorf library call (the C3 has no FPU).
int floorInt(float v) {
  const int i = (int)v;
  return v < (float)i ? i - 1 : i;
}

float center(int i) {
  return i + 0.5f;
}

class Canvas {
 public:
  explicit Canvas(uint8_t* bits) : bits_(bits) {}

  void set(int x, int y, bool on) {
    if (x < 0 || y < 0 || x >= kWidth || y >= kHeight) {
      return;
    }
    uint8_t& byte = bits_[y * kRowBytes + x / 8];
    const uint8_t mask = (uint8_t)(0x80u >> (x & 7));
    byte = on ? (uint8_t)(byte | mask) : (uint8_t)(byte & ~mask);
  }

  void column(int x, int y0, int y1, bool on = true) {
    if (y0 > y1) {
      const int t = y0;
      y0 = y1;
      y1 = t;
    }
    for (int y = y0; y <= y1; y++) {
      set(x, y, on);
    }
  }

  void run(int x0, int x1, int y, bool on = true) {
    for (int x = x0; x <= x1; x++) {
      set(x, y, on);
    }
  }

  void bitmap(const uint8_t* rows, int count, int width, int x0, int y0) {
    for (int r = 0; r < count; r++) {
      for (int c = 0; c < width; c++) {
        if (rows[r] & (1u << (width - 1 - c))) {
          set(x0 + c, y0 + r, true);
        }
      }
    }
  }

  void star(int x, int y, int arm) {
    if (arm < 0) {
      return;
    }
    set(x, y, true);
    for (int i = 1; i <= arm; i++) {
      set(x - i, y, true);
      set(x + i, y, true);
      set(x, y - i, true);
      set(x, y + i, true);
    }
  }

 private:
  uint8_t* bits_;
};

// Half width of row dy of a disc of radius r centered on a pixel: every
// whole-pixel (dx, dy) with dx² + dy² ≤ r². -1 when the row is empty.
int discHalf(float r, int dy) {
  const float rest = r * r - (float)(dy * dy);
  if (rest < 0.0f) {
    return -1;
  }
  int w = (int)sqrtf(rest);
  while ((float)((w + 1) * (w + 1)) <= rest) {
    w++;
  }
  while (w >= 0 && (float)(w * w) > rest) {
    w--;
  }
  return w;
}

// Gaze inside the unit circle, so diagonal looks travel no further.
void unitGaze(const Eye& e, float& gx, float& gy) {
  gx = e.gazeX;
  gy = e.gazeY;
  const float len2 = gx * gx + gy * gy;
  if (len2 > 1.0f) {
    const float len = sqrtf(len2);
    gx /= len;
    gy /= len;
  }
}

// ---------------------------------------------------------- eye shape ---

constexpr int kMaxEyeCols = 40;

// The opening of an eye, column by column: lit lash above `top`, the lower
// lid at `bottom`. `s` runs from the outer corner (-1) to the inner (+1).
struct EyeColumns {
  int x0;
  int count;
  float top[kMaxEyeCols];
  float bottom[kMaxEyeCols];
  float s[kMaxEyeCols];
  int16_t firstRow[kMaxEyeCols];  // rows whose centers lie inside the opening
  int16_t lastRow[kMaxEyeCols];

  bool inside(int x, int y) const {
    const int i = x - x0;
    return i >= 0 && i < count && y >= firstRow[i] && y <= lastRow[i];
  }

  // center(y) > top and center(y) < bottom, as whole rows.
  void settleRows(int i) {
    firstRow[i] = (int16_t)(floorInt(top[i] - 0.5f) + 1);
    const int last = floorInt(bottom[i] - 0.5f);
    lastRow[i] = (int16_t)(center(last) < bottom[i] ? last : last - 1);
  }

  float height() const {
    float h = 0.0f;
    for (int i = 0; i < count; i++) {
      h = fmaxf(h, bottom[i] - top[i]);
    }
    return h;
  }
};

bool eyeColumns(const Eye& e, EyeColumns& out) {
  const float ry = e.ry * clamp01(e.open);
  out.x0 = floorInt(e.cx - e.rx);
  out.count = floorInt(e.cx + e.rx) - out.x0 + 1;
  if (out.count <= 0 || out.count > kMaxEyeCols) {
    return false;
  }

  const float happy = clamp01(e.happy);
  const float lidLine = e.cy - ry + clamp01(e.lid) * kLidTravel * ry;
  for (int i = 0; i < out.count; i++) {
    const float u = (center(out.x0 + i) - e.cx) / e.rx;
    const float u2 = u * u;
    const float s = u * e.side;  // + toward the nose
    out.s[i] = s;
    if (u2 >= 1.0f) {
      out.top[i] = out.bottom[i] = e.cy;
      out.firstRow[i] = 1;
      out.lastRow[i] = 0;
      continue;
    }
    const float arc = sqrtf(1.0f - u2);
    const float lid = lidLine + e.tilt * s * ry * kTiltDepth;
    out.top[i] = fmaxf(e.cy - ry * arc * (1.0f + kInnerLift * s), lid);
    out.bottom[i] = e.cy + ry * (1.0f - u2) * (kLowerDepth - happy * kCheekRise);
    out.settleRows(i);
  }
  return true;
}

// ------------------------------------------------------------ the eye ---

void drawLash(Canvas& c, const Eye& e, const EyeColumns& cols) {
  int outer = -1;
  for (int i = 0; i < cols.count; i++) {
    if (cols.bottom[i] <= cols.top[i]) {
      continue;
    }
    const float thick = lerp(kLashOuterPx, kLashInnerPx, (cols.s[i] + 1.0f) * 0.5f);
    float high = cols.top[i];
    for (int n = i - 1; n <= i + 1; n += 2) {
      if (n >= 0 && n < cols.count && cols.bottom[n] > cols.top[n]) {
        high = fminf(high, (cols.top[i] + cols.top[n]) * 0.5f);  // stay connected
      }
    }
    const int y = floorInt(cols.top[i]);
    c.column(cols.x0 + i, floorInt(high) - (int)(thick + 0.5f) + 1, y);
    if (outer < 0 || cols.s[i] < cols.s[outer]) {
      outer = i;
    }
  }

  // The flick past the outer corner.
  if (outer >= 0) {
    const int x = cols.x0 + outer;
    const float y = cols.top[outer];
    for (int k = 1; k <= kFlickLenPx; k++) {
      const int fx = x - e.side * k;
      const int fy = floorInt(y - k * kFlickRise);
      c.column(fx, fy - (k < kFlickLenPx ? 1 : 0), fy);
    }
  }
}

void drawLowerLid(Canvas& c, const EyeColumns& cols) {
  for (int i = 0; i < cols.count; i++) {
    if (cols.s[i] <= kLowerReach && cols.bottom[i] > cols.top[i] + 1.0f) {
      c.set(cols.x0 + i, floorInt(cols.bottom[i]), true);
    }
  }
}

// Squared radius as a whole-pixel threshold: dx² + dy² ≤ r² exactly when
// dx² + dy² ≤ this.
int radius2(float r) {
  return r <= 0.0f ? -1 : (int)(r * r);
}

// Everything an iris needs per pixel, worked out once per eye.
struct Iris {
  int rim2;          // inside this is tone; outside, the solid rim
  int pupil2;
  int bigX;          // big highlight, offset from the center
  int big2;
  int smallX;
  int smallY;
  int small2;
  float r;
  float heartScale;  // 1 / heart size

  Iris(const Eye& e, float radius) : r(radius) {
    rim2 = radius2(radius - 1.0f);
    pupil2 = radius2(e.pupil);
    bigX = (int)roundf(kBigShineAt * radius);
    big2 = e.shine > 0.0f ? radius2(e.shine * kBigShine * radius) : -1;
    smallX = (int)roundf(kSmallShineX * radius);
    smallY = (int)roundf(kSmallShineY * radius);
    small2 = e.shine > 0.0f ? radius2(e.shine * kSmallShine * radius) : -1;
    heartScale = 1.0f / (radius * kHeartScale);
  }

  // Screentone threshold for row dy: darker toward the top.
  int tone(int dy) const {
    // bayer < level × 16 for a whole-number bayer value is bayer < ceil(...).
    return (int)ceilf(lerp(kToneTop, kToneBottom, (dy + r) / (2.0f * r)) * kBayerLevels);
  }

  // The rim (ink) keeps the iris round, highlights (paper) sit inside it,
  // then the pupil (ink) and the row's screentone.
  bool ink(int dx, int dy, int x, int y, int tone) const {
    const int d2 = dx * dx + dy * dy;
    if (d2 > rim2) {
      return true;
    }
    const int bx = dx - bigX;
    const int by = dy - bigX;
    if (bx * bx + by * by <= big2) {
      return false;
    }
    const int sx = dx - smallX;
    const int sy = dy - smallY;
    if (sx * sx + sy * sy <= small2) {
      return false;
    }
    return d2 <= pupil2 || kBayer[y & 3][x & 3] < tone;
  }

};

// Inside the heart, with X² for the column and Y, Y² for the row given.
bool insideHeart(float X2, float Y, float Y2) {
  const float a = X2 + Y2 - 1.0f;
  return a * a * a - X2 * Y2 * Y <= 0.0f;
}

constexpr int kMaxIrisRows = 16;

void drawIris(Canvas& c, const Eye& e, const EyeColumns& cols) {
  const float r = e.iris;
  if (r < kMinIrisPx) {
    return;
  }
  float gx = 0.0f;
  float gy = 0.0f;
  unitGaze(e, gx, gy);
  const int ix = floorInt(e.cx + gx * fmaxf(0.0f, e.rx * kIrisTravelX - r) + e.pupilShake);
  const int iy = floorInt(e.cy + kIrisDropPx + gy * e.ry * kIrisTravelY);
  const bool heart = e.kind == EyeKind::Heart;
  const Iris iris(e, r);

  const int rows = (int)r;
  if (rows >= kMaxIrisRows) {
    return;
  }
  float heartX2[kMaxIrisRows];  // the heart's X² by |dx|
  if (heart) {
    for (int k = 0; k <= rows; k++) {
      const float X = k * iris.heartScale;
      heartX2[k] = X * X;
    }
  }
  for (int dy = -rows; dy <= rows; dy++) {
    const int w = discHalf(r, dy);
    const int y = iy + dy;
    const int tone = iris.tone(dy);
    const float hy = -dy * iris.heartScale + kHeartLift;
    const float hy2 = hy * hy;
    for (int dx = -w; dx <= w; dx++) {
      const int x = ix + dx;
      if (!cols.inside(x, y)) {
        continue;
      }
      const bool ink = heart ? insideHeart(heartX2[dx < 0 ? -dx : dx], hy, hy2)
                             : iris.ink(dx, dy, x, y, tone);
      if (ink) {
        c.set(x, y, true);
      }
    }
  }
}

void drawSqueeze(Canvas& c, const Eye& e) {
  const float h = e.ry * kSqueezeHeight;
  const float w = e.rx * kSqueezeWidth;
  for (int y = floorInt(e.cy - h); y <= floorInt(e.cy + h); y++) {
    const float a = fabsf(center(y) - e.cy) / h;
    if (a > 1.0f) {
      continue;
    }
    const float xc = e.cx + e.side * (w * 0.5f - w * a);
    c.run(floorInt(xc - kSqueezeHalfStrokePx), floorInt(xc + kSqueezeHalfStrokePx), y);
  }
}

// Dizzy: an outward spiral. Each point turns the last by a fixed small
// rotation, in fixed point: the C3 multiplies integers in hardware but
// floats in software.
constexpr int kFixShift = 14;            // rotation in Q14 (1.0 = 16384)
constexpr float kFixOne = 16384.0f;
constexpr int kSubShift = 8;             // positions and radius in Q8 (1 px = 256)
constexpr float kSubOne = 256.0f;

void drawSpiral(Canvas& c, const Eye& e) {
  const float total = kSpiralTurns * kawaii::kTwoPi;
  const int steps = (int)(total / kSpiralStep);
  const int32_t stepCos = (int32_t)((1.0f - kSpiralStep * kSpiralStep * 0.5f) * kFixOne);
  const int32_t stepSin = (int32_t)(kSpiralStep * kFixOne);
  const int32_t grow = (int32_t)(e.rx * kSpiralSize / (float)steps * kSubOne);
  const int32_t squash = (int32_t)(kSpiralSquash * kSubOne);
  const int32_t cx = (int32_t)(e.cx * kSubOne);
  const int32_t cy = (int32_t)(e.cy * kSubOne);

  int32_t cs = (int32_t)kFixOne;
  int32_t sn = 0;
  int32_t r = 0;
  for (int k = 0; k <= steps; k++, r += grow) {
    const int32_t dx = (r * cs) >> kFixShift;
    const int32_t dy = (((r * sn) >> kFixShift) * squash) >> kSubShift;
    c.set((cx + e.side * dx) >> kSubShift, (cy + dy) >> kSubShift, true);
    const int32_t ncs = (cs * stepCos - sn * stepSin) >> kFixShift;
    sn = (sn * stepCos + cs * stepSin) >> kFixShift;
    cs = ncs;
  }
}

// A closed eye: a lash curve that sags ("‿") or, when smiling, arches ("^").
void drawClosed(Canvas& c, const Eye& e) {
  const bool smiling = e.happy > kHappyClosedAbove;
  const int x0 = floorInt(e.cx - e.rx);
  const int x1 = floorInt(e.cx + e.rx);
  int prev = 0;
  bool first = true;  // nothing drawn yet to connect to
  for (int x = x0; x <= x1; x++) {
    const float u = (center(x) - e.cx) / e.rx;
    if (u * u >= 1.0f) {
      continue;
    }
    const float bend = (1.0f - u * u);
    const float y = smiling ? e.cy + kClosedArchPx * 0.5f - kClosedArchPx * bend
                            : e.cy - kClosedSagPx * 0.5f + kClosedSagPx * bend;
    const float thick = lerp(kLashOuterPx, kLashInnerPx, (u * e.side + 1.0f) * 0.5f);
    const int yi = floorInt(y);
    const int top = first || yi < prev ? yi : prev;
    c.column(x, top - (int)(thick + 0.5f) + 2, yi);
    prev = yi;
    first = false;
  }
}

void drawEye(Canvas& c, const Eye& e) {
  if (e.rx < 1.0f) {
    return;
  }
  if (e.kind == EyeKind::Chevron) {
    drawSqueeze(c, e);
    return;
  }

  EyeColumns cols;
  if (!eyeColumns(e, cols)) {
    return;
  }
  if (cols.height() < kClosedBelowPx || e.happy > 1.0f - 1e-3f) {
    drawClosed(c, e);
    return;
  }
  if (e.kind == EyeKind::Cross) {
    drawSpiral(c, e);
  } else {
    drawIris(c, e, cols);
  }
  drawLash(c, e, cols);
  drawLowerLid(c, cols);
}

// --------------------------------------------------------------- brows ---

void drawBrow(Canvas& c, const Eye& e) {
  const float xi = e.cx + e.side * kBrowInner * e.rx;
  const float xo = e.cx - e.side * kBrowOuter * e.rx;
  const float base = e.cy - e.ry - kBrowGapPx - e.browLift * kBrowTravelPx;
  const int a = floorInt(fminf(xi, xo));
  const int b = floorInt(fmaxf(xi, xo));
  int prev = 0;
  bool first = true;
  for (int x = a; x <= b; x++) {
    const float t = clamp01((center(x) - xi) / (xo - xi));  // 0 inner .. 1 outer
    const float y = fmaxf(0.0f, base +
      e.browTilt * (kBrowTiltInnerPx * (1.0f - t) - kBrowTiltOuterPx * t) -
      kBrowArchPx * 4.0f * t * (1.0f - t));  // stays on the panel
    const int thick = (int)(lerp(kBrowInnerThickPx, kBrowOuterThickPx, t) + 0.5f);
    const int yi = floorInt(y);
    const int lo = first ? yi : (yi < prev ? yi : prev);  // stay connected
    const int hi = first ? yi : (yi > prev ? yi : prev);
    c.column(x, lo, hi + thick - 1);
    prev = yi;
    first = false;
  }
}

// --------------------------------------------------------------- mouth ---

float lipAt(const kawaii::Mouth& m, float u) {
  const float omega = clamp01(m.omega);
  const float single = 1.0f - u * u;
  const float twinU = 2.0f * fabsf(u) - 1.0f;
  const float twin = 1.0f - twinU * twinU;
  const float lip = m.cy + m.curve * ((1.0f - omega) * single + omega * twin);
  return m.wave == 0.0f ? lip : lip + m.wave * sinf(u * kMouthWaveCycles * kawaii::kPi);
}

void drawTeeth(Canvas& c, const kawaii::Mouth& m, const int* top, const int* bottom, int x0,
               int count, float teeth, float fang) {
  if (teeth < kMinTeeth) {
    return;
  }
  const float halfW = m.width * 0.5f;
  const int fangL = floorInt(m.cx - halfW * kFangAt);
  const int fangR = floorInt(m.cx + halfW * kFangAt);
  const int fangLen = fang > kBigFang ? 2 : (fang > kMinTeeth ? 1 : 0);
  for (int i = 1; i + 1 < count; i++) {
    const int x = x0 + i;
    const int height = bottom[i] - top[i];
    if (height < 3) {
      continue;
    }
    const int rows = height >= (int)kTallMouthPx ? 2 : 1;
    int clear = rows;
    if (x == fangL || x == fangR) {
      clear += fangLen;
    }
    clear = clear < height - 1 ? clear : height - 1;
    c.column(x, top[i] + 1, top[i] + clear, false);
  }
}

void drawMouth(Canvas& c, const kawaii::Mouth& m, float teeth, float fang) {
  const float halfW = m.width * 0.5f;
  if (halfW < 0.5f) {
    return;
  }
  const int x0 = floorInt(m.cx - halfW);
  const int count = floorInt(m.cx + halfW) - x0 + 1;
  if (count <= 0 || count > kMaxMouthCols) {
    return;
  }
  const bool filled = fmaxf(0.0f, m.openUp) + fmaxf(0.0f, m.openDown) > kOpenMouthPx;

  int top[kMaxMouthCols];
  int bottom[kMaxMouthCols];
  int prevTop = 0;
  int prevBottom = 0;
  bool connected = false;  // the column to the left was part of the mouth
  for (int i = 0; i < count; i++) {
    const float u = (center(x0 + i) - m.cx) / halfW;
    if (u * u > 1.0f) {
      top[i] = bottom[i] = 0;  // no mouth here: zero height, nothing to connect to
      connected = false;
      continue;
    }
    const float rim = sqrtf(1.0f - u * u);
    const float lip = lipAt(m, u);
    top[i] = floorInt(lip - fmaxf(0.0f, m.openUp) * rim);
    bottom[i] = floorInt(lip + fmaxf(0.0f, m.openDown) * rim);
    const int t = connected && prevTop < top[i] ? prevTop : top[i];
    const int b = connected && prevBottom > bottom[i] ? prevBottom : bottom[i];
    const bool end = !connected || i == count - 1;
    if (!filled || (m.hollow && !end)) {
      c.column(x0 + i, t, top[i]);
      if (filled) {
        c.column(x0 + i, bottom[i], b);
      }
    } else {
      c.column(x0 + i, t, b);
    }
    prevTop = top[i];
    prevBottom = bottom[i];
    connected = true;
  }
  if (filled && !m.hollow) {
    drawTeeth(c, m, top, bottom, x0, count, teeth, fang);
  }
}

// ------------------------------------------------------------- symbols ---

void drawSymbols(Canvas& c, const Face& f) {
  const Eye& l = f.left;   // screen-left: its outer side is the panel's left edge
  const Eye& r = f.right;
  if (f.vein > kSymbolAbove) {
    c.bitmap(kVein, 7, 7, floorInt(l.cx - l.rx * kBrowOuter) - 8, 0);
  }
  if (f.sweat > kSymbolAbove) {
    c.bitmap(kSweat, 7, 5, floorInt(r.cx + r.rx * kBrowOuter) + 2, floorInt(r.cy - r.ry));
  }
  if (f.gloom > kSymbolAbove) {
    const int len = (int)(f.gloom * kGloomLenPx + 0.5f);
    for (const Eye* e : {&l, &r}) {
      for (int x = floorInt(e->cx - e->rx); x <= floorInt(e->cx + e->rx); x += kGloomSpacingPx) {
        for (int y = 0; y < len; y += 2) {
          c.set(x, y, true);  // dashed
        }
      }
    }
  }
  const int lines = (int)(clamp01(f.blush) * kBlushLines + 0.5f);
  for (const Eye* e : {&l, &r}) {
    const int bx = floorInt(e->cx - e->side * e->rx * 0.55f);
    const int by = floorInt(e->cy + e->ry + 2.0f);
    for (int k = 0; k < lines; k++) {
      const int x = bx + (k - 1) * 3;
      for (int d = 0; d < 3; d++) {
        c.set(x + d, by - d, true);  // "///"
      }
    }
    if (f.glint >= kMinGlint) {
      c.star(floorInt(e->cx - e->side * (e->rx + kGlintOutsetPx)), floorInt(e->cy - e->ry),
             (int)(f.glint - kMinGlint));
    }
  }
}

}  // namespace

bool render(const Face& face, uint8_t* frame, size_t frameBytes) {
  if (frame == nullptr || frameBytes < kFrameBytes) {
    return false;
  }
  memset(frame, 0, kFrameBytes);
  Canvas canvas(frame);

  drawEye(canvas, face.left);
  drawEye(canvas, face.right);
  drawBrow(canvas, face.left);
  drawBrow(canvas, face.right);
  drawMouth(canvas, face.mouth, face.teeth, face.fang);
  drawSymbols(canvas, face);
  for (uint8_t i = 0; i < face.sparkCount && i < kMaxSparks; i++) {
    canvas.star(face.sparks[i].x, face.sparks[i].y, face.sparks[i].arm);
  }
  return true;
}

}  // namespace anime
