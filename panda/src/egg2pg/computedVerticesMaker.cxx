// Filename: computedVerticesMaker.cxx
// Created by:  drose (01Mar99)
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

#include "computedVerticesMaker.h"
#include "characterMaker.h"
#include "config_egg2pg.h"

#include "characterJoint.h"
#include "character.h"
#include "computedVertices.h"
#include "eggNode.h"
#include "eggGroup.h"
#include "eggVertex.h"
#include "internalName.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ComputedVerticesMaker::
ComputedVerticesMaker() {
  _coords = PTA_Vertexf::empty_array(0);
  _norms = PTA_Normalf::empty_array(0);
  _colors = PTA_Colorf::empty_array(0);
  _current_vc = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::begin_new_space
//       Access: Public
//  Description: Should be called before beginning the definition for
//               a new transform space.
////////////////////////////////////////////////////////////////////
void ComputedVerticesMaker::
begin_new_space() {
  _current_jw.clear();
  _current_vc = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::add_joint
//       Access: Public
//  Description: Adds the joint with its associated membership amount
//               to the current transform space definition.
////////////////////////////////////////////////////////////////////
void ComputedVerticesMaker::
add_joint(EggNode *joint, double membership) {
  // This must be called between a call to begin_new_space() and
  // mark_space().
  nassertv(_current_vc == NULL);

  if (membership == 0.0) {
    return;
  }

  if (membership < 0.0) {
    // Not sure what a negative membership should mean.  Probably it's
    // just a skinning error; for now we'll print a warning and ignore
    // it.
    egg2pg_cat.warning()
      << "Joint " << joint->get_name() << " has vertices with membership "
      << membership << ".\n";
    return;
  }

  JointWeights::iterator jwi = _current_jw.find(joint);

  if (jwi != _current_jw.end()) {
    // We'd already added this joint previously.  Increment its total
    // membership.
    (*jwi).second += membership;
  } else {
    // This is the first time we've added this joint.
    _current_jw[joint] = membership;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::add_vertex_joints
//       Access: Public
//  Description: Adds the joints the vertex belongs to, along with
//               their respective memberships, to the current
//               transform space definition.
////////////////////////////////////////////////////////////////////
void ComputedVerticesMaker::
add_vertex_joints(EggVertex *vertex, EggNode *object) {
  if (vertex->gref_size() == 0) {
    // This vertex belongs in the same group as the primitive that
    // contains it.
    EggGroupNode *egg_joint = object->get_parent();

    // We actually walk up to find the first group above that that's a
    // joint, or the character root itself, so we won't (a) be fooled
    // by meaningless transforms on non-joints within a character
    // hierarchy, or (b) consider meaninglessly different groups to be
    // significant.
    EggGroup *egg_group = (EggGroup *)NULL;
    if (egg_joint->is_of_type(EggGroup::get_class_type())) {
      egg_group = DCAST(EggGroup, egg_joint);
    }
    while (egg_group != (EggGroup *)NULL &&
           egg_group->get_group_type() != EggGroup::GT_joint &&
           egg_group->get_dart_type() == EggGroup::DT_none) {
      nassertv(egg_group->get_parent() != (EggGroupNode *)NULL);
      egg_joint = egg_group->get_parent();
      egg_group = (EggGroup *)NULL;
      if (egg_joint->is_of_type(EggGroup::get_class_type())) {
        egg_group = DCAST(EggGroup, egg_joint);
      }
    }

    add_joint(egg_joint, 1.0);

  } else {
    // This vertex belongs in the joint or joints that reference it.
    EggVertex::GroupRef::const_iterator gri;
    for (gri = vertex->gref_begin(); gri != vertex->gref_end(); ++gri) {
      EggGroup *egg_joint = (*gri);
      double membership = egg_joint->get_vertex_membership(vertex);
      add_joint(egg_joint, membership);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::mark_space
//       Access: Public
//  Description: Completes the definition of a transform space as a
//               set of joints and memberships.  From this point until
//               the next call to begin_new_space(), vertices may be
//               added to the transform space via calls to
//               add_vertex(), add_normal(), etc.
////////////////////////////////////////////////////////////////////
void ComputedVerticesMaker::
mark_space() {
  // This must be called after a call to begin_new_space().
  nassertv(_current_vc == NULL);

  _current_jw.normalize_weights();

  // This will look up a previously-defined VertexCollection, if we've
  // used this transform space before, or it will implicitly create a
  // new one if we haven't.
  _current_vc = &_transforms[_current_jw];
}


////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::add_vertex
//       Access: Public
//  Description: Adds a vertex value to the currently-defined
//               transform space, and returns its index number within
//               the array.
////////////////////////////////////////////////////////////////////
int ComputedVerticesMaker::
add_vertex(const Vertexd &vertex, const EggMorphVertexList &morphs,
           const LMatrix4d &transform) {
  // This must be called after a call to mark_space(), and before a
  // call to begin_new_space().
  nassertr(_current_vc != NULL, -1);

  Vertexf tv = LCAST(float, vertex * transform);
  int index = _current_vc->_vmap.add_value(tv, morphs, _coords);
  _current_vc->_vindex.insert(index);

  // Now create any morph sliders.
  EggMorphVertexList::const_iterator mli;
  for (mli = morphs.begin(); mli != morphs.end(); ++mli) {
    const EggMorphVertex &morph = (*mli);
    LVector3d offset = morph.get_offset() * transform;
    if (!offset.almost_equal(LVector3d(0.0, 0.0, 0.0), 0.0001)) {
      MorphList &mlist = _morphs[morph.get_name()];

      // Have we already morphed this vertex?
      VertexMorphList::iterator vmi = mlist._vmorphs.find(index);
      if (vmi != mlist._vmorphs.end()) {
        // Yes, we have.
        nassertr((*vmi).second.almost_equal(LCAST(float, offset), 0.001), index);

      } else {
        // No, we haven't yet; morph it now.
        mlist._vmorphs[index] = LCAST(float, offset);
      }
    }
  }

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::add_normal
//       Access: Public
//  Description: Adds a normal value to the currently-defined
//               transform space, and returns its index number within
//               the array.
////////////////////////////////////////////////////////////////////
int ComputedVerticesMaker::
add_normal(const Normald &normal, const EggMorphNormalList &morphs,
           const LMatrix4d &transform) {
  // This must be called after a call to mark_space(), and before a
  // call to begin_new_space().
  nassertr(_current_vc != NULL, -1);

  Normald norm = normal * transform;
  norm.normalize();
  int index = _current_vc->_nmap.add_value(LCAST(float, norm), morphs, _norms);
  _current_vc->_nindex.insert(index);

  // Now create any morph sliders.
  EggMorphNormalList::const_iterator mli;
  for (mli = morphs.begin(); mli != morphs.end(); ++mli) {
    const EggMorphNormal &morph = (*mli);
    LVector3d offset = morph.get_offset() * transform;
    if (!offset.almost_equal(LVector3d(0.0, 0.0, 0.0), 0.0001)) {
      MorphList &mlist = _morphs[morph.get_name()];

      // Have we already morphed this normal?
      NormalMorphList::iterator vmi = mlist._nmorphs.find(index);
      if (vmi != mlist._nmorphs.end()) {
        // Yes, we have.
        nassertr((*vmi).second.almost_equal(LCAST(float, offset), 0.001), index);
      } else {
        // No, we haven't yet; morph it now.
        mlist._nmorphs[index] = LCAST(float, offset);
      }
    }
  }

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::add_texcoord
//       Access: Public
//  Description: Adds a texcoord value to the named array (texture
//               coordinates are unrelated to the current transform
//               space), and returns its index number within the
//               array.
////////////////////////////////////////////////////////////////////
int ComputedVerticesMaker::
add_texcoord(const InternalName *name,
             const TexCoordd &texcoord, const EggMorphTexCoordList &morphs,
             const LMatrix3d &transform) {
  TexCoordDef &def = _tdefmap[name];

  TexCoordf ttc = LCAST(float, texcoord * transform);
  int index = def._tmap.add_value(ttc, morphs, _texcoords[name]);
  def._tindex.insert(index);

  // Now create any morph sliders.
  EggMorphTexCoordList::const_iterator mli;
  for (mli = morphs.begin(); mli != morphs.end(); ++mli) {
    const EggMorphTexCoord &morph = (*mli);
    LVector2d offset = morph.get_offset() * transform;
    if (!offset.almost_equal(LVector2d(0.0, 0.0), 0.0001)) {
      MorphList &mlist = _morphs[morph.get_name()];

      // Have we already morphed this texcoord?
      TexCoordMorphList &tmorphs = mlist._tmorphs[name];
      TexCoordMorphList::iterator vmi = tmorphs.find(index);
      if (vmi != tmorphs.end()) {
        // Yes, we have.
        nassertr((*vmi).second.almost_equal(LCAST(float, offset), 0.001), index);
      } else {
        // No, we haven't yet; morph it now.
        tmorphs[index] = LCAST(float, offset);
      }
    }
  }

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::add_color
//       Access: Public
//  Description: Adds a color value to the array (color values
//               are unrelated to the current transform space), and
//               returns its index number within the array.
////////////////////////////////////////////////////////////////////
int ComputedVerticesMaker::
add_color(const Colorf &color, const EggMorphColorList &morphs) {
  int index = _cmap.add_value(color, morphs, _colors);
  _cindex.insert(index);

  // Now create any morph sliders.
  EggMorphColorList::const_iterator mli;
  for (mli = morphs.begin(); mli != morphs.end(); ++mli) {
    const EggMorphColor &morph = (*mli);
    LVector4f offset = morph.get_offset();
    if (!offset.almost_equal(LVector4f(0.0, 0.0, 0.0, 0.0), 0.0001)) {
      MorphList &mlist = _morphs[morph.get_name()];

      // Have we already morphed this color?
      ColorMorphList::iterator vmi = mlist._cmorphs.find(index);
      if (vmi != mlist._cmorphs.end()) {
        // Yes, we have.
        nassertr((*vmi).second.almost_equal(offset, 0.001), index);
      } else {
        // No, we haven't yet; morph it now.
        mlist._cmorphs[index] = offset;
      }
    }
  }

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::make_computed_vertices
//       Access: Public
//  Description: After all spaces have been defined and all vertices
//               added, creates a new ComputedVertices object and
//               returns it.
////////////////////////////////////////////////////////////////////
ComputedVertices *ComputedVerticesMaker::
make_computed_vertices(Character *character, CharacterMaker &char_maker) {
  // We must first build up a set of all the unique kinds of vertex
  // transforms.
  typedef pset<ComputedVertices::VertexTransform> VertexTransforms;
  VertexTransforms transforms;

  TransformSpaces::const_iterator tsi;
  for (tsi = _transforms.begin();
       tsi != _transforms.end();
       ++tsi) {
    const JointWeights &jw = (*tsi).first;
    const VertexCollection &vc = (*tsi).second;

    JointWeights::const_iterator jwi;
    for (jwi = jw.begin(); jwi != jw.end(); ++jwi) {
      double weight = (*jwi).second;
      EggNode *egg_joint = (*jwi).first;
      int joint_index = char_maker.egg_to_index(egg_joint);

      // Look for a VertexTransform that matches this template.
      ComputedVertices::VertexTransform new_vt;
      new_vt._joint_index = joint_index;
      new_vt._effect = (float)weight;

      // This will either insert the VertexTransform into the set and
      // return its newly-created iterator, or it will return the
      // iterator referring to the previously-inserted VertexTransform
      // like this.
      VertexTransforms::iterator vti = transforms.insert(new_vt).first;

      // We can discard the const-ness of the set's iterator, because
      // we will only be changing a part of the VertexTransform that
      // doesn't affect its sort order within the set.
      ComputedVertices::VertexTransform &insert_vt =
        (ComputedVertices::VertexTransform &)*vti;

      // Now add in all the vertices and normals.
      copy(vc._vindex.begin(), vc._vindex.end(),
           back_inserter(insert_vt._vindex));
      copy(vc._nindex.begin(), vc._nindex.end(),
           back_inserter(insert_vt._nindex));
    }
  }

  // Ok, now we have the set of all VertexTransforms.  Create a
  // ComputedVertices object that reflects this.
  ComputedVertices *comp_verts = new ComputedVertices;
  copy(transforms.begin(), transforms.end(),
       back_inserter(comp_verts->_transforms));

  character->_cv._coords = _coords;
  character->_cv._norms = _norms;
  character->_cv._colors = _colors;

  // Temporary: the ComputedVertices object currently doesn't support
  // multitexture.
  character->_cv._texcoords = _texcoords[InternalName::get_texcoord().p()];

  // Finally, add in all the morph definitions.
  Morphs::const_iterator mi;
  for (mi = _morphs.begin(); mi != _morphs.end(); ++mi) {
    const string &name = (*mi).first;
    const MorphList &mlist = (*mi).second;

    int slider_index = char_maker.create_slider(name);

    if (!mlist._vmorphs.empty()) {
      // We push an empty MorphVertex object and then modify it,
      // rather than filling it first and then pushing it, just to
      // avoid unnecessary copying of data.
      comp_verts->_vertex_morphs.push_back(ComputedVerticesMorphVertex());
      ComputedVerticesMorphVertex &mv = comp_verts->_vertex_morphs.back();
      mv._slider_index = slider_index;

      VertexMorphList::const_iterator vmi;
      for (vmi = mlist._vmorphs.begin();
           vmi != mlist._vmorphs.end();
           ++vmi) {
        mv._morphs.push_back(ComputedVerticesMorphValue3((*vmi).first,
                                                         (*vmi).second));
      }
    }

    if (!mlist._nmorphs.empty()) {
      comp_verts->_normal_morphs.push_back(ComputedVerticesMorphNormal());
      ComputedVerticesMorphNormal &mv = comp_verts->_normal_morphs.back();
      mv._slider_index = slider_index;

      NormalMorphList::const_iterator vmi;
      for (vmi = mlist._nmorphs.begin();
           vmi != mlist._nmorphs.end();
           ++vmi) {
        mv._morphs.push_back(ComputedVerticesMorphValue3((*vmi).first,
                                                         (*vmi).second));
      }
    }

    if (!mlist._cmorphs.empty()) {
      comp_verts->_color_morphs.push_back(ComputedVerticesMorphColor());
      ComputedVerticesMorphColor &mv = comp_verts->_color_morphs.back();
      mv._slider_index = slider_index;

      ColorMorphList::const_iterator vmi;
      for (vmi = mlist._cmorphs.begin();
           vmi != mlist._cmorphs.end();
           ++vmi) {
        mv._morphs.push_back(ComputedVerticesMorphValue4((*vmi).first,
                                                         (*vmi).second));
      }
    }

    TexCoordMorphMap::const_iterator mmi;
    for (mmi = mlist._tmorphs.begin(); mmi != mlist._tmorphs.end(); ++mmi) {
      const InternalName *name = (*mmi).first;
      const TexCoordMorphList &tmorphs = (*mmi).second;

      // Temporary check: the ComputedVertices object currently
      // doesn't support multitexture.
      if (name == InternalName::get_texcoord()) {
        comp_verts->_texcoord_morphs.push_back(ComputedVerticesMorphTexCoord());
        ComputedVerticesMorphTexCoord &mv = comp_verts->_texcoord_morphs.back();
        mv._slider_index = slider_index;
        
        TexCoordMorphList::const_iterator vmi;
        for (vmi = tmorphs.begin(); vmi != tmorphs.end(); ++vmi) {
          mv._morphs.push_back(ComputedVerticesMorphValue2((*vmi).first,
                                                           (*vmi).second));
        }
      }
    }
  }

  comp_verts->make_orig(character);
  return comp_verts;
}


////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVerticesMaker::
write(ostream &out) const {
  out << "ComputedVerticesMaker, "
      << _transforms.size() << " transform spaces, "
      << _coords.size() << " vertices, "
      << _norms.size() << " normals, "
      << _colors.size() << " colors, "
      << _texcoords.size() << " named uv sets.\n";

  TransformSpaces::const_iterator tsi;
  for (tsi = _transforms.begin(); tsi != _transforms.end(); ++tsi) {
    const JointWeights &jw = (*tsi).first;
    const VertexCollection &vc = (*tsi).second;
    out << "  " << jw << " has "
        << vc._vindex.size() << " vertices and "
        << vc._nindex.size() << " normals\n";
  }

  Morphs::const_iterator mi;
  for (mi = _morphs.begin(); mi != _morphs.end(); ++mi) {
    const string &name = (*mi).first;
    const MorphList &mlist = (*mi).second;
    out << name << " morphs "
        << mlist._vmorphs.size() << " vertices, "
        << mlist._nmorphs.size() << " normals, "
        << mlist._tmorphs.size() << " uvs, and "
        << mlist._cmorphs.size() << " colors.\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::JointWeights::Ordering operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool ComputedVerticesMaker::JointWeights::
operator < (const JointWeights &other) const {
  const_iterator i = begin();
  const_iterator j = other.begin();

  while (i != end() && j != other.end()) {
    if ((*i).first != (*j).first) {
      return (*i).first < (*j).first;
    }
    if ((*i).second != (*j).second) {
      return (*i).second < (*j).second;
    }
    ++i;
    ++j;
  }

  if (i == end() && j != other.end()) {
    // The first i.size() items are equivalent, but list j is longer.
    return true;
  }

  if (i != end() && j == other.end()) {
    // The first j.size() items are equivalent, but list i is longer.
    return false;
  }

  // The lists are equivalent.
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::JointWeights::normalize_weights
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVerticesMaker::JointWeights::
normalize_weights() {
  if (!empty()) {
    double net_weight = 0.0;

    iterator i;
    for (i = begin(); i != end(); ++i) {
      double weight = (*i).second;
      nassertv(weight > 0.0);
      net_weight += weight;
    }
    nassertv(net_weight != 0.0);

    for (i = begin(); i != end(); ++i) {
      (*i).second /= net_weight;
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ComputedVerticesMaker::JointWeights::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ComputedVerticesMaker::JointWeights::
output(ostream &out) const {
  out << "jw(";
  if (!empty()) {
    const_iterator i = begin();
    out << (*i).first->get_name() << ":" << (*i).second;
    for (++i; i != end(); ++i) {
      out << " " << (*i).first->get_name() << ":" << (*i).second;
    }
  }
  out << ")";
}
