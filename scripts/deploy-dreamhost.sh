#!/usr/bin/env bash
# Publish the Generated Haikus page to haiku-matic.com (DreamHost), as the site's home page.
#   scripts/deploy-dreamhost.sh          build and upload
#   DRY=1 scripts/deploy-dreamhost.sh    build and show what would change, upload nothing
#   LOCAL=DIR scripts/deploy-dreamhost.sh  build into DIR to look at or serve; no login, upload nothing
# Builds the site the way the Pages workflow does (scripts/build-site.py) and uploads only what
# generated-haikus.html loads: the page (as index.html and generated-haikus.html), the generator and
# style workers with their decoders and model, the music, the icon, the About art and link preview
# image, and an .htaccess for the .wasm and .mjs types. The key is ~/.ssh/dreamhost_ed25519 (user
# cosmichaiku). The page needs HTTPS: the workers check the model's SHA-256 with crypto.subtle.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
HOST=${DREAMHOST_HOST:-cosmichaiku@iad1-shared-b7-22.dreamhost.com}
DEST=${DREAMHOST_DIR:-haiku-matic.com}
KEY=${DREAMHOST_KEY:-$HOME/.ssh/dreamhost_ed25519}
SSH="ssh -i $KEY -o IdentitiesOnly=yes"
RSYNC="rsync -rlptvz"

SITE=$(mktemp -d); OUT=$(mktemp -d)
trap 'rm -rf "$SITE" "$OUT"' EXIT
python3 "$ROOT/scripts/build-site.py" --output "$SITE" >/dev/null

# haiku-matic.com puts its own name in the crawl heading instead of the top bar;
# vidiotbox.net keeps "VIDIOTBOX / GENERATED hAIkus" in its top bar.
# SEO metadata is authored in generated-haikus.html and shared by both copies.
# Both copies name https://haiku-matic.com/ as the canonical address: it is the original (2026-11-01). The
# vidiotbox.net copy is only for staging and tells search engines not to index it; haiku-matic.com must not.
python3 - "$SITE/generated-haikus.html" "$OUT/index.html" "$ROOT/scripts/build-site.py" <<'EOF'
import sys
from pathlib import Path
from runpy import run_path
page = open(sys.argv[1]).read()
for old, new in [(' <a id="about" class="about-button" href="haiku-matic-about.html" aria-haspopup="dialog" aria-controls="aboutDialog">About</a>\n', ''),
                 ('<meta name="robots" content="noindex">   <!-- staging copy; deploy-dreamhost.sh removes this for haiku-matic.com -->\n', ''),
                 ('<a class="brand" href="/">VIDIOTBOX / GENERATED hAIkus</a>',   # About first, the bar left-justified
                  '<a id="about" class="about-button" href="haiku-matic-about.html" aria-haspopup="dialog" aria-controls="aboutDialog">About hAIku Toy</a>'),
                 # The page's main heading names it (2026-11-01, for search engines), in the eyebrow's look; the
                 # poem line keeps the big heading's look (.sky-title).
                 ('<h1>A sky<br>without edges</h1>',
                  '<h1 class="eyebrow" style="font-size:18px;letter-spacing:2px;line-height:normal;text-transform:none">hAIku Generator Toy '
                  '<span style="white-space:nowrap">- by Paul Hodara</span></h1>\n'
                  '<p class="sky-title">A sky<br>without edges</p>'),
                 ('content="https://vidiotbox.net/assets/haiku-matic/haiku-matic-og.jpg',
                  'content="https://haiku-matic.com/assets/haiku-matic/haiku-matic-og.jpg')]:
    if page.count(old) != 1:
        sys.exit(f"deploy-dreamhost.sh: expected one {old!r} in generated-haikus.html")
    page = page.replace(old, new)
if 'noindex' in page:
    sys.exit('deploy-dreamhost.sh: haiku-matic.com would tell search engines not to index it')
