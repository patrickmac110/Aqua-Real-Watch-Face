#include <pebble.h>
#include <string.h>

#ifndef AQUA_VIEWER_APP
#define AQUA_VIEWER_APP 0
#endif

extern uint32_t MESSAGE_KEY_WEATHER_TEMP;
extern uint32_t MESSAGE_KEY_WEATHER_CODE;
extern uint32_t MESSAGE_KEY_WEATHER_IS_DAY;
extern uint32_t MESSAGE_KEY_WEATHER_UNIT;
extern uint32_t MESSAGE_KEY_WEATHER_SKY_PHASE;
extern uint32_t MESSAGE_KEY_SETTINGS_TIME_SCALE;
extern uint32_t MESSAGE_KEY_SETTINGS_DATE_SIZE;
extern uint32_t MESSAGE_KEY_SETTINGS_WEATHER_SIZE;
extern uint32_t MESSAGE_KEY_SETTINGS_WEATHER_BG;
extern uint32_t MESSAGE_KEY_SETTINGS_WEATHER_FG;
extern uint32_t MESSAGE_KEY_SETTINGS_DATE_BOLD;
extern uint32_t MESSAGE_KEY_SETTINGS_DATE_FORMAT;
extern uint32_t MESSAGE_KEY_SETTINGS_TIME_STYLE;
extern uint32_t MESSAGE_KEY_SETTINGS_TIME_FORMAT;
extern uint32_t MESSAGE_KEY_SETTINGS_TIME_FONT;
extern uint32_t MESSAGE_KEY_SETTINGS_GOLD_BRIGHTNESS;

// Aqua Real is intentionally Emery-only. Its 200x228 framebuffer and 64-color
// display let this watchface combine photo-derived, dithered sprites with a
// compact fixed-point scene renderer.

#define DISPLAY_W 200
#define DISPLAY_H 228
#define FISH_COUNT 7
#define BUBBLE_COUNT 8
#define PLANT_COUNT 3
#define PLANT_FRAME_COUNT 40
#define PLANT_LEFT_PIXELS (56 * 80)
#define PLANT_MIDDLE_PIXELS (54 * 70)
#define PLANT_RIGHT_PIXELS (56 * 66)
#define PLANT_FRAME_PIXELS (PLANT_LEFT_PIXELS + PLANT_MIDDLE_PIXELS \
                            + PLANT_RIGHT_PIXELS)
#define PLANT_FRAME_BYTES ((PLANT_FRAME_PIXELS * 3 + 7) / 8)
#define CAUSTIC_FRAME_COUNT 8
#define CAUSTIC_FRAME_W 50
#define CAUSTIC_FRAME_H 57
#define CAUSTIC_FRAME_PIXELS (CAUSTIC_FRAME_W * CAUSTIC_FRAME_H)
#define CAUSTIC_PACKED_BYTES ((CAUSTIC_FRAME_PIXELS + 1) / 2)
#define ANIMATION_FRAME_MS 100
#define BEZIER_ONE 4096
#define CLOCK_SCALE 4
#define DOUBLOON_FRAME_W 26
#define DOUBLOON_FRAME_H 38
#define DOUBLOON_COLON_X (DOUBLOON_FRAME_W * 5)
#define DOUBLOON_COLON_W 10
#define DOUBLOON_SHEET_W 140
#define DOUBLOON_SHEET_H 76
#define DOUBLOON_PACKED_BYTES ((DOUBLOON_SHEET_W * DOUBLOON_SHEET_H) / 2)
#define SMOOTH_GLYPH_COUNT 5
#define SMOOTH_MASK_W 64
#define SMOOTH_MASK_H 72
#define SMOOTH_MAX_RUNS_PER_ROW 8
#define VIEWER_WATER_PERCENT 40
#define WATER_MIN_Y 6
#define WATER_MAX_Y 58
#define FLOOR_Y 173
#define PERSIST_WEATHER_TEMP 40
#define PERSIST_WEATHER_CODE 41
#define PERSIST_WEATHER_DAY 42
#define PERSIST_WEATHER_UNIT 43
#define PERSIST_WEATHER_SKY_PHASE 44
#define PERSIST_TIME_SCALE 50
#define PERSIST_DATE_SIZE 51
#define PERSIST_WEATHER_SIZE 52
#define PERSIST_WEATHER_BG 53
#define PERSIST_WEATHER_FG 54
#define PERSIST_DATE_BOLD 55
#define PERSIST_DATE_FORMAT 56
#define PERSIST_TIME_STYLE 57
#define PERSIST_TIME_FORMAT 58
#define PERSIST_TIME_FONT 59
#define PERSIST_GOLD_BRIGHTNESS 60

#define COLOR8(red, green, blue) \
  (0xC0 | ((red) << 4) | ((green) << 2) | (blue))

typedef enum {
  SPECIES_BUTTERFLY = 0,
  SPECIES_GOURAMI,
  SPECIES_CLOWN,
  SPECIES_TETRA,
  SPECIES_COUNT
} Species;

typedef enum {
  SKY_MORNING = 0,
  SKY_NOON,
  SKY_EVENING,
  SKY_NIGHT,
  SKY_PHASE_COUNT
} SkyPhase;

typedef enum {
  WEATHER_CLEAR = 0,
  WEATHER_CLOUD,
  WEATHER_FOG,
  WEATHER_RAIN,
  WEATHER_SNOW,
  WEATHER_STORM,
  WEATHER_KIND_COUNT
} WeatherKind;

typedef enum {
  TIME_STYLE_GLASS = 0,
  TIME_STYLE_STENCIL,
  TIME_STYLE_DOUBLOON,
  TIME_STYLE_SMOOTH_GLASS,
  TIME_STYLE_COUNT
} TimeStyle;

typedef enum {
  TIME_FONT_GOTHIC_09 = 0,
  TIME_FONT_GOTHIC_14,
  TIME_FONT_GOTHIC_14_BOLD,
  TIME_FONT_GOTHIC_18,
  TIME_FONT_GOTHIC_18_BOLD,
  TIME_FONT_GOTHIC_24,
  TIME_FONT_GOTHIC_24_BOLD,
  TIME_FONT_GOTHIC_28,
  TIME_FONT_GOTHIC_28_BOLD,
  TIME_FONT_BITHAM_18_LIGHT,
  TIME_FONT_BITHAM_30_BLACK,
  TIME_FONT_BITHAM_34_LIGHT,
  TIME_FONT_BITHAM_34_MEDIUM_NUMBERS,
  TIME_FONT_BITHAM_42_LIGHT,
  TIME_FONT_BITHAM_42_BOLD,
  TIME_FONT_BITHAM_42_MEDIUM_NUMBERS,
  TIME_FONT_ROBOTO_CONDENSED_21,
  TIME_FONT_ROBOTO_BOLD_49,
  TIME_FONT_DROID_SERIF_28_BOLD,
  TIME_FONT_LECO_20_BOLD_NUMBERS,
  TIME_FONT_LECO_26_BOLD_NUMBERS,
  TIME_FONT_LECO_28_LIGHT_NUMBERS,
  TIME_FONT_LECO_32_BOLD_NUMBERS,
  TIME_FONT_LECO_36_BOLD_NUMBERS,
  TIME_FONT_LECO_38_BOLD_NUMBERS,
  TIME_FONT_LECO_42_NUMBERS,
  TIME_FONT_LECO_60_NUMBERS,
  TIME_FONT_LECO_60_BOLD_NUMBERS,
  TIME_FONT_COUNT
} TimeFont;

typedef enum {
  TIME_FORMAT_SYSTEM = 0,
  TIME_FORMAT_12_HOUR,
  TIME_FORMAT_24_HOUR,
  TIME_FORMAT_COUNT
} TimeFormat;

typedef enum {
  DATE_FORMAT_FULL = 0,
  DATE_FORMAT_LONG,
  DATE_FORMAT_SHORT,
  DATE_FORMAT_MONTH_DAY,
  DATE_FORMAT_WEEKDAY,
  DATE_FORMAT_NUMERIC,
  DATE_FORMAT_HIDDEN,
  DATE_FORMAT_COUNT
} DateFormat;

typedef struct {
  uint8_t top;
  uint8_t middle;
  uint8_t bottom;
  uint8_t cloud;
  uint8_t shadow;
} SkyStyle;

typedef struct {
  GBitmap *sheet;
  uint16_t resource_id;
  int16_t frame_w;
  int16_t frame_h;
} SpeciesAsset;

typedef struct {
  Species species;
  int16_t p0_x;
  int16_t p0_y;
  int16_t p1_x;
  int16_t p1_y;
  int16_t p2_x;
  int16_t p2_y;
  int16_t p3_x;
  int16_t p3_y;
  int16_t x;
  int16_t y;
  int16_t angle_q8;
  uint16_t scale_q8;
  uint16_t base_scale_q8;
  uint16_t width_scale_q8;
  uint16_t speed_q8;
  uint16_t target_speed_q8;
  uint16_t behavior_ms;
  uint32_t travel_ms;
  uint32_t elapsed_ms;
  uint32_t tail_ms;
  uint16_t wait_ms;
  uint8_t direction;
  uint8_t entry_direction;
  uint8_t depth;
  uint8_t lane;
  uint8_t phase;
  bool return_path;
  bool active;
} Fish;

typedef struct {
  int32_t x_q8;
  int32_t y_q8;
  int16_t rise_q8;
  uint8_t size;
  uint8_t phase;
} Bubble;

typedef struct {
  int16_t frame_w;
  int16_t frame_h;
  int16_t x;
  int16_t y;
  uint16_t pixel_offset;
  bool foreground;
} PlantAsset;

typedef struct {
  int16_t center_x;
  int16_t center_y;
  int16_t x_scale_q8;
  int16_t y_scale_q8;
  int16_t shear_q8;
  int16_t roll_q8;
  int8_t yaw;
  int8_t pitch;
  uint32_t ripple_phase_q8;
} GlyphTransform;

typedef struct {
  uint8_t width;
  uint8_t height;
  uint8_t run_count[SMOOTH_MASK_H];
  uint8_t runs[SMOOTH_MASK_H][SMOOTH_MAX_RUNS_PER_ROW][2];
} SmoothGlyph;

static Window *s_window;
static Layer *s_scene_layer;
static AppTimer *s_animation_timer;
static GBitmap *s_background;
static GBitmap *s_caustic_noise;
static SpeciesAsset s_species[SPECIES_COUNT];
static Fish s_fish[FISH_COUNT];
static Bubble s_bubbles[BUBBLE_COUNT];
static int16_t s_surface_profile[DISPLAY_W];
static uint8_t s_floor_horizon[DISPLAY_W];
static ResHandle s_plant_animation_resource;
static uint8_t s_plant_frame[PLANT_FRAME_BYTES + 1];
static int8_t s_loaded_plant_frame = -1;
static ResHandle s_caustic_frames_resource;
static uint8_t s_caustic_packed[2][CAUSTIC_PACKED_BYTES];
static int8_t s_loaded_caustic_frame[2] = {-1, -1};
static uint8_t s_doubloon_packed[DOUBLOON_PACKED_BYTES];
static bool s_doubloon_loaded;
static SmoothGlyph s_smooth_glyphs[SMOOTH_GLYPH_COUNT];
static bool s_smooth_cache_dirty = true;

static uint32_t s_random_state = 0xA91F47C3;
static uint32_t s_animation_ms;
static int s_hour;
static int s_minute;
static int16_t s_battery_percent = 100;
static int16_t s_water_y = WATER_MIN_Y;
static int16_t s_weather_temp;
static uint8_t s_weather_code;
static uint8_t s_weather_is_day = 1;
static char s_weather_unit = 'C';
static bool s_weather_valid;
static uint8_t s_sky_phase = SKY_NOON;
static uint8_t s_time_scale = 100;
static uint8_t s_time_style = TIME_STYLE_SMOOTH_GLASS;
static uint8_t s_time_format = TIME_FORMAT_SYSTEM;
static uint8_t s_time_font = TIME_FONT_BITHAM_42_LIGHT;
static uint8_t s_gold_brightness = 135;
static uint8_t s_date_size = 14;
static uint8_t s_date_bold;
static uint8_t s_date_format = DATE_FORMAT_FULL;
static uint8_t s_weather_size = 18;
static uint8_t s_weather_background;
static uint8_t s_weather_foreground;
static char s_date_text[32];
static bool s_backlight_on;
#if AQUA_VIEWER_APP
static bool s_app_focused;
#endif
static bool s_frame_clock_valid;
static time_t s_last_frame_seconds;
static uint16_t s_last_frame_milliseconds;

static const int8_t s_wave[32] = {
   0,  3,  6,  9, 11, 13, 15, 16,
  16, 16, 15, 13, 11,  9,  6,  3,
   0, -3, -6, -9,-11,-13,-15,-16,
 -16,-16,-15,-13,-11, -9, -6, -3
};

static const uint8_t s_digits[10][9] = {
  {0x3E, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3E},
  {0x0C, 0x1C, 0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F},
  {0x3E, 0x63, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7F},
  {0x3E, 0x63, 0x03, 0x06, 0x1C, 0x06, 0x03, 0x63, 0x3E},
  {0x06, 0x0E, 0x1E, 0x36, 0x66, 0x7F, 0x06, 0x06, 0x06},
  {0x7F, 0x60, 0x60, 0x7E, 0x03, 0x03, 0x03, 0x63, 0x3E},
  {0x1E, 0x30, 0x60, 0x7E, 0x63, 0x63, 0x63, 0x63, 0x3E},
  {0x7F, 0x03, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x18},
  {0x3E, 0x63, 0x63, 0x63, 0x3E, 0x63, 0x63, 0x63, 0x3E},
  {0x3E, 0x63, 0x63, 0x63, 0x3F, 0x03, 0x06, 0x0C, 0x78}
};

// Broken horizontal strokes and heavier feet give this alternate face the
// utilitarian stencil character of lettering painted on a ship's hull while
// retaining the same inexpensive vector extrusion and independent 3D motion.
static const uint8_t s_stencil_digits[10][9] = {
  {0x36, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x36},
  {0x0C, 0x1C, 0x2C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x36},
  {0x36, 0x63, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x6D},
  {0x36, 0x63, 0x03, 0x06, 0x1A, 0x06, 0x03, 0x63, 0x36},
  {0x06, 0x0E, 0x1E, 0x36, 0x66, 0x6D, 0x06, 0x06, 0x06},
  {0x6D, 0x60, 0x60, 0x6C, 0x03, 0x03, 0x03, 0x63, 0x36},
  {0x1E, 0x30, 0x60, 0x6C, 0x63, 0x63, 0x63, 0x63, 0x36},
  {0x6D, 0x03, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x18},
  {0x36, 0x63, 0x63, 0x63, 0x36, 0x63, 0x63, 0x63, 0x36},
  {0x36, 0x63, 0x63, 0x63, 0x33, 0x03, 0x06, 0x0C, 0x78}
};

static const uint8_t s_doubloon_palette[16] = {
  0,
  COLOR8(1,0,0), COLOR8(1,1,0), COLOR8(2,0,0),
  COLOR8(2,1,0), COLOR8(2,2,0), COLOR8(2,2,1),
  COLOR8(3,1,0), COLOR8(3,2,0), COLOR8(3,2,1),
  COLOR8(3,3,0), COLOR8(3,3,1), COLOR8(3,3,2),
  COLOR8(2,1,1), COLOR8(1,1,1), COLOR8(2,2,2),
};

// Every public Emery system font that can render digits and a colon. Exact
// nominal sizes are retained as separate choices; the clock size control then
// scales the cached glyph masks and automatically fits them to the tank.
static const char *const s_time_font_keys[TIME_FONT_COUNT] = {
  FONT_KEY_GOTHIC_09,
  FONT_KEY_GOTHIC_14,
  FONT_KEY_GOTHIC_14_BOLD,
  FONT_KEY_GOTHIC_18,
  FONT_KEY_GOTHIC_18_BOLD,
  FONT_KEY_GOTHIC_24,
  FONT_KEY_GOTHIC_24_BOLD,
  FONT_KEY_GOTHIC_28,
  FONT_KEY_GOTHIC_28_BOLD,
  FONT_KEY_BITHAM_18_LIGHT_SUBSET,
  FONT_KEY_BITHAM_30_BLACK,
  FONT_KEY_BITHAM_34_LIGHT_SUBSET,
  FONT_KEY_BITHAM_34_MEDIUM_NUMBERS,
  FONT_KEY_BITHAM_42_LIGHT,
  FONT_KEY_BITHAM_42_BOLD,
  FONT_KEY_BITHAM_42_MEDIUM_NUMBERS,
  FONT_KEY_ROBOTO_CONDENSED_21,
  FONT_KEY_ROBOTO_BOLD_SUBSET_49,
  FONT_KEY_DROID_SERIF_28_BOLD,
  FONT_KEY_LECO_20_BOLD_NUMBERS,
  FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM,
  FONT_KEY_LECO_28_LIGHT_NUMBERS,
  FONT_KEY_LECO_32_BOLD_NUMBERS,
  FONT_KEY_LECO_36_BOLD_NUMBERS,
  FONT_KEY_LECO_38_BOLD_NUMBERS,
  FONT_KEY_LECO_42_NUMBERS,
  FONT_KEY_LECO_60_NUMBERS_AM_PM,
  FONT_KEY_LECO_60_BOLD_NUMBERS_AM_PM,
};

