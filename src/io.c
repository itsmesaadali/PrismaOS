#include "../include/io.h"
#include "../include/tty.h"

void shutdown()
{
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
}

void reboot()
{
    uint8_t temp;

    asm volatile("cli"); /* disable all interrupts */

    /* Clear all keyboard buffers (output and command buffers) */
    do
    {
        temp = input_bytes(KBRD_INTRFC); /* empty user data */
        if (check_flag(temp, KBRD_BIT_KDATA) != 0)
            input_bytes(KBRD_IO); /* empty keyboard data */
    } while (check_flag(temp, KBRD_BIT_UDATA) != 0);

    output_bytes(KBRD_INTRFC, KBRD_RESET); /* pulse CPU reset line */
loop:
    asm volatile("hlt"); /* if that didn't work, halt the CPU */
    goto loop;           /* if a NMI is received, halt again */
}

uint8_t input_bytes(uint16_t port)
{
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0"
                         : "= a"(ret)
                         : "Nd"(port));
    return ret;
}

void output_bytes(uint16_t port, uint8_t val)
{
    __asm__ __volatile__("outb %0, %1"
                         :
                         : "a"(val), "Nd"(port));
}

uint8_t inw(uint16_t port)
{
    uint8_t ret;
    __asm__ __volatile__("in %1, %0"
                         : "= a"(ret)
                         : "d"(port));
    return ret;
}

void outw(uint16_t port, uint16_t data)
{
    __asm__ __volatile__("out %0, %1"
                         :
                         : "a"(data), "d"(port));
}

void kbd_flush(void)
{
    uint8_t temp;
    int timeout = 1000;
    
    while (timeout-- > 0)
    {
        temp = input_bytes(KBRD_INTRFC);
        if (check_flag(temp, KBRD_BIT_KDATA) != 0)
            input_bytes(KBRD_IO);
        else
            break;
    }
}

uint8_t scan(void)
{
    unsigned char brk;
    static uint8_t key = 0;
    uint8_t read_char = input_bytes(0x60); // keyboard port
    brk = read_char & 0x80;
    read_char = read_char & 0x7f;
    if (brk)
    {
        return key = 0;
    }
    else if (read_char != key)
    {
        return key = read_char;
    }
    else
    {
        return 0;
    }
}

void move_cursor(int row, int col)
{
    unsigned short pos = (row * VGA_WIDTH) + col;
    output_bytes(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    output_bytes(FB_DATA_PORT, (unsigned char)(pos & 0xFF));
    output_bytes(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    output_bytes(FB_DATA_PORT, (unsigned char)((pos >> 8) & 0xFF));
}

void enable_hardware_cursor(unsigned char start, unsigned char end)
{
    /* Cursor start register = 0x0A, cursor end register = 0x0B on CRT controller */
    uint8_t prev;
    output_bytes(FB_COMMAND_PORT, 0x0A);
    prev = input_bytes(FB_DATA_PORT);
    output_bytes(FB_DATA_PORT, (prev & 0xC0) | (start & 0x1F));
    output_bytes(FB_COMMAND_PORT, 0x0B);
    prev = input_bytes(FB_DATA_PORT);
    output_bytes(FB_DATA_PORT, (prev & 0xE0) | (end & 0x1F));
}

void print_prompt(void)
{
    // Print prompt in bright green for better visibility
    terminal_set_colors(COLOR_LIGHT_GREEN, COLOR_BLACK);
    printk("$ > ");
    terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
    move_cursor(get_terminal_row(), get_terminal_col());
}