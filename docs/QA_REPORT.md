# Aqua Real release QA

Release date: September 7, 2026

Target: Pebble Time 2 (`emery`), Pebble SDK 4.33.1

## Build results

| Bundle | Version | Resource flash | Static RAM | Reported free heap |
| --- | ---: | ---: | ---: | ---: |
| Aqua Real watchface | 1.7.0 | 221,682 B | 41,058 B | 90,014 B |
| Aqua Real Aquarium app | 1.0.0 | 216,362 B | 20,160 B | 110,912 B |

Both builds completed successfully. The linker emits the SDK toolchain's
existing RWX load-segment warning; no compilation or resource-limit error is
present.

## Emulator checks

| Check | Result |
| --- | --- |
| Watchface, 170% Bitham 42 Light smooth clock | 40 distinct frames from 40 samples |
| Complete prepared seaweed loop | Sample window spans all 40 100 ms frames |
| Backlight-off animation gate | Two delayed screenshots were pixel-identical |
| Watch-app foreground animation | 40 distinct frames from 40 samples |
| Watch-app launch/focus lifecycle | Resumes on post-launch `did_focus` event |
| Watch-app obscured lifecycle | Cancels timer on pre-loss `will_focus` event |
| Waterline variants | Watchface captured at 100%, 55%, and 15% |
| Store scene variants | Morning/clear, noon/cloud, evening/rain, night/storm |
| Watch-app overlays | No clock, date, weather float, or battery text |
| Watch-app waterline | Compiled fixed at the watchface's 40% position |

## Large system-font optimization

Smooth native glyphs are still rasterized only when the displayed minute,
font, or time-format changes. Version 1.7.0 additionally converts each glyph
to compact horizontal row spans during that cache refresh. Animated draws now
transform one refracted origin per occupied row and offset its spans with the
glyph's affine transform, instead of rescanning a full mask and recalculating
both endpoints for every run on every face/depth pass.

At 125% and above, redundant extrusion passes are reduced; at 170% and above,
the renderer draws the far depth silhouette and face. The outline remains 3D
while worst-case per-frame work is bounded as users select the largest Emery
fonts and the 220% size setting.

## Remaining release-owner checks

Physical Pebble Time 2 testing is still recommended before public publication,
especially a subjective battery/runtime check and legibility review in outdoor
light. Store account fields, support email, source URL, and weather-service
deployment tier belong to the publisher and cannot be completed in source.
