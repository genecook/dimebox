goals:
  1. Risc-v compliant iss.
  2. Integer only, machine mode only, 64(32?)-bit.
  3. Iss to emulate core(s), main memory, memory mapped device (uart),
     (probably non 'compliant') system timer, synchronization primitives(?).
  4. Run risc-v integer test suites.
  5. Simulator binary to run on linux, mac os-x 'out of the box'.
  6. Iss features:
       + emulation of riscv core, memory, devices, memory 'monitors' (exclusive ops in arm-speak)
       + elf reader
       + disassembly
       + gdb support
  7. Development strategy:
     * Reuse of windvane components:
       + physical memory, logical memory api, exclusive monitor(?)
       + elf reader
       + gdb interface
       + (pla51) uart
     * cmake
     * git
     * Test suite execution at check in?
     * Open source libraries to include:
       + elftoolchain
       + boost program options
     * Ubuntu 18, g++ 8.3
     * (gnu?) risc-v toolchain - asm, ld, c, ulib(?). elf reader, etc.
     
       
