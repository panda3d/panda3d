// Filename: factory.I
// Created by:  drose (08May00)
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

////////////////////////////////////////////////////////////////////
//     Function: Factory::make_instance
//       Access: Public
//  Description: Attempts to create a new instance of some class of
//               the indicated type, or some derivative if necessary.
//               If an instance of the exact type cannot be created,
//               the specified preferred will specify which derived
//               class will be preferred.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE Type *Factory<Type>::
make_instance(TypeHandle handle, const FactoryParams &params) {
  return (Type *)FactoryBase::make_instance(handle, params);
}

////////////////////////////////////////////////////////////////////
//     Function: Factory::make_instance
//       Access: Public
//  Description: Attempts to create a new instance of some class of
//               the indicated type, or some derivative if necessary.
//               If an instance of the exact type cannot be created,
//               the specified preferred will specify which derived
//               class will be preferred.
//
//               This flavor of make_instance() accepts a string name
//               that indicates the desired type.  It must be the name
//               of some already-registered type.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE Type *Factory<Type>::
make_instance(const string &type_name, const FactoryParams &params) {
  return (Type *)FactoryBase::make_instance(type_name, params);
}

////////////////////////////////////////////////////////////////////
//     Function: Factory::make_instance_more_general
//       Access: Public
//  Description: Attempts to create an instance of the type requested,
//               or some base type of the type requested.  Returns the
//               new instance created, or NULL if the instance could
//               not be created.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE Type *Factory<Type>::
make_instance_more_general(TypeHandle handle, const FactoryParams &params) {
  return (Type *)FactoryBase::make_instance_more_general(handle, params);
}

////////////////////////////////////////////////////////////////////
//     Function: Factory::make_instance_more_general
//       Access: Public
//  Description: Attempts to create an instance of the type requested,
//               or some base type of the type requested.  Returns the
//               new instance created, or NULL if the instance could
//               not be created.
//
//               This flavor of make_instance_more_general() accepts a
//               string name that indicates the desired type.  It must
//               be the name of some already-registered type.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE Type *Factory<Type>::
make_instance_more_general(const string &type_name,
                           const FactoryParams &params) {
  return (Type *)FactoryBase::make_instance_more_general(type_name, params);
}

////////////////////////////////////////////////////////////////////
//     Function: Factory::register_factory
//       Access: Public
//  Description: Registers a new kind of thing the Factory will be
//               able to create.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE void Factory<Type>::
register_factory(TypeHandle handle, CreateFunc *func) {
  FactoryBase::register_factory(handle, (BaseCreateFunc *)func);
}
