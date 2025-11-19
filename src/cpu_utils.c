#include "../include/tty.h"
#include "../include/string.h"
#include "cpu_utils.h"
#include "../include/utils.h" // for printk()

// ================= Float printing =================
void print_float(float val) {
    int int_part = (int)val;
    float frac = val - (float)int_part;
    if (frac < 0) frac = -frac;
    int decimal = (int)(frac * 100);

    char buf[32], tmp[16];
    strcpy(buf, "");
    itoas(int_part, tmp);
    strcat(buf, tmp);
    strcat(buf, ".");
    if (decimal < 10) strcat(buf, "0");
    itoas(decimal, tmp);
    strcat(buf, tmp);
    printk("%s", buf);
}

// ================= Integer to string =================
void itoas(int num, char *str) {
    int i = 0, is_neg = 0;
    if (num == 0) { str[i++] = '0'; str[i] = '\0'; return; }
    if (num < 0) { is_neg = 1; num = -num; }
    char temp[10]; int j = 0;
    while (num != 0) { temp[j++] = (num % 10) + '0'; num /= 10; }
    if (is_neg) str[i++] = '-';
    while (j > 0) str[i++] = temp[--j];
    str[i] = '\0';
}

// ================= Center text =================
void print_center(const char *text, int width) {
    int len = strlen(text);
    int pad = (width - len) / 2;
    for (int i = 0; i < pad; i++) printk(" ");
    printk("%s", text);
    for (int i = 0; i < width - len - pad; i++) printk(" ");
}


// ================= Gantt Chart with VGA Colors =================
void print_gantt_chart_vga(Process proc[], int n) {
    int time = 0;
    int unit_width = 2; // Width per time unit
    int colors[] = {
        COLOR_RED, COLOR_GREEN, COLOR_LIGHT_BLUE, 
        COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN
    };

    // ---------- Top border ----------
    terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
    printk("  ");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < proc[i].burst_time * unit_width; j++)
            printk("-");
        printk(" ");
    }

    // ---------- Process row ----------
    printk("\n|");
    for (int i = 0; i < n; i++) {
        int pid_len = 2; // P1, P2 etc.
        int total_width = proc[i].burst_time * unit_width;
        int left_pad = (total_width - pid_len) / 2;
        int right_pad = total_width - pid_len - left_pad;

        // Print left padding
        terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
        for (int j = 0; j < left_pad; j++) printk(" ");

        // Set process color and print process label
        terminal_set_colors(COLOR_BLACK, colors[i % 6]); // font black, background colored
        char buf[4];
        itoas(proc[i].pid, buf);
        printk("P%s", buf);

        // Print right padding
        for (int j = 0; j < right_pad; j++) printk(" ");
        terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
        printk("|");
    }

    // ---------- Bottom border ----------
    printk("\n  ");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < proc[i].burst_time * unit_width; j++)
            printk("-");
        printk(" ");
    }

    // ---------- Time row ----------
    printk("\n0");
    time = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < proc[i].burst_time * unit_width; j++) printk(" ");
        time += proc[i].burst_time;
        printk("%d", time);
    }
    printk("\n\n");

    // Reset colors
    terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
}


void print_priority_table(Process proc[], int n)
{
    char buf[16];
    int total_wt = 0, total_tt = 0;

    int width_pid = strlen("Process");
    int width_bt  = strlen("Burst");
    int width_pr  = strlen("Priority");
    int width_wt  = strlen("Waiting");
    int width_tt  = strlen("Turnaround");

    for (int i = 0; i < n; i++)
    {
        itoas(proc[i].pid, buf);
        if (strlen(buf) > width_pid) width_pid = strlen(buf);

        itoas(proc[i].burst_time, buf);
        if (strlen(buf) > width_bt) width_bt = strlen(buf);

        itoas(proc[i].priority, buf);
        if (strlen(buf) > width_pr) width_pr = strlen(buf);

        itoas(proc[i].waiting_time, buf);
        if (strlen(buf) > width_wt) width_wt = strlen(buf);

        itoas(proc[i].turnaround_time, buf);
        if (strlen(buf) > width_tt) width_tt = strlen(buf);
    }

    // --------- Print top border ---------
    printk("+");
    for (int i = 0; i < width_pid; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_bt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_pr; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_wt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_tt; i++) printk("-");
    printk("+\n");

    // --------- Header ---------
    printk("|"); print_center("Process", width_pid);
    printk("|"); print_center("Burst", width_bt);
    printk("|"); print_center("Priority", width_pr);
    printk("|"); print_center("Waiting", width_wt);
    printk("|"); print_center("Turnaround", width_tt);
    printk("|\n");

    // --------- Separator ---------
    printk("+");
    for (int i = 0; i < width_pid; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_bt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_pr; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_wt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_tt; i++) printk("-");
    printk("+\n");

    // --------- Rows ---------
    for (int i = 0; i < n; i++)
    {
        printk("|");
        itoas(proc[i].pid, buf); print_center(buf, width_pid);

        printk("|");
        itoas(proc[i].burst_time, buf); print_center(buf, width_bt);

        printk("|");
        itoas(proc[i].priority, buf); print_center(buf, width_pr);

        printk("|");
        itoas(proc[i].waiting_time, buf); print_center(buf, width_wt);

        printk("|");
        itoas(proc[i].turnaround_time, buf); print_center(buf, width_tt);
        printk("|\n");

        total_wt += proc[i].waiting_time;
        total_tt += proc[i].turnaround_time;
    }

    // --------- Bottom border ---------
    printk("+");
    for (int i = 0; i < width_pid; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_bt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_pr; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_wt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_tt; i++) printk("-");
    printk("+\n");

    // --------- Averages ---------
    printk("Average Waiting Time   : ");
    print_float((float)total_wt / n);

    printk("\nAverage Turnaround Time: ");
    print_float((float)total_tt / n);

    printk("\n");
}
