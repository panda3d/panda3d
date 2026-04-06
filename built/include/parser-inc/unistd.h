#pragma once

#include <stdio.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

extern char *optarg;
extern int optind, opterr, optopt;
