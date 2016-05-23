#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

// window
static Window *s_main_window;
// text layers
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;
// fonts
static GFont s_time_font;
static GFont s_date_font;
static GFont s_weather_font;
// weather data
static char temperature_buffer[8];
static char conditions_buffer[32];
static char weather_layer_buffer[32];
// battery
static int s_battery_level;
static Layer *s_battery_layer;

static void battery_callback(BatteryChargeState state) {
    s_battery_level = state.charge_percent;

    // update meter
    layer_mark_dirty(s_battery_layer);
}

static void update_time() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
            "%H:%M" : "%I:%M", tick_time);

    text_layer_set_text(s_time_layer, s_buffer);

    // Copy date into buffer from tm structure
    static char date_buffer[16];
    strftime(date_buffer, sizeof(date_buffer), "%a %d.%m", tick_time);

    // Show the date
    text_layer_set_text(s_date_layer, date_buffer);
}

static void update_weather(struct tm *tick_time) {
    // Get weather update every 15 minutes
    if(tick_time->tm_min % 15 == 0) {
        // Begin dictionary
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);

        // Add a key-value pair
        dict_write_uint8(iter, 0, 0);

        // Send the message!
        app_message_outbox_send();
    }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
    update_weather(tick_time);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    // Find the width of the bar
    int width = (int)(float)(((float)s_battery_level / 100.0F) * 150.0F);

    // Draw the background
    graphics_context_set_fill_color(ctx, GColorClear);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    // Draw the bar
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}


static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // time text layer
    s_time_layer = text_layer_create(
            GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50)
            );

    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_text(s_time_layer, "00:00");
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_44));
    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // weather text layer
    s_weather_layer = text_layer_create(
            GRect(0, PBL_IF_ROUND_ELSE(125, 120), bounds.size.w, 25)
            );
    text_layer_set_background_color(s_weather_layer, GColorRed);
    text_layer_set_text_color(s_weather_layer, GColorWhite);
    text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
    text_layer_set_text(s_weather_layer, "Loading...");

    s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_20));
    text_layer_set_font(s_time_layer, s_time_font);

    layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));

    // date text layer
    s_date_layer = text_layer_create(
            GRect(0, PBL_IF_ROUND_ELSE(25, 20), bounds.size.w, 25)
            );
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, GColorWhite);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    text_layer_set_text(s_date_layer, "Mon 01.01");

    s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_DOS_20));
    text_layer_set_font(s_time_layer, s_time_font);

    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // battery layer
    s_battery_layer = layer_create(GRect(14, 54, 115, 2));
    layer_set_update_proc(s_battery_layer, battery_update_proc);
    layer_add_child(window_get_root_layer(window), s_battery_layer);
}

static void main_window_unload(Window *window) {
    // destory time elements
    text_layer_destroy(s_time_layer);
    fonts_unload_custom_font(s_time_font);

    text_layer_destroy(s_date_layer);
    fonts_unload_custom_font(s_date_font);

    // destroy weather elements
    text_layer_destroy(s_weather_layer);
    fonts_unload_custom_font(s_weather_font);

    // destroy battery elements
    layer_destroy(s_battery_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    // Read tuples for data
    Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
    Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

    // If all data is available, use it
    if(temp_tuple && conditions_tuple) {
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    }

    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorRed);

    window_set_window_handlers(s_main_window, (WindowHandlers) {
            .load = main_window_load,
            .unload = main_window_unload
            });

    window_stack_push(s_main_window, true);

    update_time();

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    // register calllbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    // register for battery level updated
    battery_state_service_subscribe(battery_callback);
    battery_callback(battery_state_service_peek());

    // Open AppMessage
    const int inbox_size = 128;
    const int outbox_size = 128;
    app_message_open(inbox_size, outbox_size);
}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