static const GPoint s_floor_horizon_points[] = {
  {0, 184}, {12, 183}, {25, 180}, {38, 181}, {52, 179},
  {64, 176}, {75, 171}, {84, 170}, {94, 175}, {103, 179},
  {112, 178}, {122, 172}, {132, 164}, {142, 168}, {152, 175},
  {164, 179}, {177, 181}, {188, 179}, {199, 180},
};

// Every weather family retains the color identity of its solar phase instead
// of replacing the sky with one generic gray overlay. Values are native
// Pebble 2-bit-per-channel colors, so the framebuffer can use them directly.
static const SkyStyle s_sky_styles[SKY_PHASE_COUNT][WEATHER_KIND_COUNT] = {
  [SKY_MORNING] = {
    [WEATHER_CLEAR] = {COLOR8(0,1,2), COLOR8(2,1,2), COLOR8(3,2,1),
                       COLOR8(3,3,3), COLOR8(1,1,2)},
    [WEATHER_CLOUD] = {COLOR8(0,1,2), COLOR8(1,1,2), COLOR8(2,2,2),
                       COLOR8(2,2,2), COLOR8(1,1,1)},
    [WEATHER_FOG]   = {COLOR8(1,1,2), COLOR8(1,2,2), COLOR8(2,2,2),
                       COLOR8(2,2,2), COLOR8(1,1,2)},
    [WEATHER_RAIN]  = {COLOR8(0,0,1), COLOR8(0,1,1), COLOR8(1,1,2),
                       COLOR8(1,1,2), COLOR8(0,0,1)},
    [WEATHER_SNOW]  = {COLOR8(1,1,2), COLOR8(2,2,3), COLOR8(3,3,3),
                       COLOR8(3,3,3), COLOR8(1,2,2)},
    [WEATHER_STORM] = {COLOR8(0,0,1), COLOR8(1,0,1), COLOR8(1,1,1),
                       COLOR8(1,1,1), COLOR8(0,0,0)},
  },
  [SKY_NOON] = {
    [WEATHER_CLEAR] = {COLOR8(0,1,3), COLOR8(0,2,3), COLOR8(2,3,3),
                       COLOR8(3,3,3), COLOR8(1,2,2)},
    [WEATHER_CLOUD] = {COLOR8(0,1,2), COLOR8(1,2,2), COLOR8(2,2,2),
                       COLOR8(3,3,3), COLOR8(1,1,2)},
    [WEATHER_FOG]   = {COLOR8(1,2,2), COLOR8(2,2,2), COLOR8(2,3,3),
                       COLOR8(3,3,3), COLOR8(1,2,2)},
    [WEATHER_RAIN]  = {COLOR8(0,1,1), COLOR8(1,1,2), COLOR8(1,2,2),
                       COLOR8(1,2,2), COLOR8(0,1,1)},
    [WEATHER_SNOW]  = {COLOR8(1,2,3), COLOR8(2,3,3), COLOR8(3,3,3),
                       COLOR8(3,3,3), COLOR8(1,2,3)},
    [WEATHER_STORM] = {COLOR8(0,0,1), COLOR8(0,1,1), COLOR8(1,0,1),
                       COLOR8(1,1,1), COLOR8(0,0,0)},
  },
  [SKY_EVENING] = {
    [WEATHER_CLEAR] = {COLOR8(1,0,2), COLOR8(3,0,1), COLOR8(3,2,0),
                       COLOR8(3,2,2), COLOR8(1,0,1)},
    [WEATHER_CLOUD] = {COLOR8(1,0,2), COLOR8(2,1,2), COLOR8(2,1,1),
                       COLOR8(2,1,2), COLOR8(1,0,1)},
    [WEATHER_FOG]   = {COLOR8(1,1,2), COLOR8(2,1,2), COLOR8(2,2,2),
                       COLOR8(2,2,2), COLOR8(1,1,1)},
    [WEATHER_RAIN]  = {COLOR8(0,0,1), COLOR8(1,0,2), COLOR8(1,1,1),
                       COLOR8(1,1,2), COLOR8(0,0,1)},
    [WEATHER_SNOW]  = {COLOR8(1,1,2), COLOR8(2,2,3), COLOR8(3,2,2),
                       COLOR8(3,3,3), COLOR8(1,1,2)},
    [WEATHER_STORM] = {COLOR8(0,0,1), COLOR8(1,0,1), COLOR8(1,0,2),
                       COLOR8(1,1,1), COLOR8(0,0,0)},
  },
  [SKY_NIGHT] = {
    [WEATHER_CLEAR] = {COLOR8(0,0,0), COLOR8(0,0,1), COLOR8(0,1,1),
                       COLOR8(1,1,2), COLOR8(0,0,1)},
    [WEATHER_CLOUD] = {COLOR8(0,0,0), COLOR8(0,0,1), COLOR8(1,1,1),
                       COLOR8(1,1,2), COLOR8(0,0,1)},
    [WEATHER_FOG]   = {COLOR8(0,0,1), COLOR8(1,1,1), COLOR8(1,1,2),
                       COLOR8(1,1,2), COLOR8(0,0,1)},
    [WEATHER_RAIN]  = {COLOR8(0,0,0), COLOR8(0,0,1), COLOR8(0,1,1),
                       COLOR8(0,1,1), COLOR8(0,0,0)},
    [WEATHER_SNOW]  = {COLOR8(0,0,1), COLOR8(1,1,2), COLOR8(1,2,2),
                       COLOR8(2,2,3), COLOR8(0,1,1)},
    [WEATHER_STORM] = {COLOR8(0,0,0), COLOR8(0,0,1), COLOR8(1,0,1),
                       COLOR8(1,1,1), COLOR8(0,0,0)},
  },
};

// Ranked high-frequency noise breaks gradients without recreating the ordered
// green-dot grid that previously appeared above the waterline.
static const uint8_t s_blue_noise8[64] = {
   2, 47,  5, 38, 32, 19, 16, 60,
  53, 52, 55, 20, 39, 37, 27, 22,
   3,  6, 33, 43, 50, 24, 31, 41,
  17, 46,  0, 56,  9, 48, 13, 61,
  25, 36, 59, 35, 42, 12, 10, 21,
  18, 51, 23, 34, 29, 49, 15, 54,
  40, 11, 28, 62, 30, 14, 57,  1,
  45,  4, 58,  7, 26, 63, 44,  8,
};

static const PlantAsset s_plants[PLANT_COUNT] = {
  {.frame_w = 56, .frame_h = 80, .x = 0,   .y = 144,
   .pixel_offset = 0, .foreground = false},
  {.frame_w = 54, .frame_h = 70, .x = 38,  .y = 148,
   .pixel_offset = PLANT_LEFT_PIXELS, .foreground = true},
  {.frame_w = 56, .frame_h = 66, .x = 144, .y = 159,
   .pixel_offset = PLANT_LEFT_PIXELS + PLANT_MIDDLE_PIXELS,
   .foreground = true},
};

static uint32_t prv_random(void) {
  s_random_state = s_random_state * 1664525u + 1013904223u;
  return s_random_state;
}

static int16_t prv_abs_i16(int16_t value) {
  return value < 0 ? -value : value;
}

static int32_t prv_abs_i32(int32_t value) {
  return value < 0 ? -value : value;
}

static int16_t prv_clamp_i16(int16_t value, int16_t low, int16_t high) {
  if (value < low) {
    return low;
  }
  if (value > high) {
    return high;
  }
  return value;
}

static int16_t prv_wave_smooth(uint32_t phase_q8) {
  const uint8_t index = (phase_q8 >> 8) & 31;
  const uint8_t next = (index + 1) & 31;
  const uint8_t fraction = phase_q8 & 255;
  return s_wave[index]
      + (((int16_t)s_wave[next] - s_wave[index]) * fraction) / 256;
}

static int16_t prv_water_y_for_battery(int16_t percent) {
  percent = prv_clamp_i16(percent, 0, 100);
  return WATER_MIN_Y
      + ((100 - percent) * (WATER_MAX_Y - WATER_MIN_Y) + 50) / 100;
}

static uint8_t prv_sky_phase_for_local_hour(int hour) {
  if (hour < 6 || hour >= 21) {
    return SKY_NIGHT;
  }
  if (hour < 10) {
    return SKY_MORNING;
  }
  if (hour < 17) {
    return SKY_NOON;
  }
  return SKY_EVENING;
}

static int16_t prv_surface_y_formula(int16_t x) {
  const uint32_t phase_q8 = (s_animation_ms * 256) / 310;
  const int16_t long_wave = prv_wave_smooth(
      phase_q8 + ((uint32_t)(x + 16) * 256) / 13) / 3;
  const int16_t cross_wave = prv_wave_smooth(
      ((uint32_t)(x + 24) * 256) / 7 - phase_q8 / 2) / 8;
  return s_water_y + long_wave + cross_wave;
}

static void prv_update_surface_profile(void) {
  for (int16_t x = 0; x < DISPLAY_W; ++x) {
    s_surface_profile[x] = prv_surface_y_formula(x);
  }
}

static int16_t prv_surface_y_at(int16_t x) {
  return s_surface_profile[prv_clamp_i16(x, 0, DISPLAY_W - 1)];
}

static void prv_init_floor_horizon(void) {
  const uint8_t count = ARRAY_LENGTH(s_floor_horizon_points);
  for (uint8_t point = 0; point < count - 1; ++point) {
    const GPoint start = s_floor_horizon_points[point];
    const GPoint end = s_floor_horizon_points[point + 1];
    const int16_t span = end.x - start.x;
    for (int16_t x = start.x; x <= end.x; ++x) {
      s_floor_horizon[x] = start.y
          + ((int32_t)(end.y - start.y) * (x - start.x)) / span;
    }
  }
}

static int16_t prv_lane_y(uint8_t lane) {
  const int16_t top = s_water_y + 18;
  const int16_t bottom = FLOOR_Y - 3;
  return top + ((int32_t)(bottom - top) * lane) / 6;
}

static uint8_t prv_map_value(const GBitmap *map, int16_t x, int16_t y,
                             bool wrap) {
  if (!map || gbitmap_get_format(map) != GBitmapFormat8Bit) {
    return 0;
  }
  const GRect bounds = gbitmap_get_bounds(map);
  if (wrap) {
    x %= bounds.size.w;
    y %= bounds.size.h;
    if (x < 0) {
      x += bounds.size.w;
    }
    if (y < 0) {
      y += bounds.size.h;
    }
  } else if (x < 0 || y < 0 || x >= bounds.size.w || y >= bounds.size.h) {
    return 0;
  }
  const uint8_t *data = gbitmap_get_data((GBitmap *)map);
  const uint16_t stride = gbitmap_get_bytes_per_row((GBitmap *)map);
  return data[y * stride + x] & 0x3f;
}

static void prv_update_date_text(const struct tm *time_value) {
  const char *format = "%A, %b %e";
  switch (s_date_format) {
    case DATE_FORMAT_LONG:
      format = "%A, %B %e";
      break;
    case DATE_FORMAT_SHORT:
      format = "%a, %b %e";
      break;
    case DATE_FORMAT_MONTH_DAY:
      format = "%B %e";
      break;
    case DATE_FORMAT_WEEKDAY:
      format = "%A";
      break;
    case DATE_FORMAT_NUMERIC:
      format = "%m/%d/%Y";
      break;
    case DATE_FORMAT_HIDDEN:
      s_date_text[0] = '\0';
      return;
    default:
      break;
  }
  strftime(s_date_text, sizeof(s_date_text), format, time_value);
}

static void prv_set_clock(const struct tm *time_value) {
  if (s_hour != time_value->tm_hour || s_minute != time_value->tm_min) {
    s_smooth_cache_dirty = true;
  }
  s_hour = time_value->tm_hour;
  s_minute = time_value->tm_min;
  prv_update_date_text(time_value);
}

static bool prv_uses_24_hour_time(void) {
  if (s_time_format == TIME_FORMAT_12_HOUR) {
    return false;
  }
  if (s_time_format == TIME_FORMAT_24_HOUR) {
    return true;
  }
  return clock_is_24h_style();
}

static void prv_load_species(Species species, uint16_t resource_id,
                             int16_t frame_w, int16_t frame_h) {
  SpeciesAsset *asset = &s_species[species];
  asset->resource_id = resource_id;
  asset->frame_w = frame_w;
  asset->frame_h = frame_h;
  asset->sheet = gbitmap_create_with_resource(resource_id);
  if (!asset->sheet) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to load fish sheet %d", species);
  }
}

static void prv_destroy_species(void) {
  for (int species = 0; species < SPECIES_COUNT; ++species) {
    if (s_species[species].sheet) {
      gbitmap_destroy(s_species[species].sheet);
      s_species[species].sheet = NULL;
    }
  }
}

static int16_t prv_lerp_i16(int16_t a, int16_t b, uint16_t t_q12) {
  return a + (((int32_t)(b - a) * t_q12) >> 12);
}

static int16_t prv_bezier_component(int16_t p0, int16_t p1,
                                    int16_t p2, int16_t p3,
                                    uint16_t t_q12) {
  const int16_t a = prv_lerp_i16(p0, p1, t_q12);
  const int16_t b = prv_lerp_i16(p1, p2, t_q12);
  const int16_t c = prv_lerp_i16(p2, p3, t_q12);
  const int16_t d = prv_lerp_i16(a, b, t_q12);
  const int16_t e = prv_lerp_i16(b, c, t_q12);
  return prv_lerp_i16(d, e, t_q12);
}

static void prv_path_point(const Fish *fish, uint16_t t_q12,
                           int16_t *x, int16_t *y) {
  if (fish->return_path) {
    // A half-sine enters the tank, eases to zero horizontal velocity, then
    // returns to the same edge. p1_x stores the randomized turnaround depth.
    const int32_t angle = ((int32_t)t_q12 * (TRIG_MAX_ANGLE / 2)) / BEZIER_ONE;
    *x = fish->p0_x + ((int32_t)(fish->p1_x - fish->p0_x)
        * sin_lookup(angle)) / TRIG_MAX_RATIO;
  } else {
    *x = prv_bezier_component(fish->p0_x, fish->p1_x,
                              fish->p2_x, fish->p3_x, t_q12);
  }
  *y = prv_bezier_component(fish->p0_y, fish->p1_y,
                            fish->p2_y, fish->p3_y, t_q12);
}

static int16_t prv_path_pitch_degrees(const Fish *fish, uint16_t t_q12) {
  const uint16_t t_before = t_q12 > 72 ? t_q12 - 72 : 0;
  const uint16_t t_after = t_q12 < BEZIER_ONE - 72 ? t_q12 + 72 : BEZIER_ONE;
  int16_t x0, y0, x1, y1;
  prv_path_point(fish, t_before, &x0, &y0);
  prv_path_point(fish, t_after, &x1, &y1);

  const int16_t dx = prv_abs_i16(x1 - x0);
  const int16_t dy = y1 - y0;
  int32_t angle = atan2_lookup(dy, dx ? dx : 1);
  if (angle > TRIG_MAX_ANGLE / 2) {
    angle -= TRIG_MAX_ANGLE;
  }
  int16_t degrees = (int16_t)((angle * 360) / TRIG_MAX_ANGLE);
  if (!fish->direction) {
    degrees = -degrees;
  }
  return prv_clamp_i16(degrees, -29, 29);
}

