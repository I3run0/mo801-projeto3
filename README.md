# MO801 Project 3: Hardware-Accelerated Dot Product for Logistic Regression

This project demonstrates the design, integration, and benchmarking of a custom hardware accelerator for the dot product operation, targeting machine learning workloads (e.g., logistic regression) on a LiteX-based System-on-Chip (SoC).

## Features
- **Hardware Accelerator**: Implements a dot product accelerator in Migen/LiteX (`hardware_accelerator/dot_product_accel.py`).
- **Software Driver**: C driver for interfacing with the accelerator (`software/drivers/dot_product_accel_driver.c`).
- **Benchmarking**: C code to benchmark dot product performance with and without hardware acceleration (`software/dot_product_benchmark.c`).
- **Build System**: Makefile for building the software to run on the SoC.

## Directory Structure
- `hardware_accelerator/` — Hardware accelerator source code (Python/Migen).
- `software/` — C source code for benchmarks, drivers, and build scripts.
- `build/` — Build artifacts and generated files.
- `notas-projeto3.md` — Project notes and TODOs.

## Building the Project
1. **Set up the LiteX environment** and required dependencies (Migen, LiteX, toolchains).
2. **Build the hardware** (SoC with accelerator) using the provided Python scripts.
3. **Build the software**:
   ```sh
   cd software
   make
   ```
   This produces the `demo.bin` binary for the SoC.

## Running Benchmarks
- Load the built bitstream and software onto your FPGA or simulation environment.
- The benchmark will measure and print the execution time of the dot product operation with and without hardware acceleration.

## Measuring Execution Time
- The timer peripheral is used to measure elapsed cycles.
- See `notas-projeto3.md` for details on timing and methodology.

## Customization
- Modify `hardware_accelerator/dot_product_accel.py` to change the accelerator design.
- Update the driver and benchmark code in `software/` as needed.

## References
- [LiteX](https://github.com/enjoy-digital/litex)
- [Migen](https://github.com/m-labs/migen)
