#include "../include/tty.h"
#include "../include/string.h"
#include "../include/utils.h"
#include "../include/cpu_utils.h"

#define MAX_PROCESSES 10


void print_srtf_table(Process proc[], int n)
{
    char buf[16];
    int total_wt = 0, total_tt = 0;
    int width_pid = strlen("PID");
    int width_arr = strlen("Arrival");
    int width_bt = strlen("Burst");
    int width_wt = strlen("Waiting");
    int width_tt = strlen("Turnaround");

    // Determine column widths dynamically
    for (int i = 0; i < n; i++)
    {
        itoas(proc[i].pid, buf);
        if (strlen(buf) > width_pid) width_pid = strlen(buf);

        itoas(proc[i].arrival_time, buf);
        if (strlen(buf) > width_arr) width_arr = strlen(buf);

        itoas(proc[i].burst_time, buf);
        if (strlen(buf) > width_bt) width_bt = strlen(buf);

        itoas(proc[i].waiting_time, buf);
        if (strlen(buf) > width_wt) width_wt = strlen(buf);

        itoas(proc[i].turnaround_time, buf);
        if (strlen(buf) > width_tt) width_tt = strlen(buf);
    }

    // Top border
    printk("+");
    for (int i = 0; i < width_pid; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_arr; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_bt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_wt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_tt; i++) printk("-");
    printk("+\n");

    // Header
    printk("|");
    print_center("PID", width_pid);
    printk("|");
    print_center("Arrival", width_arr);
    printk("|");
    print_center("Burst", width_bt);
    printk("|");
    print_center("Waiting", width_wt);
    printk("|");
    print_center("Turnaround", width_tt);
    printk("|\n");

    // Separator
    printk("+");
    for (int i = 0; i < width_pid; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_arr; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_bt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_wt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_tt; i++) printk("-");
    printk("+\n");

    // Rows
    for (int i = 0; i < n; i++)
    {
        printk("|");
        itoas(proc[i].pid, buf);
        print_center(buf, width_pid);

        printk("|");
        itoas(proc[i].arrival_time, buf);
        print_center(buf, width_arr);

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
    for (int i = 0; i < width_pid; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_arr; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_bt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_wt; i++) printk("-");
    printk("+");
    for (int i = 0; i < width_tt; i++) printk("-");
    printk("+\n");

    // Averages
    printk("Average Waiting Time   : ");
    print_float((float)total_wt / n);
    printk("\nAverage Turnaround Time: ");
    print_float((float)total_tt / n);
    printk("\n");
}

void srtf()
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

        printk("Enter arrival time for P%d: ", proc[i].pid);
        proc[i].arrival_time = read_int();

        proc[i].waiting_time = 0;
        proc[i].turnaround_time = 0;
    }

    // ===== Remaining time array =====
    int remaining[MAX_PROCESSES];
    for (int i = 0; i < n; i++)
        remaining[i] = proc[i].burst_time;

    int completed = 0;
    int time = 0;
    int last_pid = -1;

    Process gantt[256];
    int gcount = 0;

    // ===== Preemptive SRTF Simulation =====
    while (completed < n)
    {
        int best = -1;
        int min_remaining = 999999;

        // Find process with shortest remaining time among arrived processes
        for (int i = 0; i < n; i++)
        {
            if (proc[i].arrival_time <= time && remaining[i] > 0)
            {
                if (remaining[i] < min_remaining)
                {
                    min_remaining = remaining[i];
                    best = i;
                }
            }
        }

        // No process has arrived yet → CPU idle
        if (best == -1)
        {
            time++;
            continue;
        }

        // ===== Store Gantt Chart info =====
        if (last_pid != proc[best].pid)
        {
            gantt[gcount].pid = proc[best].pid;
            gantt[gcount].burst_time = 1; // 1 unit
            gcount++;
            last_pid = proc[best].pid;
        }
        else
        {
            gantt[gcount - 1].burst_time++;
        }

        // Execute 1 unit
        remaining[best]--;
        time++;

        // Completed?
        if (remaining[best] == 0)
        {
            proc[best].turnaround_time = time - proc[best].arrival_time;
            proc[best].waiting_time = proc[best].turnaround_time - proc[best].burst_time;
            completed++;
        }
    }

    // ===== Print Results =====
    printk("\nSRTF (Preemptive) Scheduling Table:\n");
    print_srtf_table(proc, n);

    // ===== Print Gantt Chart =====
    printk("\nGantt Chart:\n");
    print_gantt_chart_vga(gantt, gcount);
}