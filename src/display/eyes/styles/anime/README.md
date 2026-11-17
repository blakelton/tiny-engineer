# Anime eye style

Expressive anime eyes, selected with `eyes_style=anime`. User-facing behavior is documented in [docs/api.md#anime-eyes](../../../../../docs/api.md#anime-eyes).

Anime is a **look** on the expressive face engine in [`../kawaii/`](../kawaii/README.md). Everything that decides *what* the face does is shared with kawaii:
- moods and personalities per animation, with accents while idle or working;
- easing and springs;
- the typing gaze;
- speech and lip sync for the voice clips;
- the bell celebration.

This folder only decides *how it looks*. The firmware driver both looks share is [`../face_player.cpp`](../face_player.cpp); [`../anime.cpp`](../anime.cpp) hands it the anime look.

Everything here is plain C++ with no Arduino dependencies, tested in [`test/test_anime`](../../../../../test/test_anime/test_anime.cpp).

## Files

| File | Owns |
| --- | --- |
| [`presets.h`](presets.h) | `expressionFor`: the anime face for each of the engine's moods, including the brow, teeth, and symbol fields only this look draws |
| [`compose.h`](compose.h) | `composeFace`: turns the shared pose and the eased expression into an anime `Face` (eye geometry, brows, mouth, symbols) |
| [`face.h`](face.h) | `render`: draws a `Face` into the 128×32 1-bit buffer. Pure, no state between calls |

## The look

Manga on a lit screen: ink is lit, paper is dark. The lash line, brows, iris tone, and mouth are lit; the whites of the eyes, highlights, and teeth are dark.

- **Eyes:** an almond whose upper lid peaks toward the nose, with a heavy lash line that thins inward and flicks past the outer corner, and a thin partial lower lid. The lid (`lid`, `tilt`) and cheeks (`happy`) cover the iris rather than shrinking it.
- **Iris:** screentone that darkens toward the top, a solid rim and pupil, one big and one small highlight. Shock shrinks the whole iris (`pupil` below 1).
- **Brows:** thick at the inner end, tapering outward; `browLift`, `browTilt` (angry V or worried), and `browAsym` (one brow up).
- **Mouth:** a line when closed; filled when open, with top teeth as a gap and fangs at `fang`.
- **Symbols:** anger vein, sweat drop, gloom lines, and blush hatching, drawn once their amount passes 0.5.
- **Eye kinds:** `Round` is the normal eye, `Chevron` squeezes into `> <`, `Heart` puts a heart in the iris, and `Cross` is a dizzy spiral.

## Tuning

Every value is a named constant at the top of the file that uses it:
- eye, lash, iris, brow, mouth, and symbol geometry: [`face.cpp`](face.cpp);
- eye, iris, and mouth sizes: [`compose.h`](compose.h);
- each mood's face: [`presets.cpp`](presets.cpp).

Timing and behavior (accents, lip sync, the bell) live in the engine; see [`../kawaii/README.md`](../kawaii/README.md#tuning).

## Frame budget

The same 600 k-instruction frame budget as kawaii applies (see [`../kawaii/README.md`](../kawaii/README.md#frame-budget)). Measured on an emulated C3:
- heart eyes cost about 560 k;
- Excited at the welcome pose about 485 k;
- most faces 330–450 k.

The iris works out its thresholds once per eye and its screentone once per row. Each eye column's opening is stored as whole rows, and the dizzy spiral runs in fixed point (the C3 multiplies integers in hardware but floats in software).

## Adding a mood

Moods belong to the engine. Add one by following [`../kawaii/README.md`](../kawaii/README.md#adding-a-mood), then give it an anime face in `expressionFor` ([`presets.cpp`](presets.cpp)).
