// Filename: paramTexture.cxx
// Created by:  rdb (11Dec14)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "paramTexture.h"
#include "dcast.h"

TypeHandle ParamTextureSampler::_type_handle;
TypeHandle ParamTextureImage::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureSampler::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ParamTextureSampler::
output(ostream &out) const {
  out << "texture ";

  if (_texture != (Texture *)NULL) {
    out << _texture->get_name();
  } else {
    out << "(empty)";
  }
  out << ", ";

  _sampler.output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureSampler::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ParamValue.
////////////////////////////////////////////////////////////////////
void ParamTextureSampler::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureSampler::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ParamTextureSampler::
write_datagram(BamWriter *manager, Datagram &dg) {
  ParamValueBase::write_datagram(manager, dg);
  manager->write_pointer(dg, _texture);
  _sampler.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureSampler::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int ParamTextureSampler::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ParamValueBase::complete_pointers(p_list, manager);
  _texture = DCAST(Texture, p_list[pi++]);
  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureSampler::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ParamValue is encountered
//               in the Bam file.  It should create the ParamValue
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ParamTextureSampler::
make_from_bam(const FactoryParams &params) {
  ParamTextureSampler *param = new ParamTextureSampler;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureSampler::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ParamValue.
////////////////////////////////////////////////////////////////////
void ParamTextureSampler::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParamValueBase::fillin(scan, manager);
  manager->read_pointer(scan);
  _sampler.read_datagram(scan, manager);
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureImage::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ParamTextureImage::
output(ostream &out) const {
  out << "texture ";

  if (_texture != (Texture *)NULL) {
    out << _texture->get_name();
  } else {
    out << "(empty)";
  }

  if (_access & A_read) {
    if (_access & A_write) {
      out << ", read-write";
    } else {
      out << ", read-only";
    }
  } else if (_access & A_write) {
    out << ", write-only";
  }

  if (_access & A_layered) {
    out << ", all layers";
  } else {
    out << ", layer " << _bind_layer;
  }

  out << ", level " << _bind_level;
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureImage::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ParamValue.
////////////////////////////////////////////////////////////////////
void ParamTextureImage::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureImage::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ParamTextureImage::
write_datagram(BamWriter *manager, Datagram &dg) {
  ParamValueBase::write_datagram(manager, dg);
  manager->write_pointer(dg, _texture);
  dg.add_uint8(_access);
  dg.add_int8(_bind_level);
  dg.add_int32(_bind_layer);
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureImage::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int ParamTextureImage::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ParamValueBase::complete_pointers(p_list, manager);
  _texture = DCAST(Texture, p_list[pi++]);
  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureImage::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ParamValue is encountered
//               in the Bam file.  It should create the ParamValue
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ParamTextureImage::
make_from_bam(const FactoryParams &params) {
  ParamTextureImage *param = new ParamTextureImage;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTextureImage::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ParamValue.
////////////////////////////////////////////////////////////////////
void ParamTextureImage::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParamValueBase::fillin(scan, manager);
  manager->read_pointer(scan);
  _access = scan.get_uint8();
  _bind_level = scan.get_int8();
  _bind_layer = scan.get_int32();
}
