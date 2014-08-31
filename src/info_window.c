#include <pebble.h>
#include "info_window.h"
	
Window *info_window;
TextLayer *total_v, *y_pos_tl, *x_pos_tl, *hit_tl, *miss_tl, *isdisabled_tl, *temp_v_tl;
bool info_exists = 0;

bool info_window_exists(){
	return info_exists;
}

void refresh_info(int16_t y, int16_t x, int total, int miss, int hit, bool disabled, int temp){
	static char total_buffer[20];
	snprintf(total_buffer, sizeof(total_buffer), "%d vibes total", total);
	text_layer_set_text(total_v, total_buffer);
	
	static char x_buffer[20];
	snprintf(x_buffer, sizeof(x_buffer), "X (0): %d", x);
	text_layer_set_text(y_pos_tl, x_buffer);
	
	static char y_buffer[20];
	snprintf(y_buffer, sizeof(y_buffer), "Y (0): %d", y);
	text_layer_set_text(x_pos_tl, y_buffer);
	
	static char hit_buffer[20];
	snprintf(hit_buffer, sizeof(hit_buffer), "Hits: %d", hit);
	text_layer_set_text(hit_tl, hit_buffer);
	
	static char miss_buffer[20];
	snprintf(miss_buffer, sizeof(miss_buffer), "Misses: %d", miss);
	text_layer_set_text(miss_tl, miss_buffer);
	
	static char temp_buffer[20];
	snprintf(temp_buffer, sizeof(temp_buffer), "Temp vibes: %d", temp);
	text_layer_set_text(temp_v_tl, temp_buffer);
	
	if(disabled){
		text_layer_set_text(isdisabled_tl, "Vibration lock: true");
	}
	else{
		text_layer_set_text(isdisabled_tl, "Vibration lock: false");
	}
}

void select(ClickRecognizerRef ref, void *context){
	window_stack_push(info_window, true);
}


void back(ClickRecognizerRef ref, void *context){
	//Locked
}

TextLayer *text_layer_init(GRect frame){
	TextLayer *layer;
	layer = text_layer_create(frame);
	text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(layer, GTextAlignmentCenter);
	text_layer_set_text(layer, "Loading data...");
	text_layer_set_background_color(layer, GColorClear);
	text_layer_set_text_color(layer, GColorWhite);
	return layer;
}

void window_load_info(Window *w){
	info_exists = true;
	Layer *window_layer = window_get_root_layer(w);
	
	total_v = text_layer_init(GRect(0, 0, 144, 40));
	layer_add_child(window_layer, text_layer_get_layer(total_v));
	
	y_pos_tl = text_layer_init(GRect(0, 20, 144, 40));
	layer_add_child(window_layer, text_layer_get_layer(y_pos_tl));
	
	x_pos_tl = text_layer_init(GRect(0, 40, 144, 40));
	layer_add_child(window_layer, text_layer_get_layer(x_pos_tl));
	
	hit_tl = text_layer_init(GRect(0, 60, 144, 40));
	layer_add_child(window_layer, text_layer_get_layer(hit_tl));
	
	miss_tl = text_layer_init(GRect(0, 80, 144, 40));
	layer_add_child(window_layer, text_layer_get_layer(miss_tl));
	
	isdisabled_tl = text_layer_init(GRect(0, 100, 144, 40));
	layer_add_child(window_layer, text_layer_get_layer(isdisabled_tl));
	
	temp_v_tl = text_layer_init(GRect(0, 120, 144, 40));
	layer_add_child(window_layer, text_layer_get_layer(temp_v_tl));
}

void window_unload_info(Window *w){
	text_layer_destroy(total_v);
	text_layer_destroy(y_pos_tl);
	text_layer_destroy(x_pos_tl);
	text_layer_destroy(miss_tl);
	text_layer_destroy(hit_tl);
	text_layer_destroy(temp_v_tl);
	text_layer_destroy(isdisabled_tl);
	info_exists = false;
}

void info_window_init(){
	info_window = window_create();
	window_set_window_handlers(info_window, (WindowHandlers){
		.load = window_load_info,
		.unload = window_unload_info,
	});
	window_set_background_color(info_window, GColorBlack);
}

void info_window_deinit(){
	window_destroy(info_window);
}