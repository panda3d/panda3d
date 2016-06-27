/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggUtilities.h
 * @author drose
 * @date 1999-01-28
 */

#ifndef EGGUTILITIES_H
#define EGGUTILITIES_H

// eggUtilities.h Handy functions that operate on egg structures, but don't
// necessarily belong in any one class.

#include "pandabase.h"

#include "eggTexture.h"
#include "pt_EggTexture.h"

#include "filename.h"
#include "pointerTo.h"

#include "pset.h"
#include "pmap.h"

class EggNode;
class EggVertex;

typedef pset< PT_EggTexture > EggTextures;
typedef pmap<Filename, EggTextures> EggTextureFilenames;


/**
 * Extracts from the egg subgraph beginning at the indicated node a set of all
 * the texture objects referenced, grouped together by filename.  Texture
 * objects that share a common filename (but possibly differ in other
 * properties) are returned together in the same element of the map.
 */
void
get_textures_by_filename(const EggNode *node, EggTextureFilenames &result);


/**
 * Splits a vertex into two or more vertices, each an exact copy of the
 * original and in the same vertex pool.  See the more detailed comments in
 * eggUtilities.I.
 */
template<class FunctionObject>
void
split_vertex(EggVertex *vert, const FunctionObject &sequence);


#include "eggUtilities.I"

#endif
