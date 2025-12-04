bits 32                  ; Assemble in 32-bit mode for protected mode kernel
global _start            ; Export the _start symbol for linker
extern kernel_early      ; Declare external function kernel_early
extern main              ; Declare external function main

section .text
    align 4              ; Align instructions/data to 4-byte boundary (optional, better performance)
    dd 0x1BADB002        ; Multiboot magic number required by GRUB for Multiboot compliance
    dd 0x00               ; Multiboot flags (0 for default settings)
    dd - (0x1BAD002 + 0x00) ; Checksum = -(magic + flags), required by Multiboot spec

_start:
    cli                   ; Disable interrupts during early kernel setup
    mov esp, stack        ; Initialize stack pointer to start of reserved stack memory
    call kernel_early     ; Call early initialization routine before main (e.g., set up memory, drivers)
    call main             ; Call the main kernel function
    hlt                   ; Halt CPU (stop execution) once main returns

section .bss
    resb 8192             ; Reserve 8 KB for the kernel stack

stack:                   ; Label marking the start of stack memory