static void prv_update_fish_pose(Fish *fish, bool snap_angle) {
  uint16_t t_q12 = fish->travel_ms
      ? (uint16_t)((fish->elapsed_ms * BEZIER_ONE) / fish->travel_ms)
      : 0;
  if (t_q12 > BEZIER_ONE) {
    t_q12 = BEZIER_ONE;
  }

  if (fish->return_path) {
    fish->direction = t_q12 < BEZIER_ONE / 2
        ? fish->entry_direction : !fish->entry_direction;
    const int16_t turn_distance = prv_abs_i16(t_q12 - BEZIER_ONE / 2);
    fish->width_scale_q8 = turn_distance < 420
        ? 88 + ((int32_t)turn_distance * 168) / 420
        : 256;
  } else {
    fish->direction = fish->entry_direction;
    fish->width_scale_q8 = 256;
  }

  prv_path_point(fish, t_q12, &fish->x, &fish->y);
  const int16_t target_angle_q8 = prv_path_pitch_degrees(fish, t_q12) << 8;
  if (snap_angle) {
    fish->angle_q8 = target_angle_q8;
  } else {
    fish->angle_q8 += (target_angle_q8 - fish->angle_q8) / 4;
  }

  const uint8_t swell = fish->depth == 0 ? 6 : (fish->depth == 1 ? 10 : 14);
  const uint32_t depth_phase_q8 = (uint32_t)t_q12 * 2
      + ((uint32_t)fish->phase << 8);
  const int16_t scale = fish->base_scale_q8
      + (prv_wave_smooth(depth_phase_q8) * swell) / 16;
  fish->scale_q8 = prv_clamp_i16(scale, 142, 276);
}

// return_mode: -1 chooses randomly, 0 forces a crossing, 1 forces a turn-back.
static void prv_start_fish_path(Fish *fish, bool keep_entry_direction,
                                int8_t return_mode) {
  if (!keep_entry_direction) {
    fish->entry_direction = prv_random() & 1;
  }
  fish->direction = fish->entry_direction;
  fish->return_path = return_mode < 0
      ? ((prv_random() % 5) == 0) : (return_mode != 0);

  const int8_t lane_change = (int8_t)(prv_random() % 3) - 1;
  const uint8_t next_lane = prv_clamp_i16(fish->lane + lane_change, 0, 6);
  const int16_t swim_top = s_water_y + 16;
  const int16_t start_y = prv_clamp_i16(
      prv_lane_y(fish->lane) + (int16_t)(prv_random() % 15) - 7,
      swim_top, FLOOR_Y - 2);
  const int16_t end_y = prv_clamp_i16(
      prv_lane_y(next_lane) + (int16_t)(prv_random() % 17) - 8,
      swim_top, FLOOR_Y - 2);
  const int16_t bend_1 = (int16_t)(prv_random() % 69) - 34;
  const int16_t bend_2 = (int16_t)(prv_random() % 69) - 34;

  fish->p0_x = fish->entry_direction ? -44 : DISPLAY_W + 44;
  fish->p3_x = fish->return_path
      ? fish->p0_x : (fish->entry_direction ? DISPLAY_W + 44 : -44);
  if (fish->return_path) {
    fish->p1_x = fish->entry_direction
        ? 92 + (prv_random() % 104)
        : 4 + (prv_random() % 104);
    fish->p2_x = fish->p1_x;
  } else if (fish->entry_direction) {
    fish->p1_x = 36;
    fish->p2_x = 164;
  } else {
    fish->p1_x = 164;
    fish->p2_x = 36;
  }
  fish->p0_y = start_y;
  fish->p1_y = prv_clamp_i16(start_y + bend_1, swim_top, FLOOR_Y);
  fish->p2_y = prv_clamp_i16(end_y + bend_2, swim_top, FLOOR_Y);
  fish->p3_y = end_y;
  fish->lane = next_lane;
  fish->travel_ms = (fish->return_path ? 16000 : 19000)
      + (prv_random() % 15000) + (fish->depth == 0 ? 2500 : 0);
  fish->elapsed_ms = 0;
  fish->wait_ms = 0;
  fish->width_scale_q8 = 256;
  fish->active = true;
  prv_update_fish_pose(fish, true);
}

static void prv_init_fish(void) {
  const Species species[FISH_COUNT] = {
    SPECIES_TETRA, SPECIES_GOURAMI, SPECIES_CLOWN,
    SPECIES_BUTTERFLY, SPECIES_TETRA, SPECIES_CLOWN, SPECIES_GOURAMI
  };
  const uint8_t directions[FISH_COUNT] = {1, 0, 1, 0, 0, 1, 0};
  const uint8_t lanes[FISH_COUNT] = {0, 2, 5, 1, 6, 3, 4};
  const uint8_t depths[FISH_COUNT] = {0, 0, 1, 1, 1, 2, 2};
  const uint16_t scales[FISH_COUNT] = {158, 171, 200, 214, 188, 246, 234};
  const uint8_t phases[FISH_COUNT] = {3, 19, 8, 27, 14, 22, 10};
  const uint8_t initial_progress[FISH_COUNT] = {8, 44, 73, 47, 86, 0, 59};

  for (int i = 0; i < FISH_COUNT; ++i) {
    s_fish[i] = (Fish) {
      .species = species[i],
      .entry_direction = directions[i],
      .direction = directions[i],
      .depth = depths[i],
      .lane = lanes[i],
      .base_scale_q8 = scales[i],
      .scale_q8 = scales[i],
      .width_scale_q8 = 256,
      .speed_q8 = 220 + ((i * 29) % 73),
      .target_speed_q8 = 256,
      .behavior_ms = 700 + i * 430,
      .tail_ms = phases[i] * 137,
      .phase = phases[i],
    };
    prv_start_fish_path(&s_fish[i], true, i == 3 ? 1 : 0);
    if (i == 5) {
      s_fish[i].active = false;
      s_fish[i].wait_ms = 3200;
      s_fish[i].x = s_fish[i].p0_x;
    } else {
      s_fish[i].elapsed_ms =
          (s_fish[i].travel_ms * initial_progress[i]) / 100;
      prv_update_fish_pose(&s_fish[i], true);
    }
  }
}

static void prv_choose_swim_speed(Fish *fish) {
  // Most decisions are unhurried changes of pace. Roughly one in eight is a
  // brief dart, with a deliberately shorter behavior window and faster tail.
  if ((prv_random() % 8) == 0) {
    fish->target_speed_q8 = 420 + (prv_random() % 121);
    fish->behavior_ms = 450 + (prv_random() % 751);
  } else {
    fish->target_speed_q8 = 178 + (prv_random() % 171);
    fish->behavior_ms = 1500 + (prv_random() % 4201);
  }
}

static void prv_update_fish(Fish *fish, uint16_t elapsed_ms) {
  if (!fish->active) {
    if (fish->wait_ms > elapsed_ms) {
      fish->wait_ms -= elapsed_ms;
      return;
    }
    prv_start_fish_path(fish, false, -1);
    return;
  }

  if (fish->behavior_ms <= elapsed_ms) {
    prv_choose_swim_speed(fish);
  } else {
    fish->behavior_ms -= elapsed_ms;
  }
  const int16_t speed_difference = fish->target_speed_q8 - fish->speed_q8;
  fish->speed_q8 += speed_difference /
      (fish->target_speed_q8 > 380 ? 3 : 8);
  fish->speed_q8 = prv_clamp_i16(fish->speed_q8, 156, 548);

  uint32_t path_step_ms = ((uint32_t)elapsed_ms * fish->speed_q8) >> 8;
  if (fish->return_path) {
    const uint16_t t_q12 = fish->travel_ms
        ? (uint16_t)((fish->elapsed_ms * BEZIER_ONE) / fish->travel_ms) : 0;
    const int16_t turn_distance = prv_abs_i16(t_q12 - BEZIER_ONE / 2);
    if (turn_distance < 520) {
      const uint16_t turn_speed_q8 = 148
          + ((uint32_t)turn_distance * 108) / 520;
      path_step_ms = (path_step_ms * turn_speed_q8) >> 8;
    }
  }
  fish->elapsed_ms += path_step_ms;
  fish->tail_ms += ((uint32_t)elapsed_ms * fish->speed_q8) >> 8;
  if (fish->elapsed_ms >= fish->travel_ms) {
    fish->active = false;
    fish->wait_ms = 900 + (prv_random() % 6200);
    fish->x = fish->p3_x;
    fish->y = fish->p3_y;
    return;
  }
  prv_update_fish_pose(fish, false);
}

static int16_t prv_remap_water_y(int16_t value, int16_t old_top,
                                 int16_t new_top) {
  const int16_t bottom = FLOOR_Y;
  value = prv_clamp_i16(value, old_top, bottom);
  return new_top + ((int32_t)(value - old_top) * (bottom - new_top))
      / (bottom - old_top);
}

static void prv_remap_fish_for_waterline(int16_t old_water_y,
                                         int16_t new_water_y) {
  const int16_t old_top = old_water_y + 16;
  const int16_t new_top = new_water_y + 16;
  for (int i = 0; i < FISH_COUNT; ++i) {
    Fish *fish = &s_fish[i];
    fish->p0_y = prv_remap_water_y(fish->p0_y, old_top, new_top);
    fish->p1_y = prv_remap_water_y(fish->p1_y, old_top, new_top);
    fish->p2_y = prv_remap_water_y(fish->p2_y, old_top, new_top);
    fish->p3_y = prv_remap_water_y(fish->p3_y, old_top, new_top);
    if (fish->active) {
      prv_update_fish_pose(fish, true);
    }
  }
}

static void prv_reset_bubble(Bubble *bubble, bool initial) {
  const bool left_stream = (prv_random() & 1) == 0;
  const int16_t source_x = left_stream ? 41 : 160;
  bubble->x_q8 = (source_x + (int16_t)(prv_random() % 9) - 4) << 8;
  const int16_t bubble_top = s_water_y + 7;
  const int16_t initial_span = DISPLAY_H - 15 - bubble_top;
  bubble->y_q8 = (initial ? (bubble_top + (prv_random() % initial_span))
                          : (211 + (prv_random() % 16))) << 8;
  bubble->rise_q8 = (7 + (prv_random() % 8)) << 8;
  bubble->size = 1 + (prv_random() % 3);
  bubble->phase = prv_random() & 31;
}

static void prv_init_bubbles(void) {
  for (int i = 0; i < BUBBLE_COUNT; ++i) {
    prv_reset_bubble(&s_bubbles[i], true);
  }
}

static void prv_update_world(uint16_t elapsed_ms) {
  s_animation_ms += elapsed_ms;
  prv_update_surface_profile();
  for (int i = 0; i < FISH_COUNT; ++i) {
    prv_update_fish(&s_fish[i], elapsed_ms);
  }
  for (int i = 0; i < BUBBLE_COUNT; ++i) {
    Bubble *bubble = &s_bubbles[i];
    bubble->y_q8 -= ((int32_t)bubble->rise_q8 * elapsed_ms) / 1000;
    if ((bubble->y_q8 >> 8)
        < prv_surface_y_at(bubble->x_q8 >> 8) + 2) {
      prv_reset_bubble(bubble, false);
    }
  }
}

static void prv_draw_surface_wave(GContext *ctx, int16_t base_y,
                                  uint8_t phase_offset, GColor color,
                                  uint8_t width, int16_t y_offset) {
  (void)phase_offset;
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, width);
  GPoint previous = GPoint(0, base_y + y_offset
      + prv_surface_y_at(0) - s_water_y);
  for (int16_t x = 5; x <= DISPLAY_W; x += 5) {
    const int16_t sample_x = x < DISPLAY_W ? x : DISPLAY_W - 1;
    const GPoint point = GPoint(x, base_y + y_offset
        + prv_surface_y_at(sample_x) - s_water_y);
    graphics_draw_line(ctx, previous, point);
    previous = point;
  }
}

static WeatherKind prv_weather_kind(void) {
  if (!s_weather_valid || s_weather_code == 0) {
    return WEATHER_CLEAR;
  }
  if (s_weather_code <= 3) {
    return WEATHER_CLOUD;
  }
  if (s_weather_code == 45 || s_weather_code == 48) {
    return WEATHER_FOG;
  }
  if ((s_weather_code >= 51 && s_weather_code <= 67)
      || (s_weather_code >= 80 && s_weather_code <= 82)) {
    return WEATHER_RAIN;
  }
  if ((s_weather_code >= 71 && s_weather_code <= 77)
      || (s_weather_code >= 85 && s_weather_code <= 86)) {
    return WEATHER_SNOW;
  }
  if (s_weather_code >= 95) {
    return WEATHER_STORM;
  }
  return WEATHER_CLOUD;
}

static uint8_t prv_sky_mix(uint8_t first, uint8_t second, uint8_t amount,
                           int16_t x, int16_t y) {
  // Shift each 8 px tile independently so the dither never reads as a
  // repeated dot matrix.  Most of the sky is solid color; this is reserved
  // for narrow tonal transitions and soft cloud edges.
  const uint8_t tile_x = x >> 3;
  const uint8_t tile_y = y >> 3;
  const uint8_t noise = s_blue_noise8[
      (((y + tile_x * 3) & 7) << 3) | ((x + tile_y * 5) & 7)];
  return noise < amount ? second : first;
}

static void prv_set_sky_pixel(GBitmap *framebuffer, int16_t x, int16_t y,
                              uint8_t color) {
  if (x < 0 || x >= DISPLAY_W || y < 0 || y >= DISPLAY_H
      || y >= prv_surface_y_at(x)) {
    return;
  }
  GBitmapDataRowInfo row = gbitmap_get_data_row_info(framebuffer, y);
  if (x >= row.min_x && x <= row.max_x) {
    row.data[x] = color;
  }
}

static void prv_draw_sky_disc(GBitmap *framebuffer, int16_t center_x,
                              int16_t center_y, int16_t radius,
                              uint8_t color, bool crescent) {
  for (int16_t y = center_y - radius; y <= center_y + radius; ++y) {
    if (y < 0 || y >= DISPLAY_H) {
      continue;
    }
    GBitmapDataRowInfo row = gbitmap_get_data_row_info(framebuffer, y);
    for (int16_t x = center_x - radius; x <= center_x + radius; ++x) {
      if (x < row.min_x || x > row.max_x || y >= prv_surface_y_at(x)) {
        continue;
      }
      const int16_t dx = x - center_x;
      const int16_t dy = y - center_y;
      if (dx * dx + dy * dy > radius * radius) {
        continue;
      }
      if (crescent) {
        const int16_t cut_x = dx - radius / 2;
        const int16_t cut_y = dy + 1;
        if (cut_x * cut_x + cut_y * cut_y <= radius * radius) {
          continue;
        }
      }
      row.data[x] = color;
    }
  }
}

static void prv_draw_sky_cloud(GBitmap *framebuffer, int16_t center_x,
                               int16_t center_y, int16_t width,
                               int16_t radius, const SkyStyle *style) {
  const int16_t left = center_x - width / 2;
  const int16_t right = center_x + width / 2;
  for (int16_t y = center_y - radius; y <= center_y + radius / 2; ++y) {
    if (y < 0 || y >= DISPLAY_H) {
      continue;
    }
    GBitmapDataRowInfo row = gbitmap_get_data_row_info(framebuffer, y);
    for (int16_t x = left; x <= right; ++x) {
      if (x < row.min_x || x > row.max_x || y >= prv_surface_y_at(x)) {
        continue;
      }
      const int16_t dx_left = x - (center_x - width / 4);
      const int16_t dx_mid = x - center_x;
      const int16_t dx_right = x - (center_x + width / 4);
      const int16_t dy = y - center_y;
      const bool base = y >= center_y - 1 && y <= center_y + radius / 2;
      const bool bulb_left = dx_left * dx_left + dy * dy <= radius * radius;
      const bool bulb_mid = dx_mid * dx_mid + dy * dy
          <= (radius + 2) * (radius + 2);
      const bool bulb_right = dx_right * dx_right + dy * dy
          <= (radius - 1) * (radius - 1);
      if (!base && !bulb_left && !bulb_mid && !bulb_right) {
        continue;
      }
      const int16_t transition = (y - center_y + 2) * 16;
      row.data[x] = transition <= 0 ? style->cloud
          : (transition >= 64 ? style->shadow
                              : prv_sky_mix(style->cloud, style->shadow,
                                            transition, x, y));
    }
  }
}

static void prv_draw_sky_fog(GBitmap *framebuffer, const SkyStyle *style) {
  for (uint8_t band = 0; band < 4; ++band) {
    const int16_t base_y = ((band + 1) * s_water_y) / 5;
    for (int16_t x = 0; x < DISPLAY_W; ++x) {
      const int16_t y = base_y + prv_wave_smooth(
          ((uint32_t)x * 256) / 17 + ((uint32_t)band * 7 << 8)) / 12;
      if (s_blue_noise8[((y & 7) << 3) | (x & 7)] < 48) {
        prv_set_sky_pixel(framebuffer, x, y, style->cloud);
        if ((x + band) % 3) {
          prv_set_sky_pixel(framebuffer, x, y + 1, style->shadow);
        }
      }
    }
  }
}

