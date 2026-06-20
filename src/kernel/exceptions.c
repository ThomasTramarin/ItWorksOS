#include "exceptions.h"
#include "klib/cui.h"
#include "vga.h"

void exceptions_init(void) {
  for (int i = 0; i < 32; i++) {
    isr_register_handler(i, exceptions_handler);
  }
}

static const char *exception_name[32] = {
    "Divide Error (#DE)",
    "Debug Exception (#DB)",
    "Non-Maskable Interrupt (#NMI)",
    "Breakpoint (#BP)",
    "Overflow (#OF)",
    "Bound Range Exceeded (#BR)",
    "Invalid Opcode (#UD)",
    "Device Not Available (#NM)",
    "Double Fault (#DF)",
    "Coprocessor Segment Overrun (#CSO)",
    "Invalid TSS (#TS)",
    "Segment Not Present (#NP)",
    "Stack-Segment Fault (#SS)",
    "General Protection Fault (#GP)",
    "Page Fault (#PF)",
    "Reserved",
    "x87 Floating-Point Exception (#MF)",
    "Alignment Check (#AC)",
    "Machine Check (#MC)",
    "SIMD Floating-Point Exception (#XM/#XF)",
    "Virtualization Exception (#VE)",
    "Control Protection Exception (#CP)",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception (#HV)",
    "VMM Communication Exception (#VC)",
    "Security Exception (#SX)",
    "Reserved"};

void exceptions_handler(struct registers *regs) {
  // clear screen
  cui_clear(0, 0, VGA_COLS - 1, VGA_ROWS - 1);
  cui_cursor_set_xy(0, 0);

  cui_printf("!!! KERNEL PANIC !!!\n\n");

  if (regs->int_no < 32) {
    cui_printf("EXCEPTION: %s (no. %d)\n", exception_name[regs->int_no],
               regs->int_no);
  } else {
    cui_printf("EXCEPTION: Unknown (no. %d)\n", regs->int_no);
  }

  cui_printf("EIP: 0x%x    ERROR CODE: 0x%x\n\n", regs->eip, regs->err_code);

  cui_printf("REGISTERS DUMP:\n");
  cui_printf("EAX: 0x%x  EBX: 0x%x  ECX: 0x%x  EDX: 0x%x\n", regs->eax,
             regs->ebx, regs->ecx, regs->edx);
  cui_printf("ESI: 0x%x  EDI: 0x%x  EBP: 0x%x  ESP: 0x%x\n", regs->esi,
             regs->edi, regs->ebp, regs->kern_esp);
  cui_printf("CS:  0x%x  DS:  0x%x  EFLAGS: 0x%x\n\n", regs->cs, regs->ds,
             regs->eflags);

  cui_klog("System halted, you have to reboot\n");

  vga_flush();

  while (1) {
    asm volatile("hlt");
  }
}