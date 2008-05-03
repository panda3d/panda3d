#include <stdlib.h>
#include <stdio.h>
#include "zbuffer.h"

#define ACMP(a) 1
#define ZCMP(z,zpix) 1
#define FNAME(name) name ## _anone_znone
#include "ztriangle_two.h"

#define ACMP(a) 1
#define ZCMP(z,zpix) ((zpix) < (z))
#define FNAME(name) name ## _anone_zless
#include "ztriangle_two.h"

#define ACMP(a) ((a) > 0x8000)
#define ZCMP(z,zpix) 1
#define FNAME(name) name ## _abin_znone
#include "ztriangle_two.h"

#define ACMP(a) ((a) > 0x8000)
#define ZCMP(z,zpix) ((zpix) < (z))
#define FNAME(name) name ## _abin_zless
#include "ztriangle_two.h"
