#include "../include/tty.h"
#include "../include/string.h"
#include "../include/utils.h"
#include "../include/cpu_utils.h"


#define MAX_PROCESSES 10


void priority_non_preemptive()
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

    // ----- Input -----
    for (int i = 0; i < n; i++)
    {
        proc[i].pid = i + 1;

        printk("Enter burst time for P%d: ", proc[i].pid);
        proc[i].burst_time = read_int();

        printk("Enter priority for P%d (lower = higher priority): ", proc[i].pid);
        proc[i].priority = read_int();
    }

    // ----- Sort by priority -----
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (proc[j].priority < proc[i].priority)
            {
                Process tmp = proc[i];
                proc[i] = proc[j];
                proc[j] = tmp;
            }

    // ----- Compute WT & TAT -----
    proc[0].waiting_time = 0;

    for (int i = 1; i < n; i++)
        proc[i].waiting_time = proc[i - 1].waiting_time + proc[i - 1].burst_time;

    for (int i = 0; i < n; i++)
        proc[i].turnaround_time = proc[i].waiting_time + proc[i].burst_time;

    // ----- Output -----
    printk("\nPriority (Non-Preemptive) Scheduling Table:\n");
    print_priority_table(proc, n);

    printk("\nGantt Chart:\n");
    print_gantt_chart_vga(proc, n);
}