/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggUtilities.I
 * @author drose
 * @date 1999-02-10
 */

#include "eggGroup.h"
#include "eggPrimitive.h"
#include "eggVertexPool.h"

#include <algorithm>

/**
 * Splits a vertex into two or more vertices, each an exact copy of the
 * original and in the same vertex pool.
 *
 * The splitting is based on some arbitrary property of the primitives that
 * own the vertex.  In the extreme, each primitive may get a different copy of
 * the vertex, although it is also possible for some primitives to still share
 * vertices.
 *
 * This decision is made based on the function object 'sequence'.  This object
 * must define the following function:
 *
 * int operator () (const EggPrimitive *prim) const;
 *
 * This function returns a sequence number, which determines which primitives
 * will share which vertices.  The sequence number 0 refers to the original
 * vertex pointer; other sequence numbers indicate new vertices.  Other than
 * that, the sequence number is totally arbitrary.  Primitives for which the
 * sequence number is the same will end up sharing the same copy of the
 * vertex.
 */
template<class FunctionObject>
void
split_vertex(EggVertex *vert, const FunctionObject &sequence) {
  // Did we start in a happy world?
  vert->test_pref_integrity();
  vert->test_gref_integrity();

  EggVertexPool *pool = vert->get_pool();

  // Define a map of ints to vert pointers, to indicate which sequence numbers
  // we have already created vertices for.
  typedef pmap<int, EggVertex *> Sequences;
  Sequences _sequences;

  // Get a copy of the list of primitives that reference this vertex.  We must
  // have a copy because we will be modifying the list as we traverse it.
  typedef pvector<EggPrimitive *> Prims;
  Prims prims;
  prims.reserve(vert->pref_size());
  std::copy(vert->pref_begin(), vert->pref_end(), std::back_inserter(prims));

  // Now walk through the list of primitives that reference this vertex.
  Prims::const_iterator pri;
  for (pri = prims.begin(); pri != prims.end(); ++pri) {
    EggPrimitive *prim = *pri;
    prim->test_ref_count_integrity();

    int seq = sequence(prim);

    if (seq != 0) {
      // Here's a new sequence number!  Have we already defined it?
      EggVertex *new_vert = nullptr;

      Sequences::const_iterator si = _sequences.find(seq);

      if (si != _sequences.end()) {
        // Yes, we've seen this sequence number before.  Use the same vertex.
        new_vert = (*si).second;

      } else {
        // No, this is the first time we've encountered this sequence.  Split
        // the vertex.
        new_vert = new EggVertex(*vert);
        pool->add_vertex(new_vert);
        _sequences[seq] = new_vert;

        // The new vertex gets all the same group memberships as the old one.
        EggVertex::GroupRef::const_iterator gri;
        for (gri = vert->gref_begin(); gri != vert->gref_end(); ++gri) {
          EggGroup *group = *gri;
          group->ref_vertex(new_vert, group->get_vertex_membership(vert));
        }
      }

      // Now replace the vertex in the primitive.
      EggPrimitive::iterator pi;
      for (pi = prim->begin(); pi != prim->end(); ++pi) {
        if (*pi == vert) {
          prim->replace(pi, new_vert);
        }
      }
    }
  }

#ifndef NDEBUG
  // Now verify everything is still happy.
  vert->test_pref_integrity();
  vert->test_gref_integrity();

  Sequences::const_iterator si;
  for (si = _sequences.begin();
       si != _sequences.end();
       ++si) {
    EggVertex *new_vert = (*si).second;
    new_vert->test_gref_integrity();
    new_vert->test_pref_integrity();
  }
#endif  // NDEBUG

}
