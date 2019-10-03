/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transferBufferContext.h
 * @author rdb
 * @date 2019-10-03
 */

#ifndef TRANSFERBUFFERCONTEXT_H
#define TRANSFERBUFFERCONTEXT_H

#include "pandabase.h"
#include "savedContext.h"

/**
 * This is a staging buffer used for downloading textures from the GPU.
 * It does not inherit from BufferContext because this type of buffer is
 * ephemeral; it does not make much sense to put it on an LRU chain.
 */
class EXPCL_PANDA_GOBJ TransferBufferContext : public SavedContext {
public:
  virtual bool is_transfer_done() const=0;
  virtual void finish_transfer()=0;

protected:
  INLINE void notify_done();

private:
  PT(AsyncFuture) _future;

  friend class PreparedGraphicsObjects;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedContext::init_type();
    register_type(_type_handle, "TransferBufferContext",
                  SavedContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "transferBufferContext.I"

#endif
