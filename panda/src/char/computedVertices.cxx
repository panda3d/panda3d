// Filename: computedVertices.cxx
// Created by:  drose (01Mar99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "computedVertices.h"
#include "characterJoint.h"
#include "character.h"
#include "config_char.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "ioPtaDatagramLinMath.h"

#include <algorithm>

TypeHandle ComputedVertices::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: ComputedVertices::VertexTransform::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ComputedVertices::VertexTransform::
VertexTransform(const VertexTransform &copy) :
  _joint_index(copy._joint_index),
  _effect(copy._effect),
  _vindex(copy._vindex),
  _nindex(copy._nindex)
{
}

////////////////////////////////////////////////////////////////////
//     Function: compute_morphs
//  Description: This is a utility function called by update(), below.
//               It applies each of the supplied morphs to the
//               vertices in the table, according to the values of the
//               relevant slider.
////////////////////////////////////////////////////////////////////
template<class ValueType, class MorphType>
static void
compute_morphs(ValueType *table, const pvector<MorphType> &morph_list,
               Character *character) {
  TYPENAME pvector<MorphType>::const_iterator mli;
  for (mli = morph_list.begin(); mli != morph_list.end(); ++mli) {
    const MorphType &morph = (*mli);
    const CharacterSlider *slider;
    DCAST_INTO_V(slider, character->get_part(morph._slider_index));
    
    float slider_value = slider->_value;
    
    if (slider_value != 0.0f) {
      typedef TYPENAME MorphType::Morphs Morphs;
      typedef TYPENAME MorphType::MorphValue MorphValue;
      TYPENAME Morphs::const_iterator mi;
      for (mi = morph._morphs.begin(); mi != morph._morphs.end(); ++mi) {
        typedef TYPENAME MorphValue::VecType VecType;
        ushort index = (*mi)._index;
        const VecType &v = (*mi)._vector;
        
        table[index] += v * slider_value;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void ComputedVertices::VertexTransform::
write_datagram(Datagram &dest)
{
  int i;
  dest.add_int16(_joint_index);
  dest.add_float32(_effect);
  dest.add_uint16(_vindex.size());
  for(i = 0; i < (int)_vindex.size(); i++)
  {
    dest.add_uint16(_vindex[i]);
  }
  dest.add_uint16(_nindex.size());
  for(i = 0; i < (int)_nindex.size(); i++)
  {
    dest.add_uint16(_nindex[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::read_datagram
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVertices::VertexTransform::
read_datagram(DatagramIterator &source)
{
  int i;
  _joint_index = source.get_int16();
  _effect = source.get_float32();
  int vsize = source.get_uint16();
  for(i = 0; i < vsize; i++)
  {
    _vindex.push_back(source.get_uint16());
  }
  int nsize = source.get_uint16();
  for(i = 0; i < nsize; i++)
  {
    _nindex.push_back(source.get_uint16());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVertices::update
//       Access: Public
//  Description: Recomputes all of the _coords, _norms, etc. values
//               based on the values in _orig_coords, _orig_norms,
//               etc., and the current positions of all of the joints.
////////////////////////////////////////////////////////////////////
void ComputedVertices::
update(Character *character) {
  nassertv(character != (Character *)NULL);
  nassertv(character->_cv._coords.size() == _orig_coords.size());
  nassertv(character->_cv._norms.size() == _orig_norms.size());
  nassertv(character->_cv._texcoords.size() == _orig_texcoords.size());
  nassertv(character->_cv._colors.size() == _orig_colors.size());

  const Vertexf *orig_coords = _orig_coords;
  const Normalf *orig_norms = _orig_norms;

  memset(character->_cv._coords, 0, sizeof(Vertexf) * character->_cv._coords.size());
  memset(character->_cv._norms, 0, sizeof(Normalf) * character->_cv._norms.size());

  if (!_vertex_morphs.empty()) {
    // We have some vertex morphs.  Compute them first.
    int table_size = sizeof(Vertexf) * _orig_coords.size();
    Vertexf *morphed_coords = (Vertexf *)alloca(table_size);
    memcpy(morphed_coords, _orig_coords, table_size);

    compute_morphs(morphed_coords, _vertex_morphs, character);
    orig_coords = morphed_coords;
  }

  if (!_normal_morphs.empty()) {
    // We also have some normal morphs.  Compute them too.
    int table_size = sizeof(Normalf) * _orig_norms.size();
    Normalf *morphed_norms = (Normalf *)alloca(table_size);
    memcpy(morphed_norms, _orig_norms, table_size);

    compute_morphs(morphed_norms, _normal_morphs, character);
    orig_norms = morphed_norms;
  }

  if (!_texcoord_morphs.empty()) {
    // We have some uv morphs.  These don't particularly need to be
    // done before the joints are computed, but do them now anyway.
    int table_size = sizeof(TexCoordf) * _orig_texcoords.size();

    // **** Is this right?  Test it!
    //    TexCoordf *morphed_texcoords = (TexCoordf *)alloca(table_size);
    memcpy(character->_cv._texcoords, _orig_texcoords, table_size);

    compute_morphs(character->_cv._texcoords.p(), _texcoord_morphs, character);
  }

  if (!_color_morphs.empty()) {
    // We have some color morphs.  Do these now too.
    int table_size = sizeof(Colorf) * _orig_colors.size();

    // **** Is this right?  Test it!
    // Colorf *morphed_colors = (Colorf *)alloca(table_size);
    memcpy(character->_cv._colors, _orig_colors, table_size);

    compute_morphs(character->_cv._colors.p(), _color_morphs, character);
  }

  // Now that we've computed all the morphs, it's safe to transform
  // vertices into their proper coordinate space, according to the
  // current positions of all the joints.

  LMatrix4f mat = LMatrix4f::ident_mat();
  int last_joint_index = -1;

  VertexTransforms::const_iterator vti;
  for (vti = _transforms.begin(); vti != _transforms.end(); ++vti) {
    const VertexTransform &vt = (*vti);

    // We cache the matrix from the last joint, because we are likely
    // to encounter the same joint several times in a row.
    if (vt._joint_index != last_joint_index) {
      last_joint_index = vt._joint_index;

      // We won't encounter -1 after the first few joints.
      nassertv(vt._joint_index >= 0);
      CharacterJoint *joint;
      DCAST_INTO_V(joint, character->get_part(vt._joint_index));

      mat =
        joint->_initial_net_transform_inverse *
        joint->_net_transform;
    }

    Vertices::const_iterator vi;
    for (vi = vt._vindex.begin(); vi != vt._vindex.end(); ++vi) {
      int i = (*vi);
      character->_cv._coords[i] += (orig_coords[i] * mat) * vt._effect;
    }
    for (vi = vt._nindex.begin(); vi != vt._nindex.end(); ++vi) {
      int i = (*vi);
      character->_cv._norms[i] += (orig_norms[i] * mat) * vt._effect;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVertices::make_orig
//       Access: Public
//  Description: Copies all the values loaded in the _coords, _norms,
//               etc. arrays into the corresponding _orig_coords,
//               etc. arrays.
////////////////////////////////////////////////////////////////////
void ComputedVertices::
make_orig(Character *character) {
  nassertv(character != (Character *)NULL);

  if (character->_cv._coords.empty()) {
    _orig_coords.clear();
  } else {
    _orig_coords = PTA_Vertexf::empty_array(0);
    _orig_coords.v() = character->_cv._coords.v();
  }

  if (character->_cv._norms.empty()) {
    _orig_norms.clear();
  } else {
    _orig_norms = PTA_Normalf::empty_array(0);
    _orig_norms.v() = character->_cv._norms.v();
  }

  if (character->_cv._colors.empty()) {
    _orig_colors.clear();
  } else {
    _orig_colors = PTA_Colorf::empty_array(0);
    _orig_colors.v() = character->_cv._colors.v();
  }

  if (character->_cv._texcoords.empty()) {
    _orig_texcoords.clear();
  } else {
    _orig_texcoords = PTA_TexCoordf::empty_array(0);
    _orig_texcoords.v() = character->_cv._texcoords.v();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVertices::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVertices::
write(ostream &out, Character *character) const {
  VertexTransforms::const_iterator vti;

  out << "ComputedVertices:\n";
  for (vti = _transforms.begin(); vti != _transforms.end(); ++vti) {
    const VertexTransform &vt = (*vti);
    string name;
    if (vt._joint_index >= 0) {
      name = character->get_part(vt._joint_index)->get_name();
    } else {
      name = "(root)";
    }

    out << "  " << name << " * " << vt._effect << " : "
    << vt._vindex.size() << " vertices and "
    << vt._nindex.size() << " normals\n";
  }

  VertexMorphs::const_iterator vmi;
  for (vmi = _vertex_morphs.begin(); vmi != _vertex_morphs.end(); ++vmi) {
    out << "  vertex " << (*vmi) << "\n";
  }

  NormalMorphs::const_iterator nmi;
  for (nmi = _normal_morphs.begin(); nmi != _normal_morphs.end(); ++nmi) {
    out << "  normal " << (*nmi) << "\n";
  }

  TexCoordMorphs::const_iterator tmi;
  for (tmi = _texcoord_morphs.begin(); tmi != _texcoord_morphs.end(); ++tmi) {
    out << "  uv " << (*tmi) << "\n";
  }

  ColorMorphs::const_iterator cmi;
  for (cmi = _color_morphs.begin(); cmi != _color_morphs.end(); ++cmi) {
    out << "  color " << (*cmi) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVertices::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void ComputedVertices::
write_datagram(BamWriter *manager, Datagram &me)
{
  int i;

  me.add_uint16(_transforms.size());
  for(i = 0; i < (int)_transforms.size(); i++){
    _transforms[i].write_datagram(me);
  }

  me.add_uint16(_vertex_morphs.size());
  for(i = 0; i < (int)_vertex_morphs.size(); i++){
    _vertex_morphs[i].write_datagram(me);
  }

  me.add_uint16(_normal_morphs.size());
  for(i = 0; i < (int)_normal_morphs.size(); i++){
    _normal_morphs[i].write_datagram(me);
  }

  me.add_uint16(_texcoord_morphs.size());
  for(i = 0; i < (int)_texcoord_morphs.size(); i++){
    _texcoord_morphs[i].write_datagram(me);
  }

  me.add_uint16(_color_morphs.size());
  for(i = 0; i < (int)_color_morphs.size(); i++){
    _color_morphs[i].write_datagram(me);
  }

  //Original Coordinates for vertices, colors, normals and textures
  WRITE_PTA(manager, me, IPD_Vertexf::write_datagram, _orig_coords)
  WRITE_PTA(manager, me, IPD_Normalf::write_datagram, _orig_norms)
  WRITE_PTA(manager, me, IPD_Colorf::write_datagram, _orig_colors)
  WRITE_PTA(manager, me, IPD_TexCoordf::write_datagram, _orig_texcoords)
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVertices::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void ComputedVertices::
fillin(DatagramIterator& scan, BamReader* manager)
{
  int i, size;

  size = scan.get_uint16();
  for(i = 0; i < size; i++){
    VertexTransform vt;
    vt.read_datagram(scan);
     _transforms.push_back(vt);
  }

  size = scan.get_uint16();
  for(i = 0; i < size; i++){
    ComputedVerticesMorphVertex vm;
    vm.read_datagram(scan);
    _vertex_morphs.push_back(vm);
  }

  size = scan.get_uint16();
  for(i = 0; i < size; i++){
    ComputedVerticesMorphNormal nm;
    nm.read_datagram(scan);
    _normal_morphs.push_back(nm);
  }

  size = scan.get_uint16();
  for(i = 0; i < size; i++){
    ComputedVerticesMorphTexCoord tm;
    tm.read_datagram(scan);
    _texcoord_morphs.push_back(tm);
  }

  size = scan.get_uint16();
  for(i = 0; i < size; i++){
    ComputedVerticesMorphColor cm;
    cm.read_datagram(scan);
    _color_morphs.push_back(cm);
  }

  //Original Coordinates for vertices, colors, normals and textures
  READ_PTA(manager, scan, IPD_Vertexf::read_datagram, _orig_coords)
  READ_PTA(manager, scan, IPD_Normalf::read_datagram, _orig_norms)
  READ_PTA(manager, scan, IPD_Colorf::read_datagram, _orig_colors)
  READ_PTA(manager, scan, IPD_TexCoordf::read_datagram, _orig_texcoords)
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVertices::make_ComputedVertices
//       Access: Protected
//  Description: Factory method to generate a ComputedVertices object
////////////////////////////////////////////////////////////////////
TypedWritable* ComputedVertices::
make_ComputedVertices(const FactoryParams &params)
{
  ComputedVertices *me = new ComputedVertices;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVertices::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a ComputedVertices object
////////////////////////////////////////////////////////////////////
void ComputedVertices::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_ComputedVertices);
}





