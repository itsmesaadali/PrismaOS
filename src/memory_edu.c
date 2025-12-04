#include "../include/tty.h"
#include "../include/string.h"
#include "../include/memory.h"
#include "../include/utils.h"
#include "../include/io.h"

Block mem[MAX_BLOCKS];
int block_count = 1;

#define MEMVIS_WIDTH 60   // Visual bar width

// ========================= INIT ===========================
void memory_init() {
    mem[0].pid = -1;
    mem[0].start = 0;
    mem[0].size = TOTAL_MEMORY;
    mem[0].free = 1;
    block_count = 1;
}

// ========================= HELPERS =========================
void merge_free_blocks() {
    for (int i = 0; i < block_count - 1; i++) {
        if (mem[i].free && mem[i + 1].free) {
            mem[i].size += mem[i + 1].size;

            // Shift left
            for (int j = i + 1; j < block_count - 1; j++)
                mem[j] = mem[j + 1];

            block_count--;
            i--;
        }
    }
}

void split_block(int index, int pid, int size) {
    Block old = mem[index];

    mem[index].pid = pid;
    mem[index].free = 0;
    mem[index].size = size;

    // create remainder free block
    if (old.size > size) {
        for (int i = block_count; i > index + 1; i--)
            mem[i] = mem[i - 1];

        mem[index + 1].pid = -1;
        mem[index + 1].free = 1;
        mem[index + 1].start = old.start + size;
        mem[index + 1].size = old.size - size;

        block_count++;
    }
}

// ========================= ALLOCATION ======================
int find_block(int size, int strategy) {
    int best = -1;

    for (int i = 0; i < block_count; i++) {
        if (!mem[i].free || mem[i].size < size)
            continue;

        if (strategy == 1) return i; // FIRST-FIT
        if (strategy == 2) { // BEST-FIT
            if (best == -1 || mem[i].size < mem[best].size)
                best = i;
        }
        if (strategy == 3) { // WORST-FIT
            if (best == -1 || mem[i].size > mem[best].size)
                best = i;
        }
    }
    return best;
}

void allocate_memory() {
    int pid, size, strategy;
    
    kbd_flush(); // Clear keyboard buffer
    
    printk("\n--- Allocate Memory ---\n");
    printk("Enter PID: ");
    pid = read_int();
    
    if (pid < 0) {
        printk("\nInvalid PID!\n");
        return;
    }

    printk("Enter size (bytes): ");
    size = read_int();
    
    if (size <= 0 || size > TOTAL_MEMORY) {
        printk("\nInvalid size!\n");
        return;
    }

    printk("\nStrategy:\n");
    printk("  1. First-Fit\n");
    printk("  2. Best-Fit\n");
    printk("  3. Worst-Fit\n");
    printk("Choice: ");
    strategy = read_int();
    
    if (strategy < 1 || strategy > 3) {
        printk("\nInvalid strategy!\n");
        return;
    }

    int index = find_block(size, strategy);

    if (index == -1) {
        printk("\n[ERROR] Allocation failed - no suitable block found\n");
        return;
    }

    split_block(index, pid, size);
    printk("\n[SUCCESS] Allocated %d bytes to Process %d\n", size, pid);
}

// ========================= FREE ============================
void free_memory() {
    int pid;
    
    kbd_flush(); // Clear keyboard buffer
    
    printk("\n--- Free Memory ---\n");
    printk("Enter PID to free: ");
    pid = read_int();
    
    if (pid < 0) {
        printk("\nInvalid PID!\n");
        return;
    }

    int found = 0;

    for (int i = 0; i < block_count; i++) {
        if (!mem[i].free && mem[i].pid == pid) {
            mem[i].pid = -1;
            mem[i].free = 1;
            found = 1;
        }
    }

    if (!found) {
        printk("\n[ERROR] Process %d not found!\n", pid);
        return;
    }

    merge_free_blocks();
    printk("\n[SUCCESS] Memory freed for Process %d\n", pid);
}

