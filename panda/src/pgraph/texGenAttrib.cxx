// Filename: texGenAttrib.cxx
// Created by:  masad (21Jun04)
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

#include "texGenAttrib.h"
#include "texturePool.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "dcast.h"

CPT(RenderAttrib) TexGenAttrib::_empty_attrib;
TypeHandle TexGenAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TexGenAttrib::
~TexGenAttrib() {
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::make
//       Access: Published, Static
//  Description: Constructs a TexGenAttrib that generates no stages at
//               all.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexGenAttrib::
make() {
  // We make it a special case and store a pointer to the empty attrib
  // forever once we find it the first time, as an optimization.
  if (_empty_attrib == (RenderAttrib *)NULL) {
    _empty_attrib = return_new(new TexGenAttrib);
  }

  return _empty_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::make
//       Access: Published, Static
//  Description: Constructs a TexGenAttrib that generates just the
//               indicated stage.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexGenAttrib::
make(TextureStage *stage, TexGenAttrib::Mode mode, 
     const string &source_name, const NodePath &light) {
  return DCAST(TexGenAttrib, make())->add_stage(stage, mode, source_name, light);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::add_stage
//       Access: Published, Static
//  Description: Returns a new TexGenAttrib just like this one,
//               with the indicated generation mode for the given
//               stage.  If this stage already exists, its mode is
//               replaced.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexGenAttrib::
add_stage(TextureStage *stage, TexGenAttrib::Mode mode, 
          const string &source_name, const NodePath &light) const {
  CPT(RenderAttrib) removed = remove_stage(stage);
  TexGenAttrib *attrib = new TexGenAttrib(*DCAST(TexGenAttrib, removed));

  ModeDef &mode_def = attrib->_stages[stage];
  mode_def._mode = mode;
  mode_def._source_name = source_name;
  mode_def._light = light;
  switch (mode) {
  case M_point_sprite:
    attrib->_no_texcoords.insert(stage);
    attrib->_point_geom_rendering |= Geom::GR_point_sprite;
    attrib->_num_point_sprites++;
    break;

  case M_light_vector:
    {
      if (!light.is_empty()) {
	Light *light_obj = light.node()->as_light();
	if (light_obj == (Light *)NULL) {
	  ostringstream strm;
	  strm << "Not a light: " << light;
	  nassert_raise(strm.str());
	  mode_def._light = NodePath();
	}
      }
	  
      attrib->_light_vectors.insert(stage);
      attrib->_geom_rendering |= Geom::GR_texcoord_light_vector;
      attrib->_num_light_vectors++;
    }
    break;

  case M_off:
    break;

  default:
    attrib->_no_texcoords.insert(stage);
  }
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::remove_stage
//       Access: Published, Static
//  Description: Returns a new TexGenAttrib just like this one,
//               with the indicated stage removed.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexGenAttrib::
remove_stage(TextureStage *stage) const {
  Stages::const_iterator si;
  si = _stages.find(stage);
  if (si == _stages.end()) {
    return this;
  }

  Mode mode = (*si).second._mode;
  TexGenAttrib *attrib = new TexGenAttrib(*this);
  attrib->_stages.erase(stage);
  attrib->_no_texcoords.erase(stage);
  if (mode == M_point_sprite) {
    attrib->_num_point_sprites--;
    if (attrib->_num_point_sprites == 0) {
      attrib->_point_geom_rendering &= ~Geom::GR_point_sprite;
    }
  } else if (mode == M_light_vector) {
    attrib->_light_vectors.erase(stage);
    attrib->_num_light_vectors--;
    if (attrib->_num_light_vectors == 0) {
      attrib->_geom_rendering &= ~Geom::GR_texcoord_light_vector;
    }
  }
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::is_empty
//       Access: Published
//  Description: Returns true if no stages are defined in the
//               TexGenAttrib, false if at least one is.
////////////////////////////////////////////////////////////////////
bool TexGenAttrib::
is_empty() const {
  return _stages.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::has_stage
//       Access: Published
//  Description: Returns true if there is a mode associated with
//               the indicated stage, or false otherwise (in which
//               case get_transform(stage) will return M_off).
////////////////////////////////////////////////////////////////////
bool TexGenAttrib::
has_stage(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  return (mi != _stages.end());
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::get_mode
//       Access: Published
//  Description: Returns the generation mode associated with
//               the named texture stage, or M_off if
//               nothing is associated with the indicated stage.
////////////////////////////////////////////////////////////////////
TexGenAttrib::Mode TexGenAttrib::
get_mode(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  if (mi != _stages.end()) {
    return (*mi).second._mode;
  }
  return M_off;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::get_source_name
//       Access: Published
//  Description: Returns the source name associated with the named
//               texture stage, or the empty string if no name is
//               associated with the indicated stage.  This is only
//               meaningful if the mode is M_light_vector, in which
//               case it indicates the name of the source texture
//               coordinate set from which the tangent and binormal
//               are derived.
////////////////////////////////////////////////////////////////////
string TexGenAttrib::
get_source_name(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  if (mi != _stages.end()) {
    return (*mi).second._source_name;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::get_light
//       Access: Published
//  Description: Returns the Light associated with the named texture
//               stage, or the empty NodePath if no light is
//               associated with the indicated stage.  This is only
//               meaningful if the mode is M_light_vector.
////////////////////////////////////////////////////////////////////
NodePath TexGenAttrib::
get_light(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  if (mi != _stages.end()) {
    return (*mi).second._light;
  }
  return NodePath();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void TexGenAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_tex_gen(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TexGenAttrib::
output(ostream &out) const {
  out << get_type() << ":";

  Stages::const_iterator mi;
  for (mi = _stages.begin(); mi != _stages.end(); ++mi) {
    TextureStage *stage = (*mi).first;
    const ModeDef &mode_def = (*mi).second;
    out << " " << stage->get_name() << "(";
    switch (mode_def._mode) {
    case M_off:
      out << "off";
      break;

    case M_eye_sphere_map:
      out << "eye_sphere_map";
      break;

    case M_world_cube_map:
      out << "world_cube_map";
      break;
    case M_eye_cube_map:
      out << "eye_cube_map";
      break;

    case M_world_normal:
      out << "world_normal";
      break;
    case M_eye_normal:
      out << "eye_normal";
      break;

    case M_world_position:
      out << "world_position";
      break;
    case M_eye_position:
      out << "eye_position";
      break;

    case M_point_sprite:
      out << "point_sprite";
      break;

    case M_light_vector:
      out << "light_vector: \"" << mode_def._source_name << "\", "
          << mode_def._light;
      break;

    case M_unused:
      break;
    }
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TexGenAttrib
//               types to return a unique number indicating whether
//               this TexGenAttrib is equivalent to the other one.
//
//               This should return 0 if the two TexGenAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two TexGenAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int TexGenAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const TexGenAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  
  Stages::const_iterator ai, bi;
  ai = _stages.begin();
  bi = ta->_stages.begin();
  while (ai != _stages.end() && bi != ta->_stages.end()) {
    if ((*ai).first < (*bi).first) {
      // This stage is in a but not in b.
      return -1;

    } else if ((*bi).first < (*ai).first) {
      // This stage is in b but not in a.
      return 1;

    } else {
      // This stage is in both; compare the stages.
      int compare = (*ai).second.compare_to((*bi).second);
      if (compare != 0) {
        return compare;
      }
      ++ai;
      ++bi;
    }
  }

  if (bi != ta->_stages.end()) {
    // a ran out first; b was longer.
    return -1;
  }

  if (ai != _stages.end()) {
    // b ran out first; a was longer.
    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               This should return the result of applying the other
//               RenderAttrib to a node in the scene graph below this
//               RenderAttrib, which was already applied.  In most
//               cases, the result is the same as the other
//               RenderAttrib (that is, a subsequent RenderAttrib
//               completely replaces the preceding one).  On the other
//               hand, some kinds of RenderAttrib (for instance,
//               ColorTransformAttrib) might combine in meaningful
//               ways.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexGenAttrib::
compose_impl(const RenderAttrib *other) const {
  const TexGenAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  // The composition is the union of the two attribs.  In the case
  // when a stage is in both attribs, we compose the stages.

  TexGenAttrib *attrib = new TexGenAttrib;

  Stages::const_iterator ai, bi;
  ai = _stages.begin();
  bi = ta->_stages.begin();
  while (ai != _stages.end() && bi != ta->_stages.end()) {
    if ((*ai).first < (*bi).first) {
      // This stage is in a but not in b.
      attrib->_stages.insert(attrib->_stages.end(), *ai);
      ++ai;

    } else if ((*bi).first < (*ai).first) {
      // This stage is in b but not in a.
      attrib->_stages.insert(attrib->_stages.end(), *bi);
      ++bi;

    } else {
      // This stage is in both; b wins.
      attrib->_stages.insert(attrib->_stages.end(), *bi);
      ++bi;
      ++ai;
    }
  }

  while (ai != _stages.end()) {
    // This stage is in a but not in b.
    attrib->_stages.insert(attrib->_stages.end(), *ai);
    ++ai;
  }

  while (bi != ta->_stages.end()) {
    // This stage is in b but not in a.
    attrib->_stages.insert(attrib->_stages.end(), *bi);
    ++bi;
  }

  // Now copy from _stages to _no_texcoords.
  Stages::const_iterator ri;
  for (ri = attrib->_stages.begin(); ri != attrib->_stages.end(); ++ri) {
    TextureStage *stage = (*ri).first;
    const ModeDef &mode_def = (*ri).second;
    Mode mode = mode_def._mode;

    switch (mode) {
    case M_point_sprite:
      attrib->_no_texcoords.insert(stage);
      attrib->_point_geom_rendering |= Geom::GR_point_sprite;
      attrib->_num_point_sprites++;
      break;
      
    case M_light_vector:
      attrib->_light_vectors.insert(stage);
      attrib->_geom_rendering |= Geom::GR_texcoord_light_vector;
      attrib->_num_light_vectors++;
      break;
      
    case M_off:
      break;
      
    default:
      attrib->_no_texcoords.insert(stage);
    }
  }

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexGenAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  const TexGenAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  // The inverse composition works a lot like the composition, except
  // we invert the ai stages.

  TexGenAttrib *attrib = new TexGenAttrib;

  Stages::const_iterator ai, bi;
  ai = _stages.begin();
  bi = ta->_stages.begin();
  while (ai != _stages.end() && bi != ta->_stages.end()) {
    if ((*ai).first < (*bi).first) {
      // This stage is in a but not in b.  Turn a off.
      attrib->_stages.insert(attrib->_stages.end(), Stages::value_type((*ai).first, ModeDef()));
      ++ai;

    } else if ((*bi).first < (*ai).first) {
      // This stage is in b but not in a.
      attrib->_stages.insert(attrib->_stages.end(), *bi);
      ++bi;

    } else {
      // This stage is in both; b wins.
      attrib->_stages.insert(attrib->_stages.end(), *bi);
      ++bi;
      ++ai;
    }
  }

  while (ai != _stages.end()) {
    // This stage is in a but not in b.
    attrib->_stages.insert(attrib->_stages.end(), Stages::value_type((*ai).first, ModeDef()));
    ++ai;
  }

  while (bi != ta->_stages.end()) {
    // This stage is in b but not in a.
    attrib->_stages.insert(attrib->_stages.end(), *bi);
    ++bi;
  }

  // Now copy from _stages to _no_texcoords.
  Stages::const_iterator ri;
  for (ri = attrib->_stages.begin(); ri != attrib->_stages.end(); ++ri) {
    TextureStage *stage = (*ri).first;
    const ModeDef &mode_def = (*ri).second;
    Mode mode = mode_def._mode;

    switch (mode) {
    case M_point_sprite:
      attrib->_no_texcoords.insert(stage);
      attrib->_point_geom_rendering |= Geom::GR_point_sprite;
      attrib->_num_point_sprites++;
      break;
      
    case M_light_vector:
      attrib->_light_vectors.insert(stage);
      attrib->_geom_rendering |= Geom::GR_texcoord_light_vector;
      attrib->_num_light_vectors++;
      break;
      
    case M_off:
      break;
      
    default:
      attrib->_no_texcoords.insert(stage);
    }
  }

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TexGenAttrib
//               types to specify what the default property for a
//               TexGenAttrib of this type should be.
//
//               This should return a newly-allocated TexGenAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of TexGenAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *TexGenAttrib::
make_default_impl() const {
  return new TexGenAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TexGenAttrib.
////////////////////////////////////////////////////////////////////
void TexGenAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TexGenAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_uint16(_stages.size());

  Stages::const_iterator si;
  for (si = _stages.begin(); si != _stages.end(); ++si) {
    TextureStage *stage = (*si).first;
    Mode mode = (*si).second._mode;

    manager->write_pointer(dg, stage);
    dg.add_uint8((unsigned int)mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int TexGenAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  pvector<Mode>::const_iterator mi;
  for (mi = _read_modes.begin(); mi != _read_modes.end(); ++mi) {
    Mode mode = (*mi);

    TextureStage *stage = DCAST(TextureStage, p_list[pi++]);
    _stages[stage]._mode = mode;

    if (mode != M_off) {
      _no_texcoords.insert(stage);
    }
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TexGenAttrib is encountered
//               in the Bam file.  It should create the TexGenAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TexGenAttrib::
make_from_bam(const FactoryParams &params) {
  TexGenAttrib *attrib = new TexGenAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TexGenAttrib.
////////////////////////////////////////////////////////////////////
void TexGenAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  size_t num_stages = scan.get_uint16();

  // For now, read in a linear list of the modes we will assign to
  // each associated TextureStage pointer.  Later, in
  // complete_pointers, we'll fill up the map the with appropriate
  // TextureStage/Mode pairing.
  _read_modes.clear();
  _read_modes.reserve(num_stages);
  for (size_t i = 0; i < num_stages; i++) {
    manager->read_pointer(scan);
    Mode mode = (Mode)scan.get_uint8();
    _read_modes.push_back(mode);
  }
}
