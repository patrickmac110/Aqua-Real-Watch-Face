# Aqua Real

A native Pebble Time 2 watchface inspired by the atmosphere of classic 3D
aquarium screensavers. It targets the **Emery** platform and its 200×228,
64-color reflective display. The companion **Aqua Real Aquarium** project is a
normal watch app for opening the same scene only when you want to watch it.

## What it does

- Seven independently moving fish across three perspective depths
- Four lifelike species with a four-step tail cadence in both directions
- Randomized curved routes with complete off-screen entrances and exits,
  changing cruise speeds, brief darts, and occasional on-screen turn-backs
- Continuous body pitch that follows each route's tangent, plus compressed
  perspective pivots, depth scaling, cool distance haze, and back-to-front sort
- Five independently suspended clock glyphs (four digits and the colon), each
  with its own yaw, pitch, roll, bob, extrusion, refraction, and face caustics
- Four clock faces: smooth system-font glass, the original pixel glass, a cut
  nautical stencil, and ornate gold doubloons. Smooth glass exposes all 28
  public Emery fonts that can render a clock, rasterizes only the five current
  glyphs, and then gives those cached masks the same independent 3D motion,
  ripples, extrusion, and face caustics
- The doubloons are dithered from a high-resolution 0–9 art plate, rotate
  independently in three axes, and combine moving metallic glints with the
  tank's refracted caustic field. A 70–180% brightness control lifts both the
  face and shaded depth while retaining engraved contrast and a gold hue
- Battery-controlled water volume, with every fish path, bubble, clock glyph,
  date, and distortion constrained beneath the moving surface
- A red navigation-marker buoy with a weighted percentage body, open lattice,
  signal lamp, and natural top-edge clipping; plus a second marine weather
  float with a condition glyph, temperature, and automatic °F/°C unit
- The full date below the clock, cached weather fallback, and hourly current
  conditions supplied by a keyless phone companion
- A native Pebble configuration entry with independent time (80–220%), date,
  and weather-float sizing; clock face and 12/24-hour controls; bold and seven
  date-layout choices; five float colors; four foreground colors; live
  previews; and settings persisted on both phone and watch
- Procedural above-water scenery for morning, noon, evening, and night. Clear,
  cloud, fog, rain, snow, and storm families each have their own 64-color
  palette and details including sun position, moon and stars, cloud banks,
  fog, precipitation, and occasional lightning
- Eight precomputed phases of two interacting cellular caustic fields, drifting
  fractal breakup, photographed relief projection, and a hard hand-traced floor
  horizon that prevents light from leaking into open water. The phases are
  streamed from a packed 4-bit resource so only two 1.4 KB interpolation frames
  occupy RAM
- Three scenery-matched plant clusters rooted at the photographed left,
  middle-left, and right floor positions, built from individually textured and
  dithered strands with veins, folds, irregular roots, and gravel integration
- A 40-frame seamless plant loop driven by upward-scrolling, multi-octave
  turbulent displacement. Frames are stored as a custom 3-bit indexed stream,
  so the watch decodes and blits one 4.5 KB frame instead of calculating curves
- Perceptual 64-color rendering using Oklab palette matching, serpentine Stucki
  error diffusion, blue-noise decorrelation, and spatially dithered sprite alpha;
  the exposed sky uses solid native-color fields with noise-softened transitions
  rather than a repeating ordered-dot overlay
- A fixed 10 FPS simulation only while the real backlight is illuminated;
  motion freezes completely when the light turns off
- Automatic Pebble 12/24-hour preference support, with optional explicit
  12-hour and 24-hour overrides

## Install the ready-made watchface

The standalone `Aqua-Real-v1.7.0.pbw` is the installable watchface. Open it on
the phone that has the Pebble app, or install through the current SDK:

```sh
pebble install Aqua-Real-v1.7.0.pbw --cloudpebble
```

After installation, open Aqua Real from the Pebble app and tap its settings
control. The configuration page is bundled into the phone companion, so it
does not depend on a separately hosted website.

## Build from source

Install the current Pebble SDK, then from this project directory run:

```sh
pebble sdk install latest
pebble build
```

The compiled bundle will be written to `build/aquarium.pbw`.

The separate foreground-only aquarium is in the sibling `aquarium-viewer/`
project. Build it from that directory with the same command; its bundle is
written to `aquarium-viewer/build/aquarium-viewer.pbw`. It shares the renderer
and prepared artwork with this project, so keep both sibling directories
together when building the complete source release.

To regenerate the palette-processed art, first install the two asset-pipeline
dependencies and run:

```sh
python3 -m pip install -r requirements-assets.txt
python3 tools/generate_assets.py
```

The high-resolution source plates are in `source_art/`; Pebble-ready output is
written to `resources/images/`.

## Water, weather, and battery behavior

At 100% battery the surface sits near the tank lid. As charge falls, the
surface descends through a 52-pixel range. Existing Bézier paths are remapped
into the new water volume rather than restarted, and sprite pixels are clipped
against the live wave contour. The clock and date settle lower while retaining
readable clearance from the bottom. The battery and weather floats remain
attached to their own sampled points on the moving crest.

