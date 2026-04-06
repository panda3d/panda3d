#pragma once

#include <sys/types.h>
#include <time.h>

struct stat;

int stat(const char *restrict, struct stat *restrict);
