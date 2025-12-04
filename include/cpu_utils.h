#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include "../include/tty.h"

typedef struct {
    int pid;
    int burst_time;
    int priority;        // used only for priority scheduling
    int arrival_time;    // needed for SRTF and other arrival-based algos
    int waiting_time;
    int turnaround_time;
} Process;



// Utilities
void print_float(float val);               // Print float safely
void itoas(int num, char *str);           // Integer to string
void print_center(const char *text, int width); // Center text in width
void print_gantt_chart_vga(Process proc[], int n); // Print dynamic Gantt chart
void print_priority_table(Process proc[], int n); // Print priority scheduling table

#endif