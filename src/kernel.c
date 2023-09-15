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
#define MAX_NUM_COMMANDS 100
#define MAX_COMMAND_LENGTH 64

#define NUM_VGA_COLORS 16
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

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

char ASCII_uppercase[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', // <--14: backspace
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
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', // <--14: backspace
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

// static const size_t VGA_WIDTH = 80;
// static const size_t VGA_HEIGHT = 25;

size_t standard_wait = 10;

char terminal_title[];
uint8_t terminal_title_color;

char command_buffer[COMMAND_BUFFER_SIZE];
char prev_command[COMMAND_BUFFER_SIZE];
size_t prev_command_size;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

size_t cursor_row;
size_t cursor_column;

bool type_uppercase = false;

char commands[MAX_NUM_COMMANDS][MAX_COMMAND_LENGTH] = {
	"help",
	"clear",
	"wait inc",
	"wait dec",
	"color",
	"rng"
};

int strcmp(const char* str1, const char* str2) {
	while (*str1 != '\0' && *str2 != '\0') {
        if (*str1 != *str2) {
            return (*str1 - *str2);
        }
        str1++;
        str2++;
    }

    return (*str1 - *str2);
}

// random size_t generator
size_t seed = 1;
const size_t a = 2001;
const size_t c = 1 << 30;
const size_t m = (1 << 63) - 1;

size_t random(size_t min, size_t max) {
	size_t range = max - min;
	seed = (a * seed + c) % m;
	return min + (size_t)(seed % range);
}

void copyCharArray(char a1[], char a2[]) {
	size_t array_size = sizeof(a1) / sizeof(a1[0]);

	for (size_t i = 0; i < array_size + 1; i++) {
		a2[i] = a1[i];
	}
}

// size_t to string converter
void size_tToCharArray(size_t number, char* charArray) {
    int i = 0;

    // Handle the case of zero
    if (number == 0) {
        charArray[0] = '0';
        charArray[1] = '\0';
        return;
    }

    // Convert each digit of the size_t to a character
    while (number > 0) {
        charArray[i++] = (char)((number % 10) + '0');
        number /= 10;
    }

    charArray[i] = '\0';

    // Reverse the charArray so that it's in the correct order
    int length = i;
    for (i = 0; i < length / 2; i++) {
        char temp = charArray[i];
        charArray[i] = charArray[length - i - 1];
        charArray[length - i - 1] = temp;
    }
}

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void terminal_init(void) {
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

void terminal_setColor(uint8_t color) {
	terminal_color = color;
}

void terminal_putEntryAt(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(void) {
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

void terminal_putChar(char c) {
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

void terminal_print(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		terminal_putChar(data[i]);
    }
}

void terminal_printString(const char* data) {
	terminal_print(data, strlen(data));
}

void outb(unsigned short port, unsigned char val) {
   asm volatile("outb %0, %1" : : "a"(val), "Nd"(port) );
}

static __inline unsigned char inb (unsigned short int port) {
    unsigned char v;

    asm volatile("inb %w1,%0":"=a" (v):"Nd" (port));
    return v;
}

char get_kbd(char c) {
    if (inb(0x60) != c){
        c = inb(0x60);
    }

    return c;
}

// this is probably one of the worst implementations of 'wait' ever.
// P.S. I don't have interrupts.
void wait(size_t wait_time) {
    for (size_t ticks = 0; ticks < wait_time * 100000000; ticks++) {
        asm volatile("nop");
    }
}

void clear_command_buffer(void) {
	for (size_t i = 0; i < COMMAND_BUFFER_SIZE; i++){
		command_buffer[i] = 0;
	}
}

// simple command interpreter
void interpret_command(const char* command) {
	bool command_found = false;

	for (size_t i = 0; i < MAX_NUM_COMMANDS; i++) {
		if (strcmp(command, commands[i]) == 0) {
			command_found = true;
			switch (i) {
				case 0: // help
					terminal_printString("\n\n");
					terminal_printString("Available commands: help, clear, wait inc, wait dec,\n");
					terminal_printString("color, rng\n");
					break;
				case 1: // clear
					for (size_t j = 0; j < VGA_HEIGHT; j++) {
						terminal_scroll();
					}
					terminal_row = 2;
					terminal_column = 0;
					break;
				case 2: // inc_wait
					standard_wait *= 2;
					break;
				case 3: // dec_wait
					if (standard_wait >= 2) {
						standard_wait /= 2;
					}
					break;
				case 4: // color
					if (terminal_color < NUM_VGA_COLORS - 1) {
						terminal_color += 1;
					}
					else {
						terminal_color = 0;
					}
					break;
				case 5: // rng
					size_t rand = random(0, 100);
					char rand_str[4];
					size_tToCharArray(rand, rand_str);
					terminal_printString("\nRandom number (0, 100):\n");
					terminal_printString(rand_str);
					break;
			}
		}
	}

	if (!command_found) {
		terminal_printString("\nUnrecognized command.\n");
	}
}

void terminal_backspace(void) {
	if (terminal_column > 0) {
		terminal_column--;
		terminal_putChar(' ');
		terminal_column--;
	}

	// go back one row if we are at the left edge of the screen
	else {
		terminal_row--;
		terminal_column = VGA_WIDTH - 1;
		terminal_putChar(' ');
		terminal_row--;
		terminal_column = VGA_WIDTH - 1;
	}
}

void cursor_enable(uint8_t cursor_start, uint8_t cursor_end) {
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void cursor_disable() {
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void cursor_move(int x, int y) {
	cursor_row = y;
	cursor_column = x;

	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void cursor_update(void) {
	uint16_t pos = cursor_row * VGA_WIDTH + cursor_column;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void main(void)
{
	terminal_init();
    terminal_setTitle("= = = KikaOS Terminal = = =");
    terminal_printString("\n\nTerminal initialized.\n > ");
	cursor_enable(0, 15);
	cursor_move(3, 3);

    // writes keyboard input onto the screen
    char c_inp = 0;
	size_t command_buffer_idx = 0;
    while(c_inp != 1) {

        c_inp = get_kbd(c_inp);

		// caps lock
        if (c_inp == 58) {
            type_uppercase = !type_uppercase;
        }

		// enter
		else if (c_inp == 28) {
			interpret_command(command_buffer);

			// record last used command to prev_command array, unless
			// it was empty of course
			if (command_buffer_idx > 0) {
				copyCharArray(command_buffer, prev_command);
				prev_command_size = command_buffer_idx;
			}

			// clean command buffer
			clear_command_buffer();
			command_buffer_idx = 0;

			terminal_printString("\n > ");
			// wait(standard_wait);
		}

		// backspace
		else if (c_inp == 14) {
			if (command_buffer_idx > 0) {
				terminal_backspace();
				command_buffer[command_buffer_idx - 1] = 0;
				command_buffer_idx--;
				// wait(standard_wait);
			}
		}

		// up arrow (gets previous command)
		else if (c_inp == 72) {
			clear_command_buffer();

			while (command_buffer_idx > 0) {
				terminal_backspace();
				command_buffer_idx--;
			}

			copyCharArray(prev_command, command_buffer);
			command_buffer_idx = prev_command_size;

			terminal_printString(command_buffer);
			// wait(standard_wait);
		}

        else if (c_inp > 0) {
			if (command_buffer_idx < COMMAND_BUFFER_SIZE - 1) {
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
	            // wait(standard_wait);
			}
        }
		cursor_move(terminal_column, terminal_row);
		wait(standard_wait);
    }

    terminal_printString("\n\n - - - END - - -\n");
}
