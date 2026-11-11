# vidiotbox-website

The source of vidiotbox.net: a static site of plain HTML files, with no WordPress and no
local build step. `AGENTS.md` covers only the RobotFace simulator in `index.html`.

## Publishing

- Pushing to `main` publishes vidiotbox.net. `.github/workflows/pages.yml` runs
  `scripts/test-build-site.py` and `scripts/test-haiku.mjs`, then `scripts/build-site.py`,
  which adds `?v=<content hash>` to local asset URLs, and deploys to GitHub Pages. Ask before
  pushing.
- If the deploy step fails with "Fetching artifact metadata failed", it is GitHub's side:
  re-run it with `gh run rerun <id>`.
- Pages load `assets/site-menu.js?v=...`; the unversioned URL can stay cached, and HTML is
  cached for up to 10 minutes (`docs/CACHE_VERSIONING.md`).
- haiku-matic.com is separate: DreamHost, updated only by `scripts/deploy-dreamhost.sh`, which
  builds it from `generated-haikus.html`. haiku-matic.com is the original, and this repo's
  generator page is a staging copy whose canonical points there (`docs/HAIKU-MATIC-SEO.md`).

## Previewing

- Serve the repo, `python3 -m http.server 8765 --bind 127.0.0.1`, and open
  `http://127.0.0.1:8765/haiku-matic.html`. Opening files directly breaks the menu's
  root-relative links and the browser generator.
- A production preview: `python3 scripts/build-site.py --output <new empty folder>`.
- Headless Chrome lays pages out at least 500 px wide; check phone width with a 390 px iframe.

## The menu

`assets/site-menu.js`, list `menuItems`. It has one haiku entry, "hAIku-Matic Generator",
which opens `haiku-matic.html`.

## hAIku-Matic

- hAIku-Matic is a separate project from Vidiotbox. Do not mention Vidiotbox (its hardware,
  case, files or flasher) in hAIku-Matic pages or the manual; only the vidiotbox.net web
  addresses stay. Describe the board as an ESP32-S3 (LCDWIKI ES3C28P).
- `haiku-matic.html` is the hub for the browser generator (`generated-haikus.html` and
  haiku-matic.com) and the pocket robot poet hardware, with links to the manual. The
  introduction and the "A Robot Poet for Your Pocket" text are the owner's own wording; keep it.
- `docs/haiku-matic-instruction-manual.html` is a copy. The master is
  `../../LilyGOTdisplayC5AI/haiku-training/docs/haiku-matic-instruction-manual.html`: edit that
  one and copy it here, so the two stay identical.
- Photos go in `docs/release-assets/haiku-matic/`. Phone photos carry GPS: convert them with
  `cwebp -metadata none` (and check any JPEG for EXIF) before they are committed.
