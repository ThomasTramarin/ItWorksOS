# Hardware Interrupts 

## Overview
An hardware interrupt is an asynchronous event raised by a hardware device (such as a keyboard controller) that requires the CPU to suspend its current execution and transfer execution to a dedicated handler.

A computer can have one or more interrupt controllers. Interrupt controllers are abstracted through `struct irq_chip`.

Each interrupt controller provides a set of hardware interrupt lines. Devices raise interrupts through these lines, while the interrupt controller is responsible for receiving, managing, and forwarding them to the CPU.

On x86, hardware interrupts can be divided into two categories:
1. **Maskable Interrupts**: these are ordinary hardware interrupts. When a device raises an interrupt, the interrupt controller detects it and signals the CPU through the **INTR** pin. They are called *maskable* because the CPU can disable them via software, by clearing the **Interrupt Flag (IF)** in `EFLAGS`.
2. **Non-Maskable Interrupts (NMI)**: these interrupts cannot be disabled by clearing the Interrupt Flag. They use a separate mechanism (a separate pin) and are commonly used for critical hardware conditions. NMI handling is separate from the generic IRQ subsystem.

## IRQ Chip
The `struct irq_chip` defines the interface used by the generic IRQ subsystem to control an interrupt controller.

This abstraction allows the same IRQ subsystem to work with different interrupt controllers.

For example, enabling or disabling an IRQ is performed through this interface, while the actual register operations are implemented by the architecture-specific controller driver.

## IRQ Map
An interrupt controller identifies an interrupt by using a **hardware IRQ number (`hwirq`)**. The kernel, however, uses a separate namespace of logical IRQ numbers (virtual).

An IRQ map (`struct irq_map`) associates these two identifiers and allows the translation `hwirq -> irq`, so when an interrupt is received from an interrupt controller, the kernel can obtain its virtual IRQ number through this structure.

## IRQ Descriptor
Each kernel IRQ is associated with an IRQ descriptor (`struct irq_desc`).
The descriptor contains information required by the IRQ subsystem to manage and dispatch that IRQ, including its interrupt controller and the registered interrupt handler.

The kernel maintains an array of descriptors, the logical IRQ number is used as index in this array to identify a specific descriptor.

The descriptor itself contains also the hardware IRQ number. So, given a logical IRQ, the descriptor permits the kernel to know the hardware IRQ and the interrupt controller

## Interrupt Flow
When a hardware device raises an interrupt, the event goes through several layers before reaching the registered interrupt handler.

At a high level, the flow is:
1. The device raises an interrupt on a hardware interrupt line.
2. The interrupt controller receives the interrupt and identifies the corresponding hardware IRQ.
3. The architecture-specific interrupt handling code receives the interrupt from the CPU.
4. The IRQ subsystem translates the hardware IRQ into a logical IRQ using the IRQ map.
5. The corresponding IRQ descriptor is retrieved from the IRQ descriptor table using the IRQ number.
6. The IRQ subsystem dispatches the interrupt to the registered handler.