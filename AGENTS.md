# Agent Instructions

## Project Overview

This repository is a single-file RobotFace simulator. The main application lives in `index.html` and renders a 240x320 pixel-style robot face on a scaled canvas.

There is no package manager or build step. Open `index.html` directly in a browser to run the simulator.

## Local Workflow

- Prefer focused edits to `index.html`; avoid splitting the app unless the user asks for a larger refactor.
- Keep generated and OS-local files out of Git. `.DS_Store` is ignored.
- Use `rg` for searching.
- For browser verification on this machine, Google Chrome is available at `/Applications/Google Chrome.app`.
- `node`, `npm`, `npx`, `deno`, and `bun` were not available when this file was created.

## Expression Architecture

Expressions are registered in two parallel structures:

- `EXPR`, an object mapping expression names to numeric ids.
- `EXPR_NAMES`, an array whose indexes must match those numeric ids.

When adding an expression, update both structures and add a matching touch button with `data-expr`.

Expression-specific drawing usually belongs in:

- `drawHair(mainCtx, expr, now)`
- `drawEye(mainCtx, rx, ry, expr, open, scan, vscan, isLeft)`
- `drawMouth(mainCtx, expr, amp, opts={})`
- `drawFaceToCtx(ctx, expr, opts={})` for full-face overlays or marks
- `update()` for expression-specific animation state
- the keyboard map near the bottom of `index.html`

Expression audio is optional. If an expression has no entry in `expressionAudio`, it simply plays no audio.

## Current Expression Notes

- `WACKY3D` is special: it renders the WACKY face to an offscreen texture and perspective-maps it onto a rotating plane.
- `LISTENING` is the only looping audio expression and also deploys the ears.
- `SCARY` is a first-class expression with a SCARY touch button and `S` keyboard shortcut. It uses spiked hair, slashed brows, red slit eyes, cheek marks, a jagged mouth, and keeps eyes open in `update()`.

## Verification

Basic visual verification can be done by opening `index.html` or with headless Chrome screenshots, for example:

```sh
/Applications/Google\ Chrome.app/Contents/MacOS/Google\ Chrome \
  --headless=new \
  --disable-gpu \
  --screenshot=/tmp/roboface.png \
  --window-size=520,760 \
  'file:///Users/paulhodara/Documents/PlatformIO/roboface-simulator/index.html'
```

To verify a non-default expression without modifying the repo, create a temporary copy that calls `setExpression(EXPR.NAME)` before `loop()`.

## Longer Memory

See `docs/PROJECT_MEMORY.md` for compact project history and design notes that should carry across future sessions.
