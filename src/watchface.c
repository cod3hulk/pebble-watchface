#include <pebble.h>

// windows
static Window *s_main_window;
// text layers
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;
// fonts
static GFont s_time_font;
static GFont s_weather_font;

static void update_time() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
            "%H:%M" : "%I:%M", tick_time);

    text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // time text layer
    s_time_layer = text_layer_create(
        GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50)
    );

    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_text(s_time_layer, "00:00");
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_44));
    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // weather text layer
    s_weather_layer = text_layer_create(
        GRect(0, PBL_IF_ROUND_ELSE(125, 120), bounds.size.w, 25)
    );
    text_layer_set_background_color(s_weather_layer, GColorClear);
    text_layer_set_text_color(s_weather_layer, GColorBlack);
    text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
    text_layer_set_text(s_weather_layer, "Loading...");

    s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_20));
    text_layer_set_font(s_time_layer, s_time_font);

    layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
}

static void main_window_unload(Window *window) {
    // destory time elements
    text_layer_destroy(s_time_layer);
    fonts_unload_custom_font(s_time_font);

    // destroy weather elements
    text_layer_destroy(s_weather_layer);
    fonts_unload_custom_font(s_weather_font);
}

static void init() {
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    window_stack_push(s_main_window, true);

    update_time();

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
