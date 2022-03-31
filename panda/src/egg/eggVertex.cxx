/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggVertex.cxx
 * @author drose
 * @date 1999-01-16
 */

#include "eggVertex.h"
#include "eggVertexPool.h"
#include "eggParameters.h"
#include "eggGroup.h"
#include "eggMiscFuncs.h"
#include "eggPrimitive.h"

#include "indent.h"
#include "luse.h"
#include "lmatrix.h"
#include "pandabase.h"

#include <math.h>
#include <algorithm>

using std::ostream;
using std::string;

TypeHandle EggVertex::_type_handle;


/**
 *
 */
EggVertex::
EggVertex() {
  _pool = nullptr;
  _forward_reference = false;
  _index = -1;
  _external_index = -1;
  _external_index2 = -1;
  set_pos(LPoint3d(0.0, 0.0, 0.0));
  test_pref_integrity();
  test_gref_integrity();
}

/**
 * Copies all properties of the vertex except its vertex pool, index number,
 * and group membership.
 */
EggVertex::
EggVertex(const EggVertex &copy)
  : EggObject(copy), EggAttributes(copy),
    _dxyzs(copy._dxyzs),
    _external_index(copy._external_index),
    _external_index2(copy._external_index2),
    _pos(copy._pos),
    _num_dimensions(copy._num_dimensions),
    _uv_map(copy._uv_map),
    _aux_map(copy._aux_map)
{
  _pool = nullptr;
  _forward_reference = false;
  _index = -1;
  test_pref_integrity();
  test_gref_integrity();
}


/**
 * Copies all properties of the vertex except its vertex pool, index number,
 * and group membership.
 */
EggVertex &EggVertex::
operator = (const EggVertex &copy) {
  EggObject::operator = (copy);
  EggAttributes::operator = (copy);
  _dxyzs = copy._dxyzs;
  _external_index = copy._external_index;
  _external_index2 = copy._external_index2;
  _pos = copy._pos;
  _num_dimensions = copy._num_dimensions;
  _uv_map = copy._uv_map;
  _aux_map = copy._aux_map;

  test_pref_integrity();
  test_gref_integrity();

  return *this;
}

/**
 *
 */
EggVertex::
~EggVertex() {
  // We should never destruct a vertex while it still thinks it belongs to a
  // VertexPool.  If we do, we've probably lost a reference count somewhere.
  nassertv(_pool == nullptr);

  // Also, a vertex shouldn't be destructed while it's being referenced by a
  // group or a primitive, for the same reason.
  nassertv(_gref.empty());
  nassertv(_pref.empty());
}

/**
 * Returns true if the vertex has the named UV coordinate pair, and the named
 * UV coordinate pair is 2-d, false otherwise.
 */
bool EggVertex::
has_uv(const string &name) const {
  UVMap::const_iterator ui = _uv_map.find(EggVertexUV::filter_name(name));
  if (ui != _uv_map.end()) {
    EggVertexUV *uv_obj = (*ui).second;
    return !uv_obj->has_w();
  }
  return false;
}

/**
 * Returns true if the vertex has the named UV coordinate triple, and the
 * named UV coordinate triple is 3-d, false otherwise.
 */
bool EggVertex::
has_uvw(const string &name) const {
  UVMap::const_iterator ui = _uv_map.find(EggVertexUV::filter_name(name));
  if (ui != _uv_map.end()) {
    EggVertexUV *uv_obj = (*ui).second;
    return uv_obj->has_w();
  }
  return false;
}

/**
 * Returns true if the vertex has the named auxiliary data quadruple.
 */
bool EggVertex::
has_aux(const string &name) const {
  AuxMap::const_iterator xi = _aux_map.find(name);
  return (xi != _aux_map.end());
}

/**
 * Returns the named UV coordinate pair on the vertex.  It is an error to call
 * this if has_uv(name) returned false.
 */
LTexCoordd EggVertex::
get_uv(const string &name) const {
  UVMap::const_iterator ui = _uv_map.find(EggVertexUV::filter_name(name));
  nassertr(ui != _uv_map.end(), LTexCoordd::zero());
  return (*ui).second->get_uv();
}

/**
 * Returns the named UV coordinate triple on the vertex.  It is an error to
 * call this if has_uvw(name) returned false.
 */
