# Haiku-Matic rendering improvements — local review

Status: deployed to DreamHost on 1 November 2026 after explicit user approval ("push it out").

## Findings

The existing production HTML already includes six actual sample poems, all technical
history, instructions, themes, styles and credits. New poems require the model; the
interface and dialog do not. The opening animation's screenshot does not establish
what Google indexes. A closed dialog is not exposed to assistive technologies until
opened, and its old button did not provide a no-JavaScript navigation path.

## Changes

- Added the approved introduction inside About. All earlier About text is preserved.
- About is a standard HTML link to `haiku-matic-about.html`, styled identically to
  the old button. With JavaScript, normal activation still opens the same modal,
  including Enter and Space. Modified clicks retain normal link navigation.
- Corrected poem headings from H3 to H2; the dialog title is H2 and its subsections
  are H3. Existing typography and dimensions are retained. The static poems have
  an accessible region name identifying them as examples.
- The build generates the informational page from the exact dialog content and
  existing styles. No second copy of the technical information is maintained.
- The new page has a unique title, description and self-canonical URL, navigation
  back to the generator, and links to instructions and credits. It is included in
  the proposed DreamHost sitemap because it provides substantial history, usage
  instructions and licensing information in an independently readable document.
- The vidiotbox.net build keeps the fallback page noindexed, consistent with the
  staging homepage. The DreamHost build makes both pages indexable.

Modified implementation files: `generated-haikus.html`, `scripts/build-site.py`,
`scripts/deploy-dreamhost.sh`, `scripts/test-build-site.py`, `scripts/test-haiku.mjs`,
and `scripts/write-sample-haiku.py`. No runtime, model, decoder or grammar files changed.
The informational HTML is generated, rather than checked in as another source file.

## Validation

- Five Python build tests passed, including source-to-fallback content propagation,
  omission of runtime/scripts, and staging versus production indexing behavior.
- `node scripts/test-haiku.mjs` passed: hashes, Free poems in all five themes,
  deterministic parity, six real static examples, and repeat-guard behavior.
- `node scripts/test-haiku-styles.mjs` passed: 100 complete 5-7-5 poems, both
  One-thing and Two-part styles in all five themes, and 26 concepts.
- Chrome generated poems through all 20 theme/style combinations (five themes
  times Free, One-thing, Two-part and Mixed), plus the All themes control.
- Reading view, crawl, pause/resume, speed, stars, music on/off, About opening and
  closing, and About Space/Enter activation were exercised locally.
- Deliberately returning HTTP 503 for model.bin left the static poems readable in
  Reading view and About fully usable.
- A local CSP blocking all scripts and workers verified that the ordinary About
  link navigates to the complete informational page without JavaScript. The page
  itself has zero script elements and no model dependency (16,235 bytes of HTML,
  plus the existing banner/icon images).
- Initial HTML headings have no skipped levels. Internal links, fragment targets,
  unique IDs, canonical metadata, and both sitemap entries were validated.
- Existing homepage metadata and JSON-LD are byte-identical to the prior deployment.
  Google Analytics ID and the `haiku_generated` event function remain unchanged.
- Measured desktop Reading view layout and typography match the baseline exactly.
  At a real 390-pixel frame width, mobile dimensions and styling also match;
  existing toolbar translation differs temporarily with focus/hover state.
- All existing model, worker, image and audio files are byte-identical between
  baseline and proposed production builds. Existing served files that change are
  index.html, generated-haikus.html and sitemap.xml; the About page is newly added.
- The complete follow-up diff was reviewed; shell syntax and `git diff --check` passed.

## Review and remaining limits

Preview: http://127.0.0.1:8093/preview/haiku-matic-about.html
Local scripts-disabled preview: http://127.0.0.1:8093/nojs/
Blocked-model preview: http://127.0.0.1:8093/blocked/
Diff and screenshots: `/tmp/haiku-render-review/`.
The approved production build was published to DreamHost. Git changes remain local and uncommitted.

No additional homepage resource requests were introduced; content generation for
About runs at build time. Detailed performance profiling on physical mobile devices
was not performed. Actual Google indexing and ranking cannot be established locally.

After approval and deployment, inspect both URLs in Search Console. Check the rendered
HTML for the introduction and sample poems, follow the About link, confirm the new
page's self-canonical and indexing permission, and submit the updated sitemap.
A model-download warning may remain; the informational page does not need that file.

## Production verification

Both homepage HTML URLs, the new informational page, robots.txt and sitemap.xml
returned HTTP 200 and matched the approved build byte for byte. Server hashes
confirmed only the two homepage copies and sitemap changed, plus the new About
page; every model, worker, image and audio file remained unchanged. Chrome verified
the model, generated a new poem and opened the updated About dialog.

A rollback backup is available outside the public directory at
`~/haiku-matic-backups/rendering-20261101-DFXknj/`.
The deployment script's IndexNow ping returned HTTP 200. Google Search Console
reinspection and the updated sitemap submission remain the next manual checks.
