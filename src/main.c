#include <pebble.h>
#include "main.h"
#include "info_window.h"
 
void try_vibration(){
	if(vibes_lock > vibes_fired){
		vibes_short_pulse();
		vibes_fired++;
		vibes_total++;
		vibes_disabled = false;
	}
	else{
		vibes_disabled = true;
	}
}
	
void accel_data_handler(AccelData *data, uint32_t num_samples) {
    for(uint32_t i = 0; i < num_samples; i++) {
        if(data[i].x<=-600 && data[i].y>=600) {
            miss_count = 0;
            hit_count++;
        }
        else {
            hit_count = 0;
            miss_count++;
        }
    }
    if(hit_count > delay) {
        try_vibration();
        hit_count = 0;
        miss_count = 0;
    }
    if(miss_count > delay) {
        hit_count = 0;
        miss_count = 0;
		vibes_fired = 0;
		vibes_disabled = false;
    }
	if(debug){ 
		APP_LOG(APP_LOG_LEVEL_INFO, "Hit: %d miss: %d vibes fired: %d", hit_count, miss_count, vibes_fired); 
	}
	if(info_window_exists()){
		int16_t y = data[0].y;
		int16_t x = data[0].x;
		refresh_info(y, x, vibes_total, miss_count, hit_count, vibes_disabled, vibes_fired);
	}
}
	
void tick_handler(struct tm *t, TimeUnits units_changed){
	static char time_buffer[] = "00:00";
	static char date_buffer[] = "26. September 2014...";
	if(clock_is_24h_style()){
		strftime(time_buffer, sizeof(time_buffer), "%H:%M", t);
	}
	else{
		strftime(time_buffer,sizeof(time_buffer),"%I:%M", t);
	}
	strftime(date_buffer,sizeof(date_buffer),"%d. %B %Y", t);
	text_layer_set_text(time_layer, time_buffer);
	text_layer_set_text(date_layer, date_buffer);
	
	int hours = t->tm_hour;
	int minutes = t->tm_min;
	//Reset daily
	if(hours == 0 && minutes == 0){
		vibes_total = 0;
	}
}

void battery_proc(Layer *layer, GContext *ctx){
	int height = 157;
	int circle_radius = 4;
	int k, l;
	for(k = 10; k > 0; k--){
		l = (13*k);
		graphics_draw_circle(ctx, GPoint(l, height), circle_radius);
	}
	
	int i, j;
	for(i = battery_percent/10; i > 0; i--){
		j = (i*13);
		graphics_fill_circle(ctx, GPoint(j, height), circle_radius);
	}
}

void charge_invert(void *data){
	invert = !invert;
	if(invert){
		if(battery_percent != 100){
			battery_percent += 10;
		}
		layer_mark_dirty(battery_layer);
	}
	else{
		if(battery_percent != 0){
			battery_percent -= 10;
		}
		layer_mark_dirty(battery_layer);
	}
	charge_timer = app_timer_register(1000, charge_invert, NULL);
}

void battery_handler(BatteryChargeState charge){
	battery_percent = charge.charge_percent;
	layer_mark_dirty(battery_layer);
	
	if(charge.is_charging){
		cancelled = 0;
		app_timer_cancel(charge_timer);
		charge_timer = app_timer_register(1000, charge_invert, NULL);
	}
	else{
		if(!cancelled){
			app_timer_cancel(charge_timer);
			cancelled = 1;
		}
	}
}

void bt_handler(bool connected){
	layer_set_hidden(inverter_layer_get_layer(bt_layer), !connected);
}

void click_config(){
	window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)select);
	window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler)back);
}

void window_load(Window *window){
	Layer *window_layer = window_get_root_layer(window);
	
	time_layer = text_layer_create(GRect(0, 40, 144, 168));
	text_layer_set_font(time_layer, BOLD);
	text_layer_set_text_color(time_layer, GColorBlack);
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	text_layer_set_background_color(time_layer, GColorClear);
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	
	date_layer = text_layer_create(GRect(0, 95, 144, 168));
	text_layer_set_font(date_layer, pixelmix);
	text_layer_set_text_color(date_layer, GColorBlack);
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	text_layer_set_background_color(date_layer, GColorClear);
	layer_add_child(window_layer, text_layer_get_layer(date_layer));
	
	battery_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(battery_layer, battery_proc);
	layer_add_child(window_layer, battery_layer);
	
	bt_icon_layer = bitmap_layer_create(GRect(0, -77, 144, 168));
	bitmap_layer_set_bitmap(bt_icon_layer, bt_icon);
	layer_add_child(window_layer, bitmap_layer_get_layer(bt_icon_layer));
	
	bt_layer = inverter_layer_create(GRect(0, 0, 144, 15));
	layer_add_child(window_layer, inverter_layer_get_layer(bt_layer));
	
	theme = inverter_layer_create(GRect(0, 0, 144, 168));
	layer_add_child(window_layer, inverter_layer_get_layer(theme));
	
	struct tm *t;
  	time_t temp;        
  	temp = time(NULL);        
  	t = localtime(&temp);
	
	tick_handler(t, MINUTE_UNIT);
	
	BatteryChargeState m8 = battery_state_service_peek();
	battery_handler(m8);
	
	bool con = bluetooth_connection_service_peek();
	bt_handler(con);
}

void window_unload(Window *window){
	text_layer_destroy(time_layer);
	layer_destroy(battery_layer);
	inverter_layer_destroy(bt_layer);
	inverter_layer_destroy(theme);
}
	
void init(){
	window = window_create();
	window_set_fullscreen(window, true);
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload,
	});
	info_window_init();
	tick_timer_service_subscribe(MINUTE_UNIT, &tick_handler);
	battery_state_service_subscribe(battery_handler);
	bluetooth_connection_service_subscribe(bt_handler);
	BOLD = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BOLD_50));
	pixelmix = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PIXELMIX_12));
	bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
	
	accel_data_service_subscribe(10, accel_data_handler);
    accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
	
	window_set_click_config_provider(window, click_config);
	
	window_stack_push(window, true);
}

void deinit(){
	fonts_unload_custom_font(BOLD);
	fonts_unload_custom_font(pixelmix);
	gbitmap_destroy(bt_icon);
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	accel_data_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	info_window_deinit();
}

int main(){
	init();
	app_event_loop();
	deinit();
}