const LTexCoord3d &EggVertex::
get_uvw(const string &name) const {
  UVMap::const_iterator ui = _uv_map.find(EggVertexUV::filter_name(name));
  nassertr(ui != _uv_map.end(), LTexCoord3d::zero());
  return (*ui).second->get_uvw();
}

/**
 * Returns the named auxiliary data quadruple on the vertex.  It is an error
 * to call this if has_aux(name) returned false.
 */
const LVecBase4d &EggVertex::
get_aux(const string &name) const {
  AuxMap::const_iterator xi = _aux_map.find(name);
  nassertr(xi != _aux_map.end(), LVecBase4d::zero());
  return (*xi).second->get_aux();
}

/**
 * Sets the indicated UV coordinate pair on the vertex.  This replaces any UV
 * coordinate pair with the same name already on the vertex, but preserves UV
 * morphs.
 */
void EggVertex::
set_uv(const string &name, const LTexCoordd &uv) {
  string fname = EggVertexUV::filter_name(name);
  PT(EggVertexUV) &uv_obj = _uv_map[fname];

  if (uv_obj.is_null()) {
    uv_obj = new EggVertexUV(fname, uv);
  } else {
    uv_obj = new EggVertexUV(*uv_obj);
    uv_obj->set_uv(uv);
  }

  nassertv(get_uv(fname) == uv);
}

/**
 * Sets the indicated UV coordinate triple on the vertex.  This replaces any
 * UV coordinate pair or triple with the same name already on the vertex, but
 * preserves UV morphs.
 */
void EggVertex::
set_uvw(const string &name, const LTexCoord3d &uvw) {
  string fname = EggVertexUV::filter_name(name);
  PT(EggVertexUV) &uv_obj = _uv_map[fname];

  if (uv_obj.is_null()) {
    uv_obj = new EggVertexUV(fname, uvw);
  } else {
    uv_obj = new EggVertexUV(*uv_obj);
    uv_obj->set_uvw(uvw);
  }

  nassertv(get_uvw(fname) == uvw);
}

/**
 * Sets the indicated auxiliary data quadruple on the vertex.  This replaces
 * any auxiliary data with the same name already on the vertex.
 */
void EggVertex::
set_aux(const string &name, const LVecBase4d &aux) {
  PT(EggVertexAux) &aux_obj = _aux_map[name];

  if (aux_obj.is_null()) {
    aux_obj = new EggVertexAux(name, aux);
  } else {
    aux_obj = new EggVertexAux(*aux_obj);
    aux_obj->set_aux(aux);
  }

  nassertv(get_aux(name) == aux);
}

/**
 * Returns the named EggVertexUV object, which defines both the UV coordinate
 * pair for this name and the UV morphs.  This object might be shared between
 * multiple vertices.  You should not attempt to modify this object; instead,
 * call modify_uv_object to return a modifiable pointer.
 */
const EggVertexUV *EggVertex::
get_uv_obj(const string &name) const {
  UVMap::const_iterator ui = _uv_map.find(EggVertexUV::filter_name(name));
  if (ui != _uv_map.end()) {
    return (*ui).second;
  }
  return nullptr;
}

/**
 * Returns the named EggVertexAux object, which defines the auxiliary data for
 * this name.  This object might be shared between multiple vertices.  You
 * should not attempt to modify this object; instead, call modify_aux_object
 * to return a modifiable pointer.
 */
const EggVertexAux *EggVertex::
get_aux_obj(const string &name) const {
  AuxMap::const_iterator xi = _aux_map.find(name);
  if (xi != _aux_map.end()) {
    return (*xi).second;
  }
  return nullptr;
}

/**
 * Returns a modifiable pointer to the named EggVertexUV object, which defines
 * both the UV coordinate pair for this name and the UV morphs.  Returns NULL
 * if there is no such named UV object.
 */
EggVertexUV *EggVertex::
modify_uv_obj(const string &name) {
  UVMap::iterator ui = _uv_map.find(EggVertexUV::filter_name(name));
  if (ui != _uv_map.end()) {
    if ((*ui).second->get_ref_count() != 1) {
      // Copy on write.
      (*ui).second = new EggVertexUV(*(*ui).second);
    }
    return (*ui).second;
  }

  return nullptr;
}

/**
 * Returns a modifiable pointer to the named EggVertexAux object, which
 * defines the auxiliary data for this name.  Returns NULL if there is no such
 * named UV object.
 */
