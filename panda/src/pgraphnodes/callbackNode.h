/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file callbackNode.h
 * @author drose
 * @date 2009-03-13
 */

#ifndef CALLBACKNODE_H
#define CALLBACKNODE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "callbackObject.h"
#include "pointerTo.h"

/**
 * A special node that can issue arbitrary callbacks to user code, either
 * during the cull or draw traversals.
 */
class EXPCL_PANDA_PGRAPHNODES CallbackNode : public PandaNode {
PUBLISHED:
  explicit CallbackNode(const std::string &name);

  INLINE void set_cull_callback(CallbackObject *object);
  INLINE void clear_cull_callback();
  INLINE CallbackObject *get_cull_callback() const;
  MAKE_PROPERTY(cull_callback, get_cull_callback, set_cull_callback);

  INLINE void set_draw_callback(CallbackObject *object);
  INLINE void clear_draw_callback();
  INLINE CallbackObject *get_draw_callback() const;
  MAKE_PROPERTY(draw_callback, get_draw_callback, set_draw_callback);

public:
  CallbackNode(const CallbackNode &copy);

  virtual PandaNode *make_copy() const;
  virtual bool safe_to_combine() const;

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool is_renderable() const;
  virtual void add_for_draw(CullTraverser *trav, CullTraverserData &data);

  virtual void output(std::ostream &out) const;

private:
  class EXPCL_PANDA_PGRAPHNODES CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return CallbackNode::get_class_type();
    }

    PT(CallbackObject) _cull_callback;
    PT(CallbackObject) _draw_callback;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "CallbackNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "callbackNode.I"

#endif
