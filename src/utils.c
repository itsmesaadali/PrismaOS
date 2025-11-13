#include "../include/utils.h"
#include "../include/tty.h"
#include <kbd.h>

void print_logo()
{
    printk("\t                                          \n");
    printk("\t\t\t\t   _____                     ____    _____ \n");
    printk("\t\t\t\t  / ____|                   / __ \\  / ____|\n");
    printk("\t\t\t\t | |      ___   _ __   ___ | |  | || (___  \n");
    printk("\t\t\t\t | |     / _ \\ | '__| / _ \\| |  | | \\___ \\ \n");
    printk("\t\t\t\t | |____| (_) || |   |  __/| |__| | ____) |\n");
    printk("\t\t\t\t  \\_____|\\___/ |_|    \\___| \\____/ |_____/ \n");
    printk("\t                                          \n");
}

void about(char *version)
{
    printk("\n\tCoreOS - v%s - A simple 32-bit Ring 0 operating system", version);
    printk("\n\tProvided under the GNU General Public License v3.0");
    printk("\n\tCreated by: Saad Ali, Ghazanfar Pasha & Roshaan Idrees");
    printk("\n\tGitHub repository: https://github.com/Teams-Workspace/CoreOS");
    printk("\n\tContributors:");
    printk("\n\t-> https://github.com/0X1Saad");
    printk("\n\t-> https://github.com/itsmesaadali");
    printk("\n");
}

int read_int() {
    char buf[16];
    memset(buf, 0, sizeof(buf));
    int idx = 0;
    uint8_t byte;

    while (1) {
        byte = scan();  // reads scancode

        if (byte == ENTER) {
            printk("\n");
            break;
        }

        char c = normalmap[byte];  // convert scancode → ASCII using your existing keymap
        if (c >= '0' && c <= '9') {
            buf[idx++] = c;
            printk("%c", c);
        } else if (byte == BACKSPACE && idx > 0) {
            idx--;
            printk("\b \b");
        }
    }

    return atoi(buf);
}
