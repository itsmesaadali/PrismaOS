#include "../include/tty.h"
#include "../include/string.h"
#include "../include/utils.h"
#include "../include/cpu_utils.h"


#define MAX_PROCESSES 10


void priority_preemptive()
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

        proc[i].waiting_time = 0;
        proc[i].turnaround_time = 0;
    }

    // ===== Create remaining time array =====
    int remaining[MAX_PROCESSES];
    for (int i = 0; i < n; i++)
        remaining[i] = proc[i].burst_time;

    // ===== Preemptive Simulation =====
    int completed = 0;
    int time = 0;
    int last_pid = -1;

    // Used for gantt storage
    Process gantt[256];
    int gcount = 0;

    while (completed < n)
    {
        int best = -1;
        int best_priority = 999999;

        // Find highest priority (lowest priority number)
        for (int i = 0; i < n; i++)
        {
            if (remaining[i] > 0 && proc[i].priority < best_priority)
            {
                best = i;
                best_priority = proc[i].priority;
            }
        }

        if (best == -1) break; // Safety

        // ----------- Store Gantt sequence -----------
        if (last_pid != proc[best].pid)
        {
            gantt[gcount].pid = proc[best].pid;
            gantt[gcount].burst_time = 1;  // 1 time unit at a time
            gcount++;
            last_pid = proc[best].pid;
        }
        else
        {
            gantt[gcount - 1].burst_time++;
        }

        // ---- Execute 1 time unit ----
        remaining[best]--;
        time++;

        // ---- If completed ----
        if (remaining[best] == 0)
        {
            proc[best].turnaround_time = time;
            proc[best].waiting_time = proc[best].turnaround_time - proc[best].burst_time;
            completed++;
        }
    }

    // ===== Print table =====
    printk("\nPriority (Preemptive) Scheduling Table:\n");
    print_priority_table(proc, n);

    // ===== Print Gantt Chart =====
    printk("\nGantt Chart:\n");
    print_gantt_chart_vga(gantt, gcount);
}