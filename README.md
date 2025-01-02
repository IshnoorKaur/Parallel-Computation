# Parallel Computation Using Processes

## Overview

This project demonstrates essential concepts of **multi-processing**, **signal handling**, and **file I/O** using the C programming language. It showcases the use of child processes to perform parallel computations and the handling of inter-process communication effectively.

---

## Getting Started

### Prerequisites

- A C compiler such as `gcc` installed on your system.
- Basic understanding of the command-line interface and C programming.

### Compilation

To build the project, use the provided `Makefile` by running:
```bash
make
This will compile the source code and generate an executable named a1.
```
#### Features
- Parallel Processing: Uses child processes to execute tasks concurrently, improving efficiency and performance.
- Signal Handling: Implements robust mechanisms for managing and communicating between processes.
- File I/O: Efficiently reads and processes data from multiple input files simultaneously.

### How It Works
- Child Process Creation: The program forks multiple child processes to handle tasks independently.
- Signal Communication: Signals are used for coordination and communication between the parent and child processes.
- File Handling: Each process works on specified files, demonstrating concurrent file operations.
