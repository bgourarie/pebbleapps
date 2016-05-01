#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer;

static void update_time(){
  
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
}

static void main_window_load(Window *window){
// extract the Layer out of the window and get it's GRect boundaries
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
// use those bounds to create a text_layer
  s_time_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  // uses predef values for x/y of the upper left pos, and then h/w are size. 
  // moar settings:
  // setting bg color to non-clear makes the rect bounds visible, which is ugly but whatever.
  text_layer_set_background_color(s_time_layer, GColorCyan);
  text_layer_set_text_color(s_time_layer, GColorGreen);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));  
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // and dont forget to add it to the window layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window){
  text_layer_destroy(s_time_layer);
}
static void init(){
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers){ .load = main_window_load, .unload = main_window_unload } );  
  // show the window
  window_stack_push(s_main_window, true);
  // make sure time is updated
  update_time();
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
}
static void deinit(){
  window_destroy(s_main_window);
}
int main(void) {
  init();
  app_event_loop();
  deinit();
}