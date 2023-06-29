# Custom Shell Program in C++

This is a custom shell program implemented in C++ that emulates the functionality of a standard Linux shell. It allows users to enter commands and arguments, which are then executed as subprocesses.

## Key Features

- **Command Execution**: The program executes commands and their arguments as subprocesses using the `fork` and `exec` family of commands.
- **Software Discovery**: It searches for the specified software in the paths listed in the PATH environment variable.
- **Path Resolution**: Handles scenarios where the software is not found or if the user provides the full path to the software.
- **Argument Passing**: The program passes user-provided arguments to the executed software.
- **Background Execution**: Supports running programs in the background by appending the '&' symbol at the end of the command.
- **Background Job Management**: Provides a built-in command `myjobs` to view details of background processes.
- **Cross-Platform Compatibility**: Adjusts certain commands to ensure compatibility with Windows systems.

## Usage

1. Launch the custom shell program.
2. Enter commands and their arguments as needed.
3. Press Enter to execute the command.
4. To run a command in the background, add the '&' symbol at the end of the command.
5. Utilize the `myjobs` command to display information about background processes.

## Examples

**Example 1: Executing a Basic Command**
$ custom-shell
custom-shell> ls
<output of the ls command>

**Example 2: Passing Arguments**
custom-shell> echo Hello, world!
Hello, world!

**Example 3: Reading a File**
custom-shell> cat myfile.txt
<contents of the myfile.txt>


**Example 4: Background Execution**
custom-shell> sleep 10 &
[Background process started]

**Example 5: Viewing Background Jobs**
custom-shell> myjobs
PID: 1234 (sleep 10)

