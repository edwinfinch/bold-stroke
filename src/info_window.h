#pragma once
void info_window_init();
void info_window_deinit();
void select(ClickRecognizerRef ref, void *context);
void back(ClickRecognizerRef ref, void *context);
void refresh_info(int16_t y, int16_t x, int total, int miss, int hit, bool disabled, int temp);
bool info_window_exists();