EggVertexAux *EggVertex::
modify_aux_obj(const string &name) {
  AuxMap::iterator xi = _aux_map.find(name);
  if (xi != _aux_map.end()) {
    if ((*xi).second->get_ref_count() != 1) {
      // Copy on write.
      (*xi).second = new EggVertexAux(*(*xi).second);
    }
    return (*xi).second;
  }

  return nullptr;
}

/**
 * Sets the indicated EggVertexUV on the vertex.  This replaces any UV
 * coordinate pair with the same name already on the vertex, including UV
 * morphs.
 */
void EggVertex::
set_uv_obj(EggVertexUV *uv) {
  _uv_map[uv->get_name()] = uv;
}

/**
 * Sets the indicated EggVertexAux on the vertex.  This replaces any auxiliary
 * data with the same name already on the vertex.
 */
void EggVertex::
set_aux_obj(EggVertexAux *aux) {
  _aux_map[aux->get_name()] = aux;
}

/**
 * Removes the named UV coordinate pair from the vertex, along with any UV
 * morphs.
 */
void EggVertex::
clear_uv(const string &name) {
  _uv_map.erase(EggVertexUV::filter_name(name));
}

/**
 * Removes the named auxiliary data from the vertex.
 */
void EggVertex::
clear_aux(const string &name) {
  _aux_map.erase(name);
}

/**
 * Creates a new vertex that lies in between the two given vertices.  The
 * attributes for the UV sets they have in common are averaged.
 *
 * Both vertices need to be either in no pool, or in the same pool.  In the
 * latter case, the new vertex will be placed in that pool.
 */
PT(EggVertex) EggVertex::
make_average(const EggVertex *first, const EggVertex *second) {
  PT(EggVertexPool) pool = first->get_pool();
  nassertr(pool == second->get_pool(), nullptr);

  // If both vertices are in a pool, the new vertex will be part of the pool
  // as well.
  PT(EggVertex) middle;
  if (pool == nullptr) {
    middle = new EggVertex;
  } else {
    middle = pool->make_new_vertex();
  }

  middle->set_pos4((first->get_pos4() + second->get_pos4()) / 2);

  if (first->has_normal() && second->has_normal()) {
    LNormald normal = (first->get_normal() + second->get_normal()) / 2;
    normal.normalize();
    middle->set_normal(normal);
  }
  if (first->has_color() && second->has_color()) {
    middle->set_color((first->get_color() + second->get_color()) / 2);
  }

  // Average out the EggVertexUV objects, but only for the UV sets that they
  // have in common.
  const_uv_iterator it;
  for (it = first->uv_begin(); it != first->uv_end(); ++it) {
    const EggVertexUV *first_uv = it->second;
    const EggVertexUV *second_uv = second->get_uv_obj(it->first);

    if (first_uv != nullptr && second_uv != nullptr) {
      middle->set_uv_obj(EggVertexUV::make_average(first_uv, second_uv));
    }
  }

  // Same for EggVertexAux.
  const_aux_iterator ai;
  for (ai = first->aux_begin(); ai != first->aux_end(); ++ai) {
    const EggVertexAux *first_aux = ai->second;
    const EggVertexAux *second_aux = second->get_aux_obj(ai->first);

    if (first_aux != nullptr && second_aux != nullptr) {
      middle->set_aux_obj(EggVertexAux::make_average(first_aux, second_aux));
    }
  }

  // Now process the morph targets.
  EggMorphVertexList::const_iterator vi, vi2;
  for (vi = first->_dxyzs.begin(); vi != first->_dxyzs.end(); ++vi) {
    for (vi2 = second->_dxyzs.begin(); vi2 != second->_dxyzs.end(); ++vi2) {
      if (vi->get_name() == vi2->get_name()) {
        middle->_dxyzs.insert(EggMorphVertex(vi->get_name(),
                              (vi->get_offset() + vi2->get_offset()) / 2));
        break;
      }
    }
  }

  EggMorphNormalList::const_iterator ni, ni2;
  for (ni = first->_dxyzs.begin(); ni != first->_dxyzs.end(); ++ni) {
    for (ni2 = second->_dxyzs.begin(); ni2 != second->_dxyzs.end(); ++ni2) {
      if (ni->get_name() == ni2->get_name()) {
        middle->_dnormals.insert(EggMorphNormal(ni->get_name(),
                                 (ni->get_offset() + ni2->get_offset()) / 2));
        break;
      }
    }
  }

  EggMorphColorList::const_iterator ci, ci2;
  for (ci = first->_drgbas.begin(); ci != first->_drgbas.end(); ++ci) {
    for (ci2 = second->_drgbas.begin(); ci2 != second->_drgbas.end(); ++ci2) {
      if (ci->get_name() == ci2->get_name()) {
        middle->_drgbas.insert(EggMorphColor(ci->get_name(),
                               (ci->get_offset() + ci2->get_offset()) / 2));
        break;
      }
    }
  }

  // Now merge the vertex memberships.
  GroupRef::iterator gi;
  for (gi = first->_gref.begin(); gi != first->_gref.end(); ++gi) {
    EggGroup *group = *gi;
    if (second->_gref.count(group)) {
      group->set_vertex_membership(middle,
        (group->get_vertex_membership(first) +
         group->get_vertex_membership(second)) / 2.);
    } else {
      // Hmm, unfortunate, only one of the vertices is member of this group,
      // so we can't make an average.  We'll have to assign the only group
      // membership we have.
      group->set_vertex_membership(middle, group->get_vertex_membership(first));
    }
  }
  // Also assign memberships to the grefs in the second vertex that aren't
  // part of the first vertex.
  for (gi = second->_gref.begin(); gi != second->_gref.end(); ++gi) {
    EggGroup *group = *gi;
    if (second->_gref.count(group) == 0) {
      group->set_vertex_membership(middle, group->get_vertex_membership(second));
    }
  }

  return middle;
}

