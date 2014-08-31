#pragma once
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

//Acceleromter stuff - taken from Jon Barlow (@orviwan)
int hit_count = 0;
int miss_count = 0;
int delay = 10;
 
//tweak these numbers for the Pebble position, think this is upright'ish. RTFM
int posx = -600;
int posy = 600;

//adjust this number for the amount of vibes you want set off before it stops
const int vibes_lock = 5;
//vibes_fired will keep track of how many vibrations have been fired while
//the arm is held up
int vibes_fired = 0;

//keeps track of the total vibes in a day
int32_t vibes_total = 0;

bool vibes_disabled = false;

//if you want to see direct variables change this to true
bool debug = false;