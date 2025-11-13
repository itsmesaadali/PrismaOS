#include "../include/tty.h"
#include "../include/string.h"
#include "../include/utils.h"

#define MAX_PROCESSES 10

typedef struct {
    int pid;
    int burst_time;
    int waiting_time;
    int turnaround_time;
} Process;

void fcfs() {
    int n;
    Process proc[MAX_PROCESSES];

    printk("\nEnter number of processes: ");
    n = read_int();

    if (n <= 0 || n > MAX_PROCESSES) {
        printk("\nInvalid number of processes!\n");
        return;
    }

    printk("\nEnter Burst Times:\n");
    for (int i = 0; i < n; i++) {
        proc[i].pid = i + 1;
        printk("P%d: ", proc[i].pid);
        proc[i].burst_time = read_int();
    }

    // Calculate waiting & turnaround times
    proc[0].waiting_time = 0;
    for (int i = 1; i < n; i++)
        proc[i].waiting_time = proc[i - 1].waiting_time + proc[i - 1].burst_time;

    for (int i = 0; i < n; i++)
        proc[i].turnaround_time = proc[i].waiting_time + proc[i].burst_time;

    // Print process info table
    printk("\nProcess | Burst | Waiting | Turnaround\n");
    printk("--------------------------------------\n");
    int total_wt = 0, total_tt = 0;
    for (int i = 0; i < n; i++) {
        printk("P%d\t  %d\t   %d\t   %d\n", proc[i].pid, proc[i].burst_time,
               proc[i].waiting_time, proc[i].turnaround_time);
        total_wt += proc[i].waiting_time;
        total_tt += proc[i].turnaround_time;
    }

    printk("\nAverage Waiting Time: %d", total_wt / n);
    printk("\nAverage Turnaround Time: %d\n", total_tt / n);

    // 🔹 Color Gantt Chart Visualization
    printk("\nGantt Chart Visualization:\n\n");

    int time = 0;

    // Top border
    printk("  ");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < proc[i].burst_time; j++)
            printk("--");
        printk(" ");
    }
    printk("\n|");

    // Process bars (colored)
    for (int i = 0; i < n; i++) {
        int color = (i % 6) + 1; // rotate colors
        terminal_set_colors(color, COLOR_BLACK);
        for (int j = 0; j < proc[i].burst_time; j++)
            printk("██");  // visual filled bar
        terminal_set_colors(COLOR_WHITE, COLOR_BLACK);
        printk("|");
    }

    // Bottom border
    printk("\n  ");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < proc[i].burst_time; j++)
            printk("--");
        printk(" ");
    }

    // Process labels and time markings
    printk("\n 0");
    time = 0;
    for (int i = 0; i < n; i++) {
        time += proc[i].burst_time;
        printk("  ");
        for (int j = 0; j < proc[i].burst_time - 1; j++)
            printk("  ");
        printk("%d", time);
    }

    printk("\n\nLegend:\n");
    for (int i = 0; i < n; i++) {
        int color = (i % 6) + 1;
        terminal_set_colors(color, COLOR_BLACK);
        printk("██");
        terminal_set_colors(COLOR_WHITE, COLOR_BLACK);
        printk(" = P%d  ", proc[i].pid);
    }
    printk("\n");
}
