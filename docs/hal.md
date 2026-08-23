# Hardware Abstraction Layer (HAL)

## Overview
The Hardware Abstraction Layer (HAL) is the component that hides architecture-specific details behind a uniform and high-level interface.

Kernel subsystems (such as the VMM or drivers) interact with the hardware exclusively through the HAL interface.

## Implementation Design
The HAL is implemented at compile-time. Each architecture provides its own implementation of the HAL functions, and the build system links only the implementation corresponding to the target architecture.

### Directory Structure
- `/include/hal/`: contains common HAL interfaces and types(architecture-independent headers)
- `src/arch/x86/hal`: contains the architecture-specific implementation of the HAL fucntions, following common prototypes 
