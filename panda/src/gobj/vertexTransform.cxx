// Filename: vertexTransform.cxx
// Created by:  drose (23Mar05)
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

#include "vertexTransform.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"

PipelineCycler<VertexTransform::CData> VertexTransform::_global_cycler;
UpdateSeq VertexTransform::_next_modified;

TypeHandle VertexTransform::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
VertexTransform::
VertexTransform() {
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
VertexTransform::
~VertexTransform() {
  // We shouldn't destruct while any TransformPalettes are holding our
  // pointer.
  nassertv(_palettes.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::mult_matrix
//       Access: Published, Virtual
//  Description: Premultiplies this transform's matrix with the
//               indicated previous matrix, so that the result is the
//               net composition of the given transform with this
//               transform.  The result is stored in the parameter
//               "result", which should not be the same matrix as
//               previous.
////////////////////////////////////////////////////////////////////
void VertexTransform::
mult_matrix(LMatrix4f &result, const LMatrix4f &previous) const {
  nassertv(&result != &previous);
  LMatrix4f me;
  get_matrix(me);
  result.multiply(me, previous);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::accumulate_matrix
//       Access: Published, Virtual
//  Description: Adds the value of this transform's matrix, modified
//               by the indicated weight, into the indicated
//               accumulation matrix.  This is used to compute the
//               result of several blended transforms.
////////////////////////////////////////////////////////////////////
void VertexTransform::
accumulate_matrix(LMatrix4f &accum, float weight) const {
  LMatrix4f me;
  get_matrix(me);
  
  accum._m.m._00 += me._m.m._00 * weight;
  accum._m.m._01 += me._m.m._01 * weight;
  accum._m.m._02 += me._m.m._02 * weight;
  accum._m.m._03 += me._m.m._03 * weight;
  
  accum._m.m._10 += me._m.m._10 * weight;
  accum._m.m._11 += me._m.m._11 * weight;
  accum._m.m._12 += me._m.m._12 * weight;
  accum._m.m._13 += me._m.m._13 * weight;
  
  accum._m.m._20 += me._m.m._20 * weight;
  accum._m.m._21 += me._m.m._21 * weight;
  accum._m.m._22 += me._m.m._22 * weight;
  accum._m.m._23 += me._m.m._23 * weight;
  
  accum._m.m._30 += me._m.m._30 * weight;
  accum._m.m._31 += me._m.m._31 * weight;
  accum._m.m._32 += me._m.m._32 * weight;
  accum._m.m._33 += me._m.m._33 * weight;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VertexTransform::
output(ostream &out) const {
  out << get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VertexTransform::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) 
    << *this << ":\n";
  LMatrix4f mat;
  get_matrix(mat);
  mat.write(out, indent_level + 2);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::get_next_modified
//       Access: Public, Static
//  Description: Returns a monotonically increasing sequence.  Each
//               time this is called, a new sequence number is
//               returned, higher than the previous value.
//
//               This is used to ensure that all
//               VertexTransform::get_modified() calls return an
//               increasing number in the same space, so that
//               TransformBlend::get_modified() is easy to determine.
//               It is similar to Geom::get_modified(), but it is in a
//               different space.
////////////////////////////////////////////////////////////////////
UpdateSeq VertexTransform::
get_next_modified() {
  ++_next_modified;

  CDWriter cdatag(_global_cycler);
  cdatag->_modified = _next_modified;

  return _next_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::mark_modified
//       Access: Protected
//  Description: Intended to be called by a derived class whenever the
//               reported transform might have changed.  Without
//               calling this method, changes to get_matrix() may not
//               be propagated through the system.
////////////////////////////////////////////////////////////////////
void VertexTransform::
mark_modified() {
  CDWriter cdata(_cycler);
  cdata->_modified = get_next_modified();
  
  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    (*pi)->update_modified(cdata->_modified);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void VertexTransform::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new VertexTransform.
////////////////////////////////////////////////////////////////////
void VertexTransform::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *VertexTransform::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void VertexTransform::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int VertexTransform::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new VertexTransform.
////////////////////////////////////////////////////////////////////
void VertexTransform::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
}
