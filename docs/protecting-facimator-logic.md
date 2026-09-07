# Protecting Facimator Logic

## Short Answer

Yes, some Facimator logic can be moved server-side, but anything required to run directly in the browser can always be copied.

If the full face renderer, animation logic, presets, GIF export, and interaction math all live in `facimator.html`, someone can save the page, inspect the JavaScript, and reuse it. Minifying or obfuscating the code can slow casual copying, but it is not real protection.

The best protection is to keep the browser as the interactive UI and move the most valuable logic or outputs behind a server API.

## What Can Be Protected

Good candidates for server-side logic:

- GIF export
- Final downloadable expression JSON
- Firmware-ready expression transforms
- Premium expression packs
- Preset generation
- Advanced animation-frame generation
- Any account, license, or paid feature checks

These are useful because the browser can send a small request like "generate a GIF from these captured expressions" and the server can return the finished file. The user still gets the feature, but the important implementation is not shipped as plain client code.

## What Cannot Really Be Protected

Anything the browser must execute locally is visible to users:

- Canvas drawing code
- UI layout code
- Basic drag controls
- Client-side presets
- Local preview animation
- Any JavaScript included in the page

This does not mean those things are worthless. It just means they should be treated as public client code.

## Recommended Approach

For Facimator, the best first move would be to protect the export path.

Keep this in the browser:

- Live face preview
- Mobile control disk
- Mode buttons
- Basic expression editing
- Fast local feedback while dragging

Move this to the server:

- `Export GIF`
- Final expression package export
- Firmware-ready conversion
- Any future paid/downloadable output

That keeps the site playful and responsive while protecting the part most likely to be copied and reused as a product feature.

## Possible Architecture

The browser would send:

```json
{
  "frames": [
    { "name": "custom", "expression": {} }
  ],
  "repeats": 3,
  "format": "gif"
}
```

The server would return:

- A generated GIF file
- A signed download URL
- Or a JSON export only if the request is allowed

## Other Protection Layers

Additional measures can help, but they should be treated as speed bumps:

- Bundle and minify JavaScript
- Split code into modules instead of one large HTML file
- Light obfuscation for production builds
- Add login or license checks for export features
- Rate-limit API endpoints
- Watermark free exports
- Keep private presets or paid expression packs server-side

## Tradeoffs

Server-side protection adds complexity:

- Hosting cost
- API design
- Slower export requests
- More deployment work
- Need for abuse/rate-limit handling

But it provides much stronger protection than hiding code in the HTML.

## Bottom Line

Do not try to protect every line of Facimator. Let the browser own the fun, immediate editing experience. Protect the output pipeline and any premium logic server-side.

The practical first step is:

**Move `Export GIF` to a server endpoint while keeping the live editor client-side.**