The bundled PebbleKit JavaScript companion asks the phone for location and
queries Open-Meteo's current temperature, WMO weather code, day/night flag,
sunrise, and sunset once per hour. Sunrise and sunset divide daylight into
morning, noon, and evening; nighttime is taken directly from the current
forecast. It chooses Fahrenheit for locales that conventionally use it and
Celsius elsewhere. The last good result is cached on both phone and watch, so
the float and sky survive a temporary phone, location, or network outage.
Location coordinates are used for the forecast request only; they are not sent
to the watch or stored by this project.

## Settings

- Time style: smooth glass, original pixel glass, nautical stencil, or gold
  doubloons
- Smooth-glass font: all 28 clock-capable Emery system fonts, grouped as
  Gothic, Bitham, Roboto, Droid Serif, and Leco
- Floating time: 80–220% in 5% steps, automatically fitted to the current
  font, 12/24-hour layout, water volume, date, and tank width
- Doubloon brightness: 70–180% in 5% steps
- Time format: follow the Pebble preference, force 12-hour, or force 24-hour
- Date layout: full, long, short, month and day, weekday, numeric, or hidden
- Date: 14, 18, 24, 28, or 42, with optional bold weight and automatic fitting
- Temperature: small (14), medium (18), or large (24)
- Weather float: deep navy, black, deep teal, pearl, or burgundy
- Weather text and icon: white, yellow, black, or ice blue

The weather text and icon also receive an opposing one-pixel edge so unusual
foreground/background combinations remain identifiable on the reflective LCD.

## Motion and power behavior

Continuous animation is expensive on any always-on watchface. Aqua Real uses
Emery's real backlight transition service: the simulation targets a consistent
10 FPS while the display is illuminated, measures real elapsed time so motion
does not slow when a frame is late, and cancels its timer as soon as the light
is fully off. The exact scene state is frozen rather than advanced in the
background; minute changes still update the static time.

The release QA run captures 40 distinct frames over the complete four-second
plant loop at 100 ms intervals and verifies that two post-backlight screenshots
are pixel-identical.

The v1.7.0 Emery build uses about 222 KB of its 256 KB resource budget, has a
41.1 KB static RAM footprint, and retains roughly 90 KB of reported heap
headroom before runtime bitmap loading. The large plant animation stays in
resource flash; only one packed plant frame is buffered. The caustics likewise
remain in flash with two compact phase buffers, avoiding the 22.8 KB expanded
bitmap allocation that could fail after the background and fish loaded. The
doubloon atlas is a 5.3 KB packed four-bit texture with a static warm-metal
palette, so it does not require another runtime bitmap allocation. The three
smooth clock caches each native glyph as compact horizontal row spans only when
the minute, font, or 12/24-hour setting changes. Animation transforms those
spans directly instead of rescanning a full mask for every face and depth pass.
Very large fonts also use a reduced, depth-preserving extrusion pass count,
which removes the previous large-font frame-rate cliff while retaining the 3D
silhouette. The three unique stored fish tail poses still play in a
`neutral → bend → neutral → opposite` sequence.

The Aqua Real Aquarium watch app uses about 216 KB of resource flash and a
20.2 KB static RAM footprint, leaving roughly 111 KB of reported heap. It has
no clock, date, battery number, or weather float; its waterline is fixed to the
watchface's 40% position and its buoy is decorative. It animates only while the
app has screen focus, pausing before notifications cover it and resuming after
it is fully visible.

## Project map

- `src/c/aquarium.c` — renderer, simulation, clock, and backlight-gated scheduler
- `src/pkjs/index.js` — cached phone geolocation and Open-Meteo weather bridge
- `src/pkjs/config.js` — self-contained configuration page, validation, and sync
- `resources/images/` — background, fish sheets, compact caustics, packed
  40-frame plant animation, and the packed doubloon digit atlas
- `source_art/` — high-resolution generated source imagery and doubloon plate
- `tools/generate_assets.py` — reproducible palette/dithering pipeline
- `tools/qa_emulator.py` — Emery emulator install and animated screenshot test
- `qa/aquarium-preview.gif` — captured emulator preview
- `qa/battery-levels.png` — 100%, 55%, and 15% waterline regression strip
- `../aquarium-viewer/` — normal watch-app edition using the shared scene engine

## Weather data and privacy

Weather data is provided by [Open-Meteo](https://open-meteo.com/) under its
published terms and CC BY 4.0 data licence. The phone sends rounded location
coordinates directly to Open-Meteo to request current conditions, sunrise, and
sunset. Coordinates are never sent to the watch or to a developer-operated
server. See the complete release privacy policy before publishing.

## Visual limitation worth knowing

Pebble Time 2 can display 64 colors total. The source art is deliberately
photorealistic, but the final image must still resolve into that fixed palette.
The dithering pipeline preserves perceived gradients, texture, and translucency
instead of pretending the screen can show full RGB photographs.
