# MP1 - MiShell

## Overview
MiShell is a simple shell implementation in C that provides basic shell functionality with additional features. This project demonstrates fundamental concepts of operating systems, and process manipulation.

## Features

### Core Features
- Command execution
- Input/output/error redirection (`>`, `<`, `2>`)
- Command piping with `|` operator
- Background processing with `&`
- Built-in commands: `cd`, `exit`

### Additional Features
1. **Environment Variable Management**
   - Environment variable expansion with `$VAR` syntax
   - Commands: `env`, `setenv`, `unsetenv`
   - Support for environment variables in paths and redirections
   - Enhanced `cd` command with `$HOME` default

## Installation & Usage

### Compiling
```bash
# Clone the repository
git clone the repo (nylls.gersan.boutoto) on Gitlab
cd MP1

# Compile the project
make
```

### Running
```bash
./mishell
```

### Examples
Basic command execution:
```bash
/home/user% ls -la
```

Using pipes:
```bash
/home/user% ls -la | grep .txt | sort
```

Input/output redirection:
```bash
/home/user% cat < input.txt > output.txt
```

Background processing:
```bash
/home/user% sleep 10 &
```

Environment variable operations:
```bash
/home/user% setenv MY_VAR hello_world
/home/user% echo $MY_VAR
/home/user% cd $HOME/Documents
/home/user% env
/home/user% unsetenv MY_VAR
```

## Project Structure
- `mishell.c`: Main source code file
- `Makefile`: Build configuration
- `.gitignore`: Git ignore configuration
- `README.md`: This documentation file

## Implementation Details

### Command Parsing
Commands are parsed to identify redirections, pipes, and arguments. The shell supports complex command structures with multiple pipes and redirections.

### Process Management
The shell creates child processes for command execution and manages pipelines by connecting the standard input/output of processes.

### Environment Variables
Environment variables are expanded during command parsing. The implementation supports variable use in commands, paths, and redirections.

## Author
Nylls Boutoto <boutotonyllsgersan@gmail.com>