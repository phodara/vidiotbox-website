# Project Memory

## Purpose

RobotFace Simulator is a browser-based preview of robot face expressions. It favors a compact, pixel-art style and procedural canvas drawing over external assets.

## Current Shape

- Main file: `index.html`
- Canvas logical size: `240x320`
- Scale: `1.5`
- UI: touch/click buttons arranged around the canvas, plus keyboard shortcuts.
- License text is in `LICENSE`; copyright notice appears under the simulator.

## Implementation Landmarks

- Color constants are RGB565-inspired and converted with `rgb565()`.
- Geometry constants near the top define eye, mouth, hair, ear, and nose placement.
- Offscreen canvases are used for hair, ears, eyes, mouth, and WACKY3D texture rendering.
- `drawFaceToCtx()` is the central full-face renderer.
- `update()` handles blinking, mouth cycling, ear deployment, eye scan, sleeping text, and WACKY3D auto-sweep.

## Expression Checklist

When adding or changing an expression:

1. Update `EXPR` and `EXPR_NAMES`.
2. Add or move a `.touch-point` button and CSS placement class.
3. Add expression-specific drawing branches where needed.
4. Add keyboard mapping if the expression should have a shortcut.
5. Add audio only if a suitable hosted audio file exists.
6. Verify visually in the browser or with a headless Chrome screenshot.

## Recent Work

- Added `SCARY` expression.
- Added `.gitignore` entry for `.DS_Store`.
- `.DS_Store` should stay untracked; it is Finder metadata only.
- Added `docs/USER_GUIDE_PUBLISH_WORKFLOW.md` to document that simulator commits
  sync to `phodara/roboface-simulator`, while the live RobotFace user guide is
  published separately to `phodara/roboface-audio`.
- Clarified that local `index.html` is the simulator app, while the live
  `roboface-audio` GitHub Pages file is `docs/user-guide/index.html` in the
  `phodara/roboface-audio` repo.

## Design Preferences

- Keep the face readable at the small canvas size.
- Use bold silhouettes and high contrast instead of subtle details.
- Prefer animation from existing timing helpers (`millis()`, frame counters, scan arrays) over adding large state machines.
- Keep expression-specific code close to the renderer section it affects.
