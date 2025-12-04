#include "../include/tty.h"
#include "../include/string.h"
#include "../include/utils.h"
#include "../include/cpu_utils.h"


#define MAX_PROCESSES 10


// =================== TABLE ===================
void print_sjf_table(Process proc[], int n)
{
    char buf[16];
    int total_wt = 0, total_tt = 0;
    int width_pid = strlen("Process"), width_bt = strlen("Burst");
    int width_wt = strlen("Waiting"), width_tt = strlen("Turnaround");

    for (int i = 0; i < n; i++)
    {
        itoas(proc[i].pid, buf);
        if (strlen(buf) > width_pid)
            width_pid = strlen(buf);
        itoas(proc[i].burst_time, buf);
        if (strlen(buf) > width_bt)
            width_bt = strlen(buf);
        itoas(proc[i].waiting_time, buf);
        if (strlen(buf) > width_wt)
            width_wt = strlen(buf);
        itoas(proc[i].turnaround_time, buf);
        if (strlen(buf) > width_tt)
            width_tt = strlen(buf);
    }

    // Top border
    printk("+");
    for (int i = 0; i < width_pid; i++)
        printk("-");
    printk("+");
    for (int i = 0; i < width_bt; i++)
        printk("-");
    printk("+");
    for (int i = 0; i < width_wt; i++)
        printk("-");
    printk("+");
    for (int i = 0; i < width_tt; i++)
        printk("-");
    printk("+\n");

    // Header
    printk("|");
    print_center("Process", width_pid);
    printk("|");
    print_center("Burst", width_bt);
    printk("|");
    print_center("Waiting", width_wt);
    printk("|");
    print_center("Turnaround", width_tt);
    printk("|\n");

    // Separator
    printk("+");
    for (int i = 0; i < width_pid; i++)
        printk("-");
    printk("+");
    for (int i = 0; i < width_bt; i++)
        printk("-");
    printk("+");
    for (int i = 0; i < width_wt; i++)
        printk("-");
    printk("+");
    for (int i = 0; i < width_tt; i++)
        printk("-");
    printk("+\n");

    // Rows
    for (int i = 0; i < n; i++)
    {
        printk("|");
        itoas(proc[i].pid, buf);
        print_center(buf, width_pid);
        printk("|");
        itoas(proc[i].burst_time, buf);
        print_center(buf, width_bt);
        printk("|");
        itoas(proc[i].waiting_time, buf);
        print_center(buf, width_wt);
        printk("|");
        itoas(proc[i].turnaround_time, buf);
        print_center(buf, width_tt);
        printk("|\n");
        total_wt += proc[i].waiting_time;
        total_tt += proc[i].turnaround_time;
    }

    // Bottom border
    printk("+");
    for (int i = 0; i < width_pid; i++)
        printk("-");
    printk("+");
    for (int i = 0; i < width_bt; i++)
        printk("-");
    printk("+");
    for (int i = 0; i < width_wt; i++)
        printk("-");
    printk("+");
    for (int i = 0; i < width_tt; i++)
        printk("-");
    printk("+\n");

    // Averages
    printk("Average Waiting Time   : ");
    print_float((float)total_wt / n);
    printk("\nAverage Turnaround Time: ");
    print_float((float)total_tt / n);
    printk("\n");
}


// =================== SJF FUNCTION ===================
void sjf()
{
    Process proc[MAX_PROCESSES];
    int n;
    printk("\nEnter number of processes: ");
    n = read_int();
    if (n <= 0 || n > MAX_PROCESSES)
    {
        printk("Invalid number of processes!\n");
        return;
    }

    for (int i = 0; i < n; i++)
    {
        proc[i].pid = i + 1;
        printk("Enter burst time for P%d: ", proc[i].pid);
        proc[i].burst_time = read_int();
    }

    // --------- Sort by burst time (SJF non-preemptive) ---------
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (proc[j].burst_time < proc[i].burst_time)
            {
                Process tmp = proc[i];
                proc[i] = proc[j];
                proc[j] = tmp;
            }

    // --------- Calculate waiting and turnaround times ---------
    proc[0].waiting_time = 0;
    for (int i = 1; i < n; i++)
        proc[i].waiting_time = proc[i - 1].waiting_time + proc[i - 1].burst_time;
    for (int i = 0; i < n; i++)
        proc[i].turnaround_time = proc[i].waiting_time + proc[i].burst_time;

    printk("\nSJF Scheduling Table:\n");
    print_sjf_table(proc, n);
    printk("\nGantt Chart:\n");
    print_gantt_chart_vga(proc, n);
}