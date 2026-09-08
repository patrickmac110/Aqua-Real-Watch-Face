# Core Devices Pebble Store handoff

The code, packages, screenshots, icons, descriptions, privacy policy, and
release notes are complete. Core Devices Developer Program Policy 4.4 requires
Patrick, as the developer-account owner, to perform the actual submissions.

## Important: do not use `pebble publish` for the private-first upload

As of Pebble Tool 5.0.40, the command-line publisher advertises an unpublished
default, but its new-app and release upload requests currently send
`isPublished=true`. Use the web dashboard for these first uploads so neither
listing accidentally becomes public.

## 1. Sign in

1. Open https://developer.repebble.com/dashboard.
2. Choose **Sign in with GitHub** and use the GitHub account that owns the two
   source repositories.
3. If this is the first sign-in, review and accept the developer-account terms.

## 2. Create the private Aqua Real watchface listing

Use `watchface/listing.md` as the copy sheet.

- Product type: **Watchface**
- Name: **Aqua Real**
- Version: **1.7.0**
- UUID: `2f8c23b6-6f2e-4a93-9b1f-4d4f2eac0817`
- Platform: **Emery / Pebble Time 2**
- Source: https://github.com/patrickmac110/Aqua-Real-Watch-Face
- Support: `patrickmac110@gmail.com`
- Binary: `watchface/Aqua-Real-v1.7.0.pbw`
- Screenshots: upload the five files in `watchface/screenshots/` in filename
  order, including the animated GIF.

Keep both the product/listing visibility and the release publication setting
off until the private install test is complete. If the dashboard distinguishes
between saving a draft and publishing a release, save the draft only.

## 3. Create the private Aqua Real Aquarium watch-app listing

Use `app/listing.md` as the copy sheet.

- Product type: **Watchapp**
- Category: **Daily**
- Name: **Aqua Real Aquarium**
- Version: **1.0.0**
- UUID: `014114c1-0071-4a5a-b265-95ba3ebe738f`
- Platform: **Emery / Pebble Time 2**
- Source: https://github.com/patrickmac110/Aqua-Real-Watch-App
- Support: `patrickmac110@gmail.com`
- Binary: `app/Aqua-Real-Aquarium-v1.0.0.pbw`
- Icons: `app/icons/icon-small-48.png` and `icon-large-144.png`
- Screenshots: upload the five files in `app/screenshots/` in filename order.

Again, save privately/unpublished for the first hardware install test.

## 4. Before making either listing public

- Install both private builds on a physical Pebble Time 2.
- Confirm the watchface freezes immediately when the backlight turns off.
- Confirm the watch app pauses under notifications and after it loses focus.
- Confirm location permission, weather refresh, settings, 12/24-hour behavior,
  very large time fonts, and the 40%-equivalent app waterline.
- Publish as non-commercial while using Open-Meteo's free endpoint, retain the
  Open-Meteo attribution, or configure an eligible paid/self-hosted endpoint.

