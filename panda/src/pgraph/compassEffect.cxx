// Filename: compassEffect.cxx
// Created by:  drose (16Jul02)
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

#include "compassEffect.h"
#include "cullTraverserData.h"
#include "config_pgraph.h"
#include "nodePath.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle CompassEffect::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::make
//       Access: Published, Static
//  Description: Constructs a new CompassEffect object.  If the
//               reference is an empty NodePath, it means the
//               CompassEffect is relative to the root of the scene
//               graph; otherwise, it's relative to the indicated
//               node.  The properties bitmask specifies the set of
//               properties that the compass node inherits from the
//               reference instead of from its parent.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) CompassEffect::
make(const NodePath &reference, int properties) {
  CompassEffect *effect = new CompassEffect;
  effect->_reference = reference;
  effect->_properties = (properties & P_all);
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of RenderEffect by calling the
//               xform() method, false otherwise.
////////////////////////////////////////////////////////////////////
bool CompassEffect::
safe_to_transform() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CompassEffect::
output(ostream &out) const {
  out << get_type() << ":";
  if (_properties == 0) {
    out << " none";
  }
  if ((_properties & P_pos) == P_pos) {
    out << " xyz";
  } else {
    if ((_properties & P_x) != 0) {
      out << " x";
    }
    if ((_properties & P_y) != 0) {
      out << " y";
    }
    if ((_properties & P_z) != 0) {
      out << " z";
    }
  }
  if ((_properties & P_rot) != 0) {
    out << " rot";
  }
  if ((_properties & P_scale) == P_scale) {
    out << " scale";
  } else {
    if ((_properties & P_sx) != 0) {
      out << " sx";
    }
    if ((_properties & P_sy) != 0) {
      out << " sy";
    }
    if ((_properties & P_sz) != 0) {
      out << " sz";
    }
  }
  if (!_reference.is_empty()) {
    out << " reference " << _reference;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this effect during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool CompassEffect::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               At the time this function is called, the current
//               node's transform and state have not yet been applied
//               to the net_transform and net_state.  This callback
//               may modify the node_transform and node_state to apply
//               an effective change to the render state at this
//               level.
////////////////////////////////////////////////////////////////////
void CompassEffect::
cull_callback(CullTraverser *, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &) const {
  if (_properties == 0) {
    // Nothing to do.
    return;
  }

  CPT(TransformState) true_net_transform = data._net_transform;
  CPT(TransformState) want_transform = net_transform(data._net_transform);

  // Now compute the transform that will convert true_net_transform to
  // want_transform.  This is inv(true_net_transform) * want_transform.
  CPT(TransformState) compass_transform =
    true_net_transform->invert_compose(want_transform);

  // And modify our local node's apparent transform so that
  // true_net_transform->compose(new_node_transform) produces the same
  // result we would have gotten had we actually computed
  // want_transform->compose(orig_node_transform).
  node_transform = node_transform->compose(compass_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::has_net_transform
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if net_transform() has been defined, and
//               therefore the RenderEffect has some effect on the
//               node's apparent net transform.
////////////////////////////////////////////////////////////////////
bool CompassEffect::
has_net_transform() const {
  return (_properties != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::net_transform
//       Access: Public, Virtual
//  Description: Given the node's parent's net transform, compute its
//               parent's new net transform after application of the
//               RenderEffect.  Presumably this interposes some
//               special transform derived from the RenderEffect.
//               This may only be called if has_net_transform(),
//               above, has been defined to return true.
////////////////////////////////////////////////////////////////////
CPT(TransformState) CompassEffect::
net_transform(const TransformState *orig_net_transform) const {
  if (_properties == 0) {
    // Nothing to do.
    return orig_net_transform;
  }

  // The reference transform: where we are acting as if we inherit
  // from.  Either the root node (identity) or the specified reference
  // node.
  CPT(TransformState) ref_transform;
  if (_reference.is_empty()) {
    ref_transform = TransformState::make_identity();
  } else {
    ref_transform = _reference.get_net_transform();
  }

  // Now compute the transform we actually want to achieve.  This is
  // all of the components from the net transform we want to inherit
  // normally from our parent, with all of the components from the ref
  // transform we want to inherit from our reference.
  CPT(TransformState) want_transform;
  if (_properties == P_all) {
    // If we want to steal the whole transform, that's easy.
    want_transform = ref_transform;

  } else {
    // How much of the pos do we want to steal?  We can always
    // determine a transform's pos, even if it's nondecomposable.
    LVecBase3f want_pos = orig_net_transform->get_pos();
    const LVecBase3f &ref_pos = ref_transform->get_pos();
    if ((_properties & P_x) != 0) {
      want_pos[0] = ref_pos[0];
    }
    if ((_properties & P_y) != 0) {
      want_pos[1] = ref_pos[1];
    }
    if ((_properties & P_z) != 0) {
      want_pos[2] = ref_pos[2];
    }

    if ((_properties & ~P_pos) == 0) {
      // If we only want to steal the pos, that's pretty easy.
      want_transform = orig_net_transform->set_pos(want_pos);
  
    } else if ((_properties & (P_rot | P_scale)) == (P_rot | P_scale)) {
      // If we want to steal everything *but* the pos, also easy.
      want_transform = ref_transform->set_pos(want_pos);
      
    } else {
      // For any other combination, we have to be able to decompose both
      // transforms.
      if (!orig_net_transform->has_components() || 
          !ref_transform->has_components()) {
        // If we can't decompose, just do the best we can: steal
        // everything but the pos.
        want_transform = ref_transform->set_pos(want_pos);

      } else {
        // If we can decompose, then take only the components we want.
        LQuaternionf want_quat = orig_net_transform->get_quat();
        if ((_properties & P_rot) != 0) {
          want_quat = ref_transform->get_quat();
        }

        LVecBase3f want_scale = orig_net_transform->get_scale();
        const LVecBase3f &ref_scale = ref_transform->get_scale();
        if ((_properties & P_sx) != 0) {
          want_scale[0] = ref_scale[0];
        }
        if ((_properties & P_sy) != 0) {
          want_scale[1] = ref_scale[1];
        }
        if ((_properties & P_sz) != 0) {
          want_scale[2] = ref_scale[2];
        }

        want_transform =
          TransformState::make_pos_quat_scale(want_pos, want_quat, want_scale);
      }
    }
  }

  return want_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived CompassEffect
//               types to return a unique number indicating whether
//               this CompassEffect is equivalent to the other one.
//
//               This should return 0 if the two CompassEffect objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two CompassEffect
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int CompassEffect::
compare_to_impl(const RenderEffect *other) const {
  const CompassEffect *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_properties != ta->_properties) {
    return _properties - ta->_properties;
  }
  int compare = _reference.compare_to(ta->_reference);
  if (compare != 0) {
    return compare;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               CompassEffect.
////////////////////////////////////////////////////////////////////
void CompassEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CompassEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderEffect::write_datagram(manager, dg);
  dg.add_uint16(_properties);
  // *** We don't write out the _reference NodePath right now.  Maybe
  // we should.
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type CompassEffect is encountered
//               in the Bam file.  It should create the CompassEffect
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *CompassEffect::
make_from_bam(const FactoryParams &params) {
  CompassEffect *effect = new CompassEffect;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

////////////////////////////////////////////////////////////////////
//     Function: CompassEffect::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new CompassEffect.
////////////////////////////////////////////////////////////////////
void CompassEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderEffect::fillin(scan, manager);
  _properties = scan.get_uint16();
}
