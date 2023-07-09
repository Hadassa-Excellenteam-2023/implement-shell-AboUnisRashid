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
#include <fcntl.h> // Added include statement for file access flags

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
 * @param inputFile The input file for redirection (STDIN).
 * @param outputFile The output file for redirection (STDOUT).
 */
void ExecuteChildProcess(const std::string& command, char** argv, const std::string& inputFile, const std::string& outputFile) {
    if (!inputFile.empty()) {
        int inputFileDescriptor = open(inputFile.c_str(), O_RDONLY);
        if (inputFileDescriptor == -1) {
            std::cerr << "Failed to open input file: " << inputFile << std::endl;
            exit(EXIT_FAILURE);
        }
        dup2(inputFileDescriptor, STDIN_FILENO);
        close(inputFileDescriptor);
    }

    if (!outputFile.empty()) {
        int outputFileDescriptor = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outputFileDescriptor == -1) {
            std::cerr << "Failed to open output file: " << outputFile << std::endl;
            exit(EXIT_FAILURE);
        }
        dup2(outputFileDescriptor, STDOUT_FILENO);
        close(outputFileDescriptor);
    }

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
 * @param inputFile The input file for redirection (STDIN).
 * @param outputFile The output file for redirection (STDOUT).
 */
void ExecuteCommand(const std::string& command, const std::vector<std::string>& args, bool runInBackground, const std::string& inputFile, const std::string& outputFile) {
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

        ExecuteChildProcess(command, argv.data(), inputFile, outputFile);
    } else {
        // Parent process
        ExecuteParentProcess(pid, runInBackground);
    }
}

/**
 * Executes a command line.
 * @param commandLine The command line to execute.
 */
void ExecuteCommandLine(const std::string& commandLine) {
    std::vector<std::string> commands = Split(commandLine, '|');
    int numCommands = static_cast<int>(commands.size());
    int pipefds[numCommands - 1][2];

    for (int i = 0; i < numCommands; ++i) {
        std::string command = commands[i];
        std::vector<std::string> args = Split(command, ' ');

        // Check for input/output redirection
        std::string inputFile;
        std::string outputFile;
        bool runInBackground = false;

        // Input redirection ("<" operator)
        auto inputFilePos = std::find(args.begin(), args.end(), "<");
        if (inputFilePos != args.end() && inputFilePos + 1 != args.end()) {
            inputFile = *(inputFilePos + 1);
            args.erase(inputFilePos, inputFilePos + 2);
        }

        // Output redirection (">" operator)
        auto outputFilePos = std::find(args.begin(), args.end(), ">");
        if (outputFilePos != args.end() && outputFilePos + 1 != args.end()) {
            outputFile = *(outputFilePos + 1);
            args.erase(outputFilePos, outputFilePos + 2);
        }

        // Background execution ("&" operator)
        auto backgroundPos = std::find(args.begin(), args.end(), "&");
        if (backgroundPos != args.end()) {
            runInBackground = true;
            args.erase(backgroundPos);
        }

        if (i < numCommands - 1) {
            if (pipe(pipefds[i]) == -1) {
                std::cerr << "Failed to create pipe." << std::endl;
                return;
            }
        }

        ExecuteCommand(args[0], args, runInBackground, inputFile, outputFile);

        if (i < numCommands - 1) {
            close(pipefds[i][1]);
            inputFile = "";
            inputFile = "pipe";
        }
    }

    for (int i = 0; i < numCommands - 1; ++i) {
        close(pipefds[i][0]);
    }
}

int main() {
    std::string commandLine;
    std::cout << "Shell> ";

    while (getline(std::cin, commandLine)) {
        if (commandLine.empty())
            continue;
        else if (commandLine == "exit")
            break;
        else
            ExecuteCommandLine(commandLine);

        std::cout << "Shell> ";
    }

    return 0;
}