static void prv_draw_sky_particles(GBitmap *framebuffer, WeatherKind kind) {
  const uint16_t step = s_animation_ms / (kind == WEATHER_SNOW ? 180 : 85);
  const int16_t particle_height = prv_clamp_i16(s_water_y + 8, 14, 66);
  const uint8_t rain_color = s_sky_phase == SKY_NIGHT
      ? COLOR8(0,2,3) : COLOR8(1,3,3);
  if (kind == WEATHER_RAIN || kind == WEATHER_STORM) {
    for (uint8_t index = 0; index < 19; ++index) {
      const int16_t x = (index * 67 + step * 3 + index * index * 5) % 218 - 9;
      const int16_t y = (index * 29 + step * (3 + index % 3))
          % particle_height - 4;
      for (int8_t pixel = 0; pixel < 4; ++pixel) {
        prv_set_sky_pixel(framebuffer, x - pixel / 2, y + pixel, rain_color);
      }
    }
  } else if (kind == WEATHER_SNOW) {
    for (uint8_t index = 0; index < 18; ++index) {
      const int16_t x = (index * 73 + step * (1 + index % 2)
          + prv_wave_smooth((step * 64) + ((uint32_t)index * 5 << 8)) / 2)
          % 210 - 5;
      const int16_t y = (index * 31 + step * (1 + index % 3))
          % particle_height - 3;
      prv_set_sky_pixel(framebuffer, x, y, COLOR8(3,3,3));
      if ((index & 3) == 0) {
        prv_set_sky_pixel(framebuffer, x + 1, y, COLOR8(2,3,3));
      }
    }
  }

  if (kind == WEATHER_STORM && s_animation_ms % 6800 < 260) {
    const int16_t bolt_x = 132 + (s_animation_ms / 6800 % 3) * 11;
    for (int8_t segment = 0; segment < 4; ++segment) {
      const int16_t x0 = bolt_x + (segment & 1 ? 3 : 0);
      const int16_t y0 = 2 + (segment * prv_clamp_i16(s_water_y - 5, 5, 45)) / 4;
      const int16_t x1 = bolt_x + (segment & 1 ? 0 : 3);
      const int16_t y1 = 2 + ((segment + 1)
          * prv_clamp_i16(s_water_y - 5, 5, 45)) / 4;
      for (int16_t y = y0; y <= y1; ++y) {
        const int16_t x = x0 + ((x1 - x0) * (y - y0)) / (y1 - y0);
        prv_set_sky_pixel(framebuffer, x, y, COLOR8(3,3,0));
      }
    }
  }
}

static void prv_draw_sky_features(GBitmap *framebuffer, WeatherKind kind,
                                  const SkyStyle *style) {
  const int16_t air_height = prv_clamp_i16(s_water_y, WATER_MIN_Y, WATER_MAX_Y);
  const int16_t upper_y = prv_clamp_i16(air_height / 3, 3, 18);
  const int16_t lower_y = prv_clamp_i16((air_height * 2) / 3, 4, 38);
  const int16_t small_radius = prv_clamp_i16(air_height / 7, 2, 8);
  const int16_t large_radius = prv_clamp_i16(air_height / 5, 3, 11);
  if (s_sky_phase == SKY_NIGHT) {
    for (uint8_t index = 0; index < 18; ++index) {
      const int16_t x = (index * 73 + index * index * 3 + 17) % DISPLAY_W;
      const int16_t y = (index * 29 + index * index * 7 + 3)
          % prv_clamp_i16(air_height, 5, 54);
      prv_set_sky_pixel(framebuffer, x, y,
          index & 1 ? COLOR8(1,2,3) : COLOR8(3,3,3));
    }
    if (kind == WEATHER_CLEAR || kind == WEATHER_CLOUD) {
      prv_draw_sky_disc(framebuffer, 151, upper_y, small_radius,
                        COLOR8(2,3,3), true);
    }
  } else if (kind != WEATHER_FOG && kind != WEATHER_STORM) {
    // Keep the solar cue between the two surface floats: the morning sun was
    // previously hidden directly behind the battery buoy.
    const int16_t sun_x = s_sky_phase == SKY_MORNING ? 78
        : (s_sky_phase == SKY_NOON ? 108 : 132);
    const int16_t sun_y = s_sky_phase == SKY_NOON ? upper_y : lower_y;
    const uint8_t sun_color = s_sky_phase == SKY_EVENING
        ? COLOR8(3,1,0) : COLOR8(3,3,1);
    prv_draw_sky_disc(framebuffer, sun_x, sun_y, small_radius,
                      sun_color, false);
  }

  if (kind == WEATHER_CLOUD) {
    // Stagger two isolated banks rather than filling the narrow air band with
    // one continuous stripe.  This stays readable even when a high battery
    // leaves only a few rows above the water.
    prv_draw_sky_cloud(framebuffer, 51, upper_y, 34, small_radius, style);
    prv_draw_sky_cloud(framebuffer, 145, lower_y, 46, large_radius, style);
  } else if (kind == WEATHER_RAIN || kind == WEATHER_SNOW
             || kind == WEATHER_STORM) {
    prv_draw_sky_cloud(framebuffer, 49, upper_y, 78, large_radius, style);
    prv_draw_sky_cloud(framebuffer, 146, lower_y, 102,
                       large_radius + 2, style);
  } else if (kind == WEATHER_FOG) {
    prv_draw_sky_fog(framebuffer, style);
  }
  prv_draw_sky_particles(framebuffer, kind);
}

static void prv_draw_air_volume(GBitmap *framebuffer) {
  const WeatherKind kind = prv_weather_kind();
  const uint8_t phase = s_sky_phase < SKY_PHASE_COUNT
      ? s_sky_phase : SKY_NOON;
  const SkyStyle *style = &s_sky_styles[phase][kind];
  for (int16_t y = 0; y <= s_water_y + 6; ++y) {
    GBitmapDataRowInfo row = gbitmap_get_data_row_info(framebuffer, y);
    for (int16_t x = row.min_x; x <= row.max_x; ++x) {
      if (y < prv_surface_y_at(x)) {
        if (y < 10) {
          row.data[x] = style->top;
        } else if (y < 18) {
          row.data[x] = prv_sky_mix(style->top, style->middle,
                                    (y - 10) * 8, x, y);
        } else if (y < 40) {
          row.data[x] = style->middle;
        } else if (y < 50) {
          row.data[x] = prv_sky_mix(style->middle, style->bottom,
                                    (y - 40) * 6, x, y);
        } else {
          row.data[x] = style->bottom;
        }
      }
    }
  }
  prv_draw_sky_features(framebuffer, kind, style);
}

static void prv_draw_surface(GContext *ctx) {
  prv_draw_surface_wave(ctx, s_water_y, 0,
                        GColorFromRGB(0, 85, 85), 2, 1);
  prv_draw_surface_wave(ctx, s_water_y, 0,
                        GColorFromRGB(170, 255, 255), 1, 0);
  graphics_context_set_stroke_width(ctx, 1);
}

static uint8_t prv_caustic_brighten(uint8_t pixel, uint8_t strength) {
  if ((pixel & 0xC0) == 0) {
    return pixel;
  }
  uint8_t blue = pixel & 3;
  uint8_t green = (pixel >> 2) & 3;
  uint8_t red = (pixel >> 4) & 3;
  if (strength >= 2) {
    red = 3;
    green = 3;
    blue = blue < 2 ? 2 : 3;
  } else if (strength == 1) {
    if (red < 3) {
      ++red;
    }
    if (green < 3) {
      ++green;
    }
    if (blue < 3) {
      ++blue;
    }
  } else if (blue < 3) {
    ++blue;
  }
  return (pixel & 0xC0) | (red << 4) | (green << 2) | blue;
}

static const uint8_t s_dither4[16] = {
   0,  8,  2, 10,
  12,  4, 14,  6,
   3, 11,  1,  9,
  15,  7, 13,  5,
};

static bool prv_load_caustic_frame(uint8_t slot, uint8_t frame) {
  if (!s_caustic_frames_resource || slot > 1
      || frame >= CAUSTIC_FRAME_COUNT) {
    return false;
  }
  const size_t loaded = resource_load_byte_range(
      s_caustic_frames_resource,
      (uint32_t)frame * CAUSTIC_PACKED_BYTES,
      s_caustic_packed[slot],
      CAUSTIC_PACKED_BYTES);
  if (loaded != CAUSTIC_PACKED_BYTES) {
    s_loaded_caustic_frame[slot] = -1;
    return false;
  }
  s_loaded_caustic_frame[slot] = frame;
  return true;
}

static int8_t prv_find_caustic_frame(uint8_t frame) {
  if (s_loaded_caustic_frame[0] == frame) {
    return 0;
  }
  if (s_loaded_caustic_frame[1] == frame) {
    return 1;
  }
  return -1;
}

static uint8_t prv_caustic_value(const uint8_t *packed, int16_t x,
                                 int16_t y) {
  const uint16_t index = y * CAUSTIC_FRAME_W + x;
  const uint8_t pair = packed[index >> 1];
  const uint8_t nibble = (index & 1) ? (pair & 0x0f) : (pair >> 4);
  return (nibble << 2) | (nibble >> 2);
}

static void prv_draw_bottom_caustics(GBitmap *framebuffer) {
  if (!s_caustic_frames_resource) {
    return;
  }
  const uint32_t phase_time = s_animation_ms / 145;
  const uint8_t frame = phase_time & (CAUSTIC_FRAME_COUNT - 1);
  const uint8_t next_frame = (frame + 1) & (CAUSTIC_FRAME_COUNT - 1);
  const uint8_t blend = ((s_animation_ms % 145) * 16) / 145;

  int8_t first_slot = prv_find_caustic_frame(frame);
  if (first_slot < 0) {
    first_slot = s_loaded_caustic_frame[0] == next_frame ? 1 : 0;
    if (!prv_load_caustic_frame(first_slot, frame)) {
      return;
    }
  }
  int8_t second_slot = prv_find_caustic_frame(next_frame);
  if (second_slot < 0) {
    second_slot = first_slot ^ 1;
    if (!prv_load_caustic_frame(second_slot, next_frame)) {
      return;
    }
  }
  const uint8_t *first_data = s_caustic_packed[first_slot];
  const uint8_t *second_data = s_caustic_packed[second_slot];

  for (int16_t y = 164; y < DISPLAY_H; ++y) {
    GBitmapDataRowInfo row = gbitmap_get_data_row_info(framebuffer, y);
    const int16_t map_y = y >> 2;
    for (int16_t x = row.min_x; x <= row.max_x; ++x) {
      if (y < s_floor_horizon[x]) {
        continue;
      }
      const int16_t map_x = x >> 2;
      const uint8_t first = prv_caustic_value(first_data, map_x, map_y);
      const uint8_t second = prv_caustic_value(second_data, map_x, map_y);
      const uint8_t intensity = (first * (16 - blend) + second * blend) >> 4;
      const uint8_t threshold = s_dither4[((y & 3) << 2) | (x & 3)];
      uint8_t strength = 255;
      if (intensity > 43 + threshold / 2) {
        strength = 2;
      } else if (intensity > 22 + threshold) {
        strength = 1;
      } else if (intensity > 7 + threshold * 2) {
        strength = 0;
      }
      if (strength != 255) {
        row.data[x] = prv_caustic_brighten(row.data[x], strength);
      }
    }
  }
}

static const uint8_t s_plant_palette[8] = {
  0x00,  // transparent
  0xC0,  // black-green shadow
  0xC1,  // blue-green shadow
  0xC4,  // deep green
  0xC5,  // teal fold
  0xD4,  // olive root/highlight
  0xC8,  // green highlight
  0xD9,  // pale green glint
};

static bool prv_load_plant_frame(void) {
  const uint8_t frame = (s_animation_ms / ANIMATION_FRAME_MS)
      % PLANT_FRAME_COUNT;
  if (s_loaded_plant_frame == frame) {
    return true;
  }
  const size_t loaded = resource_load_byte_range(
      s_plant_animation_resource,
      (uint32_t)frame * PLANT_FRAME_BYTES,
      s_plant_frame,
      PLANT_FRAME_BYTES);
  if (loaded != PLANT_FRAME_BYTES) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Plant frame %d load failed: %d", frame,
            (int)loaded);
    return false;
  }
  s_plant_frame[PLANT_FRAME_BYTES] = 0;
  s_loaded_plant_frame = frame;
  return true;
}

static uint8_t prv_plant_color(uint32_t pixel_index) {
  const uint32_t bit = pixel_index * 3;
  const uint16_t packed = s_plant_frame[bit >> 3]
      | ((uint16_t)s_plant_frame[(bit >> 3) + 1] << 8);
  return s_plant_palette[(packed >> (bit & 7)) & 7];
}

static void prv_blit_plant(GBitmap *framebuffer, const PlantAsset *plant) {
  for (int16_t source_y = 0; source_y < plant->frame_h; ++source_y) {
    const int16_t target_y = plant->y + source_y;
    if (target_y < 0 || target_y >= DISPLAY_H) {
      continue;
    }
    const GBitmapDataRowInfo target_row =
        gbitmap_get_data_row_info(framebuffer, target_y);
    for (int16_t local_x = 0; local_x < plant->frame_w; ++local_x) {
      const int16_t target_x = plant->x + local_x;
      if (target_x < target_row.min_x || target_x > target_row.max_x) {
        continue;
      }
      const uint8_t pixel = prv_plant_color(plant->pixel_offset
          + source_y * plant->frame_w + local_x);
      if (pixel) {
        target_row.data[target_x] = pixel;
      }
    }
  }
}

static void prv_draw_plant_pass(GBitmap *framebuffer, bool foreground) {
  if (!prv_load_plant_frame()) {
    return;
  }
  for (uint8_t index = 0; index < PLANT_COUNT; ++index) {
    if (s_plants[index].foreground == foreground) {
      prv_blit_plant(framebuffer, &s_plants[index]);
    }
  }
}

static uint8_t prv_depth_haze(uint8_t pixel, uint8_t depth) {
  if (depth != 0 || (pixel & 0xC0) == 0) {
    return pixel;
  }
  uint8_t blue = pixel & 3;
  const uint8_t green = (pixel >> 2) & 3;
  uint8_t red = (pixel >> 4) & 3;
  if (red > 0) {
    --red;
  }
  if (blue < 3 && blue <= green) {
    ++blue;
  }
  return (pixel & 0xC0) | (red << 4) | (green << 2) | blue;
}