/**
 * A temporary class used in EggVertex::write(), below, to hold the groups
 * that reference each vertex prior to outputting them as a formatted list.
 */
class GroupRefEntry {
public:
  GroupRefEntry(EggGroup *group, double membership)
    : _group(group), _membership(membership) { }

  bool operator < (const GroupRefEntry &other) const {
    return _group->get_name() < other._group->get_name();
  }
  void output(ostream &out) const {
    out << _group->get_name() << ":" << _membership;
  }

  EggGroup *_group;
  double _membership;
};

INLINE ostream &operator << (ostream &out, const GroupRefEntry &gre) {
  gre.output(out);
  return out;
}

/**
 * Writes the vertex to the indicated output stream in Egg format.
 */
void EggVertex::
write(ostream &out, int indent_level) const {
  test_pref_integrity();
  test_gref_integrity();

  indent(out, indent_level)
    << "<Vertex> " << _index << " {\n";

  // Now output the position.  This might have any number of dimensions up to
  // 4.
  indent(out, indent_level+1);
  for (int i = 0; i < _num_dimensions; i++) {
    out << " " << _pos[i];
  }
  out << "\n";

  UVMap::const_iterator ui;
  for (ui = _uv_map.begin(); ui != _uv_map.end(); ++ui) {
    (*ui).second->write(out, indent_level + 2);
  }

  AuxMap::const_iterator xi;
  for (xi = _aux_map.begin(); xi != _aux_map.end(); ++xi) {
    (*xi).second->write(out, indent_level + 2);
  }

  EggAttributes::write(out, indent_level+2);

  _dxyzs.write(out, indent_level + 2, "<Dxyz>", 3);

  // If the vertex is referenced by one or more groups, write that as a
  // helpful comment.
  if (!_gref.empty()) {
    // We need to build a list of group entries.
    pset<GroupRefEntry> gre;

    GroupRef::const_iterator gi;
    for (gi = _gref.begin(); gi != _gref.end(); ++gi) {
      gre.insert(GroupRefEntry(*gi, (*gi)->get_vertex_membership(this)));
    }

    // Now output the list.
    write_long_list(out, indent_level + 2, gre.begin(), gre.end(), "// ",
                    "", 72);
  }

  indent(out, indent_level)
    << "}\n";
}


/**
 * An ordering operator to compare two vertices for sorting order.  This
 * imposes an arbitrary ordering useful to identify unique vertices.
 *
 * Group membership is not considered in this comparison.  This is somewhat
 * problematic, but cannot easily be helped, because considering group
 * membership would make it difficult to add and remove groups from vertices.
 * It also makes it impossible to meaningfully compare with a concrete
 * EggVertex object (which cannot have group memberships).
 *
 * However, this is not altogether bad, because two vertices that are
 * identical in all other properties should generally also be identical in
 * group memberships, else the vertices will tend to fly apart when the joints
 * animate.
 */
