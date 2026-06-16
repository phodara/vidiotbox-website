# Protect the Vidiotbox flasher with Cloudflare Access

This guide protects the firmware flasher page and firmware binaries on:

- `https://www.vidiotbox.net/flasher.html`
- `https://www.vidiotbox.net/firmware/*`

Cloudflare Access is server-side/edge-side protection. That means visitors must authenticate before Cloudflare serves either the HTML page or the `.bin`/manifest files. This is much stronger than adding a JavaScript password prompt to the page.

## What you need

- Access to the `vidiotbox.net` DNS settings.
- A free Cloudflare account.
- The site continuing to publish from GitHub Pages.
- One or more email addresses that should be allowed to use the flasher.

## Important note

Cloudflare Access is not a shared static password. The easiest free setup is usually email one-time PIN login:

1. Visitor opens the flasher page.
2. Cloudflare asks for their email.
3. If the email is allowed, Cloudflare sends them a one-time code.
4. After login, Cloudflare lets them reach the page and firmware files.

This is usually better than one shared password because you can revoke a single email later.

## Step 1: Add `vidiotbox.net` to Cloudflare

Skip this section if `vidiotbox.net` is already using Cloudflare nameservers.

1. Log in to Cloudflare.
2. Choose **Add a domain** or **Add site**.
3. Enter `vidiotbox.net`.
4. Choose the Free plan.
5. Let Cloudflare scan existing DNS records.
6. Confirm that the existing web records are present. For GitHub Pages, typical records are:
   - `CNAME` for `www` pointing at `phodara.github.io`
   - Apex/root records for `vidiotbox.net`, if you use the bare domain
7. At your domain registrar, replace the current nameservers with the two Cloudflare nameservers shown by Cloudflare.
8. Wait for Cloudflare to show the domain as active.

## Step 2: Make sure the web hostname is proxied

Cloudflare Access only works when HTTP/HTTPS traffic passes through Cloudflare.

1. In Cloudflare, open the `vidiotbox.net` zone.
2. Go to **DNS** > **Records**.
3. Find the record for `www`.
4. Make sure the proxy status is **Proxied** with the orange cloud.
5. If you also serve the bare domain `vidiotbox.net`, make sure its web records are also **Proxied**.

Do not proxy mail records or verification records. Only proxy web-serving `A`, `AAAA`, or `CNAME` records.

## Step 3: Enable Cloudflare Zero Trust

1. Open the Cloudflare dashboard.
2. Go to **Zero Trust**.
3. If prompted, create a team name.
   - Example: `vidiotbox`
   - This creates a login domain like `vidiotbox.cloudflareaccess.com`.
4. Stay on the Free plan unless Cloudflare prompts otherwise.

## Step 4: Enable One-time PIN login

One-time PIN may already be enabled. If not:

1. In Cloudflare Zero Trust, go to **Settings** or **Integrations**.
2. Open **Authentication** or **Identity providers**.
3. Add an identity provider.
4. Choose **One-time PIN**.
5. Save it.

## Step 5: Create the Access application

1. In Cloudflare Zero Trust, go to **Access** > **Applications**.
2. Choose **Add an application**.
3. Choose **Self-hosted**.
4. Name it:
   - `Vidiotbox Firmware Flasher`
5. Configure the public hostname:
   - Subdomain: `www`
   - Domain: `vidiotbox.net`
   - Path: `flasher.html`
6. Add another protected path for firmware files.

If the dashboard allows multiple public hostnames/paths in one application, add:

- `www.vidiotbox.net/flasher.html`
- `www.vidiotbox.net/firmware/*`

If the dashboard only allows one path per application, create two Access applications with the same policy:

- Application 1: `www.vidiotbox.net/flasher.html`
- Application 2: `www.vidiotbox.net/firmware/*`

The `/firmware/*` protection is required. Protecting only `flasher.html` is not enough because the installer reads manifests and binaries directly from `/firmware/`.

## Step 6: Create the allow policy

Create an **Allow** policy for the emails that should use the flasher.

1. In the application policy step, choose **Allow**.
2. Name the policy:
   - `Allowed flasher users`
3. Under **Include**, choose **Emails**.
4. Add the exact email addresses that should be allowed.
   - Example: `you@example.com`
5. Save the policy.

Avoid using **Everyone** or broad email domains unless you really want all of those users to flash firmware.

## Step 7: Save and test

Use a fresh/private browser window so you are not relying on an existing session.

1. Open:
   - `https://www.vidiotbox.net/flasher.html`
2. You should see a Cloudflare Access login screen before the flasher page.
3. Enter an allowed email address.
4. Enter the one-time PIN from email.
5. Confirm the flasher page loads.
6. Open this direct firmware URL:
   - `https://www.vidiotbox.net/firmware/V10_Basel/manifest.json`
7. In the same logged-in browser, it should load.
8. In a different private window, it should ask for Cloudflare Access login.

## Step 8: Verify the installer still works

After login:

1. Use Chrome or Edge on a desktop/laptop.
2. Open `https://www.vidiotbox.net/flasher.html`.
3. Click the install/connect button.
4. Confirm the browser can read:
   - `firmware/V10_Basel/manifest.json`
   - or `firmware/vidiotbox_V1-RF2_Basel/manifest.json`
5. Start a test flash only when you are ready.

If the page loads but flashing fails immediately, the likely cause is that `/firmware/*` is not included in the Access application or policy.

## Optional: Use a dedicated private hostname

If you want the main site to stay simple, create a private hostname just for flashing:

- `flash.vidiotbox.net`

Then protect the whole hostname with Cloudflare Access:

- `flash.vidiotbox.net/*`

This is cleaner than protecting selected paths on `www.vidiotbox.net`, but it requires either a separate published copy of the page or routing that hostname to the same GitHub Pages site.

## Troubleshooting

### The page is not asking for login

- Confirm the DNS record is **Proxied** with the orange cloud.
- Confirm the Access application hostname is exactly `www.vidiotbox.net`.
- Confirm the path is `flasher.html` or `/flasher.html`, depending on what the Cloudflare dashboard expects.
- Test in a private/incognito window.

### The page is protected, but binaries are still public

- Add a second protected path for `www.vidiotbox.net/firmware/*`.
- Test a direct binary or manifest URL in a private/incognito window.

### The login code never arrives

- Make sure the email address is exactly included in the Allow policy.
- Check spam/junk.
- If you use email security scanning, allowlist `noreply@notify.cloudflare.com`.

### GitHub Pages gives a domain or certificate problem after DNS changes

- Confirm the GitHub Pages custom domain is still set to `www.vidiotbox.net` or `vidiotbox.net`, depending on your setup.
- Keep the repo `CNAME` file unchanged unless intentionally moving domains.
- Give DNS and certificate changes time to settle.

## Source references

- Cloudflare Access self-hosted applications: https://developers.cloudflare.com/cloudflare-one/access-controls/applications/http-apps/self-hosted-public-app/
- Cloudflare Access policies: https://developers.cloudflare.com/cloudflare-one/access-controls/policies/
- Cloudflare One-time PIN login: https://developers.cloudflare.com/cloudflare-one/integrations/identity-providers/one-time-pin/
- Cloudflare proxied DNS records: https://developers.cloudflare.com/dns/proxy-status/