static void prv_draw_transformed_fish(GBitmap *framebuffer, const Fish *fish) {
  if (!fish->active) {
    return;
  }
  const SpeciesAsset *asset = &s_species[fish->species];
  if (!asset->sheet || gbitmap_get_format(asset->sheet) != GBitmapFormat8Bit) {
    return;
  }

  static const uint8_t frame_sequence[4] = {0, 1, 0, 2};
  const uint8_t frame_index = frame_sequence[
      (fish->tail_ms / (128 + fish->species * 17)) & 3];
  const int16_t source_x = frame_index * asset->frame_w;
  const int16_t source_y = fish->direction * asset->frame_h;
  const uint8_t *source_data = gbitmap_get_data(asset->sheet);
  const uint16_t source_stride = gbitmap_get_bytes_per_row(asset->sheet);
  const uint16_t scale_x_q8 = ((uint32_t)fish->scale_q8
      * fish->width_scale_q8) >> 8;
  const uint16_t scale_y_q8 = fish->scale_q8;

  const int32_t rotation = DEG_TO_TRIGANGLE(fish->angle_q8 / 256);
  const int32_t sine = sin_lookup(rotation);
  const int32_t cosine = cos_lookup(rotation);
  const int32_t abs_sine = prv_abs_i32(sine);
  const int32_t abs_cosine = prv_abs_i32(cosine);
  const int16_t projected_w = (
      (abs_cosine * asset->frame_w * scale_x_q8) / 256
      + (abs_sine * asset->frame_h * scale_y_q8) / 256) / TRIG_MAX_RATIO;
  const int16_t projected_h = (
      (abs_sine * asset->frame_w * scale_x_q8) / 256
      + (abs_cosine * asset->frame_h * scale_y_q8) / 256) / TRIG_MAX_RATIO;
  const int16_t half_w = projected_w / 2 + 2;
  const int16_t half_h = projected_h / 2 + 2;
  const int16_t left = prv_clamp_i16(fish->x - half_w, 0, DISPLAY_W - 1);
  const int16_t right = prv_clamp_i16(fish->x + half_w, 0, DISPLAY_W - 1);
  const int16_t top = prv_clamp_i16(
      fish->y - half_h, s_water_y + 1, DISPLAY_H - 1);
  const int16_t bottom = prv_clamp_i16(fish->y + half_h, 0, DISPLAY_H - 1);

  // Inverse affine steps in source-pixel Q16. Division happens once per fish;
  // the inner loop advances with additions, keeping arbitrary pitch, depth
  // scaling, and the on-screen turnaround affordable at a steady 10 FPS.
  const int32_t step_x_x = (cosine * 256) / scale_x_q8;
  const int32_t step_x_y = (-sine * 256) / scale_y_q8;
  const int32_t step_y_x = (sine * 256) / scale_x_q8;
  const int32_t step_y_y = (cosine * 256) / scale_y_q8;
  const int32_t source_center_x_q16 = asset->frame_w << 15;
  const int32_t source_center_y_q16 = asset->frame_h << 15;

  for (int16_t y = top; y <= bottom; ++y) {
    const int16_t dx = left - fish->x;
    const int16_t dy = y - fish->y;
    int32_t sample_x_q16 = source_center_x_q16
        + dx * step_x_x + dy * step_y_x;
    int32_t sample_y_q16 = source_center_y_q16
        + dx * step_x_y + dy * step_y_y;
    const GBitmapDataRowInfo target_row =
        gbitmap_get_data_row_info(framebuffer, y);

    for (int16_t x = left; x <= right; ++x) {
      const int16_t sample_x = sample_x_q16 >> 16;
      const int16_t sample_y = sample_y_q16 >> 16;
      if (sample_x >= 0 && sample_x < asset->frame_w
          && sample_y >= 0 && sample_y < asset->frame_h
          && x >= target_row.min_x && x <= target_row.max_x
          && y > prv_surface_y_at(x) + 1) {
        const uint8_t pixel = source_data[
            (source_y + sample_y) * source_stride + source_x + sample_x];
        if (pixel & 0xC0) {
          target_row.data[x] = prv_depth_haze(pixel, fish->depth);
        }
      }
      sample_x_q16 += step_x_x;
      sample_y_q16 += step_x_y;
    }
  }
}

static void prv_draw_fish(GBitmap *framebuffer) {
  uint8_t order[FISH_COUNT];
  for (uint8_t i = 0; i < FISH_COUNT; ++i) {
    order[i] = i;
  }
  for (uint8_t i = 1; i < FISH_COUNT; ++i) {
    const uint8_t value = order[i];
    int8_t j = i - 1;
    while (j >= 0 && s_fish[order[j]].scale_q8 > s_fish[value].scale_q8) {
      order[j + 1] = order[j];
      --j;
    }
    order[j + 1] = value;
  }

  for (uint8_t i = 0; i < FISH_COUNT; ++i) {
    prv_draw_transformed_fish(framebuffer, &s_fish[order[i]]);
  }
}

static GFont prv_font_for_size(uint8_t size, bool bold);
static uint8_t prv_fitting_date_size(void);

static uint8_t prv_time_values(int8_t values[SMOOTH_GLYPH_COUNT]) {
  const bool use_24_hour = prv_uses_24_hour_time();
  const int display_hour = use_24_hour
      ? s_hour : ((s_hour % 12) ? (s_hour % 12) : 12);
  values[0] = display_hour / 10;
  values[1] = display_hour % 10;
  values[2] = -1;
  values[3] = s_minute / 10;
  values[4] = s_minute % 10;
  return use_24_hour || display_hour >= 10 ? 0 : 1;
}

static bool prv_capture_smooth_glyph(GContext *ctx, SmoothGlyph *glyph,
                                     const char *text, GFont font) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, SMOOTH_MASK_W, SMOOTH_MASK_H),
                     0, GCornerNone);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, text, font,
      GRect(0, 0, SMOOTH_MASK_W, SMOOTH_MASK_H),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);

  GBitmap *framebuffer = graphics_capture_frame_buffer(ctx);
  if (!framebuffer) {
    return false;
  }
  int16_t left = SMOOTH_MASK_W;
  int16_t right = -1;
  int16_t top = SMOOTH_MASK_H;
  int16_t bottom = -1;
  for (int16_t y = 0; y < SMOOTH_MASK_H; ++y) {
    const GBitmapDataRowInfo row =
        gbitmap_get_data_row_info(framebuffer, y);
    for (int16_t x = 0; x < SMOOTH_MASK_W; ++x) {
      if (x >= row.min_x && x <= row.max_x && (row.data[x] & 0x3f)) {
        if (x < left) left = x;
        if (x > right) right = x;
        if (y < top) top = y;
        if (y > bottom) bottom = y;
      }
    }
  }

  memset(glyph, 0, sizeof(*glyph));
  if (right >= left && bottom >= top) {
    glyph->width = right - left + 1;
    glyph->height = bottom - top + 1;
    for (int16_t y = top; y <= bottom; ++y) {
      const GBitmapDataRowInfo row =
          gbitmap_get_data_row_info(framebuffer, y);
      const uint8_t local_y = y - top;
      int16_t x = left;
      while (x <= right) {
        while (x <= right && (x < row.min_x || x > row.max_x
                              || !(row.data[x] & 0x3f))) {
          ++x;
        }
        if (x > right) {
          break;
        }
        const uint8_t run_start = x - left;
        while (x <= right && x >= row.min_x && x <= row.max_x
               && (row.data[x] & 0x3f)) {
          ++x;
        }
        const uint8_t run_end = x - left;
        uint8_t count = glyph->run_count[local_y];
        if (count < SMOOTH_MAX_RUNS_PER_ROW) {
          glyph->runs[local_y][count][0] = run_start;
          glyph->runs[local_y][count][1] = run_end;
          glyph->run_count[local_y] = count + 1;
        } else {
          // Highly decorative fonts can contain isolated antialias pixels.
          // Merge pathological extras into the final span instead of adding
          // unbounded per-frame work; normal Emery clock glyphs stay below
          // this limit and are represented exactly.
          glyph->runs[local_y][SMOOTH_MAX_RUNS_PER_ROW - 1][1] = run_end;
        }
      }
    }
  }
  graphics_release_frame_buffer(ctx, framebuffer);
  return glyph->width > 0 && glyph->height > 0;
}

static void prv_prepare_smooth_glyphs(GContext *ctx) {
  if (s_time_style != TIME_STYLE_SMOOTH_GLASS || !s_smooth_cache_dirty) {
    return;
  }
  int8_t values[SMOOTH_GLYPH_COUNT];
  const uint8_t first = prv_time_values(values);
  GFont selected_font = fonts_get_system_font(
      s_time_font_keys[prv_clamp_i16(s_time_font, 0, TIME_FONT_COUNT - 1)]);
  GFont fallback_font = fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);

  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  for (uint8_t index = 0; index < SMOOTH_GLYPH_COUNT; ++index) {
    SmoothGlyph *glyph = &s_smooth_glyphs[index];
    memset(glyph, 0, sizeof(*glyph));
    if (index < first) {
      continue;
    }
    char text[2] = {
      values[index] < 0 ? ':' : (char)('0' + values[index]), '\0'
    };
    if (!prv_capture_smooth_glyph(ctx, glyph, text, selected_font)) {
      prv_capture_smooth_glyph(ctx, glyph, text, fallback_font);
    }
  }
  s_smooth_cache_dirty = false;
}

static int16_t prv_time_source_height(void) {
  if (s_time_style == TIME_STYLE_DOUBLOON) {
    return DOUBLOON_FRAME_H;
  }
  if (s_time_style == TIME_STYLE_SMOOTH_GLASS) {
    int16_t height = 0;
    for (uint8_t index = 0; index < SMOOTH_GLYPH_COUNT; ++index) {
      if (s_smooth_glyphs[index].height > height) {
        height = s_smooth_glyphs[index].height;
      }
    }
    return height > 0 ? height : 42;
  }
  return 9 * CLOCK_SCALE;
}

static uint16_t prv_time_effective_scale(void) {
  int8_t values[SMOOTH_GLYPH_COUNT];
  const uint8_t first = prv_time_values(values);
  int16_t source_width = 0;
  uint8_t glyph_count = 0;
  for (uint8_t index = first; index < SMOOTH_GLYPH_COUNT; ++index) {
    if (s_time_style == TIME_STYLE_DOUBLOON) {
      source_width += values[index] < 0
          ? DOUBLOON_COLON_W : DOUBLOON_FRAME_W;
    } else if (s_time_style == TIME_STYLE_SMOOTH_GLASS) {
      source_width += s_smooth_glyphs[index].width > 0
          ? s_smooth_glyphs[index].width : 12;
    } else {
      source_width += values[index] < 0 ? CLOCK_SCALE : 7 * CLOCK_SCALE;
    }
    ++glyph_count;
  }
  if (source_width <= 0) {
    return 100;
  }

  uint16_t scale = s_time_scale;
  const uint8_t gaps = glyph_count > 0 ? glyph_count - 1 : 0;
  const int16_t gap = prv_clamp_i16((3 * scale + 50) / 100, 2, 7);
  const int16_t horizontal_space = DISPLAY_W - 6 - gaps * gap;
  const uint16_t horizontal_fit =
      prv_clamp_i16((horizontal_space * 100) / source_width, 40, 255);
  if (scale > horizontal_fit) {
    scale = horizontal_fit;
  }

  const int16_t date_room = s_date_format == DATE_FORMAT_HIDDEN
      ? 0 : prv_fitting_date_size() + 12;
  const int16_t vertical_space = FLOOR_Y - 6 - (s_water_y + 7) - date_room;
  const int16_t source_height = prv_time_source_height();
  const uint16_t vertical_fit = source_height > 0
      ? prv_clamp_i16((vertical_space * 100) / source_height, 40, 255)
      : scale;
  if (scale > vertical_fit) {
    scale = vertical_fit;
  }
  return scale;
}

static int16_t prv_clock_center_y(void) {
  const int16_t desired = 104 + ((s_water_y - WATER_MIN_Y) * 5) / 16;
  const int16_t source_height = prv_time_source_height();
  const uint16_t scale = prv_time_effective_scale();
  const int16_t half_height =
      (source_height * scale + 100) / 200;
  const int16_t date_room = s_date_format == DATE_FORMAT_HIDDEN
      ? 0 : prv_fitting_date_size() + 12;
  const int16_t minimum = s_water_y + 7 + half_height;
  const int16_t maximum = FLOOR_Y - 3 - half_height - date_room;
  if (maximum <= minimum) {
    return minimum;
  }
  return prv_clamp_i16(desired, minimum, maximum);
}

static GlyphTransform prv_glyph_transform(int16_t slot_center_x,
                                          uint8_t glyph_index,
                                          uint16_t scale_percent) {
  const int8_t yaw = prv_wave_smooth(
      (s_animation_ms * 256) / (480 + glyph_index * 43)
      + ((uint32_t)(glyph_index * 7) << 8));
  const int8_t pitch = prv_wave_smooth(
      (s_animation_ms * 256) / (610 + glyph_index * 37)
      + ((uint32_t)(glyph_index * 11 + 5) << 8));
  const int8_t roll = prv_wave_smooth(
      (s_animation_ms * 256) / (760 + glyph_index * 31)
      + ((uint32_t)(glyph_index * 5 + 13) << 8));
  const int16_t bob_x = prv_wave_smooth(
      (s_animation_ms * 256) / (390 + glyph_index * 29)
      + ((uint32_t)(glyph_index * 9) << 8)) / 8;
  const int16_t bob_y = prv_wave_smooth(
      (s_animation_ms * 256) / (430 + glyph_index * 47)
      + ((uint32_t)(glyph_index * 6 + 3) << 8)) / 6;
  const int16_t base_x_scale = 252 - prv_abs_i16(yaw) * 3 / 2;
  const int16_t base_y_scale = 253 - prv_abs_i16(pitch);
  return (GlyphTransform) {
    .center_x = slot_center_x + bob_x,
    .center_y = prv_clock_center_y() + bob_y,
    .x_scale_q8 = (base_x_scale * scale_percent) / 100,
    .y_scale_q8 = (base_y_scale * scale_percent) / 100,
    .shear_q8 = ((yaw - roll * 2) * scale_percent) / 100,
    .roll_q8 = ((roll * 2 + pitch / 2) * scale_percent) / 100,
    .yaw = yaw,
    .pitch = pitch,
    .ripple_phase_q8 = (s_animation_ms * 256) / (205 + glyph_index * 13)
        + ((uint32_t)(glyph_index * 4) << 8),
  };
}

static GPoint prv_transform_clock_point(const GlyphTransform *transform,
                                        int16_t x, int16_t y,
                                        uint8_t depth_layer) {
  int16_t transformed_x = transform->center_x
      + (((int32_t)x * transform->x_scale_q8
          + (int32_t)y * transform->shear_q8) >> 8)
      + (transform->yaw * depth_layer) / 13;
  int16_t transformed_y = transform->center_y
      + (((int32_t)y * transform->y_scale_q8
          + (int32_t)x * transform->roll_q8) >> 8)
      + depth_layer + (transform->pitch * depth_layer) / 20;
  const int16_t water_depth = transformed_y - prv_surface_y_at(transformed_x);
  const int16_t ripple_divisor = prv_clamp_i16(10 + water_depth / 19, 10, 18);
  const int16_t ripple = prv_wave_smooth(transform->ripple_phase_q8
      + ((uint32_t)transformed_y * 256) / 4) / ripple_divisor;
  transformed_x += ripple;
  transformed_y += prv_wave_smooth(transform->ripple_phase_q8 / 2
      + ((uint32_t)transformed_x * 256) / 7) / 28;
  return GPoint(transformed_x, transformed_y);
}

static GColor prv_clock_caustic_color(GPoint point) {
  const uint32_t phase_q8 = (s_animation_ms * 256) / 330;
  const uint8_t noise = prv_map_value(s_caustic_noise,
      point.x + s_animation_ms / 190, point.y - s_animation_ms / 270, true);
  const int16_t caustic = prv_wave_smooth(
      ((uint32_t)point.x * 256) / 6
      + ((uint32_t)point.y * 256) / 9 + phase_q8)
      + prv_wave_smooth(((uint32_t)point.x * 256) / 11
          - ((uint32_t)point.y * 256) / 6 + phase_q8 / 2) / 2
      + ((int16_t)noise - 31) / 3;
  if (caustic > 16) {
    return GColorFromRGB(255, 255, 170);
  }
  if (caustic > 7) {
    return GColorWhite;
  }
  if (caustic < -15) {
    return GColorFromRGB(85, 170, 170);
  }
  return GColorFromRGB(170, 255, 255);
}

static GColor prv_clock_side_color(uint8_t depth_layer) {
  return depth_layer >= 3
      ? GColorFromRGB(0, 0, 85)
      : GColorFromRGB(0, 85, 85);
}

static void prv_draw_clock_run(GContext *ctx, const GlyphTransform *transform,
                               int16_t x0, int16_t x1, int16_t y,
                               uint8_t depth_layer) {
  const GPoint start = prv_transform_clock_point(transform, x0, y, depth_layer);
  const GPoint end = prv_transform_clock_point(transform, x1, y, depth_layer);
  graphics_context_set_stroke_color(ctx, depth_layer
      ? prv_clock_side_color(depth_layer)
      : prv_clock_caustic_color(GPoint((start.x + end.x) / 2,
                                      (start.y + end.y) / 2)));
  graphics_draw_line(ctx, start, end);
}

static void prv_draw_clock_digit(GContext *ctx, const GlyphTransform *transform,
                                 uint8_t digit, int16_t left, int16_t top,
                                 uint8_t depth_layer) {
  for (int16_t row = 0; row < 9; ++row) {
    const uint8_t bits = s_time_style == TIME_STYLE_STENCIL
        ? s_stencil_digits[digit][row] : s_digits[digit][row];
    for (int16_t sub_y = 0; sub_y < CLOCK_SCALE; ++sub_y) {
      int16_t column = 0;
      while (column < 7) {
        while (column < 7 && (bits & (1 << (6 - column))) == 0) {
          ++column;
        }
        if (column >= 7) {
          break;
        }
        const int16_t run_start = column;
        while (column < 7 && (bits & (1 << (6 - column)))) {
          ++column;
        }
        const int16_t run_end = column - 1;
        prv_draw_clock_run(ctx, transform,
            left + run_start * CLOCK_SCALE,
            left + (run_end + 1) * CLOCK_SCALE - 1,
            top + row * CLOCK_SCALE + sub_y, depth_layer);
      }
    }
  }
}

