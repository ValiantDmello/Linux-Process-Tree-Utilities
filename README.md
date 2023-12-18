# Process Tree Utilities

This repository contains two C programs, `prctree.c` and `ztree.c`, designed to provide utilities for exploring and managing process trees on Unix-like systems.

## prctree.c

`prctree.c` allows you to explore the hierarchy of processes rooted at a specified process ID (PID). It provides information such as parent processes, child processes, sibling processes, and more.

### Usage

```bash
./prctree <root_pid> <pid> [-c | -s | -gp | -gc | -z | -zl]
```
