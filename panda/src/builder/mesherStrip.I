// Filename: mesherStrip.I
// Created by:  drose (16Sep97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


template <class PrimType>
INLINE MesherStrip<PrimType>::
MesherStrip(const MesherStrip &copy) :
  _prims(copy._prims),
  _edges(copy._edges),
  _verts(copy._verts),
  _type(copy._type),
  _index(copy._index),
  _status(copy._status),
  _planar(copy._planar),
  _plane_normal(copy._plane_normal),
  _plane_offset(copy._plane_offset),
  _row_id(copy._row_id)
{
}


////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::is_coplanar_with
//       Access: Public
//  Description: Returns true if the strip and the other strip are
//               coplanar.
////////////////////////////////////////////////////////////////////
template <class PrimType>
INLINE bool MesherStrip<PrimType>::
is_coplanar_with(const MesherStrip &other, float threshold) const {
  return (coplanarity(other) <= threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::coplanarity
//       Access: Public
//  Description: Returns the degree to which the two strips are
//               coplanar.  0.0 is exactly coplanar; numbers somewhat
//               larger than zero indicate less coplanar.  1.0 is
//               at right angles; 2.0 is exactly backfacing.  If
//               either strip is not itself planar, 3.0 is returned.
////////////////////////////////////////////////////////////////////
template <class PrimType>
INLINE float MesherStrip<PrimType>::
coplanarity(const MesherStrip &other) const {
  if (_planar && other._planar) {
    return 1.0 - dot(_plane_normal, other._plane_normal);
  } else {
    return 3.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::type_category
//       Access: Public
//  Description: Returns an integer which gives a heuristic about the
//               similarity of different strip types.  In general,
//               closer numbers are more similar.
////////////////////////////////////////////////////////////////////
template <class PrimType>
INLINE int MesherStrip<PrimType>::
type_category() const {
  switch (_type) {
  case BPT_tri:
    return 1;

  case BPT_tristrip:
    return 2;

  case BPT_quad:
  case BPT_quadstrip:
    return 5;

  default:
    return 10;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::rotate_forward
//       Access: Public
//  Description: Rotates a triangle or quad by bringing its second
//               vertex to the front.
////////////////////////////////////////////////////////////////////
template <class PrimType>
INLINE void MesherStrip<PrimType>::
rotate_forward() {
  _verts.push_back(_verts.front());
  _verts.pop_front();
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::rotate_back
//       Access: Public
//  Description: Rotates a triangle or quad by bringing its second
//               vertex to the front.
////////////////////////////////////////////////////////////////////
template <class PrimType>
INLINE void MesherStrip<PrimType>::
rotate_back() {
  _verts.push_front(_verts.back());
  _verts.pop_back();
}


////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::get_head_edge
//       Access: Public
//  Description: Returns an Edge which represents the leading edge in
//               the quadstrip or tristrip.  This Edge will not have
//               pointer equality with any shared Edge.
////////////////////////////////////////////////////////////////////
template <class PrimType>
INLINE TYPENAME MesherStrip<PrimType>::Edge MesherStrip<PrimType>::
get_head_edge() const {
  TYPENAME Verts::const_iterator vi = _verts.begin();
  return Edge(_verts.front(), *++vi);
}


////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::get_tail_edge
//       Access: Public
//  Description: Returns an Edge which represents the trailing edge in
//               the quadstrip or tristrip.  This Edge will not have
//               pointer equality with any shared Edge.
////////////////////////////////////////////////////////////////////
template <class PrimType>
INLINE TYPENAME MesherStrip<PrimType>::Edge MesherStrip<PrimType>::
get_tail_edge() const {
  TYPENAME Verts::const_reverse_iterator vi = _verts.rbegin();
  return Edge(*++vi, _verts.back());
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::operator ==
//       Access: Public
//  Description: Defines equality for strips.  This actually tests
//               only pointer equality; it's used only when removing a
//               strip from the list.
////////////////////////////////////////////////////////////////////
template <class PrimType>
INLINE bool MesherStrip<PrimType>::
operator == (const MesherStrip &other) const {
  return this == &other;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class PrimType>
INLINE bool MesherStrip<PrimType>::
operator != (const MesherStrip &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class PrimType>
MesherStrip<PrimType>::
MesherStrip(const PrimType &prim, int index, const BuilderBucket &bucket) {
  _index = index;
  _row_id = 0;
  _status = MS_alive;
  _origin = MO_unknown;
  prim.has_overall_normal();  // Force the prim to update its overall flags.
  _type = prim.get_type();

  // We save only the prim's overall properties in the _prims
  // array.  The vertices get re-added later by Mesher::add_prim().
  _prims.push_back(prim);

  if (_type == BPT_poly) {
    switch (prim.get_num_verts()) {
    case 3:
      _type = BPT_tri;
      break;

    case 4:
      _type = BPT_quad;
      break;
    }
  }

  if (_type == BPT_quad) {
    // A quad has two internal triangles; we therefore push the prim
    // properties twice.
    _prims.push_back(prim);
  }

  _planar = false;

  if (prim.get_num_verts() >= 3) {
    // However, we will examine the vertices to determine the plane equation.
    Vertexf p1 = prim.get_vertex(0).get_coord_value(bucket);
    Vertexf p2 = prim.get_vertex(1).get_coord_value(bucket);
    Vertexf p3 = prim.get_vertex(2).get_coord_value(bucket);
    _plane_normal = cross(p1-p2, p2-p3);
    float l = length(_plane_normal);

    if (l != 0.0f) {
      _plane_normal /= l;
      _planar = true;
      _plane_offset = -dot(_plane_normal, p1);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::make_prim
//       Access: Public
//  Description: Creates a PrimType element corresponding to the strip
//               represented by this node.
////////////////////////////////////////////////////////////////////
template <class PrimType>
PrimType MesherStrip<PrimType>::
make_prim(const BuilderBucket &bucket) {
  Prim p;
  BuilderPrimType dest_type;

  switch (_type) {
  case BPT_quad:
    dest_type = bucket._show_quads ? BPT_poly : BPT_tristrip;
    break;

  case BPT_tristrip:
  case BPT_quadstrip:
    dest_type = BPT_tristrip;
    break;

  case BPT_trifan:
    dest_type = BPT_trifan;
    break;

  default:
    dest_type = _type;
  }

  if (dest_type != BPT_tristrip && dest_type != BPT_trifan) {
    // The easy case: a simple primitive.
    p.set_attrib(_prims.front());
    TYPENAME Verts::iterator vi;
    for (vi = _verts.begin(); vi != _verts.end(); ++vi) {
      const Vertex *v = *vi;
      nassertr(v != (Vertex *)NULL, p);
      p.add_vertex(*v);
    }
    p.set_type(dest_type);

  } else {
    // The harder case: a tristrip of some kind.
    convert_to_type(dest_type);
    p.set_attrib(_prims.front());

    BuilderPrimType type = dest_type;

    // Now store all the vertices, as well as each individual
    // triangle's attributes.
    TYPENAME Verts::iterator vi;
    TYPENAME Prims::iterator pi;
    pi = _prims.begin();
    int count = 0;
    for (vi = _verts.begin();
         vi != _verts.end() && pi != _prims.end();
         ++vi) {
      const Vertex *v = *vi;
      nassertr(v != (Vertex *)NULL, p);

      if (++count >= 3) {
        // Beginning with the third vertex, we increment pi.  Thus, the
        // first two vertices stand alone, then each vertex beginning
        // with the third completes a triangle.
        p.add_component(*pi);
        ++pi;
      }
      p.add_vertex(*v);
    }

    p.set_type(type);

    // If either of these fail, there weren't num_prims + 2 vertices in
    // the tristrip!
    nassertr(vi==_verts.end(), p);
    nassertr(pi==_prims.end(), p);
  }

  return p;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::measure_sheet
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class PrimType>
void MesherStrip<PrimType>::
measure_sheet(const Edge *edge, int new_row, int &num_prims, int &num_rows,
             int first_row_id, int this_row_id, int this_row_distance) {
  if (new_row) {
    // If we would create a new row by stepping here, we won't stay if
    // there was any other row already defined here.
    if (_row_id >= first_row_id) {
      return;
    }
  } else {
    // On the other hand, if this is a continuation of the current
    // row, we'll stay if the other row had to travel farther to get
    // here.
    if (_row_id >= first_row_id && _row_distance <= this_row_distance) {
      return;
    }
  }

  num_prims += _prims.size();
  if (new_row) {
    ++num_rows;
    this_row_id = first_row_id + num_rows - 1;
  }

  _row_id = this_row_id;

  TYPENAME Edges::iterator ei;
  TYPENAME Edge::Strips::iterator si;

  if (_type == BPT_quad) {
    // If this is a quad, it has four neighbors: two in the direction
    // we are testing, and two in an orthagonal direction.

    const Vertex *a = edge->_a;
    const Vertex *b = edge->_b;

    // We use these vertices to differentiate the edges that run in
    // our primary direction from those in the secondary direction.
    // For each edge, we count the number of vertices that the edge
    // shares with our starting edge.  There are then three cases:

    // (a) The edge shares two vertices.  It is the direction we came
    // from; forget it.

    // (b) The edge shares one vertex.  It is at right angles to our
    // starting edge.  This is the primary direction if new_row is
    // true, and the secondary direction if new_row is false.

    // (c) The edge shares no vertices.  It is directly opposite our
    // starting edge.  This is the primary direction if new_row is
    // false, and the secondary direction if new_row is true.


    // Here's a silly little for loop that executes the following code
    // twice: once with secondary == 0, and once with secondary == 1.
    // This is because we want to find all the primary edges first,
    // and then all the secondary edges.

    for (int secondary = 0; secondary <= 1; secondary++) {
      // How many common vertices are we looking for this pass (see
      // above)?

      int want_count;
      if (secondary) {
        want_count = new_row ? 0 : 1;
      } else {
        want_count = new_row ? 1 : 0;
      }

      for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
        int common_verts =
          ((*ei)->_a == a || (*ei)->_a == b) +
          ((*ei)->_b == a || (*ei)->_b == b);

        if (common_verts == want_count) {
          // Here's the edge.  Look at all its connections.  Hopefully,
          // there will only be one besides ourselves, but there may be
          // more.  Pick the best.

          TYPENAME Edge::Strips &strips = (*ei)->_strips;
          MesherStrip *mate = NULL;
          for (si = strips.begin(); si != strips.end(); ++si) {
            if ((*si)->_row_id < first_row_id) {
              if (mate==NULL || pick_sheet_mate(**si, *mate)) {
                mate = *si;
              }
            }
          }
          if (mate!=NULL) {
            mate->measure_sheet(*ei, secondary, num_prims, num_rows,
                               first_row_id, this_row_id,
                               this_row_distance + secondary);
          }
        }
      }
    }

  } else {
    // Otherwise, this is not a quad.  It's certainly not a triangle,
    // because we've built all the single triangles already.
    nassertv(_type != BPT_tri);

    // Therefore, it must be a tristrip or quadstrip.
    nassertv(_type == BPT_tristrip || _type == BPT_quadstrip);

    // Since it's a strip, it only has two neighbors: the one we came
    // from, and the other one.  Find the other one.

    for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
      if (!(*ei)->matches(*edge)) {
        // Here's the edge.  Same drill as above.

        TYPENAME Edge::Strips &strips = (*ei)->_strips;
        MesherStrip *mate = NULL;
        for (si = strips.begin(); si != strips.end(); ++si) {
          if ((*si)->_row_id < first_row_id) {
            if (mate==NULL || pick_sheet_mate(**si, *mate)) {
              mate = *si;
            }
          }
        }
        if (mate!=NULL) {
          mate->measure_sheet(*ei, false, num_prims, num_rows,
                             first_row_id, this_row_id, this_row_distance);
        }
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::cut_sheet
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class PrimType>
void MesherStrip<PrimType>::
cut_sheet(int first_row_id, int do_mate, const BuilderBucket &bucket) {
  TYPENAME Edges::iterator ei;
  TYPENAME Edge::Strips::iterator si;

  // First, start the process going on any neighbors that belong to a
  // later row.  (We must do these first, because we'll change our
  // neighbor list when we start to mate.)

  // We need to build a temporary list of neighbors first, because
  // calling cut_sheet() recursively will start things mating, and
  // could damage our edge list.

  typedef plist<MesherStrip *> StripPtrs;
  StripPtrs strip_ptrs;

  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    TYPENAME Edge::Strips &strips = (*ei)->_strips;
    for (si = strips.begin(); si != strips.end(); ++si) {
      if ((*si)->_row_id > _row_id) {
        // Here's a different row in the sheet!
        strip_ptrs.push_back(*si);
      }
    }
  }

  // Now walk the temporary list and do some damage.  We pass do_mate
  // = true to each of these neighbors, because as far as we know,
  // they're the first nodes of a particular row.
  TYPENAME StripPtrs::iterator spi;
  for (spi = strip_ptrs.begin(); spi != strip_ptrs.end(); ++spi) {
    if ((*spi)->_status == MS_alive) {
      (*spi)->cut_sheet(first_row_id, true, bucket);
    }
  }


  if (do_mate && _status == MS_alive) {
    // Now mate like a bunny until we don't have any more eligible mates.
    int not_any;
    do {
      not_any = true;

      ei = _edges.begin();
      while (ei != _edges.end() && not_any) {
        TYPENAME Edge::Strips &strips = (*ei)->_strips;
        si = strips.begin();
        while (si != strips.end() && not_any) {
          if (*si != this && (*si)->_row_id == _row_id) {
            // Here's one!
            not_any = false;
            MesherStrip *mate = *si;

            // We also recurse on these guys so they can spread the
            // word to their own neighbors.  This time we don't need
            // to build a temporary list, because we'll be restarting
            // from the beginning of our edge list after we do this.
            // We also pass do_mate = false to these guys because
            // we're the ones doing the mating here.
            mate->cut_sheet(first_row_id, false, bucket);

            if (_status == MS_alive && mate->_status == MS_alive) {
              // Now mate.  This will either succeed or fail.  It ought
              // to succeed, but if it doesn't, no harm done; it will
              // simply remove the common edge and return.  We'll go
              // around again and not encounter this neighbor next time.
              mate_pieces(*ei, *this, *mate, bucket);
            }
          }
          if (not_any) {
            ++si;
          }
        }
        if (not_any) {
          ++ei;
        }
      }
    } while (!not_any);

    // All done.  Mark this one as down for the count.
    _row_id = -first_row_id;
  }
}




////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::mate
//       Access: Public
//  Description: Finds a neighboring strip and joins up with it to
//               make a larger strip.  Returns true if mating was
//               successful or at least possible, false if the strip
//               has no neighbors.
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
mate(const BuilderBucket &bucket) {
  // We must walk through the list of our neighbors and choose our
  // best mate.
  nassertr(_status == MS_alive, false);

  MesherStrip *mate;
  Edge *common_edge;

  if (!find_ideal_mate(mate, common_edge, bucket)) {
    // We have no more eligible neighbors.  Call us done.
    _status = MS_done;

    return false;
  }

  nassertr(!mate->_prims.empty(), false);
  nassertr(!mate->_verts.empty(), false);

  mate_pieces(common_edge, *this, *mate, bucket);

  // Whether the mate failed or not, the strip still (probably) has
  // other neighbors to consider.  Return true regardless.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::find_ideal_mate
//       Access: Public
//  Description: Searches our neighbors for the most suitable mate.
//               Returns true if one is found, false if we have no
//               neighbors.
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
find_ideal_mate(MesherStrip *&mate, Edge *&common_edge,
              const BuilderBucket &bucket) {
  TYPENAME Edges::iterator ei;

  mate = NULL;
  common_edge = NULL;

  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    TYPENAME Edge::Strips &strips = (*ei)->_strips;
    TYPENAME Edge::Strips::iterator si;
    for (si = strips.begin(); si != strips.end(); ++si) {
      if (*si != this) {
        if (mate==NULL || pick_mate(**si, *mate, **ei, *common_edge,
                                   bucket)) {
          mate = *si;
          common_edge = *ei;
        }
      }
    }
  }

  return (mate!=NULL);
}




////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::mate_pieces
//       Access: Public, Static
//  Description: Connects two pieces of arbitrary type, if possible.
//               Returns true if successful, false if failure.
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
mate_pieces(Edge *common_edge, MesherStrip &front, MesherStrip &back,
            const BuilderBucket &bucket) {
  nassertr(front._status == MS_alive, false);
  nassertr(back._status == MS_alive, false);
  nassertr(&front != &back, false);

  bool success = true;
  // remove_sides tracks whether we want to remove all but the leading
  // edges of the newly joined piece if we succeed.
  bool remove_sides = true;

  bool is_coplanar = front.is_coplanar_with(back, bucket._coplanar_threshold);

  if (front._type==BPT_tri && back._type==BPT_tri) {

    if (is_coplanar && bucket._retesselate_coplanar &&
        front._prims.front() == back._prims.front() &&
        convex_quad(common_edge, front, back, bucket)) {

      // If we're joining two equivalent coplanar triangles, call it a
      // quad.
      front._type = BPT_quad;

      // We add one additional vertex for the new triangle, the one
      // vertex we didn't already share.
      const Vertex *new_vert = back.find_uncommon_vertex(common_edge);

      // Now we just need to find the right place to insert it.  It
      // belongs in the middle of the common edge, i.e. after the first
      // vertex that is on the common edge and before the second vertex.
      TYPENAME Verts::iterator a = front._verts.begin();
      TYPENAME Verts::iterator b = a;
      ++b;

      if (common_edge->contains_vertex(*a)) {
        if (common_edge->contains_vertex(*b)) {
          // It goes between a and b.
          front._verts.insert(b, new_vert);
        } else {
          // It goes at the end.
          front._verts.push_back(new_vert);
        }
      } else {
        // It goes between b and c.
        ++b;
        front._verts.insert(b, new_vert);
      }

      front._prims.splice(front._prims.end(), back._prims);
      back._verts.clear();

      // We leave all four surrounding edges for now, since the quad
      // might still be joined up in any direction.
      remove_sides = false;

    } else {
      // Otherwise, connect the two tris into a tristrip.
      front._type = BPT_tristrip;

      const Vertex *new_vert = back.find_uncommon_vertex(common_edge);
      front.rotate_to_back(common_edge);

      front._verts.push_back(new_vert);
      front._prims.splice(front._prims.end(), back._prims);
      back._verts.clear();
    }

  } else if ((front._type==BPT_quad || front._type==BPT_quadstrip) &&
             (back._type==BPT_quad || back._type==BPT_quadstrip)) {
    // Joining two quads, two quadstrips, or a quad and a quadstrip.
    // This makes another quadstrip.

    // We expect this to succeed every time with quadstrips.
    success = mate_strips(common_edge, front, back, BPT_quadstrip);

    if (!success) {
      // Although it might fail in rare circumstances (specifically,
      // if the two strips we attempted to join were backfacing to
      // each other).  If so, remove the adjoining edge so these two
      // don't get tried again.
      common_edge->remove(&front);
      common_edge->remove(&back);
    }

  } else {

    // Otherwise.  This might be two tristrips, a quad and a tristrip,
    // a triangle and a quad, a triangle and a tristrip, a triangle
    // and a quadstrip, or a tristrip and a quadstrip.  In any case,
    // we'll end up with a tristrip.

    // This might fail if the tristrips don't match polarity.
    success = mate_strips(common_edge, front, back, BPT_tristrip);

    if (!success) {
      // If it does fail, we'll try reversing the connection.  This
      // makes sense if we are joining a tri or tristrip to a quad or
      // quadstrip, which might fail in one direction but succeed in
      // the other.
      success = mate_strips(common_edge, back, front, BPT_tristrip);

      if (success) {
        // Yay!  Now return all the stuff to front.
        front._verts.splice(front._verts.end(), back._verts);
        front._prims.splice(front._prims.end(), back._prims);
      } else {
        // A miserable failure.  Never try to join these two again.

        common_edge->remove(&front);
        common_edge->remove(&back);
      }
    }
  }

  if (success) {
    front.combine_edges(back, remove_sides);
    if (!remove_sides) {
      // If we didn't want to remove the side edges, at least remove
      // the join edge, which is now internal.
      common_edge->remove(&front);
    }

    nassertr(back._prims.empty(), false);
    nassertr(back._verts.empty(), false);

    // Strip back is no more.
    back._status = MS_dead;

    // The result is planar if and only if we joined two coplanar
    // pieces.
    front._planar = is_coplanar;
    front._origin = MO_mate;
  }

  return success;
}


////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::mate_strips
//       Access: Public, Static
//  Description: Stitches two strips together, producing in "front" a
//               new strip of the indicated type (quadstrip or
//               tristrip).  The front strip stores the result, and
//               the back strip is emptied on success.
//
//               Returns true if successful, false if failure
//               (generally because of incorrect polarity of
//               tristrips), in which case nothing has changed (or at
//               least, not much).
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
mate_strips(Edge *common_edge, MesherStrip &front, MesherStrip &back,
            BuilderPrimType type) {
  // If we start with a quad or tri, rotate the vertices around so we
  // start with the common edge.
  if (front._type==BPT_tri || front._type==BPT_quad) {
    front.rotate_to_back(common_edge);
  }
  if (back._type==BPT_tri || back._type==BPT_quad) {
    back.rotate_to_front(common_edge);
  }

  bool reverse_front = common_edge->matches(front.get_head_edge());
  bool reverse_back = !common_edge->matches(back.get_head_edge());

  bool invert_front = false;
  bool invert_back = false;

  if (reverse_front && front.is_odd()) {
    // If we're going to reverse the front strip, we have to be
    // careful.  This will also reverse the facing direction if it has
    // an odd number of prims.
    if (!front.can_invert()) {
      return false;
    }
    invert_front = true;
  }

  if (must_invert(front, back, reverse_back, type)) {
    if (!back.can_invert()) {
      return false;
    }
    invert_back = true;
    back.invert();
  }

  if (invert_front) {
    front.invert();
  }

  if (reverse_front) {
    reverse(front._verts.begin(), front._verts.end());
    reverse(front._prims.begin(), front._prims.end());
  }

  if (reverse_back) {
    reverse(back._verts.begin(), back._verts.end());
    reverse(back._prims.begin(), back._prims.end());
  }

  bool will_reverse = front.would_reverse_tail(type);
  bool is_headtotail = (front.get_tail_edge() == back.get_head_edge());
  if (will_reverse == is_headtotail) {
    // Instead of crapping out, for now we'll just recover and carry on.
    //    builder_cat.info() << "Recovering from attempt to join backfacing strips.\n";
    if (reverse_back) {
      reverse(back._verts.begin(), back._verts.end());
      reverse(back._prims.begin(), back._prims.end());
    }
    if (invert_back) {
      back.invert();
    }
    if (reverse_front) {
      reverse(front._verts.begin(), front._verts.end());
      reverse(front._prims.begin(), front._prims.end());
    }
    if (invert_front) {
      front.invert();
    }
    return false;
  }

  front.convert_to_type(type);
  back.convert_to_type(type);

  /*
  if (! (front.get_tail_edge() == back.get_head_edge()) ) {
    builder_cat.error()
         << "\nFailure, trying to connect " << front
         << "\nto " << back
         << "\nreverse_front = " << reverse_front
         << " reverse_back = " << reverse_back
         << " invert_front = " << invert_front
         << "\n";
    Edges::iterator ei;

    nout << "\nFront edges:\n";
    for (ei = front._edges.begin(); ei != front._edges.end(); ++ei) {
      nout << **ei << "\n";
    }

    nout << "\nBack edges:\n";
    for (ei = back._edges.begin(); ei != back._edges.end(); ++ei) {
      nout << **ei << "\n";
    }
  }
  */

  // If this assertion fails, we were misinformed about our ability to
  // join these two strips.  Either the must_invert() call returned the
  // incorrect value, or our edge-detection logic failed and we
  // attempted to join two oppositely-facing strips.
  //nassertr(front.get_tail_edge() == back.get_head_edge(), false);

  front._verts.pop_back();
  front._verts.pop_back();
  front._verts.splice(front._verts.end(), back._verts);
  front._prims.splice(front._prims.end(), back._prims);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::must_invert
//       Access: Public, Static
//  Description: Returns false if the strips can be mated as they
//               currently are.  Returns true if the back strip must
//               be inverted first.
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
must_invert(const MesherStrip &front, const MesherStrip &back,
            bool will_reverse_back, BuilderPrimType type) {
  bool invert = false;

  if ((front._type == BPT_quad || front._type == BPT_quadstrip) &&
      type == BPT_tristrip) {
    // If we'll be converting from quads to tris, the tail edge of the
    // front strip will always be even.

  } else if (front.is_odd()) {
    // Otherwise, we have to flip if the tail edge is odd.
    invert = !invert;
  }

  if (will_reverse_back) {
    // With the back strip, we don't care about what will happen to
    // its tail edge when we convert it, but we do care what happens
    // to its front edge if we reverse it.
    if (back.is_odd()) {
      // Specifically, the front edge will be reversed when the strip
      // is reversed only if the strip is odd.
      invert = !invert;
    }
  }

  return invert;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::convex_quad
//       Access: Public, Static
//  Description: Returns true if the quad that would be formed by
//               connecting coplanar tris front and back along
//               common_edge is convex, false otherwise.
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
convex_quad(Edge *common_edge, MesherStrip &front, MesherStrip &back,
            const BuilderBucket &bucket) {
  // Find the edge from the apex of one triangle to the apex of the
  // other.  This is the "other" diagonal of the quad-to-be, other
  // than the common_edge.
  const Vertex *a = front.find_uncommon_vertex(common_edge);
  const Vertex *b = back.find_uncommon_vertex(common_edge);
  nassertr(a!=NULL && b!=NULL, false);

  Vertexf a3, b3, c3, d3;
  a3 = a->get_coord_value(bucket);
  b3 = b->get_coord_value(bucket);

  c3 = common_edge->_a->get_coord_value(bucket);
  d3 = common_edge->_b->get_coord_value(bucket);

  // Project both edges into the 2-d axis plane most nearly
  // perpendicular to the normal.  We're assuming both tris have the
  // same normal.

  nassertr(front._planar, false);

  LVector3f &n = front._plane_normal;
  int xi, yi;

  // Find the largest dimension of the normal.
  if (fabs(n[0]) > fabs(n[1])) {
    if (fabs(n[0]) > fabs(n[2])) {
      xi = 1;
      yi = 2;
    } else {
      xi = 0;
      yi = 1;
    }
  } else {
    if (fabs(n[1]) > fabs(n[2])) {
      xi = 0;
      yi = 2;
    } else {
      xi = 0;
      yi = 1;
    }
  }

  LVecBase2f a2, b2, c2, d2;
  a2.set(a3[xi], a3[yi]);
  b2.set(b3[xi], b3[yi]);
  c2.set(c3[xi], c3[yi]);
  d2.set(d3[xi], d3[yi]);

  // Now (c2-d2) is the common edge, and (a2-b2) is the new edge.  The
  // quad is convex iff (c2-d2) intersects (a2-b2).  We actually only
  // need to test whether (c2-d2) intersects the infinite line passing
  // through (a2-b2).

  // The equation for the infinite line containing (a2-b2):
  // Ax + By + C = 0
  double A = (b2[1] - a2[1]);
  double B = (a2[0] - b2[0]);
  double C = -(A*b2[0] + B*b2[1]);

  // The parametric equations for the line segment (c2-d2):
  // x = c2[0] + (d2[0]-c2[0])t
  // y = c2[1] + (d2[1]-c2[1])t

  // Solved for t:
  double t = - ((A*c2[0] + B*c2[1]) + C) / (A*(d2[0]-c2[0]) + B*(d2[1]-c2[1]));

  // Now the lines intersect if t is in [0, 1].
  return (0.0 <= t && t <= 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::count_neighbors
//       Access: Public
//  Description: Returns the number of neighbors the strip shares.
////////////////////////////////////////////////////////////////////
template <class PrimType>
int MesherStrip<PrimType>::
count_neighbors() const {
  int count = 0;
  TYPENAME Edges::const_iterator ei;

  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    count += (*ei)->_strips.size();
  }
  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::show_neighbors
//       Access: Public
//  Description: Writes all the neighbor indexes to the ostream.
////////////////////////////////////////////////////////////////////
template <class PrimType>
ostream &MesherStrip<PrimType>::
show_neighbors(ostream &out) const {
  TYPENAME Edges::const_iterator ei;
  TYPENAME Edge::Strips::const_iterator si;

  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    for (si = (*ei)->_strips.begin();
         si != (*ei)->_strips.end();
         ++si) {
      out << " " << (*si)->_index;
    }
  }
  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::find_uncommon_vertex
//       Access: Public
//  Description: Returns the first vertex found that is not shared by
//               the given edge.
////////////////////////////////////////////////////////////////////
template <class PrimType>
const TYPENAME MesherStrip<PrimType>::Vertex *MesherStrip<PrimType>::
find_uncommon_vertex(const Edge *edge) const {
  const Vertex *a = edge->_a;
  const Vertex *b = edge->_b;

  TYPENAME Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    Edge *e = (*ei);

    if (e->_a != a && e->_a != b) {
      return e->_a;
    } else if (e->_b != a && e->_b != b) {
      return e->_b;
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::find_opposite_edge
//       Access: Public
//  Description: Returns the first edge found that does not contain
//               the given vertex.  In a tri, this will be the edge
//               opposite the given vertex.
////////////////////////////////////////////////////////////////////
template <class PrimType>
const TYPENAME MesherStrip<PrimType>::Edge *MesherStrip<PrimType>::
find_opposite_edge(const Vertex *vertex) const {
  TYPENAME Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    Edge *e = (*ei);
    if (!e->contains_vertex(vertex)) {
      return e;
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::find_opposite_edge
//       Access: Public
//  Description: Returns the first edge found that shares no vertices
//               with the given edge.  In a quad, this will be the
//               edge opposite the given edge.
////////////////////////////////////////////////////////////////////
template <class PrimType>
const TYPENAME MesherStrip<PrimType>::Edge *MesherStrip<PrimType>::
find_opposite_edge(const Edge *edge) const {
  const Vertex *a = edge->_a;
  const Vertex *b = edge->_b;

  TYPENAME Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    Edge *e = (*ei);
    if (!e->contains_vertex(a) && !e->contains_vertex(b)) {
      return e;
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::find_adjacent_edge
//       Access: Public
//  Description: Returns the first edge found that shares exactly one
//               vertex with the given edge.  In a quad, this will be
//               one of two edges adjacent to the given edge.
////////////////////////////////////////////////////////////////////
template <class PrimType>
const TYPENAME MesherStrip<PrimType>::Edge *MesherStrip<PrimType>::
find_adjacent_edge(const Edge *edge) const {
  const Vertex *a = edge->_a;
  const Vertex *b = edge->_b;

  TYPENAME Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    Edge *e = (*ei);
    if (e->contains_vertex(a) != e->contains_vertex(b)) {
      return e;
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::rotate_to_front
//       Access: Public
//  Description: Rotates a triangle or quad so that the given edge is
//               first in the vertex list.
////////////////////////////////////////////////////////////////////
template <class PrimType>
void MesherStrip<PrimType>::
rotate_to_front(const Edge *edge) {
  const Vertex *a = edge->_a;
  const Vertex *b = edge->_b;

  // See if we're already there.
  if (_verts.front() == a || _verts.front() == b) {
    TYPENAME Verts::iterator vi = _verts.begin();
    ++vi;
    if (*vi == a || *vi == b) {
      // Yes!
      return;
    }

    // No, we must be right on the line.  Roll back one.
    rotate_back();

  } else {
    // Roll forward until it comes into view.
    int num_verts = _verts.size();
    while (_verts.front() != a && _verts.front() != b) {
      // Make sure either vertex exists.
      num_verts--;
      nassertv(num_verts > 0);
      rotate_forward();
    }
  }

  // Now make sure the edge actually exists.
  TYPENAME Verts::iterator vi = _verts.begin();
  ++vi;
  nassertv(*vi == a || *vi == b);
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::rotate_to_back
//       Access: Public
//  Description: Rotates a triangle or quad so that the given edge is
//               last in the vertex list.
////////////////////////////////////////////////////////////////////
template <class PrimType>
void MesherStrip<PrimType>::
rotate_to_back(const Edge *edge) {
  const Vertex *a = edge->_a;
  const Vertex *b = edge->_b;

  // See if we're already there.
  if (_verts.back() == a || _verts.back() == b) {
    TYPENAME Verts::reverse_iterator vi = _verts.rbegin();
    ++vi;
    if (*vi == a || *vi == b) {
      // Yes!
      return;
    }

    // No, we must be right on the line.  Roll forward one.
    rotate_forward();

  } else {
    // Roll backward until it comes into view.
    int num_verts = _verts.size();
    while (_verts.back() != a && _verts.back() != b) {
      // Make sure either vertex exists.
      num_verts--;
      nassertv(num_verts > 0);
      rotate_back();
    }
  }

  // Now make sure the edge actually exists.
  TYPENAME Verts::reverse_iterator vi = _verts.rbegin();
  ++vi;
  nassertv(*vi == a || *vi == b);
}


////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::can_invert
//       Access: Public
//  Description: Returns true if the strip can be inverted (reverse
//               its facing direction).  Generally, this is true for
//               quadstrips and false for tristrips.
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
can_invert() const {
  return (_type==BPT_quadstrip || _type==BPT_quad);
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::invert
//       Access: Public
//  Description: Reverses the facing of a quadstrip by reversing pairs
//               of vertices.  Returns true if successful, false if
//               failure (for instance, on a tristrip).
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
invert() {
  if (!can_invert()) {
    return false;
  }
  TYPENAME Verts::iterator vi, vi2;
  vi = _verts.begin();
  while (vi != _verts.end()) {
    vi2 = vi;
    ++vi2;
    nassertr(vi2 != _verts.end(), false);

    // Exchange vi and vi2
    const Vertex *t = *vi2;
    *vi2 = *vi;
    *vi = t;

    // Increment
    vi = vi2;
    ++vi;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::is_odd
//       Access: Public
//  Description: Returns true if the tristrip or quadstrip contains an
//               odd number of pieces.
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
is_odd() const {
  if (_type==BPT_quadstrip || _type==BPT_quad) {
    // If a quadstrip has a multiple of four vertices, it has an
    // odd number of quads.
    return (_verts.size()%4 == 0);
  } else {
    // If a tristrip has an odd number of vertices, it has an odd
    // number of tris.
    return (_verts.size()%2 == 1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::would_reverse_tail
//       Access: Public
//  Description: Returns true if convert_to_type() would reverse the
//               tail edge of the given strip, false otherwise.
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
would_reverse_tail(BuilderPrimType wantType) const {
  bool reverse = false;

  if (_type==wantType) {
    return false;
  }
  if (wantType==BPT_tristrip) {
    switch (_type) {
    case BPT_tri:
    case BPT_tristrip:
      break;

    case BPT_quad:
    case BPT_quadstrip:
      // When we convert a quadstrip to a tristrip, we reverse the
      // tail edge if we have a multiple of four verts.
      reverse = (_verts.size()%4==0);
      break;

    default:
      builder_cat.fatal() << "Invalid conversion!\n";
      abort();
      break;
    }

  } else if (wantType==BPT_quadstrip) {
    switch (_type) {
    case BPT_quad:
    case BPT_quadstrip:
      break;

    case BPT_tri:
    case BPT_tristrip:
      // We don't convert tristrips to quadstrips; fall through.

    default:
      builder_cat.fatal() << "Invalid conversion!\n";
      abort();
      break;
    }
  }

  return reverse;
}


////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::convert_to_type
//       Access: Public
//  Description: Converts the MesherStrip from whatever form it
//               is--triangle, quad, or quadstrip--into a tristrip or
//               quadstrip.
////////////////////////////////////////////////////////////////////
template <class PrimType>
void MesherStrip<PrimType>::
convert_to_type(BuilderPrimType wantType) {
  TYPENAME Verts::iterator vi, vi2;
  int even;

  if (_type==wantType) {
    return;
  }
  if (wantType==BPT_tristrip) {
    switch (_type) {
    case BPT_tri:
    case BPT_tristrip:
      break;

    case BPT_quad:
    case BPT_quadstrip:
      // To convert from quad/quadstrip to tristrip, we reverse every other
      // pair of vertices.

      vi = _verts.begin();
      even = 0;
      while (vi != _verts.end()) {
        vi2 = vi;
        ++vi2;
        nassertv(vi2 != _verts.end());

        // vi and vi2 are a pair.  Should we reverse them?
        if (even) {
          const Vertex *t = *vi2;
          *vi2 = *vi;
          *vi = t;
        }

        // Increment
        vi = vi2;
        ++vi;
        even = !even;
      }
      break;

    default:
      builder_cat.fatal() << "Invalid conversion!\n";
      abort();
    }

  } else if (wantType==BPT_quadstrip) {
    switch (_type) {
    case BPT_quad:
    case BPT_quadstrip:
      break;

    case BPT_tri:
    case BPT_tristrip:
      // We don't convert tristrips to quadstrips; fall through.

    default:
      builder_cat.fatal() << "Invalid conversion!\n";
      abort();
    }
  }

  _type = wantType;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::combine_edges
//       Access: Public
//  Description: Removes the edges from the given strip and appends
//               them to our own.  If removeSides is true, then
//               removes all the edges except the head and the tail.
////////////////////////////////////////////////////////////////////
template <class PrimType>
void MesherStrip<PrimType>::
combine_edges(MesherStrip<PrimType> &other, int removeSides) {
  TYPENAME Edges::iterator ei;
  for (ei = other._edges.begin(); ei != other._edges.end(); ++ei) {
    (*ei)->change_strip(&other, this);
  }

  _edges.splice(_edges.end(), other._edges);

  if (removeSides) {
    // Identify the head and tail edges so we can remove everything
    // else.
    Edge head = get_head_edge();
    Edge tail = get_tail_edge();

    if (!is_odd()) {
      // If the strip is odd, its true tail edge is the inverse of its
      // actual edge.
      tail = Edge(tail._b, tail._a);
    }

    Edges junk_edges;

    TYPENAME Edges::iterator next_ei;
    ei = _edges.begin();
    while (ei != _edges.end()) {
      next_ei = ei;
      ++next_ei;

      // Is this edge to be saved or is it fodder?
      if (!(**ei == head) && !(**ei == tail)) {
        // Fodder!  But we can't remove it right away, because this
        // will upset the current list; instead, we'll splice it to
        // junk_edges.
        junk_edges.splice(junk_edges.end(), _edges, ei);
      }
      ei = next_ei;
    }

    // Now we can safely remove all the to-be-junked edges.
    for (ei = junk_edges.begin(); ei != junk_edges.end(); ++ei) {
      (*ei)->remove(this);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::remove_all_edges
//       Access: Public
//  Description: Removes all active edges from the strip.  This
//               effectively renders it ineligible to mate with
//               anything else.
////////////////////////////////////////////////////////////////////
template <class PrimType>
void MesherStrip<PrimType>::
remove_all_edges() {
  // First, move all the edges to a safe place so we can traverse the
  // list without it changing on us.
  Edges junk_edges;
  junk_edges.splice(junk_edges.end(), _edges);

  // Now we can safely remove all the to-be-junked edges.
  TYPENAME Edges::iterator ei;
  for (ei = junk_edges.begin(); ei != junk_edges.end(); ++ei) {
    (*ei)->remove(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::pick_mate
//       Access: Public
//  Description: Defines an ordering to select neighbors to mate with.
//               This compares strip a with strip b and returns true
//               if strip a is the preferable choice, false if strip
//               b.
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
pick_mate(const MesherStrip &a_strip, const MesherStrip &b_strip,
          const Edge &a_edge, const Edge &b_edge,
          const BuilderBucket &bucket) const {
  // First, try to avoid polluting quads, quadstrips, and tristrips
  // with arbitrary triangles.  When we mate a tri or tristrip to a
  // quadstrip, we end up with a tristrip that may be less versatile
  // than the original quadstrip.  Better to avoid this if we can.
  // Try to choose a mate that more closely matches our own type.
  int a_cat = a_strip.type_category();
  int b_cat = b_strip.type_category();
  if (a_cat != b_cat) {
    int me_cat = type_category();
    return abs(a_cat - me_cat) < abs(b_cat - me_cat);
  }

  // Now, if we're connecting two tris, try to connect them up so they
  // make good quads.
  if (_type == BPT_tri && a_strip._type == BPT_tri &&
      b_strip._type == BPT_tri) {

    // This will depend on both coplanarity and edge length.  We can't
    // use just one or the other, because some tris are nearly
    // isosceles, and some have more than one coplanar neighbor.
    // Hopefully the combination of both factors will zero us in on
    // the correct neighbor first.

    double a_coplanar = coplanarity(a_strip);
    double b_coplanar = coplanarity(b_strip);
    double coplanar_diff = a_coplanar - b_coplanar;

    double a_length = a_edge.compute_length(bucket);
    double b_length = b_edge.compute_length(bucket);
    double length_diff = (a_length - b_length) / (a_length + b_length);

    // These weights were chosen empirically to yield fairly good results.
    double sum = 4.0 * coplanar_diff - 1.0 * length_diff;
    return sum < 0;
  }

  // Then, get the smallest strip.
  if (a_strip._prims.size() != b_strip._prims.size()) {
    return a_strip._prims.size() < b_strip._prims.size();
  }

  // Finally, get the strip with the fewest neighbors.
  return a_strip.count_neighbors() < b_strip.count_neighbors();
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::pick_sheet_mate
//       Access: Public
//  Description: Defines an ordering to select neighbors to follow
//               when measuring out a quadsheet.  This is only called
//               when three or more prims share a single edge, which
//               should be rarely--generally only when coplanar polys
//               are going on.
////////////////////////////////////////////////////////////////////
template <class PrimType>
bool MesherStrip<PrimType>::
pick_sheet_mate(const MesherStrip &a_strip, const MesherStrip &b_strip) const {
  // First, try to get the poly which is closest to our own normal.
  if (_planar && a_strip._planar && b_strip._planar) {
    float a_diff = dot(_plane_normal, a_strip._plane_normal);
    float b_diff = dot(_plane_normal, b_strip._plane_normal);

    if (fabs(a_diff - b_diff) > 0.0001f) {
      return a_diff > b_diff;
    }
  }

  // Then, pick the one that's most like our own type.
  int a_cat = a_strip.type_category();
  int b_cat = b_strip.type_category();
  if (a_cat != b_cat) {
    int me_cat = type_category();
    return abs(a_cat - me_cat) < abs(b_cat - me_cat);
  }

  // Oh, just pick any old one.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MesherStrip::output
//       Access: Public
//  Description: Formats the vertex for output in some sensible way.
////////////////////////////////////////////////////////////////////
template <class PrimType>
ostream &MesherStrip<PrimType>::
output(ostream &out) const {
  switch (_status) {
  case MS_alive:
    break;

  case MS_dead:
    out << "Dead ";
    break;

  case MS_done:
    out << "Done ";
    break;

  default:
    out << "Unknown status ";
  }

  switch (_type) {
  case BPT_tri:
    out << "Tri";
    break;

  case BPT_quad:
    out << "Quad";
    break;

  case BPT_tristrip:
    out << "TriStrip";
    break;

  case BPT_trifan:
    out << "TriFan";
    break;

  case BPT_quadstrip:
    out << "QuadStrip";
    break;

  default:
    out << "Unknown";
  }

  if (_planar) {
    out << " (planar)";
  }

  out << " " << _index << " [";

  TYPENAME Verts::const_iterator vi;
  for (vi = _verts.begin(); vi != _verts.end(); vi++) {
    out << " " << **vi;
  }
  out << " ]: " << _prims.size()
      << " prims, " << count_neighbors() << " neighbors";

  show_neighbors(out);

  out << " edges";
  TYPENAME Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ei++) {
    out << " " << (void *)(*ei);
  }

  return out << ".";
}

