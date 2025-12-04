#include "../include/tty.h"
#include "../include/string.h"
#include "../include/io.h"
#include "../include/bool.h"
#include "../include/kbd.h"

size_t terminal_row;
size_t terminal_column;
static uint16_t *const VGA_MEMORY = (uint16_t *)0xb8000;
uint8_t terminal_color;
uint16_t *terminal_buffer;
static font_scale_t current_font_scale = FONT_SIZE_NORMAL;

#define BUFFER_SIZE 1024

static inline uint8_t make_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

static inline uint16_t make_vgaentry(char c, uint8_t color)
{
    uint16_t c16 = c;
    uint16_t color16 = color;
    return c16 | color16 << 8;
}

void terminal_initialize(enum vga_color font_color, enum vga_color background_color)
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = make_color(font_color, background_color);
    terminal_buffer = VGA_MEMORY;
    size_t y;
    for (y = 0; y < VGA_HEIGHT; y++)
    {
        size_t x;
        for (x = 0; x < VGA_WIDTH; x++)
        {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = make_vgaentry(' ', terminal_color);
        }
    }
    move_cursor(0, 0);
    enable_hardware_cursor(0x00, 0x0F); // show full-size blinking cursor
}

void terminal_scroll()
{
    int y, x;
    for (y = 0; y < (int)VGA_HEIGHT - 1; y++)
    {
        for (x = 0; x < (int)VGA_WIDTH; x++)
        {
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    // clear last line
    for (x = 0; x < (int)VGA_WIDTH; x++)
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = make_vgaentry(' ', terminal_color);
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = make_vgaentry(c, color);
}

void terminal_putchar(char c)
{
    if (c == '\n' || c == '\r')
    {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT)
            terminal_scroll();
    }
    else if (c == '\t')
    {
        terminal_column += 4;
        if (terminal_column >= VGA_WIDTH)
        {
            terminal_column = 0;
            terminal_row++;
            if (terminal_row >= VGA_HEIGHT)
                terminal_scroll();
        }
    }
    else if (c == '\b')
    {
        if (terminal_column > 0) terminal_column--;
        terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
    }
    else
    {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        terminal_column++;
        if (terminal_column >= VGA_WIDTH)
        {
            terminal_column = 0;
            terminal_row++;
            if (terminal_row >= VGA_HEIGHT)
                terminal_scroll();
        }
    }
    move_cursor(get_terminal_row(), get_terminal_col());
}

void terminal_write(const char *data, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++)
        terminal_putchar(data[i]);
    move_cursor(get_terminal_row(), get_terminal_col());
}

int putchar(int ic)
{
    char c = (char)ic;
    terminal_write(&c, sizeof(c));
    return ic;
}

void term_putc(char c, enum vga_color char_color)
{
    uint8_t color = make_color(char_color, (terminal_color >> 4));
    if (c == '\n' || c == '\r')
    {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) terminal_scroll();
    }
    else
    {
        terminal_putentryat(c, color, terminal_column, terminal_row);
        terminal_column++;
        if (terminal_column >= VGA_WIDTH)
        {
            terminal_column = 0;
            terminal_row++;
            if (terminal_row >= VGA_HEIGHT) terminal_scroll();
        }
    }
    move_cursor(get_terminal_row(), get_terminal_col());
}

// This function prints an entire string onto the screen
// void term_print(const char *str)
// {
//     int i;
//     for (i = 0; str[i] != '\0'; i++)
//     { // Keep placing characters until we hit the null-terminating character ('\0')
//         term_putc(str[i]);
//     }
// }

static void print(const char *data, size_t data_length)
{
    size_t i;
    for (i = 0; i < data_length; i++)
        putchar((int)((const unsigned char *)data)[i]);
}

int normalize(double *val)
{
    int exponent = 0;
    double value = *val;

    while (value >= 1.0)
    {
        value /= 10.0;
        ++exponent;
    }

    while (value < 0.1)
    {
        value *= 10.0;
        --exponent;
    }
    *val = value;
    return exponent;
}

