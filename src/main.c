#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;
static GFont s_time_font,  s_date_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static int s_battery_level;
static Layer *s_battery_layer;

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  // ???
  int width = (int)(float)(((float) s_battery_level / 100.0F ) * 111.0F);
  // draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx,bounds,0,GCornerNone);
  // draw the bar
  graphics_context_set_fill_color(ctx,GColorWhite);
  graphics_fill_rect(ctx, GRect(0,0,width,bounds.size.h),0,GCornerNone);
}
static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  layer_mark_dirty(s_battery_layer);
}
static void update_date(){
//   APP_LOG(0, '%s', "updating date right now");
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char s_buffer[12];
  strftime(s_buffer, sizeof(s_buffer), "%a %b %d", tick_time);
  text_layer_set_text(s_date_layer, s_buffer);
}
static void update_time(){
//   APP_LOG('0', '%s', ' time updating');
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // write the curr hour/min into buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  // display it in the textlayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  if(tick_time->tm_min % 30 == 0){
//     APP_LOG('0', '%s', ' sending app message');
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter,0,0);
    app_message_outbox_send();
  }
  if(tick_time->tm_hour==0 && tick_time->tm_min == 0)
    update_date();
}
static void main_window_load(Window *window){

  // extract the Layer out of the window and get it's GRect boundaries
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
// use those bounds to create a text_layer
  s_time_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(58, 54), bounds.size.w, 50));
  // uses predef values for x/y of the upper left pos, and then h/w are size. 
  // create the GFont;
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CHALKDUSTER_40));
  
  // also for date:
  s_date_layer = text_layer_create(
    GRect(0,5, bounds.size.w, 40)
  );
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CHALKDUSTER_20));
  // also for weather:
  s_weather_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(125,120), bounds.size.w, 25) );
  
  //style the weather
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorMintGreen);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer,"Loading...");
  text_layer_set_font(s_weather_layer, s_date_font);
  
  // create the background:
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TUTORIAL_BG);
  s_background_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_background_layer,s_background_bitmap);

  // create the battery meter:
  s_battery_layer = layer_create(GRect(18,52,111,4));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
 
  
  // add background bitmap BEFORE other stuff
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // add battery to window:
  layer_add_child(window_get_root_layer(window),s_battery_layer);
  
  // setting bg color to non-clear makes the rect bounds visible, which is ugly but whatever.
  text_layer_set_background_color(s_time_layer, GColorClear);
// check https://developer.pebble.com/guides/tools-and-resources/color-picker
  text_layer_set_text_color(s_time_layer, GColorMintGreen);
  text_layer_set_font(s_time_layer, s_time_font);  
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorGreen);
  text_layer_set_font(s_date_layer, s_date_font);  
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  // and dont forget to add it to the window layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
}

static void main_window_unload(Window *window){
  fonts_unload_custom_font(s_time_font);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_date_layer);
  fonts_unload_custom_font(s_date_font);
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  layer_destroy(s_battery_layer);
}
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char temp_buffer[8];
  static char cond_buffer[32];
  static char weather_layer_buffer[32];
  
  Tuple *temp_tuple       = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS); 

  if(temp_tuple && conditions_tuple) {
    snprintf(temp_buffer, sizeof(temp_buffer),"%dF", (int)temp_tuple->value->int32);
    snprintf(cond_buffer,sizeof(cond_buffer),"%s",conditions_tuple->value->cstring);
    snprintf(weather_layer_buffer,sizeof(weather_layer_buffer),"%s, %s",temp_buffer,cond_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
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
static void init(){
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers){ .load = main_window_load, .unload = main_window_unload } );  
  // show the window
  window_stack_push(s_main_window, true);
  // make sure time is updated
  update_time();
  update_date();
  window_set_background_color(s_main_window, GColorBlack);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //register for battery updates:
  battery_state_service_subscribe(battery_callback);
  // set initial battery level:
  battery_callback(battery_state_service_peek());
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
   // open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}
static void deinit(){
  window_destroy(s_main_window);
}
int main(void) {
  init();
  app_event_loop();
  deinit();
}