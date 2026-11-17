#pragma once

#include "display/eyes/styles/kawaii/expression.h"

namespace anime {

// The anime look's face for each mood. It uses the shared engine's moods
// and Expression (kawaii/), including the brow, teeth, and symbol fields
// that only this look draws. Less cute, more expressive: brows carry the
// emotion, eyes narrow and sharpen, mouths show teeth and fangs.
kawaii::Expression expressionFor(kawaii::Mood mood);

}  // namespace anime
