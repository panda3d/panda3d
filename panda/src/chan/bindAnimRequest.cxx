/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bindAnimRequest.cxx
 * @author drose
 * @date 2008-08-05
 */

#include "bindAnimRequest.h"
#include "animBundleNode.h"
#include "animControl.h"
#include "partBundle.h"

TypeHandle BindAnimRequest::_type_handle;

/**
 *
 */
BindAnimRequest::
BindAnimRequest(const std::string &name,
                const Filename &filename, const LoaderOptions &options,
                Loader *loader,
                AnimControl *control, int hierarchy_match_flags,
                const PartSubset &subset) :
  ModelLoadRequest(name, filename, options, loader),
  _control(control),
  _hierarchy_match_flags(hierarchy_match_flags),
  _subset(subset)
{
}

/**
 * Performs the task: that is, loads and binds the animation.
 */
AsyncTask::DoneStatus BindAnimRequest::
do_task() {
  ModelLoadRequest::do_task();

  PartBundle *part = _control->get_part();

  if (_control->get_ref_count() == 1) {
    // We're holding the only remaining reference to this AnimControl.
    // Therefore, forget the bind attempt; no one cares anyway.
    _control->fail_anim(part);
    return DS_done;
  }

  PT(PandaNode) model = get_model();
  if (model == nullptr) {
    // Couldn't load the file.
    _control->fail_anim(part);
    return DS_done;
  }
  _control->set_anim_model(model);

  AnimBundle *anim = AnimBundleNode::find_anim_bundle(model);
  if (anim == nullptr) {
    // No anim bundle.
    _control->fail_anim(part);
    return DS_done;
  }

  if (!part->do_bind_anim(_control, anim, _hierarchy_match_flags, _subset)) {
    // Couldn't bind.
    _control->fail_anim(part);
    return DS_done;
  }

  return DS_done;
}
