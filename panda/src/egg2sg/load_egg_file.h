// Filename: load_egg_file.h
// Created by:  drose (09Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef LOAD_EGG_FILE_H
#define LOAD_EGG_FILE_H

#include <pandabase.h>

#include <namedNode.h>
#include <pt_NamedNode.h>
#include <coordinateSystem.h>

class EggData;

////////////////////////////////////////////////////////////////////
//     Function: load_egg_file
//  Description: A convenience function; the primary interface to this
//               package.  Loads up the indicated egg file, and
//               returns the root of a scene graph.  Returns NULL if
//               the file cannot be read for some reason.
//
//               Also see the EggLoader class, which can exercise a
//               bit more manual control over the loading process.
////////////////////////////////////////////////////////////////////
EXPCL_PANDAEGG PT_NamedNode 
load_egg_file(const string &filename, CoordinateSystem cs = CS_default);

////////////////////////////////////////////////////////////////////
//     Function: load_egg_data
//  Description: Another convenience function; works like
//               load_egg_file() but starts from an already-filled
//               EggData structure.  The structure is destroyed in the
//               loading.
////////////////////////////////////////////////////////////////////
EXPCL_PANDAEGG PT_NamedNode 
load_egg_data(EggData &data);

#endif
