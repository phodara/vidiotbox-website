# Haiku-Matic SEO audit and deployment

Reviewed and deployed 1 November 2026.

## Authoritative source and deployment

The production source is `generated-haikus.html` in this repository, not the
`generated-haikus.html` in haiku-training and not this repository's `index.html`.
The latter is the separate Vidiotbox RobotFace homepage; it was left unchanged.

`scripts/build-site.py` copies the static site and adds content hashes to asset URLs.
`scripts/deploy-dreamhost.sh` builds it, transforms `generated-haikus.html` into the
DreamHost homepage, and publishes identical `index.html` and `generated-haikus.html`.
It also generates the production `.htaccess`, `robots.txt`, and `sitemap.xml`.
The root robots and sitemap files in this repository belong to vidiotbox.net.
The GitHub Pages workflow publishes vidiotbox.net; it does not deploy DreamHost.

Before editing, a local production build matched every managed file on DreamHost,
including the model, workers, images, music, and server configuration. Two additional
server favicon files were preserved. Existing unrelated local work was left alone.

## Changes

- Title: `Free AI Haiku Generator | Haiku-Matic by Paul Hodara`
- Description: `Generate original 5-7-5 haiku with Haiku-Matic, a free AI haiku generator powered by a tiny neural network running in your browser. No sign-up required.`
- Open Graph title and description match the new metadata.
- WebApplication JSON-LD description matches the new description. Its existing
  author, free Offer, browser operation, JavaScript/WebAssembly requirements,
  application category, and language remain accurate and unchanged.
- The title now comes directly from the source page; the deployment script no
  longer replaces it with the older title.

The existing social image, branding, stylesheet, body HTML, About content, analytics
ID G-845EZ02BY8, JavaScript, model and binary assets are unchanged. No additional
structured-data claims or invented reviews/ratings were added. Software-app rich
result eligibility was not established by this audit.

## Crawlable HTML and semantics

The production homepage has one H1 identifying the generator and author, a main
landmark, six complete static sample poems in article elements, and live poems
added by JavaScript. Metadata, canonical URL, JSON-LD, sample poems, instructions,
credits and technical history are present in the initial HTML response.

The About content is inside a closed dialog. It is present in source but not
visible until opened, so its presence in HTML alone does not establish how Google
weights or indexes it. After rendering, JavaScript eventually removes the static
samples as live poems replace them; nondefault theme/style links remove them too.
The existing poem labels are H3 headings without an intervening H2 in main.
These are audit observations; the body and interaction design were preserved
exactly, with no hidden keyword blocks or crawler-specific content added.

Google's rendered view still needs to be assessed in Search Console URL Inspection.
See https://developers.google.com/search/docs/crawling-indexing/javascript/javascript-seo-basics.

## Canonicalization and indexing checks

- Homepage returns HTTP 200; no robots noindex or X-Robots-Tag was found there.
- One canonical link and one og:url both identify https://haiku-matic.com/.
- No duplicate title, description, OG title/description or WebApplication blocks.
- HTTP and HTTPS www homepage variants redirect with 301 to the HTTPS bare domain.
- /index.html and /generated-haikus.html return 200 with the same homepage canonical.
  These are intentional duplicate entry points; no redirect was added because the
  existing navigation and behavior were to be preserved.
- Theme/style query URLs return 200 and use the same homepage canonical.
- An unknown path returns a genuine 404.
- Production robots.txt allows crawling and references the production sitemap.
- Production sitemap has only the canonical homepage.
- .htaccess agrees with the build, redirects www, and sets WASM/module MIME types.
- All local links and static page asset references resolved and returned HTTP 200.
- The vidiotbox.net source keeps its deliberate noindex; the DreamHost build removes
  it. This prevents accidentally treating the staging instruction as production SEO.

Search Console indexing status and Google's selected canonical are user-reported;
this audit did not independently access the account or request Google reindexing.

## Verification and rollback

The four existing asset-build tests passed. Metadata, parsed JSON-LD, sitemap,
local link targets, production asset responses and heading counts were checked.
Production build comparisons proved all CSS, body HTML, inline executable scripts,
About text, poems, generation code and assets identical before and after.

After deployment, both public HTML URLs matched the verified build. Server SHA-256
comparison confirmed that only index.html and generated-haikus.html changed; every
other live file, including the extra favicons, remained identical. Chrome displayed
the new tab title and reported: "Model verified. Writing haiku on your device…".
The existing deployment script's IndexNow notification returned HTTP 200. IndexNow
is not a Google indexing submission.

The previous HTML, .htaccess, robots.txt and sitemap.xml were backed up on DreamHost
outside the public directory at `~/haiku-matic-backups/seo-20261101-1652/`.
To restore, copy those files back into `~/haiku-matic.com/`. Revert the corresponding
local source edits as well before any later deployment.

Next: inspect https://haiku-matic.com/ in Google Search Console, test the live URL,
and request indexing. Monitor impressions, queries, clicks and click-through rate;
metadata updates do not guarantee higher rankings or Google's exact displayed title.