int EggVertex::
compare_to(const EggVertex &other) const {
  if (_external_index != other._external_index) {
    return (int)_external_index - (int)other._external_index;
  }
  if (_external_index2 != other._external_index2) {
    return (int)_external_index2 - (int)other._external_index2;
  }
  if (_num_dimensions != other._num_dimensions) {
    return (int)_num_dimensions - (int)other._num_dimensions;
  }

  int compare =
    _pos.compare_to(other._pos, egg_parameters->_pos_threshold);
  if (compare != 0) {
    return compare;
  }
  compare = _dxyzs.compare_to(other._dxyzs, egg_parameters->_pos_threshold);
  if (compare != 0) {
    return compare;
  }

  // Merge-compare the uv maps.
  UVMap::const_iterator ai, bi;
  ai = _uv_map.begin();
  bi = other._uv_map.begin();
  while (ai != _uv_map.end() && bi != other._uv_map.end()) {
    if ((*ai).first < (*bi).first) {
      return -1;

    } else if ((*bi).first < (*ai).first) {
      return 1;

    } else {
      int compare = (*ai).second->compare_to(*(*bi).second);
      if (compare != 0) {
        return compare;
      }
    }
    ++ai;
    ++bi;
  }
  if (bi != other._uv_map.end()) {
    return -1;
  }
  if (ai != _uv_map.end()) {
    return 1;
  }

  // Merge-compare the aux maps.
  AuxMap::const_iterator ci, di;
  ci = _aux_map.begin();
  di = other._aux_map.begin();
  while (ci != _aux_map.end() && di != other._aux_map.end()) {
    if ((*ci).first < (*di).first) {
      return -1;

    } else if ((*di).first < (*ci).first) {
      return 1;

    } else {
      int compare = (*ci).second->compare_to(*(*di).second);
      if (compare != 0) {
        return compare;
      }
    }
    ++ci;
    ++di;
  }
  if (di != other._aux_map.end()) {
    return -1;
  }
  if (ci != _aux_map.end()) {
    return 1;
  }

  return EggAttributes::compare_to(other);
}

/**
 * Returns the number of primitives that own this vertex whose vertices are
 * interpreted to be in a local coordinate system.
 */
int EggVertex::
get_num_local_coord() const {
  test_pref_integrity();

  PrimitiveRef::const_iterator pri;

  int count = 0;
  for (pri = pref_begin(); pri != pref_end(); ++pri) {
    EggPrimitive *prim = *pri;
    count += (prim->is_local_coord() ? 1 : 0);
  }
  return count;
}

/**
 * Returns the number of primitives that own this vertex whose vertices are
 * interpreted in the global coordinate system.
 */
int EggVertex::
get_num_global_coord() const {
  test_pref_integrity();

  PrimitiveRef::const_iterator pri;

  int count = 0;
  for (pri = pref_begin(); pri != pref_end(); ++pri) {
    EggPrimitive *prim = *pri;
    count += (prim->is_local_coord() ? 0 : 1);
  }
  return count;
}


/**
 * Applies the indicated transformation matrix to the vertex.
 */
void EggVertex::
transform(const LMatrix4d &mat) {
  _pos = _pos * mat;

  EggMorphVertexList::iterator mi;
  for (mi = _dxyzs.begin(); mi != _dxyzs.end(); ++mi) {
    // We can safely cast the morph object to a non-const, because we're not
    // changing its name, which is the only thing the set cares about
    // preserving.
    EggMorphVertex &morph = (EggMorphVertex &)(*mi);

    morph.set_offset((*mi).get_offset() * mat);
  }

  UVMap::iterator ui;
  for (ui = _uv_map.begin(); ui != _uv_map.end(); ++ui) {
    (*ui).second->transform(mat);
  }

  EggAttributes::transform(mat);
}


/**
 * Returns an iterator that can, in conjunction with gref_end(), be used to
 * traverse the entire set of groups that reference this vertex.  Each
 * iterator returns a pointer to a group.
 *
 * This interface is not safe to use outside of PANDAEGG.DLL.
 */
EggVertex::GroupRef::const_iterator EggVertex::
gref_begin() const {
  return _gref.begin();
}

/**
 * Returns an iterator that can, in conjunction with gref_begin(), be used to
 * traverse the entire set of groups that reference this vertex.  Each
 * iterator returns a pointer to a group.
 *
 * This interface is not safe to use outside of PANDAEGG.DLL.
 */
EggVertex::GroupRef::const_iterator EggVertex::
gref_end() const {
  return _gref.end();
}

