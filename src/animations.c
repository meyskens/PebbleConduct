#include "animations.h"
#include "ui_manager.h"
#include "data.h"
#include "constants.h"

// Animation duration for page transitions (ms)
#define PAGE_ANIMATION_DURATION 250

// Animation state
static Animation *s_page_animation = NULL;
static bool s_is_animating = false;

// Container layer for main page content
static Layer *s_main_page_container = NULL;

// Container layer for timetable page content
static Layer *s_timetable_page_container = NULL;

// Screen bounds for animation calculations
static GRect s_screen_bounds;

// Lamp animation
static bool s_lamp_blink_state = false;

// Forward declarations
static void lamp_blink_handler(void *context);
static void lamp_bg_update_proc(Layer *layer, GContext *ctx);
static void animation_started_handler(Animation *animation, void *context);
static void animation_stopped_handler(Animation *animation, bool finished, void *context);

static void lamp_blink_handler(void *context) {
  s_lamp_blink_state = !s_lamp_blink_state;
  if (s_lamp_bg_layer) {
    layer_mark_dirty(s_lamp_bg_layer);
  }
  // Schedule next blink in 1 second
  app_timer_register(1000, lamp_blink_handler, NULL);
}

static void lamp_bg_update_proc(Layer *layer, GContext *ctx) {
  // Cycle between bright green and white
  graphics_context_set_fill_color(ctx, s_lamp_blink_state ? GColorGreen : GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void init_animations(void) {
  // Animation containers will be created when window loads
  s_main_page_container = NULL;
  s_timetable_page_container = NULL;
  s_is_animating = false;
}

void set_animation_containers(Layer *main_container, Layer *timetable_container, Layer *lamp_container) {
  s_main_page_container = main_container;
  s_timetable_page_container = timetable_container;
  s_lamp_page_container = lamp_container;
  // Store the screen bounds for animation calculations
  s_screen_bounds = layer_get_bounds(main_container);
}

// Animation complete handler for lamp entry
static void lamp_entry_stopped_handler(Animation *animation, bool finished, void *context) {
  // Animation complete - lamp is now fully visible
}

void start_lamp_animation(void) {
  if (s_lamp_active) return;
  s_lamp_active = true;
  s_lamp_blink_state = true;

  // Turn on backlight and keep it on
  light_enable(true);

  if (!s_lamp_page_container) {
    // Fallback if no lamp container
    return;
  }

  // Show lamp page
  layer_set_hidden(s_lamp_page_container, false);

  // Set up background layer
  if (s_lamp_bg_layer) {
    layer_set_update_proc(s_lamp_bg_layer, lamp_bg_update_proc);
    layer_mark_dirty(s_lamp_bg_layer);
  }

  // Animate lamp page sliding in from the right
  GRect from_frame = s_screen_bounds;
  from_frame.origin.x = s_screen_bounds.size.w; // Start from right edge
  GRect to_frame = s_screen_bounds;
  to_frame.origin.x = 0; // End at normal position

  layer_set_frame(s_lamp_page_container, from_frame);

  PropertyAnimation *entry_anim = property_animation_create_layer_frame(
      s_lamp_page_container, &from_frame, &to_frame);

  animation_set_duration((Animation *)entry_anim, PAGE_ANIMATION_DURATION);
  animation_set_curve((Animation *)entry_anim, AnimationCurveEaseInOut);

  AnimationHandlers handlers = {
    .started = NULL,
    .stopped = lamp_entry_stopped_handler
  };
  animation_set_handlers((Animation *)entry_anim, handlers, NULL);

  animation_schedule((Animation *)entry_anim);

  // Start blinking
  app_timer_register(1000, lamp_blink_handler, NULL);
}

// Animation complete handler for lamp exit
static void lamp_exit_stopped_handler(Animation *animation, bool finished, void *context) {
  // Hide lamp page after animation
  if (s_lamp_page_container) {
    layer_set_hidden(s_lamp_page_container, true);
    // Reset position
    GRect frame = layer_get_frame(s_lamp_page_container);
    frame.origin.x = 0;
    layer_set_frame(s_lamp_page_container, frame);
  }
  // Turn off backlight
  light_enable(false);
}

void stop_lamp_animation(void) {
  if (!s_lamp_active) return;
  s_lamp_active = false;
  
  if (!s_lamp_page_container) {
    // Fallback if no lamp container
    light_enable(false);
    return;
  }
  
  // Animate lamp page sliding out to the right
  GRect from_frame = layer_get_frame(s_lamp_page_container);
  GRect to_frame = from_frame;
  to_frame.origin.x = s_screen_bounds.size.w; // Move to right edge
  
  PropertyAnimation *exit_anim = property_animation_create_layer_frame(
      s_lamp_page_container, &from_frame, &to_frame);
  
  animation_set_duration((Animation *)exit_anim, PAGE_ANIMATION_DURATION);
  animation_set_curve((Animation *)exit_anim, AnimationCurveEaseInOut);
  
  AnimationHandlers handlers = {
    .started = NULL,
    .stopped = lamp_exit_stopped_handler
  };
  animation_set_handlers((Animation *)exit_anim, handlers, NULL);
  
  animation_schedule((Animation *)exit_anim);
}

bool is_lamp_active(void) {
  return s_lamp_active;
}

bool is_animation_running(void) {
  return s_is_animating;
}

void animate_to_page(int target_page) {
  if (s_is_animating || target_page == s_current_page) {
    return;
  }

  if (!s_main_page_container || !s_timetable_page_container) {
    // Fall back to instant switch if containers not set up
    s_current_page = target_page;
    update_page_visibility();
    return;
  }

  GRect bounds = s_screen_bounds;

  // Determine animation direction
  bool moving_down = (target_page > s_current_page);

  // Get current frames
  GRect main_frame = layer_get_frame(s_main_page_container);
  GRect timetable_frame = layer_get_frame(s_timetable_page_container);

  // Set up "from" positions
  GRect main_from = main_frame;
  GRect timetable_from = timetable_frame;

  // Set up "to" positions
  GRect main_to = main_frame;
  GRect timetable_to = timetable_frame;

  if (moving_down) {
    // Moving to timetable (slide down)
    // Main page slides up and out
    main_to.origin.y = -bounds.size.h;
    // Timetable page starts from below and slides in
    timetable_from.origin.y = bounds.size.h;
    timetable_to.origin.y = 0;
  } else {
    // Moving to main (slide up)
    // Main page starts from above and slides in
    main_from.origin.y = -bounds.size.h;
    main_to.origin.y = 0;
    // Timetable page slides down and out
    timetable_to.origin.y = bounds.size.h;
  }

  // Ensure timetable page is visible before animation
  layer_set_hidden(s_timetable_page_container, false);
  layer_set_hidden(s_main_page_container, false);

  // Set initial positions
  layer_set_frame(s_main_page_container, main_from);
  layer_set_frame(s_timetable_page_container, timetable_from);

  // Create animations for both containers
  PropertyAnimation *main_anim = property_animation_create_layer_frame(
      s_main_page_container, &main_from, &main_to);
  PropertyAnimation *timetable_anim = property_animation_create_layer_frame(
      s_timetable_page_container, &timetable_from, &timetable_to);

  // Configure animations
  animation_set_duration((Animation *)main_anim, PAGE_ANIMATION_DURATION);
  animation_set_duration((Animation *)timetable_anim, PAGE_ANIMATION_DURATION);
  animation_set_curve((Animation *)main_anim, AnimationCurveEaseInOut);
  animation_set_curve((Animation *)timetable_anim, AnimationCurveEaseInOut);

  // Create spawn animation (both run simultaneously)
  s_page_animation = animation_spawn_create(
      (Animation *)main_anim,
      (Animation *)timetable_anim,
      NULL);

  // Set up animation handlers
  AnimationHandlers handlers = {
    .started = animation_started_handler,
    .stopped = animation_stopped_handler
  };
  animation_set_handlers(s_page_animation, handlers, NULL);

  // Update page state
  s_current_page = target_page;
  s_is_animating = true;

  // Schedule the animation
  animation_schedule(s_page_animation);
}

static void animation_started_handler(Animation *animation, void *context) {
  // Animation started
  s_is_animating = true;
}

static void animation_stopped_handler(Animation *animation, bool finished, void *context) {
  s_is_animating = false;
  s_page_animation = NULL;

  // Ensure proper visibility after animation
  update_page_visibility();
  
  // Ensure lamp page stays hidden if not active
  if (!s_lamp_active && s_lamp_page_container) {
    layer_set_hidden(s_lamp_page_container, true);
  }
}
