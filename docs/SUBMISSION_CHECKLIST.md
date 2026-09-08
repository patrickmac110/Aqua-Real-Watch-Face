# Core Devices Pebble Store submission checklist

This kit is ready for the current Core Devices Pebble developer portal except
for the final account-owner submission actions and physical-hardware verification.

Core Devices Developer Program Policy 4.4 requires Patrick, as the developer
account owner, to perform the final submission rather than an automated tool
submitting on his behalf.

## Before uploading either listing

- [x] Add the publisher support email to both listings and the privacy policy.
- [x] Create public source repositories for both editions.
- [x] Add source and project URLs to both listing sheets.
- [ ] Decide whether the releases are non-commercial and expected to stay
  within Open-Meteo's free API limits. For commercial or larger-scale use,
  configure an eligible paid or self-hosted weather endpoint before publishing.
- [ ] Keep the Open-Meteo attribution in the descriptions/documentation.
- [ ] Test both PBWs on physical Pebble Time 2 hardware.

## Aqua Real watchface 1.7.0

- [ ] Create or open the **Aqua Real** watchface listing.
- [ ] Upload `watchface/Aqua-Real-v1.7.0.pbw` as a new release and paste the
  release notes from `watchface/listing.md`.
- [ ] Publish that release.
- [ ] Create/update the Emery asset collection and paste the description.
- [ ] Upload the five unframed files in `watchface/screenshots/` in filename
  order. The fifth item is an animated GIF.
- [ ] Optionally upload `watchface/marketing-banner-1440x320.png`.
- [ ] Preview, then publish privately for a final install test before making it
  public. A public listing cannot simply be switched back to private.

## Aqua Real Aquarium watch app 1.0.0

- [ ] Create the **Aqua Real Aquarium** watchapp listing.
- [ ] Select **Daily** as the category.
- [ ] Upload `app/icons/icon-large-144.png` and
  `app/icons/icon-small-48.png`.
- [ ] Upload `app/Aqua-Real-Aquarium-v1.0.0.pbw` and paste the release notes
  from `app/listing.md`.
- [ ] Publish that release.
- [ ] Create the Emery asset collection and paste the description.
- [ ] Upload the five unframed files in `app/screenshots/` in filename order.
- [ ] Upload `app/marketing-banner-1440x320.png`.
- [ ] Preview and privately install-test the listing, then publish it publicly.

## Verification already completed

- [x] Both PBWs build for Emery with Pebble SDK 4.33.1.
- [x] Watchface release version is higher than 1.6.0 and retains its UUID.
- [x] Watch app has its own UUID and is declared as a non-watchface app.
- [x] Watchface large smooth-font animation produced 40 unique sampled frames.
- [x] Watchface framebuffer remained pixel-identical after backlight shutoff.
- [x] Watch app animation produced 40 unique sampled frames after launch.
- [x] Watch app omits clock/date/weather badge and battery text, uses the fixed
  40%-equivalent waterline, and retains weather/time-of-day scenery.
- [x] Screenshots are raw 200×228 emulator captures with no decorative frame.
- [x] Store descriptions are below the 1,600-character portal limit.
- [x] SHA-256 checksums are included in `SHA256SUMS.txt`.
