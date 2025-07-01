# MO801 Project 3: Hardware-Accelerated Dot Product for Logistic Regression

This project demonstrates the design, integration, and benchmarking of a custom hardware accelerator for the dot product operation, targeting machine learning workloads (e.g., logistic regression) on a LiteX-based System-on-Chip (SoC).

## Overview

- **Hardware Accelerator:** Implements a dot product accelerator in Migen/LiteX (`hardware_accelerator/dot_product_accel.py`).
- **Software Drivers:** Modular C drivers for the accelerator and peripherals (`software/drivers/`).
- **Benchmarking:** C code to benchmark dot product performance with and without hardware acceleration (`software/dot_product_benchmark.c`).
- **Build System:** Makefile for building the software to run on the SoC.

## Directory Structure

- `hardware_accelerator/` — Hardware accelerator source code (Python/Migen).
- `software/` — C source code for benchmarks, drivers, and build scripts.
- `build/` — Build artifacts and generated files.

## Getting Started

### Prerequisites

- LiteX and Migen installed
- FPGA toolchain for your board (e.g., Gowin for Sipeed Tang Nano 9K)
- Python 3.x
- C cross-compiler for your SoC

### Building the Hardware

1. Set up the LiteX environment and required dependencies.
2. Build the SoC with the accelerator:
   ```sh
   python sipeed_tang_nano_9k.py --build --load
   ```

### Building the Software

1. Enter the software directory:
   ```sh
   cd software
   ```
2. Build the firmware:
   ```sh
   make BUILD_DIR=../build/sipeed_tang_nano_9k
   ```
   This produces the `demo.bin` binary for the SoC.

### Loading the Firmware

While still in the `software` directory, load the binary to the board:
```sh
litex_term /dev/ttyUSB1 --kernel=demo.bin
```

## Running Benchmarks

- The benchmark will measure and print the execution time of the dot product operation with and without hardware acceleration.
- Results are displayed via the serial terminal.

## Customization

- Modify `hardware_accelerator/dot_product_accel.py` to change the accelerator design.
- Update the driver and benchmark code in `software/` as needed.

## References

- [LiteX](https://github.com/enjoy-digital/litex)
- [Migen](https://github.com/m-labs/migen)

---