open(sys.argv[2], 'w').write(page)
render_about = run_path(sys.argv[3])['render_haiku_about']
Path(sys.argv[2]).with_name('haiku-matic-about.html').write_text(render_about(page, staging=False))
EOF
cp "$OUT/index.html" "$OUT/generated-haikus.html"
cp -R "$SITE/haiku-generator" "$SITE/haiku-styles" "$OUT/"
rm -f "$OUT/haiku-generator/index.html"   # the generator page links to the rest of vidiotbox.net
mkdir -p "$OUT/assets/audio" "$OUT/assets/haiku-matic" "$OUT/philosophy-assets"
cp "$SITE/assets/haiku-matic/haiku-matic-banner.webp" "$SITE/assets/haiku-matic/haiku-matic-og.jpg" "$OUT/assets/haiku-matic/"
cp "$SITE/assets/audio/shika-no-toone-shakuhachi.mp3" "$OUT/assets/audio/"
cp "$ROOT/philosophy-assets/happy.png" "$OUT/philosophy-assets/"   # build-site.py does not copy it
cat > "$OUT/.htaccess" <<'EOF'
# One address for search engines: www.haiku-matic.com redirects to haiku-matic.com, keeping the path and query.
RewriteEngine On
RewriteCond %{HTTP_HOST} ^www\.haiku-matic\.com$ [NC]
RewriteRule ^(.*)$ https://haiku-matic.com/$1 [R=301,L]
AddType application/wasm .wasm
AddType text/javascript .mjs
# Workers and model-info.json are versioned by the page; let browsers revalidate the rest.
<FilesMatch "\.(html|json)$">
  Header set Cache-Control "no-cache"
</FilesMatch>
EOF

# Search engines (2026-11-01): crawl everything, with no crawl delay (DreamHost's default robots.txt asked for 10 s,
# which Bing honours), and find the page through the sitemap. Both replace whatever is on the server.
cat > "$OUT/robots.txt" <<'EOF'
User-agent: *
Allow: /

Sitemap: https://haiku-matic.com/sitemap.xml
EOF
cat > "$OUT/sitemap.xml" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">
  <url>
    <loc>https://haiku-matic.com/</loc>
    <lastmod>$(date -u +%Y-%m-%d)</lastmod>
  </url>
  <url>
    <loc>https://haiku-matic.com/haiku-matic-about.html</loc>
    <lastmod>$(date -u +%Y-%m-%d)</lastmod>
  </url>
</urlset>
EOF

# IndexNow (Bing, Yandex, Seznam, Naver; DuckDuckGo, Yahoo and Ecosia use Bing): the key is public by design and
# lives in a file of its own name at the root; after each upload the script tells IndexNow the page changed.
INDEXNOW_KEY=772647d8a8dd7563f85c24af05d633ea
echo "$INDEXNOW_KEY" > "$OUT/$INDEXNOW_KEY.txt"

# Web permissions, whatever the local ones are (mktemp makes its folder private; macOS rsync has no --chmod).
find "$OUT" -type d -exec chmod 755 {} + && find "$OUT" -type f -exec chmod 644 {} +
if [ -n "${LOCAL:-}" ]; then mkdir -p "$LOCAL" && cp -R "$OUT/." "$LOCAL/" && echo "Built into $LOCAL; nothing uploaded"; exit 0; fi
$SSH "$HOST" true || { echo "cannot log in to $HOST with $KEY" >&2; exit 1; }
# --delete only inside the folders this script owns, so other files in $DEST are left alone.
for dir in haiku-generator haiku-styles; do
  $RSYNC ${DRY:+--dry-run} --delete -e "$SSH" "$OUT/$dir/" "$HOST:$DEST/$dir/"
done
$RSYNC ${DRY:+--dry-run} -e "$SSH" --exclude haiku-generator --exclude haiku-styles "$OUT/" "$HOST:$DEST/"
echo "${DRY:+DRY=1: nothing uploaded. }Published to https://${DEST%/}/"
if [ -z "${DRY:-}" ]; then
  status=$(curl -s -o /dev/null -w '%{http_code}' "https://api.indexnow.org/indexnow?url=https://haiku-matic.com/&key=$INDEXNOW_KEY" || true)
  case "$status" in 200|202) echo "IndexNow notified ($status)" ;; *) echo "IndexNow ping returned ${status:-no answer}; the upload itself is fine" ;; esac
fi
