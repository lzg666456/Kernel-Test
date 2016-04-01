#include <system.h>
#include <video.h>

using namespace Kernel::IO;

Terminal::Terminal() { }

void Terminal::init(int gfx_mode) {
	this->gfx_mode = gfx_mode;
	cursor_x = cursor_y = 0;
	hide_textmode_cursor();
	clear();
}

void Terminal::init(void) {
	init(0);
}

void Terminal::hide_textmode_cursor() {
	outb(0x3D4, 14);
	outb(0x3D5, 0xFF);
	outb(0x3D4, 15);
	outb(0x3D5, 0xFF);
}

void Terminal::putc_textmode(const char chr, uint8_t color) {
	if(chr=='\n') { cursor_y++; cursor_x = 0; return; }
	int loc = VID_CALC_POS(cursor_x, cursor_y);
	VID[loc * 2] = chr;
	VID[loc * 2 + 1] = color;
	cursor_x++;
}

void Terminal::putc_gfx(const char chr, uint8_t color) {
	/* Draw character with default font: */

}

void Terminal::putc(const char chr, uint8_t color) {
	if(gfx_mode) putc_gfx(chr, color);
	else putc_textmode(chr, color);
}

void Terminal::putc(const char chr) {
	putc(chr, COLOR_DEFAULT);
}

void Terminal::puts(const char * str, uint8_t color) {
	while (*str) putc(*str++, color);
}

void Terminal::puts(const char * str) {
	while (*str) putc(*str++, COLOR_DEFAULT);
}

void Terminal::clear() {
	reset_cursor();
	for (int i = 0; i<2000; i++)
		putc(' ', COLOR(VIDBlack, VIDBlack));
	reset_cursor();
}

void Terminal::fill(uint8_t bgcolor) {
	reset_cursor();
	for (int i = 0; i<2000; i++)
		putc(' ', COLOR(bgcolor, VIDBlack));
	reset_cursor();
}

void Terminal::reset_cursor() {
	cursor_x = cursor_y = 0;
}

Point Terminal::go_to(uint8_t x, uint8_t y) {
	Point oldp;
	oldp.X = cursor_x;
	oldp.Y = cursor_y;
	cursor_x = x;
	cursor_y = y;
	return oldp;
}

char printf_buff[256];
#define term_printf(fmt, color) strfmt(printf_buff, fmt); \
								puts(printf_buff, color); \
							
void Terminal::printf(uint8_t color, const char *fmt, ...) {
	term_printf(fmt, color);
}

void Terminal::printf(const char *fmt, ...) {
	term_printf(fmt, COLOR_DEFAULT);
}

void Terminal::printf(uint8_t color, const char *fmt, va_list args) {
	vasprintf(printf_buff, fmt, args);
	puts(printf_buff, color);
}

void Terminal::printf(const char *fmt, va_list args, char ign) {
	vasprintf(printf_buff, fmt, args);
	puts(printf_buff, COLOR_DEFAULT);
}