static void prv_draw_clock_colon(GContext *ctx, const GlyphTransform *transform,
                                 int16_t left, int16_t top,
                                 uint8_t depth_layer) {
  for (int dot = 0; dot < 2; ++dot) {
    const int16_t dot_y = top + (dot ? 6 : 3) * CLOCK_SCALE;
    for (int16_t sub_y = 0; sub_y < CLOCK_SCALE; ++sub_y) {
      prv_draw_clock_run(ctx, transform, left,
          left + CLOCK_SCALE - 1, dot_y + sub_y, depth_layer);
    }
  }
}

static void prv_draw_vector_time(GContext *ctx) {
  int8_t values[SMOOTH_GLYPH_COUNT];
  const uint8_t first = prv_time_values(values);
  const uint16_t scale = prv_time_effective_scale();
  const int16_t source_digit_width = 7 * CLOCK_SCALE;
  const int16_t projected_digit_width =
      (source_digit_width * scale + 50) / 100;
  const int16_t projected_colon_width =
      (CLOCK_SCALE * scale + 50) / 100;
  const int16_t gap = prv_clamp_i16(
      (3 * scale + 50) / 100, 2, 7);
  int16_t total_width = -gap;
  for (uint8_t index = first; index < 5; ++index) {
    total_width += (values[index] < 0
        ? projected_colon_width : projected_digit_width) + gap;
  }
  GlyphTransform transforms[5];
  int16_t cursor = (DISPLAY_W - total_width) / 2;
  uint8_t visual_index = 0;
  for (uint8_t index = first; index < 5; ++index, ++visual_index) {
    const int16_t width = values[index] < 0
        ? projected_colon_width : projected_digit_width;
    transforms[index] = prv_glyph_transform(
        cursor + width / 2, visual_index, scale);
    cursor += width + gap;
  }

  graphics_context_set_stroke_width(ctx, 1);
  for (int8_t depth = 4; depth >= 1; --depth) {
    for (uint8_t index = first; index < 5; ++index) {
      if (values[index] < 0) {
        prv_draw_clock_colon(ctx, &transforms[index],
            -CLOCK_SCALE / 2, -(9 * CLOCK_SCALE) / 2, depth);
      } else {
        prv_draw_clock_digit(ctx, &transforms[index], values[index],
            -source_digit_width / 2, -(9 * CLOCK_SCALE) / 2, depth);
      }
    }
  }
  for (uint8_t index = first; index < 5; ++index) {
    if (values[index] < 0) {
      prv_draw_clock_colon(ctx, &transforms[index],
          -CLOCK_SCALE / 2, -(9 * CLOCK_SCALE) / 2, 0);
    } else {
      prv_draw_clock_digit(ctx, &transforms[index], values[index],
          -source_digit_width / 2, -(9 * CLOCK_SCALE) / 2, 0);
    }
  }
}

static uint8_t prv_doubloon_brighten(uint8_t pixel, uint8_t strength) {
  uint8_t blue = pixel & 3;
  uint8_t green = (pixel >> 2) & 3;
  uint8_t red = (pixel >> 4) & 3;
  if (strength >= 2) {
    red = 3;
    green = 3;
    blue = blue < 2 ? 2 : 3;
  } else if (strength == 1) {
    if (red < 3) {
      ++red;
    }
    if (green < 3) {
      ++green;
    }
    if (blue == 0 && red == 3 && green == 3) {
      blue = 1;
    }
  }
  return (pixel & 0xC0) | (red << 4) | (green << 2) | blue;
}

static uint8_t prv_adjust_doubloon_brightness(uint8_t pixel,
                                               uint8_t brightness) {
  uint8_t blue = pixel & 3;
  uint8_t green = (pixel >> 2) & 3;
  uint8_t red = (pixel >> 4) & 3;
  if (brightness < 100) {
    red = (red * brightness + 50) / 100;
    green = (green * brightness + 50) / 100;
    blue = (blue * brightness + 50) / 100;
  } else if (brightness > 100) {
    const uint8_t boost = brightness - 100;
    red += ((3 - red) * boost + 50) / 100;
    green += ((3 - green) * boost + 50) / 100;
    if (blue < 2) {
      blue += ((2 - blue) * boost + 75) / 150;
    }
  }
  return (pixel & 0xC0) | (red << 4) | (green << 2) | blue;
}

static uint8_t prv_doubloon_face_color(
    uint8_t pixel, GPoint point, int16_t source_x, int16_t source_y,
    uint8_t visual_index, const GlyphTransform *transform) {
  pixel = prv_adjust_doubloon_brightness(pixel, s_gold_brightness);
  const int16_t sweep = (source_x * 5 + source_y * 2
      + s_animation_ms / 42 + visual_index * 13
      + (transform->yaw + 16) * 2) & 63;
  const int16_t distance = prv_abs_i16(sweep - 31);
  const int16_t facing = 16 - prv_abs_i16(transform->yaw);
  uint8_t strength = distance <= (facing > 8 ? 2 : 1)
      ? 2 : (distance <= 5 ? 1 : 0);

  // A second, slower field is the same refracted caustic illumination used by
  // the glass face, but expressed as metallic brightening instead of cyan.
  const uint32_t phase_q8 = (s_animation_ms * 256) / 360;
  const int16_t caustic = prv_wave_smooth(
      ((uint32_t)point.x * 256) / 7
      + ((uint32_t)point.y * 256) / 10 + phase_q8)
      + prv_wave_smooth(((uint32_t)point.x * 256) / 13
          - ((uint32_t)point.y * 256) / 8 + phase_q8 / 2) / 2;
  if (caustic > 17 && strength < 1) {
    strength = 1;
  }
  return prv_doubloon_brighten(pixel, strength);
}

static void prv_set_doubloon_pixel(GBitmap *framebuffer, GPoint point,
                                    uint8_t color, uint8_t footprint) {
  for (uint8_t dy = 0; dy < footprint; ++dy) {
    const int16_t y = point.y + dy;
    if (y < 0 || y >= DISPLAY_H) {
      continue;
    }
    GBitmapDataRowInfo row = gbitmap_get_data_row_info(framebuffer, y);
    for (uint8_t dx = 0; dx < footprint; ++dx) {
      const int16_t x = point.x + dx;
      if (x >= row.min_x && x <= row.max_x
          && y > prv_surface_y_at(x) + 1) {
        row.data[x] = color;
      }
    }
  }
}

static void prv_draw_doubloon_glyph(
    GBitmap *framebuffer, const GlyphTransform *transform, int8_t value,
    uint8_t visual_index, uint8_t depth_layer, uint16_t scale) {
  if (!s_doubloon_loaded) {
    return;
  }
  const int16_t source_left = value < 0
      ? DOUBLOON_COLON_X : (value % 5) * DOUBLOON_FRAME_W;
  const int16_t source_top = value < 0
      ? 0 : (value / 5) * DOUBLOON_FRAME_H;
  const int16_t source_width = value < 0
      ? DOUBLOON_COLON_W : DOUBLOON_FRAME_W;
  const uint8_t footprint = scale >= 120 ? 2 : 1;

  for (int16_t y = 0; y < DOUBLOON_FRAME_H; ++y) {
    for (int16_t x = 0; x < source_width; ++x) {
      const uint16_t source_index =
          (source_top + y) * DOUBLOON_SHEET_W + source_left + x;
      const uint8_t pair = s_doubloon_packed[source_index >> 1];
      const uint8_t palette_index = source_index & 1
          ? pair & 0x0f : pair >> 4;
      if (palette_index == 0) {
        continue;
      }
      const uint8_t pixel = s_doubloon_palette[palette_index];
      const GPoint point = prv_transform_clock_point(
          transform, x - source_width / 2, y - DOUBLOON_FRAME_H / 2,
          depth_layer);
      const uint8_t color = depth_layer
          ? prv_adjust_doubloon_brightness(
              depth_layer >= 3 ? COLOR8(1,0,0) : COLOR8(1,1,0),
              s_gold_brightness < 100 ? s_gold_brightness
                  : 100 + (s_gold_brightness - 100) / 2)
          : prv_doubloon_face_color(pixel, point, x, y,
                                    visual_index, transform);
      prv_set_doubloon_pixel(framebuffer, point, color, footprint);
    }
  }
}

static void prv_draw_doubloon_time(GContext *ctx) {
  int8_t values[SMOOTH_GLYPH_COUNT];
  const uint8_t first = prv_time_values(values);
  const uint16_t scale = prv_time_effective_scale();
  const int16_t projected_digit_width =
      (DOUBLOON_FRAME_W * scale + 50) / 100;
  const int16_t projected_colon_width =
      (DOUBLOON_COLON_W * scale + 50) / 100;
  const int16_t gap = prv_clamp_i16(
      (3 * scale + 50) / 100, 2, 7);
  int16_t total_width = -gap;
  for (uint8_t index = first; index < 5; ++index) {
    total_width += (values[index] < 0
        ? projected_colon_width : projected_digit_width) + gap;
  }

  GlyphTransform transforms[5];
  int16_t cursor = (DISPLAY_W - total_width) / 2;
  uint8_t visual_index = 0;
  for (uint8_t index = first; index < 5; ++index, ++visual_index) {
    const int16_t width = values[index] < 0
        ? projected_colon_width : projected_digit_width;
    transforms[index] = prv_glyph_transform(
        cursor + width / 2, visual_index, scale);
    cursor += width + gap;
  }

  GBitmap *framebuffer = graphics_capture_frame_buffer(ctx);
  if (!framebuffer) {
    return;
  }
  for (int8_t depth = 4; depth >= 1; --depth) {
    visual_index = 0;
    for (uint8_t index = first; index < 5; ++index, ++visual_index) {
      prv_draw_doubloon_glyph(framebuffer, &transforms[index], values[index],
                              visual_index, depth, scale);
    }
  }
  visual_index = 0;
  for (uint8_t index = first; index < 5; ++index, ++visual_index) {
    prv_draw_doubloon_glyph(framebuffer, &transforms[index], values[index],
                            visual_index, 0, scale);
  }
  graphics_release_frame_buffer(ctx, framebuffer);
}

static void prv_draw_smooth_glyph(GContext *ctx,
                                  const GlyphTransform *transform,
                                  const SmoothGlyph *glyph,
                                  uint8_t depth_layer) {
  if (!glyph->width || !glyph->height) {
    return;
  }
  const int16_t left = -(int16_t)glyph->width / 2;
  const int16_t top = -(int16_t)glyph->height / 2;
  if (depth_layer) {
    graphics_context_set_stroke_color(ctx,
                                      prv_clock_side_color(depth_layer));
  }
  for (int16_t y = 0; y < glyph->height; ++y) {
    const uint8_t count = glyph->run_count[y];
    if (!count) {
      continue;
    }

    // Refraction varies mostly by depth, so one refracted origin per scanline
    // is visually equivalent at Emery resolution. The previous renderer did
    // two full wave transforms for every span and re-scanned the entire mask
    // on every depth pass, which made 49-60 px system fonts miss frame budget.
    const GPoint origin = prv_transform_clock_point(
        transform, 0, top + y, depth_layer);
    for (uint8_t run = 0; run < count; ++run) {
      const int16_t x0 = left + glyph->runs[y][run][0];
      const int16_t x1 = left + glyph->runs[y][run][1] - 1;
      const GPoint start = GPoint(
          origin.x + (((int32_t)x0 * transform->x_scale_q8) >> 8),
          origin.y + (((int32_t)x0 * transform->roll_q8) >> 8));
      const GPoint end = GPoint(
          origin.x + (((int32_t)x1 * transform->x_scale_q8) >> 8),
          origin.y + (((int32_t)x1 * transform->roll_q8) >> 8));
      if (!depth_layer) {
        graphics_context_set_stroke_color(ctx, prv_clock_caustic_color(
            GPoint((start.x + end.x) / 2, (start.y + end.y) / 2)));
      }
      graphics_draw_line(ctx, start, end);
    }
  }
}

static void prv_draw_smooth_time(GContext *ctx) {
  int8_t values[SMOOTH_GLYPH_COUNT];
  const uint8_t first = prv_time_values(values);
  const uint16_t scale = prv_time_effective_scale();
  const int16_t gap = prv_clamp_i16(
      (3 * scale + 50) / 100, 2, 7);
  int16_t total_width = -gap;
  for (uint8_t index = first; index < SMOOTH_GLYPH_COUNT; ++index) {
    total_width += (s_smooth_glyphs[index].width * scale + 50) / 100
        + gap;
  }

  GlyphTransform transforms[SMOOTH_GLYPH_COUNT];
  int16_t cursor = (DISPLAY_W - total_width) / 2;
  uint8_t visual_index = 0;
  for (uint8_t index = first; index < SMOOTH_GLYPH_COUNT;
       ++index, ++visual_index) {
    const int16_t width =
        (s_smooth_glyphs[index].width * scale + 50) / 100;
    transforms[index] = prv_glyph_transform(
        cursor + width / 2, visual_index, scale);
    cursor += width + gap;
  }

  graphics_context_set_stroke_width(ctx,
      scale >= 190 ? 3 : (scale >= 130 ? 2 : 1));
  const int8_t depth_step = scale >= 170 ? 3 : (scale >= 125 ? 2 : 1);
  for (int8_t depth = 3; depth >= 1; depth -= depth_step) {
    for (uint8_t index = first; index < SMOOTH_GLYPH_COUNT; ++index) {
      prv_draw_smooth_glyph(
          ctx, &transforms[index], &s_smooth_glyphs[index], depth);
    }
  }
  for (uint8_t index = first; index < SMOOTH_GLYPH_COUNT; ++index) {
    prv_draw_smooth_glyph(
        ctx, &transforms[index], &s_smooth_glyphs[index], 0);
  }
  graphics_context_set_stroke_width(ctx, 1);
}

static void prv_draw_time(GContext *ctx) {
  if (s_time_style == TIME_STYLE_DOUBLOON) {
    if (s_doubloon_loaded) {
      prv_draw_doubloon_time(ctx);
      return;
    }
  }
  if (s_time_style == TIME_STYLE_SMOOTH_GLASS) {
    prv_draw_smooth_time(ctx);
    return;
  }
  prv_draw_vector_time(ctx);
}

static GFont prv_font_for_size(uint8_t size, bool bold) {
  if (size >= 42) {
    return fonts_get_system_font(
        bold ? FONT_KEY_BITHAM_42_BOLD : FONT_KEY_BITHAM_42_LIGHT);
  }
  if (size >= 28) {
    return fonts_get_system_font(
        bold ? FONT_KEY_GOTHIC_28_BOLD : FONT_KEY_GOTHIC_28);
  }
  if (size >= 24) {
    return fonts_get_system_font(
        bold ? FONT_KEY_GOTHIC_24_BOLD : FONT_KEY_GOTHIC_24);
  }
  if (size >= 18) {
    return fonts_get_system_font(
        bold ? FONT_KEY_GOTHIC_18_BOLD : FONT_KEY_GOTHIC_18);
  }
  return fonts_get_system_font(
      bold ? FONT_KEY_GOTHIC_14_BOLD : FONT_KEY_GOTHIC_14);
}

static uint8_t prv_next_smaller_date_size(uint8_t size) {
  if (size >= 42) return 28;
  if (size >= 28) return 24;
  if (size >= 24) return 18;
  return 14;
}

static uint8_t prv_fitting_date_size(void) {
  uint8_t size = s_date_size;
  while (true) {
    GFont font = prv_font_for_size(size, s_date_bold);
    const GSize measured = graphics_text_layout_get_content_size(
        s_date_text, font, GRect(0, 0, 400, 56),
        GTextOverflowModeFill, GTextAlignmentCenter);
    if (measured.w <= DISPLAY_W - 4 || size == 14) {
      return size;
    }
    size = prv_next_smaller_date_size(size);
  }
}

static GFont prv_fitting_date_font(void) {
  return prv_font_for_size(prv_fitting_date_size(), s_date_bold);
}

