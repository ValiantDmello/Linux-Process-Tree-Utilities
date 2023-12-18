# Process Tree Utilities

This repository contains two C programs, `prctree.c` and `ztree.c`, designed to provide utilities for exploring and managing process trees on Linux system.

## prctree.c

`prctree.c` allows you to explore the hierarchy of processes rooted at a specified process ID (PID). It provides information such as parent processes, child processes, sibling processes, and more.

### Usage

```bash
./prctree <root_pid> <pid> [-c | -s | -gp | -gc | -z | -zl]
```
* <root_pid>: The root process ID.
* <pid>: The target process ID.
* Options: <br>
-c: Print child processes.<br>
-s: Print sibling processes.<br>
-gp: Print the grandparent process.<br>
-gc: Print grandchild processes.<br>
-z: Check if the specified process is a zombie process.<br>
-zl: Print the process IDs of zombie child processes.<br>

### Example:
```bash
./prctree 1 1234 -c
```

This command prints the child processes of the process with PID 1234 in the process tree rooted at PID 1.

## ztree.c

`ztree.c` provides utilities for managing zombie processes within the process tree rooted at a specified PID. It includes functions to terminate zombie parents, terminate parents based on elapsed time, and terminate parents with a specific number of defunct child processes.

### Usage

```bash
./ztree <root_pid> [-t <PROC_ELTIME> | -b <NO_OF_DFCS>]
```

* <root_pid>: The root process ID.
* Options: <br>
-t <PROC_ELTIME>: Terminate parents with an elapsed time greater than <PROC_ELTIME> minutes.<br>
-b <NO_OF_DFCS>: Terminate parents with a number of defunct child processes greater than or equal to <NO_OF_DFCS>.<br>

### Examples

```bash
./ztree 1 -t 5.0
```
This command terminates parent processes in the process tree rooted at PID 1 with an elapsed time greater than 5.0 minutes.
