// Filename: qpgeom.cxx
// Created by:  drose (06Mar05)
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

#include "qpgeom.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle qpGeom::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeom::
qpGeom() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeom::
qpGeom(const qpGeom &copy) :
  /*
  TypedWritableReferenceCount(copy),
  BoundedObject(copy),
  */
  Geom(copy),
  _cycler(copy._cycler)  
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void qpGeom::
operator = (const qpGeom &copy) {
  /*
  TypedWritableReferenceCount::operator = (copy);
  BoundedObject::operator = (copy);
  */
  Geom::operator = (copy);
  _cycler = copy._cycler;
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeom::
~qpGeom() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::make_copy
//       Access: Published, Virtual
//  Description: Temporarily redefined from Geom base class.
////////////////////////////////////////////////////////////////////
Geom *qpGeom::
make_copy() const {
  return new qpGeom(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::modify_vertex_data
//       Access: Published
//  Description: Returns a modifiable pointer to the GeomVertexData,
//               so that application code may directly maniuplate the
//               geom's underlying data.
////////////////////////////////////////////////////////////////////
PT(qpGeomVertexData) qpGeom::
modify_vertex_data() {
  // Perform copy-on-write: if the reference count on the vertex data
  // is greater than 1, assume some other Geom has the same pointer,
  // so make a copy of it first.

  CDWriter cdata(_cycler);
  if (cdata->_data->get_ref_count() > 1) {
    cdata->_data = new qpGeomVertexData(*cdata->_data);
  }
  mark_bound_stale();
  return cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::set_vertex_data
//       Access: Published
//  Description: Replaces the Geom's underlying vertex data table with
//               a completely new table.
////////////////////////////////////////////////////////////////////
void qpGeom::
set_vertex_data(PT(qpGeomVertexData) data) {
  CDWriter cdata(_cycler);
  cdata->_data = data;
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::add_primitive
//       Access: Published
//  Description: Adds a new GeomPrimitive structure to the Geom
//               object.  This specifies a particular subset of
//               vertices that are used to define geometric primitives
//               of the indicated type.
////////////////////////////////////////////////////////////////////
void qpGeom::
add_primitive(qpGeomPrimitive *primitive) {
  CDWriter cdata(_cycler);
  cdata->_primitives.push_back(primitive);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::remove_primitive
//       Access: Published
//  Description: Removes the ith primitive from the list.
////////////////////////////////////////////////////////////////////
void qpGeom::
remove_primitive(int i) {
  CDWriter cdata(_cycler);
  nassertv(i >= 0 && i < (int)cdata->_primitives.size());
  cdata->_primitives.erase(cdata->_primitives.begin() + i);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::clear_primitives
//       Access: Published
//  Description: Removes all the primitives from the Geom object (but
//               keeps the same table of vertices).  You may then
//               re-add primitives one at a time via calls to
//               add_primitive().
////////////////////////////////////////////////////////////////////
void qpGeom::
clear_primitives() {
  CDWriter cdata(_cycler);
  cdata->_primitives.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeom::
output(ostream &out) const {
  CDReader cdata(_cycler);

  // Get a list of the primitive types contained within this object.
  pset<TypeHandle> types;
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); 
       pi != cdata->_primitives.end();
       ++pi) {
    types.insert((*pi)->get_type());
  }

  out << "Geom [";
  pset<TypeHandle>::iterator ti;
  for (ti = types.begin(); ti != types.end(); ++ti) {
    out << " " << (*ti);
  }
  out << " ], " << cdata->_data->get_num_vertices() << " vertices.";
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeom::
write(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);

  // Get a list of the primitive types contained within this object.
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); 
       pi != cdata->_primitives.end();
       ++pi) {
    (*pi)->write(out, cdata->_data, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::draw
//       Access: Public
//  Description: Actually draws the Geom with the indicated GSG.
////////////////////////////////////////////////////////////////////
void qpGeom::
draw(GraphicsStateGuardianBase *gsg) const {
  CDReader cdata(_cycler);

  if (gsg->begin_draw_primitives(cdata->_data)) {
    Primitives::const_iterator pi;
    for (pi = cdata->_primitives.begin(); 
         pi != cdata->_primitives.end();
         ++pi) {
      (*pi)->draw(gsg);
    }
    gsg->end_draw_primitives();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::recompute_bound
//       Access: Protected, Virtual
//  Description: Recomputes the dynamic bounding volume for this Geom.
//               This includes all of the vertices.
////////////////////////////////////////////////////////////////////
BoundingVolume *qpGeom::
recompute_bound() {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = BoundedObject::recompute_bound();
  nassertr(bound != (BoundingVolume*)0L, bound);

  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bound);

  // Now actually compute the bounding volume by putting it around all
  // of our vertices.
  CDReader cdata(_cycler);

  const qpGeomVertexFormat *format = cdata->_data->get_format();

  int array_index = format->get_array_with(InternalName::get_vertex());
  if (array_index < 0) {
    // No vertex data.
    return bound;
  }

  const qpGeomVertexArrayFormat *array_format = format->get_array(array_index);
  const qpGeomVertexDataType *data_type = 
    array_format->get_data_type(InternalName::get_vertex());

  int stride = array_format->get_stride();
  int start = data_type->get_start();
  int num_components = data_type->get_num_components();

  CPTA_uchar array_data = cdata->_data->get_array_data(array_index);

  if (stride == 3 * sizeof(PN_float32) && start == 0 && num_components == 3 &&
      (array_data.size() % stride) == 0) {
    // Here's an easy special case: it's a standalone table of vertex
    // positions, with nothing else in the middle, so we can use
    // directly as an array of LPoint3f's.
    const LPoint3f *vertices_begin = (const LPoint3f *)&array_data[0];
    const LPoint3f *vertices_end = (const LPoint3f *)&array_data[array_data.size()];
    gbv->around(vertices_begin, vertices_end);

  } else {
    // Otherwise, we have to copy the vertex positions out one at time.
    pvector<LPoint3f> vertices;

    size_t p = start;
    while (p + stride <= array_data.size()) {
      const PN_float32 *v = (const PN_float32 *)&array_data[p];

      LPoint3f vertex;
      qpGeomVertexData::to_vec3(vertex, v, num_components);

      vertices.push_back(vertex);
      p += stride;
    }

    const LPoint3f *vertices_begin = &vertices[0];
    const LPoint3f *vertices_end = vertices_begin + vertices.size();

    gbv->around(vertices_begin, vertices_end);
  }

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeom::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeom::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeom is encountered
//               in the Bam file.  It should create the qpGeom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeom::
make_from_bam(const FactoryParams &params) {
  qpGeom *object = new qpGeom;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeom::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *qpGeom::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeom::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int qpGeom::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeom::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
}
