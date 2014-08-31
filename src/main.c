#include <pebble.h>
	
Window *window;
BitmapLayer *bt_icon_layer;
GBitmap *bt_icon;
TextLayer *time_layer, *date_layer;
Layer *battery_layer;
InverterLayer *bt_layer;
InverterLayer *theme;
GFont *BOLD, *pixelmix;
AppTimer *charge_timer;
bool cancelled = 0;
bool phone_is_connected = 0;
int battery_percent;
bool invert;
	
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
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload,
	});
	tick_timer_service_subscribe(MINUTE_UNIT, &tick_handler);
	battery_state_service_subscribe(battery_handler);
	bluetooth_connection_service_subscribe(bt_handler);
	BOLD = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BOLD_50));
	pixelmix = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PIXELMIX_12));
	bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
	window_stack_push(window, true);
}

void deinit(){
	fonts_unload_custom_font(BOLD);
	fonts_unload_custom_font(pixelmix);
	gbitmap_destroy(bt_icon);
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
}

int main(){
	init();
	app_event_loop();
	deinit();
}