Goals
-----
  1. Risc-v compliant iss.
  2. Integer only, machine mode only, 32-bit.
  3. Iss to emulate core(s), main memory, memory mapped device (uart ns16550a?),
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
       + ns16550a uart (derive from existing pla011 implementation)
     * cmake
     * git
     * Test suite execution at check in?
     * Open source library dependencies:
       + elftoolchain
       + boost program options
     * Ubuntu 18, g++ 8.3
     * (gnu?) risc-v toolchain - asm, ld, c, ulib(?). elf reader, etc.

TO BUILD
--------
```
  sudo apt-get install bison build-essential flex libarchive-dev m4 bmake zlib1g-dev
  cd tools;./get_boost.sh
  cd tools;./get_elftoolchain.sh
  mkdir build
  cd build
  cmake ..
  cmake --build .
  make install
```



       
