// Filename: compose_matrix.cxx
// Created by:  drose (27Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "deg_2_rad.h"
#include "config_linmath.h"
#include "compose_matrix.h"

Configure(config_linmath_2);
ConfigureFn(config_linmath_2) {
}


static const bool temp_hpr_fix = config_linmath_2.GetBool("temp-hpr-fix", false);

#include "fltnames.h"
#include "compose_matrix_src.cxx"

#include "dblnames.h"
#include "compose_matrix_src.cxx"



