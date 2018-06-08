/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file factory.h
 * @author drose
 * @date 2000-05-08
 */

#ifndef FACTORY_H
#define FACTORY_H

#include "pandabase.h"

#include "factoryBase.h"

/**
 * A Factory can be used to create an instance of a particular subclass of
 * some general base class.  Each subclass registers itself with the Factory,
 * supplying a function that will construct an instance of that subclass; the
 * Factory can later choose a suitable subclass and return a newly-constructed
 * pointer to an object of that type on the user's demand.  This is used, for
 * instance, to manage the set of GraphicsPipes available to the user.
 *
 * This is a thin template wrapper around FactoryBase.  All it does is ensure
 * the types are correctly cast.  All of its methods are inline, and it has no
 * data members, so it is not necessary to export the class from the DLL.
 */
template<class Type>
class Factory : public FactoryBase {
public:
  typedef Type *CreateFunc(const FactoryParams &params);

  INLINE Type *make_instance(TypeHandle handle,
                             const FactoryParams &params = FactoryParams());

  INLINE Type *make_instance(const std::string &type_name,
                             const FactoryParams &params = FactoryParams());

  INLINE Type *
  make_instance_more_general(TypeHandle handle,
                             const FactoryParams &params = FactoryParams());

  INLINE Type *
  make_instance_more_general(const std::string &type_name,
                             const FactoryParams &params = FactoryParams());

  INLINE void register_factory(TypeHandle handle, CreateFunc *func,
                               void *user_data = nullptr);
};

#include "factory.I"

#endif
