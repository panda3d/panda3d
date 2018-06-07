/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelSaveRequest.h
 * @author drose
 * @date 2012-12-19
 */

#ifndef MODELSAVEREQUEST_H
#define MODELSAVEREQUEST_H

#include "pandabase.h"

#include "asyncTask.h"
#include "filename.h"
#include "loaderOptions.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "loader.h"

/**
 * A class object that manages a single asynchronous model save request.
 * Create a new ModelSaveRequest, and add it to the loader via save_async(),
 * to begin an asynchronous save.
 */
class EXPCL_PANDA_PGRAPH ModelSaveRequest : public AsyncTask {
public:
  ALLOC_DELETED_CHAIN(ModelSaveRequest);

PUBLISHED:
  explicit ModelSaveRequest(const std::string &name,
                            const Filename &filename,
                            const LoaderOptions &options,
                            PandaNode *node, Loader *loader);

  INLINE const Filename &get_filename() const;
  INLINE const LoaderOptions &get_options() const;
  INLINE PandaNode *get_node() const;
  INLINE Loader *get_loader() const;

  INLINE bool is_ready() const;
  INLINE bool get_success() const;

  MAKE_PROPERTY(filename, get_filename);
  MAKE_PROPERTY(options, get_options);
  MAKE_PROPERTY(node, get_node);
  MAKE_PROPERTY(loader, get_loader);

protected:
  virtual DoneStatus do_task();

private:
  Filename _filename;
  LoaderOptions _options;
  PT(PandaNode) _node;
  PT(Loader) _loader;
  bool _success;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTask::init_type();
    register_type(_type_handle, "ModelSaveRequest",
                  AsyncTask::get_class_type());
    }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "modelSaveRequest.I"

#endif
