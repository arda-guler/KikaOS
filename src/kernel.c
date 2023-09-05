#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__linux__)
#error "Use a GCC Cross-Compiler. Terminating..."
#endif

#if !defined(__i386__)
#error "Use an ix86-elf compiler. Terminating..."
#endif

#define COMMAND_BUFFER_SIZE 64

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

bool type_uppercase = false;

char ASCII_uppercase[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', /* <-- Tab */
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', // <-- 28: ENTER
    0, /* <-- control key */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',  0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/',   0,
    '*',
    0,  /* Alt */
    ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

char ASCII_lowercase[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', /* <-- Tab */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', // <-- 28: ENTER
    0, /* <-- control key */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
    '*',
    0,  /* Alt */
    ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

char command_buffer[COMMAND_BUFFER_SIZE];

const char *commands[] = {
	"help",
	"title"
};


static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_init(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setColor(uint8_t color)
{
	terminal_color = color;
}

void terminal_putEntryAt(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(void)
{
    // loop through all lines
    for (size_t idx_y = 2; idx_y < VGA_HEIGHT; idx_y++){
        for (size_t idx_x = 0; idx_x < VGA_WIDTH; idx_x++) {

            size_t idx_terminal_read = idx_y * VGA_WIDTH + idx_x;

            // get the character at the index we are currently reading
            uint16_t current_vga_char = terminal_buffer[idx_terminal_read];

            // print it on the line above
            size_t idx_terminal_write = (idx_y - 1) * VGA_WIDTH + idx_x;
            terminal_buffer[idx_terminal_write] = current_vga_char;
            terminal_buffer[idx_terminal_read] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setTitle(const char* title) {
    size_t prev_terminal_row = terminal_row;
    size_t prev_terminal_column = terminal_column;

    terminal_row = 0;
    terminal_column = 0;

    for (size_t idx_x = 0; idx_x < VGA_WIDTH; idx_x++){
        terminal_putChar(' ');
    }

    terminal_row = 0;
    terminal_column = 0;

    terminal_printString(title);

    terminal_row = prev_terminal_row;
    terminal_column = prev_terminal_column;
}

void terminal_putChar(char c)
{
    // this is the new line character
    if (c == '\n') {
        if (terminal_row + 1 >= VGA_HEIGHT) {
            terminal_scroll();
        }
        else{
            terminal_row = terminal_row + 1;
        }
        terminal_column = 0;
    }

    // this is a regular character, print it normally
    else {
	    terminal_putEntryAt(c, terminal_color, terminal_column, terminal_row);

        if (terminal_column + 1 >= VGA_WIDTH) {
            terminal_column = 0;

            if (terminal_row + 1 >= VGA_HEIGHT) {
                terminal_scroll();
            }
            else {
                terminal_row = terminal_row + 1;
            }

        }
        else {
            terminal_column = terminal_column + 1;
        }
    }
}

void terminal_print(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		terminal_putChar(data[i]);
    }
}

void terminal_printString(const char* data)
{
	terminal_print(data, strlen(data));
}

void outb( unsigned short port, unsigned char val )
{
   asm volatile("outb %0, %1" : : "a"(val), "Nd"(port) );
}

static __inline unsigned char inb (unsigned short int port)
{
    unsigned char v;

    asm volatile("inb %w1,%0":"=a" (v):"Nd" (port));
    return v;
}

char get_kbd(char c)
{
    if (inb(0x60) != c){
        c = inb(0x60);
    }

    return c;
}

// this is probably one of the worst implementations of 'wait' ever.
// P.S. I don't have interrupts.
void wait(size_t wait_time){
    for (size_t ticks=0; ticks < wait_time * 1000000000; ticks++) {
        asm volatile("nop");
    }
}

void clear_command_buffer(void) {
	for (size_t i=0; i < COMMAND_BUFFER_SIZE; i++){
		command_buffer[i] = 0;
	}
}

// placeholder function
void interpret_command(void) {
	asm volatile("nop");
}

void main(void)
{
	terminal_init();
    terminal_setTitle("= = = KikaOS Terminal = = =");
    terminal_printString("\n\nWrite or draw away!\n > ");

    // writes keyboard input onto the screen
    char c_inp = 0;
	size_t command_buffer_idx = 0;
    while(c_inp != 1) {

        c_inp = get_kbd(c_inp);

        if (c_inp == 58) {
            type_uppercase = !type_uppercase;
        }

		else if (c_inp == 28) {
			interpret_command();
			clear_command_buffer();
			terminal_printString("\n > ");
			wait(1);
		}

        else if (c_inp > 0) {
			char ASCII_char;
            if (type_uppercase) {
				ASCII_char = ASCII_uppercase[c_inp];
            }
            else {
				ASCII_char = ASCII_lowercase[c_inp];
            }
			terminal_putChar(ASCII_char);
			command_buffer[command_buffer_idx] = ASCII_char;
			command_buffer_idx += 1;
            wait(1);
        }
    }

    terminal_printString("\n\n - - - END - - -\n");
}
