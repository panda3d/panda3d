/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file factoryBase.h
 * @author cary
 * @date 1999-10-06
 */

#ifndef FACTORYBASE_H
#define FACTORYBASE_H

#include "pandabase.h"

#include "typedObject.h"
#include "typedReferenceCount.h"
#include "factoryParams.h"

#include "pvector.h"
#include "pmap.h"

/**
 * A Factory can be used to create an instance of a particular subclass of
 * some general base class.  Each subclass registers itself with the Factory,
 * supplying a function that will construct an instance of that subclass; the
 * Factory can later choose a suitable subclass and return a newly-constructed
 * pointer to an object of that type on the user's demand.  This is used, for
 * instance, to manage the set of GraphicsPipes available to the user.
 *
 * FactoryBase is the main definition of the thin template class Factory.
 */
class EXPCL_PANDA_PUTIL FactoryBase {
public:
  typedef TypedObject *BaseCreateFunc(const FactoryParams &params);

  // public interface
public:
  FactoryBase() = default;
  FactoryBase(const FactoryBase &copy) = delete;
  ~FactoryBase() = default;

  FactoryBase &operator = (const FactoryBase &copy) = delete;

  TypedObject *make_instance(TypeHandle handle,
                             const FactoryParams &params);

  INLINE TypedObject *make_instance(const std::string &type_name,
                                    const FactoryParams &params);

  TypedObject *make_instance_more_general(TypeHandle handle,
                                          const FactoryParams &params);

  INLINE TypedObject *make_instance_more_general(const std::string &type_name,
                                                 const FactoryParams &params);

  TypeHandle find_registered_type(TypeHandle handle);

  void register_factory(TypeHandle handle, BaseCreateFunc *func, void *user_data = nullptr);

  size_t get_num_types() const;
  TypeHandle get_type(size_t n) const;

  void clear_preferred();
  void add_preferred(TypeHandle handle);
  size_t get_num_preferred() const;
  TypeHandle get_preferred(size_t n) const;

  void write_types(std::ostream &out, int indent_level = 0) const;

private:
  // internal utility functions
  TypedObject *make_instance_exact(TypeHandle handle, FactoryParams params);
  TypedObject *make_instance_more_specific(TypeHandle handle,
                                           FactoryParams params);

private:
  // internal mechanics and bookkeeping
  struct Creator {
    BaseCreateFunc *_func;
    void *_user_data;
  };

  typedef pmap<TypeHandle, Creator> Creators;
  Creators _creators;

  typedef pvector<TypeHandle> Preferred;
  Preferred _preferred;
};

#include "factoryBase.I"

#endif /* FACTORY_H */
