// Filename: mesherConfig.h
// Created by:  drose (27Oct00)
// 
////////////////////////////////////////////////////////////////////
#ifndef MESHERCONFIG_H
#define MESHERCONFIG_H

#include <pandabase.h>

// This is just a file to declare a definition or two global to the
// mesher compilation.

// Define this to support making triangle fans in addition to triangle
// strips.  Fans may improve the grouping in certain models, although
// in most real cases the don't seem to help very much (and can
// actually hurt, by bitching the heuristic).
#define SUPPORT_FANS

#endif
