// Filename: transformBlendPalette.cxx
// Created by:  drose (24Mar05)
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

#include "transformBlendPalette.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle TransformBlendPalette::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
TransformBlendPalette::
TransformBlendPalette() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
TransformBlendPalette::
TransformBlendPalette(const TransformBlendPalette &copy) :
  _blends(copy._blends)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
operator = (const TransformBlendPalette &copy) {
  _blends = copy._blends;
  clear_index();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TransformBlendPalette::
~TransformBlendPalette() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::set_blend
//       Access: Published
//  Description: Replaces the blend at the nth position with the
//               indicated value.
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
set_blend(int n, const TransformBlend &blend) {
  nassertv(n >= 0 && n < (int)_blends.size());
  _blends[n] = blend;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::remove_blend
//       Access: Published
//  Description: Removes the blend at the nth position.
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
remove_blend(int n) {
  nassertv(n >= 0 && n < (int)_blends.size());
  _blends.erase(_blends.begin() + n);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::add_blend
//       Access: Published
//  Description: Adds a new blend to the palette, and returns its
//               index number.  If there is already an identical blend
//               in the palette, simply returns that number instead.
////////////////////////////////////////////////////////////////////
int TransformBlendPalette::
add_blend(const TransformBlend &blend) {
  consider_rebuild_index();

  BlendIndex::iterator bi;
  bi = _blend_index.find(&blend);
  if (bi != _blend_index.end()) {
    // Already had it.
    return (*bi).second;
  }

  int new_position = (int)_blends.size();
  _blends.push_back(blend);
  clear_index();

  return new_position;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
write(ostream &out, int indent_level) const {
  for (int i = 0; i < (int)_blends.size(); i++) {
    indent(out, indent_level)
      << i << ". " << _blends[i] << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::clear_index
//       Access: Private
//  Description: Resets the index so that it will be rebuilt next time
//               it is needed.
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
clear_index() {
  _blend_index.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::rebuild_index
//       Access: Private
//  Description: Rebuilds the index so that we can easily determine
//               what blend combinations are already present in the
//               palette.
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
rebuild_index() {
  _blend_index.clear();

  // We'll also count up these two statistics while we rebuild the
  // index.
  _num_transforms = 0;
  _max_simultaneous_transforms = 0;

  pset<const VertexTransform *> transforms;

  for (int i = 0; i < (int)_blends.size(); ++i) {
    const TransformBlend &blend = _blends[i];
    _blend_index[&blend] = i;

    for (int ti = 0; ti < blend.get_num_transforms(); ++ti) {
      transforms.insert(blend.get_transform(ti));
    }
    _max_simultaneous_transforms = max(_max_simultaneous_transforms,
                                       blend.get_num_transforms());
  }

  _num_transforms = transforms.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::recompute_modified
//       Access: Private
//  Description: Recomputes the modified stamp from the various
//               TransformBlend objects, if necessary.
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
recompute_modified(TransformBlendPalette::CDWriter &cdata) {
  // Update the global_modified sequence number first, to prevent race
  // conditions.
  cdata->_global_modified = VertexTransform::get_global_modified();

  // Now get the local modified number.
  UpdateSeq seq;
  Blends::const_iterator bi;
  for (bi = _blends.begin(); bi != _blends.end(); ++bi) {
    seq = max(seq, (*bi).get_modified());
  }

  cdata->_modified = seq;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::clear_modified
//       Access: Private
//  Description: Clears the modified stamp to force it to be
//               recomputed.
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
clear_modified() {
  CDWriter cdata(_cycler);
  cdata->_global_modified = UpdateSeq();
  cdata->_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TransformBlendPalette.
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int TransformBlendPalette::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);
  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TransformBlendPalette is encountered
//               in the Bam file.  It should create the TransformBlendPalette
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TransformBlendPalette::
make_from_bam(const FactoryParams &params) {
  TransformBlendPalette *object = new TransformBlendPalette;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TransformBlendPalette.
////////////////////////////////////////////////////////////////////
void TransformBlendPalette::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *TransformBlendPalette::CData::
make_copy() const {
  return new CData(*this);
}