static void prv_draw_date(GContext *ctx) {
  if (s_date_format == DATE_FORMAT_HIDDEN || s_date_text[0] == '\0') {
    return;
  }
  const int16_t bob_x = prv_wave_smooth((s_animation_ms * 256) / 530
      + (7 << 8)) / 6;
  const int16_t bob_y = prv_wave_smooth((s_animation_ms * 256) / 610
      + (15 << 8)) / 10;
  const int16_t source_height = prv_time_source_height();
  const uint16_t scale = prv_time_effective_scale();
  const int16_t time_half_height =
      (source_height * scale + 100) / 200;
  const int16_t y = prv_clock_center_y() + time_half_height + 5 + bob_y;
  const uint8_t fitted_size = prv_fitting_date_size();
  const int16_t height = fitted_size + 10;
  const GRect shadow_rect = GRect(bob_x - 1, y + 1, DISPLAY_W, height);
  const GRect face_rect = GRect(bob_x, y, DISPLAY_W, height);
  const GFont font = prv_fitting_date_font();
  graphics_context_set_text_color(ctx, GColorFromRGB(0, 0, 85));
  graphics_draw_text(ctx, s_date_text,
      font, shadow_rect,
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  graphics_context_set_text_color(ctx,
      prv_clock_caustic_color(GPoint(DISPLAY_W / 2, y + 7)));
  graphics_draw_text(ctx, s_date_text,
      font, face_rect,
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static GColor prv_weather_background_color(bool upper) {
  switch (s_weather_background) {
    case 1:
      return upper ? GColorFromRGB(85, 85, 85) : GColorBlack;
    case 2:
      return upper ? GColorFromRGB(0, 170, 170)
                   : GColorFromRGB(0, 85, 85);
    case 3:
      return upper ? GColorWhite : GColorFromRGB(170, 255, 255);
    case 4:
      return upper ? GColorFromRGB(170, 0, 85)
                   : GColorFromRGB(85, 0, 0);
    default:
      return upper ? GColorFromRGB(0, 85, 170)
                   : GColorFromRGB(0, 0, 85);
  }
}

static GColor prv_weather_foreground_color(void) {
  switch (s_weather_foreground) {
    case 1:
      return GColorFromRGB(255, 255, 85);
    case 2:
      return GColorBlack;
    case 3:
      return GColorFromRGB(170, 255, 255);
    default:
      return GColorWhite;
  }
}

static void prv_draw_weather_icon(GContext *ctx, GPoint center,
                                  int16_t radius, GColor primary,
                                  GColor accent) {
  const WeatherKind kind = prv_weather_kind();
  graphics_context_set_stroke_width(ctx, 1);
  if (kind == WEATHER_CLEAR) {
    graphics_context_set_fill_color(ctx, accent);
    graphics_fill_circle(ctx, center, radius - 1);
    graphics_context_set_stroke_color(ctx, primary);
    graphics_draw_line(ctx, GPoint(center.x - radius - 2, center.y),
                       GPoint(center.x - radius - 1, center.y));
    graphics_draw_line(ctx, GPoint(center.x + radius + 1, center.y),
                       GPoint(center.x + radius + 2, center.y));
    graphics_draw_line(ctx, GPoint(center.x, center.y - radius - 2),
                       GPoint(center.x, center.y - radius - 1));
    graphics_draw_line(ctx, GPoint(center.x, center.y + radius + 1),
                       GPoint(center.x, center.y + radius + 2));
  } else if (kind == WEATHER_CLOUD || kind == WEATHER_RAIN
             || kind == WEATHER_SNOW || kind == WEATHER_STORM) {
    graphics_context_set_fill_color(ctx, primary);
    graphics_fill_circle(ctx, GPoint(center.x - radius / 2, center.y), radius);
    graphics_fill_circle(ctx, GPoint(center.x + radius / 2,
                                     center.y + 1), radius);
    graphics_fill_rect(ctx, GRect(center.x - radius - 2, center.y,
                                  radius * 2 + 4, radius),
                       0, GCornerNone);
    graphics_context_set_stroke_color(ctx, accent);
    if (kind == WEATHER_RAIN || kind == WEATHER_STORM) {
      graphics_draw_line(ctx, GPoint(center.x - radius / 2,
                                     center.y + radius + 1),
                         GPoint(center.x - radius / 2 - 1,
                                center.y + radius + 3));
      graphics_draw_line(ctx, GPoint(center.x + radius / 2 + 1,
                                     center.y + radius + 1),
                         GPoint(center.x + radius / 2,
                                center.y + radius + 3));
    } else if (kind == WEATHER_SNOW) {
      graphics_draw_pixel(ctx, GPoint(center.x - radius / 2,
                                      center.y + radius + 2));
      graphics_draw_pixel(ctx, GPoint(center.x + radius / 2,
                                      center.y + radius + 3));
    }
  } else if (kind == WEATHER_FOG) {
    graphics_context_set_stroke_color(ctx, primary);
    for (int8_t row = -3; row <= 3; row += 3) {
      graphics_draw_line(ctx, GPoint(center.x - radius - 2, center.y + row),
                         GPoint(center.x + radius + 2, center.y + row));
    }
  } else {
    graphics_context_set_stroke_color(ctx, primary);
    graphics_draw_circle(ctx, center, radius);
    graphics_draw_pixel(ctx, center);
  }
}

static void prv_draw_battery_buoy(GContext *ctx) {
  const int16_t center_x = 42 + prv_wave_smooth(
      (s_animation_ms * 256) / 520 + (3 << 8)) / 12;
  const int16_t surface_y = prv_surface_y_at(center_x);
  const int16_t body_top = surface_y - 4;
  const int16_t tower_top = body_top - 18;
  const int16_t tilt = prv_wave_smooth(
      (s_animation_ms * 256) / 730 + (9 << 8)) / 8;
#if !AQUA_VIEWER_APP
  char label[8];
  snprintf(label, sizeof(label), "%d%%", s_battery_percent);
#endif

  // The label housing straddles the water and flares toward its weighted base.
  // It is intentionally not clamped to the display top: at a full battery the
  // tower and lamp may continue naturally beyond the screen.
  graphics_context_set_stroke_width(ctx, 1);
  for (int16_t row = 0; row < 23; ++row) {
    const int16_t half_width = 16 + row / 6;
    const int16_t row_lean = (tilt * (11 - row)) / 18;
    graphics_context_set_stroke_color(ctx, row > 17
        ? GColorFromRGB(170, 0, 0) : GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx,
        GPoint(center_x - half_width + row_lean, body_top + row),
        GPoint(center_x + half_width + row_lean, body_top + row));
  }
  graphics_context_set_stroke_color(ctx, GColorFromRGB(255, 85, 85));
  graphics_draw_line(ctx, GPoint(center_x - 13, body_top + 1),
                     GPoint(center_x - 17, body_top + 16));
  graphics_context_set_stroke_color(ctx, GColorFromRGB(85, 0, 0));
  graphics_draw_line(ctx, GPoint(center_x + 15, body_top + 2),
                     GPoint(center_x + 19, body_top + 18));
#if !AQUA_VIEWER_APP
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
      GRect(center_x - 22, body_top, 44, 22),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
#endif

  // Open navigation-marker lattice: two tapered rails with alternating cross
  // braces.  Offset the lamp and upper rail by tilt so the whole buoy rolls
  // gently with the sampled surface wave.
  const int16_t upper_center = center_x + tilt;
  graphics_context_set_stroke_width(ctx, 2);
  graphics_context_set_stroke_color(ctx, GColorFromRGB(255, 0, 0));
  graphics_draw_line(ctx, GPoint(center_x - 12, body_top),
                     GPoint(upper_center - 3, tower_top));
  graphics_draw_line(ctx, GPoint(center_x + 12, body_top),
                     GPoint(upper_center + 3, tower_top));
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, GColorFromRGB(255, 85, 85));
  graphics_draw_line(ctx, GPoint(center_x - 10, body_top - 2),
                     GPoint(center_x + 7, body_top - 9));
  graphics_draw_line(ctx, GPoint(center_x + 10, body_top - 2),
                     GPoint(center_x - 7, body_top - 9));
  graphics_draw_line(ctx, GPoint(center_x - 7, body_top - 10),
                     GPoint(upper_center + 3, tower_top + 2));
  graphics_draw_line(ctx, GPoint(center_x + 7, body_top - 10),
                     GPoint(upper_center - 3, tower_top + 2));

  graphics_context_set_fill_color(ctx, GColorFromRGB(170, 0, 0));
  graphics_fill_circle(ctx, GPoint(upper_center, tower_top - 5), 7);
  graphics_context_set_fill_color(ctx, GColorFromRGB(255, 0, 0));
  graphics_fill_circle(ctx, GPoint(upper_center, tower_top - 6), 5);
  graphics_context_set_fill_color(ctx, GColorFromRGB(255, 170, 170));
  graphics_fill_circle(ctx, GPoint(upper_center - 2, tower_top - 8), 1);

  graphics_context_set_stroke_color(ctx, GColorFromRGB(85, 85, 0));
  graphics_draw_line(ctx, GPoint(center_x, body_top + 22),
                     GPoint(center_x + 1, body_top + 29));
}

static void prv_draw_temp_unit(GContext *ctx, int16_t x, int16_t y,
                               GColor color) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 1);
  if (s_weather_unit == 'F') {
    graphics_draw_line(ctx, GPoint(x, y), GPoint(x, y + 5));
    graphics_draw_line(ctx, GPoint(x, y), GPoint(x + 3, y));
    graphics_draw_line(ctx, GPoint(x, y + 2), GPoint(x + 2, y + 2));
  } else {
    graphics_draw_line(ctx, GPoint(x + 1, y), GPoint(x + 3, y));
    graphics_draw_line(ctx, GPoint(x, y + 1), GPoint(x, y + 4));
    graphics_draw_line(ctx, GPoint(x + 1, y + 5), GPoint(x + 3, y + 5));
  }
}

