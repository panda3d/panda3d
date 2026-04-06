#pragma once

#include "em_asm.h"
#include "em_js.h"

typedef short emscripten_align1_short;

typedef long long emscripten_align4_int64;
typedef long long emscripten_align2_int64;
typedef long long emscripten_align1_int64;

typedef int emscripten_align2_int;
typedef int emscripten_align1_int;

typedef float emscripten_align2_float;
typedef float emscripten_align1_float;

typedef double emscripten_align4_double;
typedef double emscripten_align2_double;
typedef double emscripten_align1_double;

typedef void (*em_callback_func)(void);
typedef void (*em_arg_callback_func)(void*);
typedef void (*em_str_callback_func)(const char *);
