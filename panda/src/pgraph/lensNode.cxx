/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lensNode.cxx
 * @author drose
 * @date 2002-02-26
 */

#include "lensNode.h"
#include "geometricBoundingVolume.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "perspectiveLens.h"
#include "geomNode.h"

TypeHandle LensNode::_type_handle;

/**
 *
 */
LensNode::
LensNode(const std::string &name, Lens *lens) :
  PandaNode(name)
{
  if (lens == nullptr) {
    lens = new PerspectiveLens;
  }
  set_lens(0, lens);
}

/**
 *
 */
LensNode::
LensNode(const LensNode &copy) :
  PandaNode(copy),
  _lenses(copy._lenses)
{
}

/**
 * Transforms the contents of this PandaNode by the indicated matrix, if it
 * means anything to do so.  For most kinds of PandaNodes, this does nothing.
 */
void LensNode::
xform(const LMatrix4 &mat) {
  PandaNode::xform(mat);
  // We need to actually transform the lens here.
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *LensNode::
make_copy() const {
  return new LensNode(*this);
}

/**
 * Sets the indicated lens.  Although a LensNode normally holds only one lens,
 * it may optionally include multiple lenses, each with a different index
 * number.  The different lenses may be referenced by index number on the
 * DisplayRegion.  Adding a new lens automatically makes it active.
 */
void LensNode::
set_lens(int index, Lens *lens) {
  nassertv(index >= 0 && index < max_lenses); // Sanity check

  while (index >= (int)_lenses.size()) {
    LensSlot slot;
    slot._is_active = false;
    _lenses.push_back(slot);
  }

  _lenses[index]._lens = lens;
  _lenses[index]._is_active = true;

  if (_shown_frustum != nullptr) {
    show_frustum();
  }
}

/**
 * Sets the active flag for the nth lens.  When a lens is inactive, it is not
 * used for rendering, and any DisplayRegions associated with it are
 * implicitly inactive as well.  Returns true if the flag is changed, false if
 * it already had this value.
 */
bool LensNode::
set_lens_active(int index, bool flag) {
  nassertr(index >= 0 && index < max_lenses, false);

  while (index >= (int)_lenses.size()) {
    LensSlot slot;
    slot._is_active = false;
    _lenses.push_back(slot);
  }

  if (_lenses[index]._is_active == flag) {
    return false;
  }

  _lenses[index]._is_active = flag;

  if (_shown_frustum != nullptr) {
    show_frustum();
  }
  return true;
}

/**
 * Returns true if the given point is within the bounds of the lens of the
 * LensNode (i.e.  if the camera can see the point).
 */
bool LensNode::
is_in_view(int index, const LPoint3 &pos) {
  Lens *lens = get_lens(index);
  nassertr(lens != nullptr, false);
  PT(BoundingVolume) bv = lens->make_bounds();
  if (bv == nullptr) {
    return false;
  }
  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bv);
  int ret = gbv->contains(pos);
  return (ret != 0);
}

/**
 * Enables the drawing of the lens's frustum to aid in visualization.  This
 * actually creates a GeomNode which is parented to the LensNode.
 */
void LensNode::
show_frustum() {
  if (_shown_frustum != nullptr) {
    hide_frustum();
  }
  PT(GeomNode) geom_node = new GeomNode("frustum");
  _shown_frustum = geom_node;
  add_child(_shown_frustum);

  for (Lenses::const_iterator li = _lenses.begin();
       li != _lenses.end();
       ++li) {
    if ((*li)._is_active && (*li)._lens != nullptr) {
      geom_node->add_geom((*li)._lens->make_geometry());
    }
  }
}

/**
 * Disables the drawing of the lens's frustum to aid in visualization.
 */
void LensNode::
hide_frustum() {
  if (_shown_frustum != nullptr) {
    remove_child(_shown_frustum);
    _shown_frustum = nullptr;
  }
}

/**
 *
 */
void LensNode::
output(std::ostream &out) const {
  PandaNode::output(out);

  out << " (";
  for (Lenses::const_iterator li = _lenses.begin();
       li != _lenses.end();
       ++li) {
    if ((*li)._is_active && (*li)._lens != nullptr) {
      out << " ";
      (*li)._lens->output(out);
    }
  }
  out << " )";
}

/**
 *
 */
void LensNode::
write(std::ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);

  for (Lenses::const_iterator li = _lenses.begin();
       li != _lenses.end();
       ++li) {
    if ((*li)._is_active && (*li)._lens != nullptr) {
      (*li)._lens->write(out, indent_level + 2);
    }
  }
}

/**
 * Tells the BamReader how to create objects of type LensNode.
 */
void LensNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void LensNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);

  if (manager->get_file_minor_ver() < 41) {
    // Prior to bam 6.41, we stored only one lens.
    manager->write_pointer(dg, get_lens(0));
  } else {
    dg.add_uint16(_lenses.size());

    Lenses::const_iterator li;
    for (li = _lenses.begin(); li != _lenses.end(); ++li) {
      manager->write_pointer(dg, (*li)._lens);
      dg.add_bool((*li)._is_active);
    }
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int LensNode::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);

  Lenses::iterator li;
  for (li = _lenses.begin(); li != _lenses.end(); ++li) {
    (*li)._lens = DCAST(Lens, p_list[pi++]);
  }

  if (_shown_frustum != nullptr) {
    show_frustum();
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type LensNode is encountered in the Bam file.  It should create the
 * LensNode and extract its information from the file.
 */
TypedWritable *LensNode::
make_from_bam(const FactoryParams &params) {
  LensNode *node = new LensNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new LensNode.
 */
void LensNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  if (manager->get_file_minor_ver() < 41) {
    // Prior to bam 6.41, we stored only one lens.
    _lenses.resize(1);
    manager->read_pointer(scan);

  } else {
    _lenses.resize(scan.get_uint16());
    Lenses::iterator li;
    for (li = _lenses.begin(); li != _lenses.end(); ++li) {
      manager->read_pointer(scan);
      (*li)._is_active = scan.get_bool();
    }
  }
}
