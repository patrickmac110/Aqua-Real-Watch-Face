# Changelog

## Aqua Real 1.7.0 — 2026-09-07

- Reworked native smooth-font rendering around cached per-row spans, avoiding
  repeated full-mask scans during every animation and depth pass.
- Reduced redundant extrusion work at very large time sizes while retaining a
  readable 3D edge.
- Added a separate normal watch app, Aqua Real Aquarium 1.0.0, for viewing the
  animated tank on demand.
- In the watch app, removed the time, date, battery value, and floating weather
  readout; retained weather/time-of-day scenery, fixed the waterline at the 40%
  position, and kept the buoy as decoration.
- Added focus-aware watch-app scheduling that pauses before the scene is
  covered and resumes only after it is fully visible.
- Added store-ready unframed screenshots, animated GIFs, watch-app icons,
  marketing banners, listing copy, privacy disclosure, checksums, and a
  submission checklist.

## Aqua Real 1.6.0

- Added all compatible Emery system fonts as smooth-glass options.
- Added larger time/date settings and a doubloon brightness control.
- Added smooth-glass and brighter gold rendering refinements.
