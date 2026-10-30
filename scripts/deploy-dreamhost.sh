#!/usr/bin/env bash
# Publish the Generated Haikus page to haiku-matic.com (DreamHost), as the site's home page.
#   scripts/deploy-dreamhost.sh          build and upload
#   DRY=1 scripts/deploy-dreamhost.sh    build and show what would change, upload nothing
# Builds the site the way the Pages workflow does (scripts/build-site.py) and uploads only what
# generated-haikus.html loads: the page (as index.html and generated-haikus.html), the generator and
# style workers with their decoders and model, the music, the icon, and an .htaccess for the .wasm and
# .mjs types. The key is ~/.ssh/dreamhost_ed25519 (user cosmichaiku). The page needs HTTPS: the
# workers check the model's SHA-256 with crypto.subtle.
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

cp "$SITE/generated-haikus.html" "$OUT/index.html"
cp "$SITE/generated-haikus.html" "$OUT/generated-haikus.html"
cp -R "$SITE/haiku-generator" "$SITE/haiku-styles" "$OUT/"
rm -f "$OUT/haiku-generator/index.html"   # the generator page links to the rest of vidiotbox.net
mkdir -p "$OUT/assets/audio" "$OUT/philosophy-assets"
cp "$SITE/assets/audio/shika-no-toone-shakuhachi.mp3" "$OUT/assets/audio/"
cp "$ROOT/philosophy-assets/happy.png" "$OUT/philosophy-assets/"   # build-site.py does not copy it
cat > "$OUT/.htaccess" <<'EOF'
AddType application/wasm .wasm
AddType text/javascript .mjs
# Workers and model-info.json are versioned by the page; let browsers revalidate the rest.
<FilesMatch "\.(html|json)$">
  Header set Cache-Control "no-cache"
</FilesMatch>
EOF

# Web permissions, whatever the local ones are (mktemp makes its folder private; macOS rsync has no --chmod).
find "$OUT" -type d -exec chmod 755 {} + && find "$OUT" -type f -exec chmod 644 {} +
$SSH "$HOST" true || { echo "cannot log in to $HOST with $KEY" >&2; exit 1; }
# --delete only inside the folders this script owns, so other files in $DEST are left alone.
for dir in haiku-generator haiku-styles; do
  $RSYNC ${DRY:+--dry-run} --delete -e "$SSH" "$OUT/$dir/" "$HOST:$DEST/$dir/"
done
$RSYNC ${DRY:+--dry-run} -e "$SSH" --exclude haiku-generator --exclude haiku-styles "$OUT/" "$HOST:$DEST/"
echo "${DRY:+DRY=1: nothing uploaded. }Published to https://${DEST%/}/"