// ========================= VISUAL ==========================
void print_memory_visual() {
    // Clear any pending keyboard input first
    kbd_flush();
    
    printk("\n");
    printk("=======================================================\n");
    printk("           MEMORY VISUALIZATION (%d bytes)          \n", TOTAL_MEMORY);
    printk("=======================================================\n");
    
    // Calculate statistics
    int total_allocated = 0;
    int total_free = 0;
    int process_count = 0;
    
    for (int i = 0; i < block_count; i++) {
        if (mem[i].free) {
            total_free += mem[i].size;
        } else {
            total_allocated += mem[i].size;
            process_count++;
        }
    }
    
    // Print statistics
    printk("\nStatistics:\n");
    printk("  Total Blocks: %d\n", block_count);
    printk("  Processes: %d\n", process_count);
    printk("  Allocated: %d bytes\n", total_allocated);
    printk("  Free: %d bytes\n\n", total_free);
    
    // Print memory blocks
    printk("Memory Layout:\n");
    printk("-------------------------------------------------------\n");
    
    for (int i = 0; i < block_count; i++) {
        // Calculate visual bar length
        int bar_len = (mem[i].size * MEMVIS_WIDTH) / TOTAL_MEMORY;
        if (bar_len < 1) bar_len = 1;
        if (bar_len > MEMVIS_WIDTH) bar_len = MEMVIS_WIDTH;
        
        // Print block number
        printk("%2d| ", i + 1);
        
        // Print visual bar with appropriate color
        if (mem[i].free) {
            // Free block - use dark grey
            terminal_set_colors(COLOR_WHITE, COLOR_DARK_GREY);
            for (int j = 0; j < bar_len; j++) {
                printk(" ");
            }
            terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
            
            printk(" FREE [%d bytes]", mem[i].size);
        } else {
            // Allocated block - use green
            terminal_set_colors(COLOR_BLACK, COLOR_GREEN);
            for (int j = 0; j < bar_len; j++) {
                printk(" ");
            }
            terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
            
            printk(" PID:%d [%d bytes]", mem[i].pid, mem[i].size);
        }
        
        printk("\n");
    }
    
    printk("-------------------------------------------------------\n");
    
    // Legend
    terminal_set_colors(COLOR_BLACK, COLOR_GREEN);
    printk("   ");
    terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
    printk(" = Allocated   ");
    
    terminal_set_colors(COLOR_WHITE, COLOR_DARK_GREY);
    printk("   ");
    terminal_set_colors(COLOR_LIGHT_GREY, COLOR_BLACK);
    printk(" = Free\n");
    
    printk("=======================================================\n");
    
    // Final flush to ensure clean state
    kbd_flush();
}

// ======================== MENU ==============================
void memory_menu() {
    int ch;
    
    memory_init();  // Initialize memory system

    while (1) {
        // Clear keyboard buffer before showing menu
        kbd_flush();
        
        printk("\n");
        printk("***************************************************\n");
        printk("*          MEMORY MANAGEMENT SIMULATOR           *\n");
        printk("***************************************************\n");
        printk("\n");
        printk("  1. Allocate Memory\n");
        printk("  2. Free Memory\n");
        printk("  3. Show Memory Visualization\n");
        printk("  4. Reset Memory\n");
        printk("  5. Exit\n");
        printk("\n");
        printk("Enter your choice: ");

        ch = read_int();

        switch (ch) {
            case 1:
                allocate_memory();
                break;
            case 2:
                free_memory();
                break;
            case 3:
                print_memory_visual();
                break;
            case 4:
                memory_init();
                printk("\n[SUCCESS] Memory has been reset!\n");
                break;
            case 5:
                printk("\nExiting Memory Manager...\n");
                return;
            default:
                printk("\n[ERROR] Invalid choice! Please enter 1-5.\n");
                break;
        }
    }
}