static void ftoa_fixed(char *buffer, double value)
{
    /* carry out a fixed conversion of a double value to a string, with a precision of 5 decimal digits.
     * Values with absolute values less than 0.000001 are rounded to 0.0
     * Note: this blindly assumes that the buffer will be large enough to hold the largest possible result.
     * The largest value we expect is an IEEE 754 double precision real, with maximum magnitude of approximately
     * e+308. The C standard requires an implementation to allow a single conversion to produce up to 512
     * characters, so that's what we really expect as the buffer size.
     */

    int exponent = 0;
    int places = 0;
    static const int width = 4;

    if (value == 0.0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (value < 0.0)
    {
        *buffer++ = '-';
        value = -value;
    }

    exponent = normalize(&value);

    while (exponent > 0)
    {
        int digit = value * 10;
        *buffer++ = digit + '0';
        value = value * 10 - digit;
        ++places;
        --exponent;
    }

    if (places == 0)
        *buffer++ = '0';

    *buffer++ = '.';

    while (exponent < 0 && places < width)
    {
        *buffer++ = '0';
        --exponent;
        ++places;
    }

    while (places < width)
    {
        int digit = value * 10.0;
        *buffer++ = digit + '0';
        value = value * 10.0 - digit;
        ++places;
    }
    *buffer = '\0';
}

void ftoa_sci(char *buffer, double value)
{
    int exponent = 0;
    int places = 0;
    static const int width = 4;

    if (value == 0.0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (value < 0.0)
    {
        *buffer++ = '-';
        value = -value;
    }

    exponent = normalize(&value);

    int digit = value * 10.0;
    *buffer++ = digit + '0';
    value = value * 10.0 - digit;
    --exponent;

    *buffer++ = '.';

    for (int i = 0; i < width; i++)
    {
        int digit = value * 10.0;
        *buffer++ = digit + '0';
        value = value * 10.0 - digit;
    }

    *buffer++ = 'e';
    itoa(buffer, exponent, 10);
}

int printk(const char *format, ...)
{
    va_list parameters;
    va_start(parameters, format);
    int written = 0;
    size_t amount;
    int rejected_bad_specifier = 0;
    while (*format != '\0')
    {
        if (*format != '%')
        {
        print_c:
            amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            print(format, amount);
            format += amount;
            written += amount;
            continue;
        }
        const char *format_begun_at = format;
        if (*(++format) == '%')
            goto print_c;
        if (rejected_bad_specifier)
        {
        incomprehensible_conversion:
            rejected_bad_specifier = 1;
            format = format_begun_at;
            goto print_c;
        }
        if (*format == 'c')
        {
            format++;
            char c = (char)va_arg(parameters, int /* char promotes to int */);
            print(&c, sizeof(c));
        }
        else if (*format == 'd')
        {
            format++;
            char *s;
            itoa(s, va_arg(parameters, int), 10);
            print(s, strlen(s));
        }
        else if (*format == 'f')
        {
            format++;
            char *s;
            ftoa_fixed(s, va_arg(parameters, double));
            print(s, strlen(s));
        }
        else if (*format == 'e')
        {
            format++;
            char *s;
            ftoa_sci(s, va_arg(parameters, double));
            print(s, strlen(s));
        }
        else if (*format == 'x')
        {
            format++;
            char *s;
            itoa(s, va_arg(parameters, unsigned int), 16);
            print("0x", 2);
            print(s, strlen(s));
        }
        else if (*format == 'p')
        {
            format++;
            char *s;
            const void *ptr = va_arg(parameters, void *);
            uintptr_t uptr = (uintptr_t)ptr;
            itoa(s, uptr, 16);
            print("0x", 2);
            print(s, strlen(s));
        }
        else if (*format == 's')
        {
            format++;
            const char *s = va_arg(parameters, const char *);
            print(s, strlen(s));
        }
        else
        {
            goto incomprehensible_conversion;
        }
    }
    va_end(parameters);
    return written;
}

int get_terminal_row(void)
{
    return terminal_row;
}

int get_terminal_col(void)
{
    return terminal_column;
}

void terminal_set_colors(enum vga_color font_color, enum vga_color background_color)
{
    terminal_color = make_color(font_color, background_color);
    // terminal_buffer = VGA_MEMORY;
    // size_t y;
    // for (y = 0; y < VGA_HEIGHT; y++)
    // {
    //     size_t x;
    //     for (x = 0; x < VGA_WIDTH; x++)
    //     {
    //         const size_t index = y * VGA_WIDTH + x;
    //         terminal_buffer[index] = make_vgaentry('\0', terminal_color);
    //     }
    // }
}

void print_color_options()
{
    printk("\nPlease select a font color. Valid options are:\n");
    terminal_set_colors(COLOR_BLUE, COLOR_BLACK);
    printk("\nCOLOR_BLUE = 1");
    terminal_set_colors(COLOR_GREEN, COLOR_BLACK);
    printk("\nCOLOR_GREEN = 2");
    terminal_set_colors(COLOR_CYAN, COLOR_BLACK);
    printk("\nCOLOR_CYAN = 3");
    terminal_set_colors(COLOR_RED, COLOR_BLACK);
    printk("\nCOLOR_RED = 4");
    terminal_set_colors(COLOR_MAGENTA, COLOR_BLACK);
    printk("\nCOLOR_MAGENTA = 5");
    terminal_set_colors(COLOR_BROWN, COLOR_BLACK);
    printk("\nCOLOR_BROWN = 6");
    terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
    printk("\nCOLOR_LIGHT_GREY = 7");
    terminal_set_colors(COLOR_DARK_GREY, COLOR_BLACK);
    printk("\nCOLOR_DARK_GREY = 8");
    terminal_set_colors(COLOR_LIGHT_BLUE, COLOR_BLACK);
    printk("\nCOLOR_LIGHT_BLUE = 9");
    terminal_set_colors(COLOR_LIGHT_GREEN, COLOR_BLACK);
    printk("\nCOLOR_LIGHT_GREEN = 10");
    terminal_set_colors(COLOR_LIGHT_CYAN, COLOR_BLACK);
    printk("\nCOLOR_LIGHT_CYAN = 11");
    terminal_set_colors(COLOR_LIGHT_RED, COLOR_BLACK);
    printk("\nCOLOR_LIGHT_RED = 12");
    terminal_set_colors(COLOR_LIGHT_MAGENTA, COLOR_BLACK);
    printk("\nCOLOR_LIGHT_MAGENTA = 13");
    terminal_set_colors(COLOR_LIGHT_BROWN, COLOR_BLACK);
    printk("\nCOLOR_LIGHT_BROWN = 14");
    terminal_set_colors(COLOR_WHITE, COLOR_BLACK);
    printk("\nCOLOR_WHITE = 15\n");
    terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
    print_prompt();
}

enum vga_color change_font_color()
{
    enum vga_color font_color;
    char buffer[BUFFER_SIZE];
    uint8_t byte;
    uint32_t color;

    memset(buffer, 0, BUFFER_SIZE);
    strcpy(&buffer[strlen(buffer)], "");

    print_color_options();

    while (true)
    {
        while (byte = scan())
        {
            if (byte == ENTER)
            {
                switch (color)
                {
                case 1:
                    terminal_set_colors(COLOR_BLUE, COLOR_BLACK);
                    font_color = COLOR_BLUE;
                    return font_color;
                    break;
                case 2:
                    terminal_set_colors(COLOR_GREEN, COLOR_BLACK);
                    font_color = COLOR_GREEN;
                    return font_color;
                    break;
                case 3:
                    terminal_set_colors(COLOR_CYAN, COLOR_BLACK);
                    font_color = COLOR_CYAN;
                    return font_color;
                    break;
                case 4:
                    terminal_set_colors(COLOR_RED, COLOR_BLACK);
                    font_color = COLOR_RED;
                    return font_color;
                    break;
                case 5:
                    terminal_set_colors(COLOR_MAGENTA, COLOR_BLACK);
                    font_color = COLOR_MAGENTA;
                    return font_color;
                    break;
                case 6:
                    terminal_set_colors(COLOR_BROWN, COLOR_BLACK);
                    font_color = COLOR_BROWN;
                    return font_color;
                    break;
                case 7:
                    terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
                    font_color = COLOR_LIGHT_GREY;
                    return font_color;
                    break;
                case 8:
                    terminal_set_colors(COLOR_DARK_GREY, COLOR_BLACK);
                    font_color = COLOR_DARK_GREY;
                    return font_color;
                    break;
                case 9:
                    terminal_set_colors(COLOR_LIGHT_BLUE, COLOR_BLACK);
                    font_color = COLOR_LIGHT_BLUE;
                    return font_color;
                    break;
                case 10:
                    terminal_set_colors(COLOR_LIGHT_GREEN, COLOR_BLACK);
                    font_color = COLOR_LIGHT_GREEN;
                    return font_color;
                    break;
                case 11:
                    terminal_set_colors(COLOR_LIGHT_CYAN, COLOR_BLACK);
                    font_color = COLOR_LIGHT_CYAN;
                    return font_color;
                    break;
                case 12:
                    terminal_set_colors(COLOR_LIGHT_RED, COLOR_BLACK);
                    font_color = COLOR_LIGHT_RED;
                    return font_color;
                    break;
                case 13:
                    terminal_set_colors(COLOR_LIGHT_MAGENTA, COLOR_BLACK);
                    font_color = COLOR_LIGHT_MAGENTA;
                    return font_color;
                    break;
                case 14:
                    terminal_set_colors(COLOR_LIGHT_BROWN, COLOR_BLACK);
                    font_color = COLOR_LIGHT_BROWN;
                    return font_color;
                    break;
                case 15:
                    terminal_set_colors(COLOR_WHITE, COLOR_BLACK);
                    font_color = COLOR_WHITE;
                    return font_color;
                    break;
                default:
                    print_color_options();
                    memset(buffer, 0, BUFFER_SIZE);
                }
            }
            else
            {
                char *s;
                char c = normalmap[byte];
                s = ctos(s, c);
                printk("%s", s);
                strcpy(&buffer[strlen(buffer)], s);
                color = atoi(buffer);
            }
            move_cursor(get_terminal_row(), get_terminal_col());
        }
    }
}

// Set VGA font height by programming the Maximum Scan Line register
static void set_vga_font_height(uint8_t scan_lines)
{
    // Select Maximum Scan Line register (index 0x09) in CRTC
    output_bytes(0x3D4, 0x09);
    
    // Read current value
    uint8_t val = input_bytes(0x3D5);
    
    // Clear lower 5 bits and set new scan line value (scan_lines - 1)
    val = (val & 0xE0) | ((scan_lines - 1) & 0x1F);
    
    // Write back
    output_bytes(0x3D5, val);
}

// Get current font scale
font_scale_t get_font_scale(void)
{
    return current_font_scale;
}

// Increase font size (make text appear larger)
void increase_font_size(void)
{
    if (current_font_scale == FONT_SIZE_SMALL)
    {
        current_font_scale = FONT_SIZE_NORMAL;
        set_vga_font_height(14); // Normal: 8x14 font
        printk("\nFont size: Normal\n");
    }
    else if (current_font_scale == FONT_SIZE_NORMAL)
    {
        current_font_scale = FONT_SIZE_LARGE;
        set_vga_font_height(8); // Large: 8x8 font (fewer scan lines = bigger chars)
        printk("\nFont size: Large\n");
    }
    else
    {
        printk("\nFont size already at maximum (Large)\n");
    }
}

// Decrease font size (make text appear smaller)
void decrease_font_size(void)
{
    if (current_font_scale == FONT_SIZE_LARGE)
    {
        current_font_scale = FONT_SIZE_NORMAL;
        set_vga_font_height(14); // Normal: 8x14 font
        printk("\nFont size: Normal\n");
    }
    else if (current_font_scale == FONT_SIZE_NORMAL)
    {
        current_font_scale = FONT_SIZE_SMALL;
        set_vga_font_height(16); // Small: 8x16 font (more scan lines = smaller chars)
        printk("\nFont size: Small\n");
    }
    else
    {
        printk("\nFont size already at minimum (Small)\n");
    }
}