static void prv_draw_weather_buoy(GContext *ctx) {
  const int16_t body_width = s_weather_size >= 24 ? 78
      : (s_weather_size >= 18 ? 64 : 52);
  const int16_t body_height = s_weather_size + 6;
  const int16_t center_x = DISPLAY_W - body_width / 2 - 2
      + prv_wave_smooth(
      (s_animation_ms * 256) / 590 + (17 << 8)) / 12;
  const int16_t surface_y = prv_surface_y_at(center_x);
  const int16_t top = prv_clamp_i16(
      surface_y - 4, 0, DISPLAY_H - body_height - 1);
  const int16_t left = center_x - body_width / 2;
  const int16_t icon_width = s_weather_size + 8;
  const int16_t icon_radius = s_weather_size >= 24 ? 5
      : (s_weather_size >= 18 ? 4 : 3);
  const GColor background = prv_weather_background_color(false);
  const GColor upper = prv_weather_background_color(true);
  const GColor foreground = prv_weather_foreground_color();
  const GColor contrast = s_weather_foreground == 2
      ? GColorWhite : GColorBlack;
  char temp[10];
  if (s_weather_valid) {
    snprintf(temp, sizeof(temp), "%d\xC2\xB0", s_weather_temp);
  } else {
    snprintf(temp, sizeof(temp), "--\xC2\xB0");
  }

  graphics_context_set_fill_color(ctx, background);
  graphics_fill_rect(ctx, GRect(left, top + 2, body_width, body_height - 1),
                     4, GCornersBottom);
  graphics_context_set_fill_color(ctx, upper);
  graphics_fill_rect(ctx, GRect(left + 2, top + 1,
                                body_width - 4, body_height - 6),
                     3, GCornersTop);
  graphics_context_set_stroke_color(ctx, foreground);
  graphics_draw_line(ctx, GPoint(left + 4, top + 1),
                     GPoint(left + body_width - 5, top + 1));
  const GPoint icon_center = GPoint(
      left + icon_width / 2, top + body_height / 2);
  prv_draw_weather_icon(ctx,
      GPoint(icon_center.x + 1, icon_center.y + 1),
      icon_radius, contrast, contrast);
  prv_draw_weather_icon(ctx, icon_center,
      icon_radius, foreground, foreground);
  graphics_context_set_text_color(ctx, contrast);
  graphics_draw_text(ctx, temp, prv_font_for_size(s_weather_size, true),
      GRect(left + icon_width - 1, top,
            body_width - icon_width - 5, body_height + 4),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  graphics_context_set_text_color(ctx, foreground);
  graphics_draw_text(ctx, temp, prv_font_for_size(s_weather_size, true),
      GRect(left + icon_width - 2, top - 1,
            body_width - icon_width - 5, body_height + 4),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  if (s_weather_valid) {
    prv_draw_temp_unit(ctx, left + body_width - 6,
                       top + (body_height - 6) / 2 + 1, contrast);
    prv_draw_temp_unit(ctx, left + body_width - 7,
                       top + (body_height - 6) / 2, foreground);
  }
  graphics_context_set_stroke_color(ctx, GColorFromRGB(85, 85, 0));
  graphics_draw_line(ctx, GPoint(center_x, top + body_height),
                     GPoint(center_x - 1, top + body_height + 6));
}

static void prv_draw_surface_floats(GContext *ctx) {
  prv_draw_battery_buoy(ctx);
#if !AQUA_VIEWER_APP
  prv_draw_weather_buoy(ctx);
#endif
  graphics_context_set_stroke_width(ctx, 1);
}

static void prv_draw_bubbles(GContext *ctx) {
  for (int i = 0; i < BUBBLE_COUNT; ++i) {
    const Bubble *bubble = &s_bubbles[i];
    const int16_t sway = prv_wave_smooth((s_animation_ms * 256) / 120
        + ((uint32_t)bubble->phase << 8)) / 5;
    const int16_t x = (bubble->x_q8 >> 8) + sway;
    const int16_t y = bubble->y_q8 >> 8;
    if (y <= prv_surface_y_at(x) + 1) {
      continue;
    }
    graphics_context_set_stroke_color(ctx, GColorFromRGB(85, 170, 170));
    if (bubble->size == 1) {
      graphics_draw_pixel(ctx, GPoint(x, y));
    } else {
      graphics_draw_circle(ctx, GPoint(x, y), bubble->size);
      graphics_context_set_stroke_color(ctx, GColorFromRGB(170, 255, 255));
      graphics_draw_pixel(ctx, GPoint(x - 1, y - bubble->size + 1));
    }
  }
}

static void prv_draw_framebuffer_scene(GContext *ctx) {
  GBitmap *framebuffer = graphics_capture_frame_buffer(ctx);
  if (!framebuffer) {
    return;
  }
  prv_draw_air_volume(framebuffer);
  prv_draw_bottom_caustics(framebuffer);
  prv_draw_plant_pass(framebuffer, false);
  prv_draw_fish(framebuffer);
  prv_draw_plant_pass(framebuffer, true);
  graphics_release_frame_buffer(ctx, framebuffer);
}

static void prv_scene_update_proc(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
#if !AQUA_VIEWER_APP
  // Native fonts are rasterized only when the minute, font, or time-format
  // changes. The normal scene draw immediately covers this temporary staging
  // area, leaving animation frames to transform compact cached row spans.
  prv_prepare_smooth_glyphs(ctx);
#endif
  if (s_background) {
    graphics_draw_bitmap_in_rect(ctx, s_background, bounds);
  } else {
    graphics_context_set_fill_color(ctx, GColorFromRGB(0, 85, 85));
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  }

  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  prv_draw_framebuffer_scene(ctx);
  prv_draw_bubbles(ctx);
#if !AQUA_VIEWER_APP
  prv_draw_time(ctx);
  prv_draw_date(ctx);
#endif
  prv_draw_surface(ctx);
  prv_draw_surface_floats(ctx);
}

static void prv_schedule_animation(void);

static bool prv_animation_allowed(void) {
#if AQUA_VIEWER_APP
  return s_app_focused;
#else
  return s_backlight_on;
#endif
}

static uint16_t prv_elapsed_since_frame(void) {
  time_t seconds;
  uint16_t milliseconds;
  time_ms(&seconds, &milliseconds);
  uint16_t elapsed = ANIMATION_FRAME_MS;
  if (s_frame_clock_valid) {
    int32_t measured = (seconds - s_last_frame_seconds) * 1000
        + (int32_t)milliseconds - s_last_frame_milliseconds;
    elapsed = prv_clamp_i16(measured, 40, 500);
  }
  s_last_frame_seconds = seconds;
  s_last_frame_milliseconds = milliseconds;
  s_frame_clock_valid = true;
  return elapsed;
}

static void prv_animation_timer_handler(void *context) {
  (void)context;
  s_animation_timer = NULL;
  if (!prv_animation_allowed()) {
    return;
  }
  prv_update_world(prv_elapsed_since_frame());
  if (s_scene_layer) {
    layer_mark_dirty(s_scene_layer);
  }
  prv_schedule_animation();
}

static void prv_schedule_animation(void) {
  if (!prv_animation_allowed() || s_animation_timer || !s_scene_layer) {
    return;
  }
  s_animation_timer = app_timer_register(
      ANIMATION_FRAME_MS, prv_animation_timer_handler, NULL);
}

static void prv_backlight_changed(bool on) {
  s_backlight_on = on;
  if (!on) {
    s_frame_clock_valid = false;
    if (s_animation_timer) {
      app_timer_cancel(s_animation_timer);
      s_animation_timer = NULL;
    }
    return;
  }
  s_frame_clock_valid = false;
  if (s_scene_layer) {
    layer_mark_dirty(s_scene_layer);
  }
  prv_schedule_animation();
}

#if AQUA_VIEWER_APP
static void prv_set_app_focus(bool in_focus) {
  s_app_focused = in_focus;
  s_frame_clock_valid = false;
  if (!in_focus) {
    if (s_animation_timer) {
      app_timer_cancel(s_animation_timer);
      s_animation_timer = NULL;
    }
    return;
  }
  if (s_scene_layer) {
    layer_mark_dirty(s_scene_layer);
  }
  prv_schedule_animation();
}

static void prv_focus_will_change(bool in_focus) {
  // Stop before a notification or other system window starts covering the
  // aquarium. Resume is deliberately deferred until the app is fully visible.
  if (!in_focus) {
    prv_set_app_focus(false);
  }
}

static void prv_focus_did_change(bool in_focus) {
  // Pebble sends this initial true event after the app launch animation. Using
  // only the legacy will-focus callback could cancel the launch timer without
  // ever receiving a matching resume event.
  prv_set_app_focus(in_focus);
}
#endif

static void prv_battery_changed(BatteryChargeState state) {
  const int16_t old_water_y = s_water_y;
  s_battery_percent = prv_clamp_i16(state.charge_percent, 0, 100);
  s_water_y = prv_water_y_for_battery(s_battery_percent);
  prv_update_surface_profile();
  if (s_scene_layer && old_water_y != s_water_y) {
    prv_remap_fish_for_waterline(old_water_y, s_water_y);
    for (int i = 0; i < BUBBLE_COUNT; ++i) {
      if ((s_bubbles[i].y_q8 >> 8)
          <= prv_surface_y_at(s_bubbles[i].x_q8 >> 8) + 2) {
        prv_reset_bubble(&s_bubbles[i], false);
      }
    }
  }
  if (s_scene_layer) {
    layer_mark_dirty(s_scene_layer);
  }
}

static void prv_load_weather_cache(void) {
  if (!persist_exists(PERSIST_WEATHER_TEMP)
      || !persist_exists(PERSIST_WEATHER_CODE)) {
    return;
  }
  s_weather_temp = persist_read_int(PERSIST_WEATHER_TEMP);
  s_weather_code = persist_read_int(PERSIST_WEATHER_CODE);
  s_weather_is_day = persist_exists(PERSIST_WEATHER_DAY)
      ? persist_read_int(PERSIST_WEATHER_DAY) : 1;
  s_weather_unit = persist_exists(PERSIST_WEATHER_UNIT)
      ? persist_read_int(PERSIST_WEATHER_UNIT) : 'C';
  s_sky_phase = persist_exists(PERSIST_WEATHER_SKY_PHASE)
      ? prv_clamp_i16(persist_read_int(PERSIST_WEATHER_SKY_PHASE),
                      SKY_MORNING, SKY_NIGHT)
      : (s_weather_is_day ? SKY_NOON : SKY_NIGHT);
  s_weather_valid = true;
}

static uint8_t prv_weather_font_size_setting(int32_t value,
                                              uint8_t fallback) {
  return value == 14 || value == 18 || value == 24 ? value : fallback;
}

static uint8_t prv_date_font_size_setting(int32_t value, uint8_t fallback) {
  return value == 14 || value == 18 || value == 24
      || value == 28 || value == 42 ? value : fallback;
}

static void prv_load_settings(void) {
  if (persist_exists(PERSIST_TIME_SCALE)) {
    s_time_scale = prv_clamp_i16(
        persist_read_int(PERSIST_TIME_SCALE), 80, 220);
  }
  if (persist_exists(PERSIST_TIME_STYLE)) {
    s_time_style = prv_clamp_i16(
        persist_read_int(PERSIST_TIME_STYLE), 0, TIME_STYLE_COUNT - 1);
  }
  if (persist_exists(PERSIST_TIME_FORMAT)) {
    s_time_format = prv_clamp_i16(
        persist_read_int(PERSIST_TIME_FORMAT), 0, TIME_FORMAT_COUNT - 1);
  }
  if (persist_exists(PERSIST_TIME_FONT)) {
    s_time_font = prv_clamp_i16(
        persist_read_int(PERSIST_TIME_FONT), 0, TIME_FONT_COUNT - 1);
  }
  if (persist_exists(PERSIST_GOLD_BRIGHTNESS)) {
    s_gold_brightness = prv_clamp_i16(
        persist_read_int(PERSIST_GOLD_BRIGHTNESS), 70, 180);
  }
  if (persist_exists(PERSIST_DATE_SIZE)) {
    s_date_size = prv_date_font_size_setting(
        persist_read_int(PERSIST_DATE_SIZE), 14);
  }
  if (persist_exists(PERSIST_DATE_BOLD)) {
    s_date_bold = persist_read_int(PERSIST_DATE_BOLD) != 0;
  }
  if (persist_exists(PERSIST_DATE_FORMAT)) {
    s_date_format = prv_clamp_i16(
        persist_read_int(PERSIST_DATE_FORMAT), 0, DATE_FORMAT_COUNT - 1);
  }
  if (persist_exists(PERSIST_WEATHER_SIZE)) {
    s_weather_size = prv_weather_font_size_setting(
        persist_read_int(PERSIST_WEATHER_SIZE), 18);
  }
  if (persist_exists(PERSIST_WEATHER_BG)) {
    s_weather_background = prv_clamp_i16(
        persist_read_int(PERSIST_WEATHER_BG), 0, 4);
  }
  if (persist_exists(PERSIST_WEATHER_FG)) {
    s_weather_foreground = prv_clamp_i16(
        persist_read_int(PERSIST_WEATHER_FG), 0, 3);
  }
}

static void prv_weather_received(DictionaryIterator *iterator, void *context) {
  (void)context;
  Tuple *temperature = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMP);
  Tuple *code = dict_find(iterator, MESSAGE_KEY_WEATHER_CODE);
  Tuple *is_day = dict_find(iterator, MESSAGE_KEY_WEATHER_IS_DAY);
  Tuple *unit = dict_find(iterator, MESSAGE_KEY_WEATHER_UNIT);
  Tuple *sky_phase = dict_find(iterator, MESSAGE_KEY_WEATHER_SKY_PHASE);
  Tuple *time_scale = dict_find(iterator, MESSAGE_KEY_SETTINGS_TIME_SCALE);
  Tuple *date_size = dict_find(iterator, MESSAGE_KEY_SETTINGS_DATE_SIZE);
  Tuple *weather_size = dict_find(iterator, MESSAGE_KEY_SETTINGS_WEATHER_SIZE);
  Tuple *weather_bg = dict_find(iterator, MESSAGE_KEY_SETTINGS_WEATHER_BG);
  Tuple *weather_fg = dict_find(iterator, MESSAGE_KEY_SETTINGS_WEATHER_FG);
  Tuple *date_bold = dict_find(iterator, MESSAGE_KEY_SETTINGS_DATE_BOLD);
  Tuple *date_format = dict_find(iterator, MESSAGE_KEY_SETTINGS_DATE_FORMAT);
  Tuple *time_style = dict_find(iterator, MESSAGE_KEY_SETTINGS_TIME_STYLE);
  Tuple *time_format = dict_find(iterator, MESSAGE_KEY_SETTINGS_TIME_FORMAT);
  Tuple *time_font = dict_find(iterator, MESSAGE_KEY_SETTINGS_TIME_FONT);
  Tuple *gold_brightness = dict_find(
      iterator, MESSAGE_KEY_SETTINGS_GOLD_BRIGHTNESS);
  bool changed = false;

  if (temperature && code) {
    s_weather_temp = prv_clamp_i16(temperature->value->int32, -99, 199);
    s_weather_code = prv_clamp_i16(code->value->int32, 0, 255);
    if (is_day) {
      s_weather_is_day = is_day->value->int32 != 0;
    }
    if (unit) {
      s_weather_unit = unit->value->int32 == 'F' ? 'F' : 'C';
    }
    s_weather_valid = true;
    persist_write_int(PERSIST_WEATHER_TEMP, s_weather_temp);
    persist_write_int(PERSIST_WEATHER_CODE, s_weather_code);
    persist_write_int(PERSIST_WEATHER_DAY, s_weather_is_day);
    persist_write_int(PERSIST_WEATHER_UNIT, s_weather_unit);
    changed = true;
  }

  if (sky_phase) {
    s_sky_phase = prv_clamp_i16(
        sky_phase->value->int32, SKY_MORNING, SKY_NIGHT);
    persist_write_int(PERSIST_WEATHER_SKY_PHASE, s_sky_phase);
    changed = true;
  }
  if (time_scale) {
    s_time_scale = prv_clamp_i16(time_scale->value->int32, 80, 220);
    persist_write_int(PERSIST_TIME_SCALE, s_time_scale);
    changed = true;
  }
  if (time_style) {
    s_time_style = prv_clamp_i16(
        time_style->value->int32, 0, TIME_STYLE_COUNT - 1);
    persist_write_int(PERSIST_TIME_STYLE, s_time_style);
    s_smooth_cache_dirty = true;
    changed = true;
  }
  if (time_format) {
    s_time_format = prv_clamp_i16(
        time_format->value->int32, 0, TIME_FORMAT_COUNT - 1);
    persist_write_int(PERSIST_TIME_FORMAT, s_time_format);
    s_smooth_cache_dirty = true;
    changed = true;
  }
  if (time_font) {
    s_time_font = prv_clamp_i16(
        time_font->value->int32, 0, TIME_FONT_COUNT - 1);
    persist_write_int(PERSIST_TIME_FONT, s_time_font);
    s_smooth_cache_dirty = true;
    changed = true;
  }
  if (gold_brightness) {
    s_gold_brightness = prv_clamp_i16(
        gold_brightness->value->int32, 70, 180);
    persist_write_int(PERSIST_GOLD_BRIGHTNESS, s_gold_brightness);
    changed = true;
  }
  if (date_size) {
    s_date_size = prv_date_font_size_setting(
        date_size->value->int32, s_date_size);
    persist_write_int(PERSIST_DATE_SIZE, s_date_size);
    changed = true;
  }
  if (date_bold) {
    s_date_bold = date_bold->value->int32 != 0;
    persist_write_int(PERSIST_DATE_BOLD, s_date_bold);
    changed = true;
  }
  if (date_format) {
    s_date_format = prv_clamp_i16(
        date_format->value->int32, 0, DATE_FORMAT_COUNT - 1);
    persist_write_int(PERSIST_DATE_FORMAT, s_date_format);
    time_t now = time(NULL);
    prv_update_date_text(localtime(&now));
    changed = true;
  }
  if (weather_size) {
    s_weather_size = prv_weather_font_size_setting(
        weather_size->value->int32, s_weather_size);
    persist_write_int(PERSIST_WEATHER_SIZE, s_weather_size);
    changed = true;
  }
  if (weather_bg) {
    s_weather_background = prv_clamp_i16(
        weather_bg->value->int32, 0, 4);
    persist_write_int(PERSIST_WEATHER_BG, s_weather_background);
    changed = true;
  }
  if (weather_fg) {
    s_weather_foreground = prv_clamp_i16(
        weather_fg->value->int32, 0, 3);
    persist_write_int(PERSIST_WEATHER_FG, s_weather_foreground);
    changed = true;
  }
  if (changed && s_scene_layer) {
    layer_mark_dirty(s_scene_layer);
  }
}

static void prv_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  (void)units_changed;
#if AQUA_VIEWER_APP
  // Open-Meteo supplies sunrise-aware phases when available. Local time is a
  // graceful fallback when location or networking is unavailable.
  if (!s_weather_valid) {
    s_sky_phase = prv_sky_phase_for_local_hour(tick_time->tm_hour);
  }
#else
  prv_set_clock(tick_time);
#endif
  if (s_scene_layer) {
    layer_mark_dirty(s_scene_layer);
  }
}

static void prv_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(root_layer);

#if AQUA_VIEWER_APP
  // The viewer is a decorative aquarium rather than a battery gauge. Its
  // water volume is permanently the same as the watchface at 40% charge.
  s_battery_percent = VIEWER_WATER_PERCENT;
#else
  const BatteryChargeState battery = battery_state_service_peek();
  s_battery_percent = prv_clamp_i16(battery.charge_percent, 0, 100);
#endif
  s_water_y = prv_water_y_for_battery(s_battery_percent);
  prv_init_floor_horizon();
  prv_update_surface_profile();
  prv_load_weather_cache();
#if !AQUA_VIEWER_APP
  prv_load_settings();
#endif

  s_background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TANK_BACKGROUND);
  prv_load_species(SPECIES_BUTTERFLY, RESOURCE_ID_IMAGE_FISH_BUTTERFLY, 56, 46);
  prv_load_species(SPECIES_GOURAMI, RESOURCE_ID_IMAGE_FISH_GOURAMI, 56, 42);
  prv_load_species(SPECIES_CLOWN, RESOURCE_ID_IMAGE_FISH_CLOWN, 52, 34);
  prv_load_species(SPECIES_TETRA, RESOURCE_ID_IMAGE_FISH_TETRA, 50, 28);
  s_caustic_frames_resource = resource_get_handle(
      RESOURCE_ID_RAW_CAUSTIC_FRAMES);
  s_loaded_caustic_frame[0] = -1;
  s_loaded_caustic_frame[1] = -1;
  s_caustic_noise = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CAUSTIC_NOISE);
#if !AQUA_VIEWER_APP
  s_doubloon_loaded = resource_load(
      resource_get_handle(RESOURCE_ID_RAW_DOUBLOON_DIGITS),
      s_doubloon_packed, sizeof(s_doubloon_packed))
      == sizeof(s_doubloon_packed);
#endif
  s_plant_animation_resource = resource_get_handle(
      RESOURCE_ID_RAW_PLANT_ANIMATION);
  s_loaded_plant_frame = -1;
  s_smooth_cache_dirty = true;
  prv_init_fish();
  prv_init_bubbles();

  s_scene_layer = layer_create(bounds);
  layer_set_update_proc(s_scene_layer, prv_scene_update_proc);
  layer_add_child(root_layer, s_scene_layer);

  time_t now = time(NULL);
  struct tm *local_now = localtime(&now);
  if (!s_weather_valid) {
    s_sky_phase = prv_sky_phase_for_local_hour(local_now->tm_hour);
  }
#if !AQUA_VIEWER_APP
  prv_set_clock(local_now);
  s_backlight_on = light_is_on();
#endif
  s_frame_clock_valid = false;
  prv_schedule_animation();
}

static void prv_window_unload(Window *window) {
  (void)window;
  if (s_animation_timer) {
    app_timer_cancel(s_animation_timer);
    s_animation_timer = NULL;
  }
  if (s_scene_layer) {
    layer_destroy(s_scene_layer);
    s_scene_layer = NULL;
  }
  prv_destroy_species();
  if (s_background) {
    gbitmap_destroy(s_background);
    s_background = NULL;
  }
  s_caustic_frames_resource = NULL;
  s_loaded_caustic_frame[0] = -1;
  s_loaded_caustic_frame[1] = -1;
  if (s_caustic_noise) {
    gbitmap_destroy(s_caustic_noise);
    s_caustic_noise = NULL;
  }
  s_doubloon_loaded = false;
  s_smooth_cache_dirty = true;
}

static void prv_init(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, false);
#if AQUA_VIEWER_APP
  app_focus_service_subscribe_handlers((AppFocusHandlers) {
    .will_focus = prv_focus_will_change,
    .did_focus = prv_focus_did_change,
  });
  tick_timer_service_subscribe(HOUR_UNIT, prv_minute_tick);
#else
  tick_timer_service_subscribe(MINUTE_UNIT, prv_minute_tick);
  backlight_service_subscribe(prv_backlight_changed);
  battery_state_service_subscribe(prv_battery_changed);
#endif
  app_message_register_inbox_received(prv_weather_received);
  app_message_open(256, 64);
}

static void prv_deinit(void) {
  app_message_deregister_callbacks();
#if AQUA_VIEWER_APP
  tick_timer_service_unsubscribe();
  app_focus_service_unsubscribe();
#else
  battery_state_service_unsubscribe();
  backlight_service_unsubscribe();
  tick_timer_service_unsubscribe();
#endif
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
