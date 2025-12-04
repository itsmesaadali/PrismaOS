#include "../include/utils.h"
#include "../include/tty.h"
#include "../include/io.h"
#include "../include/string.h"
#include <kbd.h>

void print_logo()
{
    printk("\n");
    printk("_____                     ____    _____ \n");
    printk("/ ____|                   / __ \\  / ____|\n");
    printk("| |      ___   _ __   ___ | |  | || (___  \n");
    printk("| |     / _ \\ | '__| / _ \\| |  | | \\___ \\ \n");
    printk("| |____| (_) || |   |  __/| |__| | ____) |\n");
    printk("\\_____|\\___/ |_|    \\___| \\____/ |_____/ \n");
    printk("\n");
}

void about(char *version)
{
    printk("CoreOS - v%s - A simple 32-bit Ring 0 operating system\n", version);
    printk("Provided under the GNU General Public License v3.0\n");
    printk("Created by: Saad Ali, Ghazanfar Pasha & Roshaan Idrees\n");
    printk("GitHub repository: https://github.com/Teams-Workspace/CoreOS\n");
    printk("Contributors:\n");
    printk("-> https://github.com/0X1Saad\n");
    printk("-> https://github.com/itsmesaadali\n");
}

int read_int() {
    char buf[16];
    memset(buf, 0, sizeof(buf));
    int idx = 0;
    uint8_t byte;

    // Flush keyboard buffer before reading
    kbd_flush();
    
    // Small delay to ensure keyboard is ready
    for (volatile int i = 0; i < 10000; i++);

    while (1) {
        byte = scan();  // reads scancode
        
        if (byte == 0) continue;  // Skip if no key pressed

        if (byte == ENTER) {
            printk("\n");
            break;
        }

        char c = normalmap[byte];  // convert scancode → ASCII using your existing keymap
        if (c >= '0' && c <= '9') {
            if (idx < 15) {  // Prevent buffer overflow
                buf[idx++] = c;
                printk("%c", c);
            }
        } else if (byte == BACKSPACE && idx > 0) {
            idx--;
            printk("\b \b");
        }
    }

    return atoi(buf);
}
