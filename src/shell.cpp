#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sstream>
#include <fstream>
#include <map>

// Environment variables
std::map<std::string, std::string> environmentVariables;

/**
 * Splits a string into tokens based on a delimiter.
 * @param str The string to split.
 * @param delimiter The delimiter character.
 * @return A vector of tokens.
 */
std::vector<std::string> Split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(str);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(std::move(token));
    }
    return tokens;
}

/**
 * Executes the child process.
 * @param command The command to execute.
 * @param argv The arguments for the command.
 */
void ExecuteChildProcess(const std::string& command, char** argv) {
    execvp(command.c_str(), argv);
    std::cerr << "Failed to execute command: " << command << std::endl;
    exit(EXIT_FAILURE);
}

/**
 * Executes the parent process.
 * @param pid The process ID.
 * @param runInBackground Flag indicating whether the process should run in the background.
 */
void ExecuteParentProcess(pid_t pid, bool runInBackground) {
    if (!runInBackground) {
        int status;
        waitpid(pid, &status, 0);
    }
}

/**
 * Executes a command with arguments.
 * @param command The command to execute.
 * @param args The arguments for the command.
 * @param runInBackground Flag indicating whether the command should run in the background.
 */
void ExecuteCommand(const std::string& command, const std::vector<std::string>& args, bool runInBackground) {
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to create child process." << std::endl;
        return;
    } else if (pid == 0) {
        // Child process
        std::vector<char*> argv(args.size() + 2);
        argv[0] = const_cast<char*>(command.c_str());
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i + 1] = const_cast<char*>(args[i].c_str());
        }
        argv[args.size() + 1] = nullptr;
        ExecuteChildProcess(command, argv.data());
    } else {
        // Parent process
        ExecuteParentProcess(pid, runInBackground);
    }
}

/**
 * Checks if the command is a background command.
 * @param args The command arguments.
 * @return True if the command is a background command, false otherwise.
 */
bool IsBackgroundCommand(const std::vector<std::string>& args) {
    return !args.empty() && args.back() == "&";
}

/**
 * Removes the background symbol from the command arguments.
 * @param args The command arguments.
 */
void RemoveBackgroundSymbol(std::vector<std::string>& args) {
    if (!args.empty() && args.back() == "&") {
        args.pop_back();
    }
}

/**
 * Prints the list of background jobs.
 * @param backgroundProcesses The list of background process IDs.
 */
void PrintBackgroundJobs(const std::vector<pid_t>& backgroundProcesses) {
    for (pid_t pid : backgroundProcesses) {
        std::string cmdLine;
        std::ifstream cmdLineFile("/proc/" + std::to_string(pid) + "/comm");
        std::getline(cmdLineFile, cmdLine);
        cmdLineFile.close();
        std::cout << "PID: " << pid << "  Command: " << cmdLine << std::endl;
    }
}

/**
 * Sets an environment variable.
 * @param args The command arguments.
 */
void SetEnvironmentVariable(const std::vector<std::string>& args) {
    if (args.size() == 2) {
        environmentVariables[args[0]] = args[1];
    } else {
        std::cerr << "Invalid arguments for 'set' command." << std::endl;
    }
}

/**
 * Unsets an environment variable.
 * @param args The command arguments.
 */
void UnsetEnvironmentVariable(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        environmentVariables.erase(args[0]);
    } else {
        std::cerr << "Invalid arguments for 'unset' command." << std::endl;
    }
}

/**
 * Expands environment variables in the command arguments.
 * @param args The command arguments.
 */
void ExpandEnvironmentVariables(std::vector<std::string>& args) {
    for (std::string& arg : args) {
        if (arg.empty())
            continue;

        if (arg[0] != '$' && arg[0] != '{')
            continue;

        std::string varName = arg.substr(1);
        if (varName.empty())
            continue;

        if (varName[0] == '{' && varName.back() == '}') {
            varName = varName.substr(1, varName.size() - 2);
        }

        auto it = environmentVariables.find(varName);
        if (it != environmentVariables.end()) {
            arg = it->second;
        } else {
            std::cerr << "Environment variable not found: " << varName << std::endl;
        }
    }
}

/**
 * Handles the execution of built-in commands.
 * @param command The command to handle.
 * @param args The command arguments.
 * @param backgroundProcesses The list of background process IDs.
 * @return True if the command is a built-in command and handled successfully, false otherwise.
 */
bool HandleBuiltInCommands(const std::string& command, const std::vector<std::string>& args, std::vector<pid_t>& backgroundProcesses) {
    if (command == "myjobs") {
        PrintBackgroundJobs(backgroundProcesses);
        return true;
    } else if (command == "set") {
        SetEnvironmentVariable(args);
        return true;
    } else if (command == "unset") {
        UnsetEnvironmentVariable(args);
        return true;
    }
    return false;
}

/**
 * Processes a command line.
 * @param commandLine The command line to process.
 * @param backgroundProcesses The list of background process IDs.
 */
void ProcessCommandLine(const std::string& commandLine, std::vector<pid_t>& backgroundProcesses) {
    std::vector<std::string> tokens = Split(commandLine, ' ');
    if (tokens.empty()) {
        return;
    }

    std::string command = tokens[0];
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());

    bool runInBackground = IsBackgroundCommand(args);
    if (runInBackground) {
        RemoveBackgroundSymbol(args);
    }

    ExpandEnvironmentVariables(args);

    if (HandleBuiltInCommands(command, args, backgroundProcesses)) {
        return;
    }

    ExecuteCommand(command, args, runInBackground);

    if (runInBackground) {
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Failed to create child process." << std::endl;
        } else if (pid > 0) {
            backgroundProcesses.push_back(pid);
        } else {
            return;
        }
    }
}

/**
 * Runs the shell.
 */
void RunShell() {
    std::string commandLine;
    std::vector<pid_t> backgroundProcesses;

    while (true) {
        std::cout << "shell> ";
        std::getline(std::cin, commandLine);
        ProcessCommandLine(commandLine, backgroundProcesses);
    }
}

int main() {
    RunShell();
    return EXIT_SUCCESS;
}
