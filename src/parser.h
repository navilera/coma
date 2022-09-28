#pragma once

#include <stdint.h>

void write_header(const char* str);
void write_code_line(const char* str);
void write_code_num(const char* ref);
void write_figure_num(const char* ref);
void write_replace_code_line(const char* str);
void write_quote(void);
void write_start_code_line(void);
void write_plain(const char* str);
void write_text(const char* str);

uint32_t get_val_as_2key(char k1, char k2);
uint32_t get_val_as_3key(char k1, char k2, char k3);

void hex_debug(const char* str);


