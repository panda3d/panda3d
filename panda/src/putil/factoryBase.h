// Filename: factoryBase.h
// Created by:  cary (06Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef FACTORYBASE_H
#define FACTORYBASE_H

#include "pandabase.h"

#include "typedObject.h"
#include "typedReferenceCount.h"
#include "factoryParams.h"

#include "pvector.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : FactoryBase
// Description : A Factory can be used to create an instance of a
//               particular subclass of some general base class.  Each
//               subclass registers itself with the Factory, supplying
//               a function that will construct an instance of that
//               subclass; the Factory can later choose a suitable
//               subclass and return a newly-constructed pointer to an
//               object of that type on the user's demand.  This is
//               used, for instance, to manage the set of
//               GraphicsPipes available to the user.
//
//               FactoryBase is the main definition of the thin
//               template class Factory.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL FactoryBase {
public:
  typedef TypedObject *BaseCreateFunc(const FactoryParams &params);

  // public interface
public:
  FactoryBase();
  ~FactoryBase();

  TypedObject *make_instance(TypeHandle handle,
                             const FactoryParams &params);

  INLINE TypedObject *make_instance(const string &type_name,
                                    const FactoryParams &params);

  TypedObject *make_instance_more_general(TypeHandle handle,
                                          const FactoryParams &params);

  INLINE TypedObject *make_instance_more_general(const string &type_name,
                                                 const FactoryParams &params);

  TypeHandle find_registered_type(TypeHandle handle);

  void register_factory(TypeHandle handle, BaseCreateFunc *func);

  int get_num_types() const;
  TypeHandle get_type(int n) const;

  void clear_preferred();
  void add_preferred(TypeHandle handle);
  int get_num_preferred() const;
  TypeHandle get_preferred(int n) const;

  void write_types(ostream &out, int indent_level = 0) const;

private:
  // These are private; we shouldn't be copy-constructing Factories.
  FactoryBase(const FactoryBase &copy);
  void operator = (const FactoryBase &copy);

  // internal utility functions
  TypedObject *make_instance_exact(TypeHandle handle,
                                   const FactoryParams &params);
  TypedObject *make_instance_more_specific(TypeHandle handle,
                                           const FactoryParams &params);

private:
  // internal mechanics and bookkeeping

#if (defined(WIN32_VC) || defined(WIN64_VC)) && !defined(__ICL)    //__ICL is Intel C++
  // Visual C++ seems to have a problem with building a map based on
  // BaseCreateFunc.  We'll have to typecast it on the way out.
  typedef pmap<TypeHandle, void *> Creators;
#else
  typedef pmap<TypeHandle, BaseCreateFunc *> Creators;
#endif

  Creators _creators;

  typedef pvector<TypeHandle> Preferred;
  Preferred _preferred;
};

#include "factoryBase.I"

#endif /* FACTORY_H */
