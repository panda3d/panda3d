// Filename: nurbsCurve.h
// Created by:  drose (01Mar01)
// 
////////////////////////////////////////////////////////////////////

#ifndef NURBSCURVE_H
#define NURBSCURVE_H

// This header file includes either ClassicNurbsCurve or NurbsPPCurve;
// whichever one is actually typedeffed as "NurbsCurve".

#include <pandabase.h>

#ifdef HAVE_NURBSPP
#include "nurbsPPCurve.h"
#else
#include "classicNurbsCurve.h"
#endif

#endif