/**
 * Returns the number of elements between gref_begin() and gref_end().
 *
 * This interface is not safe to use outside of PANDAEGG.DLL.
 */
EggVertex::GroupRef::size_type EggVertex::
gref_size() const {
  return _gref.size();
}

/**
 * Returns true if the indicated group references this vertex, false
 * otherwise.
 */
bool EggVertex::
has_gref(const EggGroup *group) const {
  return _gref.count((EggGroup *)group) != 0;
}

/**
 * Copies all the group references from the other vertex onto this one.  This
 * assigns the current vertex to exactly the same groups, with exactly the
 * same memberships, as the given one.
 *
 * Warning: only an EggVertex allocated from the free store may have groups
 * assigned to it.  Do not attempt to call this on a temporary concrete
 * EggVertex object; a core dump will certainly result.
 */
void EggVertex::
copy_grefs_from(const EggVertex &other) {
  if (&other == this) {
    return;
  }
  test_gref_integrity();
  other.test_gref_integrity();

  clear_grefs();
  test_gref_integrity();

  GroupRef::const_iterator gri;

  for (gri = other.gref_begin(); gri != other.gref_end(); ++gri) {
    EggGroup *group = *gri;
    nassertv(group != nullptr);

    group->ref_vertex(this, group->get_vertex_membership(&other));
  }
}

/**
 * Removes all group references from the vertex, so that it is not assigned to
 * any group.
 */
void EggVertex::
clear_grefs() {
  GroupRef gref_copy = _gref;
  GroupRef::const_iterator gri;
  for (gri = gref_copy.begin(); gri != gref_copy.end(); ++gri) {
    EggGroup *group = *gri;
    nassertv(group != nullptr);
    group->unref_vertex(this);
  }

  // Now we should have no more refs.
  nassertv(_gref.empty());
}

/**
 * Returns an iterator that can, in conjunction with pref_end(), be used to
 * traverse the entire set of primitives that reference this vertex.  Each
 * iterator returns a pointer to a primitive.
 *
 * This interface is not safe to use outside of PANDAEGG.DLL.
 */
EggVertex::PrimitiveRef::const_iterator EggVertex::
pref_begin() const {
  return _pref.begin();
}

/**
 * Returns an iterator that can, in conjunction with pref_begin(), be used to
 * traverse the entire set of primitives that reference this vertex.  Each
 * iterator returns a pointer to a primitive.
 *
 * This interface is not safe to use outside of PANDAEGG.DLL.
 */
EggVertex::PrimitiveRef::const_iterator EggVertex::
pref_end() const {
  return _pref.end();
}

/**
 * Returns the number of elements between pref_begin() and pref_end().
 *
 * This interface is not safe to use outside of PANDAEGG.DLL.
 */
EggVertex::GroupRef::size_type EggVertex::
pref_size() const {
  return _pref.size();
}

/**
 * Returns the number of times the vertex appears in the indicated primitive,
 * or 0 if it does not appear.
 */
int EggVertex::
has_pref(const EggPrimitive *prim) const {
  return _pref.count((EggPrimitive *)prim);
}

#ifdef _DEBUG

/**
 * Verifies that the gref list is correct and that all the groups included
 * actually exist and do reference the vertex.
 */
void EggVertex::
test_gref_integrity() const {
  test_ref_count_integrity();

  GroupRef::const_iterator gri;

  for (gri = gref_begin(); gri != gref_end(); ++gri) {
    EggGroup *group = *gri;
    nassertv(group != nullptr);
    group->test_ref_count_integrity();

    double membership = group->get_vertex_membership(this);
    nassertv(membership != 0.0);
  }
}

/**
 * Verifies that the pref list is correct and that all the primitives included
 * actually exist and do reference the vertex.
 */
void EggVertex::
test_pref_integrity() const {
  test_ref_count_integrity();

  PrimitiveRef::const_iterator pri;

  for (pri = pref_begin(); pri != pref_end(); ++pri) {
    EggPrimitive *prim = *pri;
    nassertv(prim != nullptr);
    prim->test_ref_count_integrity();

    EggPrimitive::iterator vi;
    vi = find(prim->begin(), prim->end(), this);
    nassertv(vi != prim->end());
  }
}

#endif  // NDEBUG

/**
 *
 */
void EggVertex::
output(ostream &out) const {
  if (get_pool() == nullptr) {
    out << "(null):" << get_index();
  } else {
    out << get_pool()->get_name() << ":" << get_index();
  }
}
