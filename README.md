# Build-A-Shell - Part 2 - IPC

This project is a continuation of the "Build-A-Shell" quest, focusing on implementing IPC (Inter-Process Communication) features in the shell. It adds support for input/output redirection and pipe functionality.

## Features

1. Redirection
    - `<` Operator: Redirects the input for a command from a file. The argument following `<` is treated as the input file (STDIN) for the executed program.
    - `>` Operator: Redirects the output of a command to a file. The argument following `>` is treated as the output file (STDOUT) for the executed program.

2. Pipe
    - `|` Operator: Allows chaining of multiple programs in a single command using a pipe. The program before the `|` operator writes its output to the pipe, and the program after the `|` operator reads its input from the pipe.

## Prerequisites

- C++ compiler
- Standard Library headers
- Linux or macOS operating system (tested on macOS)




