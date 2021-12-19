/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file epvector.h
 * @author drose
 * @date 2011-12-19
 */

#ifndef EPVECTOR_H
#define EPVECTOR_H

#include "pvector.h"

#if defined(HAVE_EIGEN) && defined(_WIN32) && !defined(_WIN64) && !defined(CPPPARSER)

#include <Eigen/StdVector>

#include <initializer_list>

/**
 * Unfortunately, on Windows, std::vector can't be used for classes with
 * explicitly alignment requirements, due to a minor mistake in the template
 * definition (one of the vector methods receives a concrete object, which the
 * compiler flags as an error, even if the method is never called).
 *
 * As a workaround, Eigen provides their own specialization of vector, using
 * their own aligned allocator.  We define that here as epvector, which is
 * meant to be a drop-in replacement for pvector for classes that include a
 * linmath object that requires alignment.  Unfortunately, this means we can't
 * use the Panda allocator, so memory allocated for this vector class won't be
 * tracked as part of Panda's memory tracking system.  Them's the breaks,
 * kids.
 */
template<class Type>
class epvector : public std::vector<Type, Eigen::aligned_allocator<Type> > {
public:
  typedef Eigen::aligned_allocator<Type> allocator;
  typedef std::vector<Type, allocator> base_class;
  typedef typename base_class::size_type size_type;

  epvector(TypeHandle type_handle = pvector_type_handle) : base_class(allocator()) { }
  epvector(const epvector<Type> &copy) : base_class(copy) { }
  epvector(size_type n, TypeHandle type_handle = pvector_type_handle) : base_class(n, Type(), allocator()) { }
  epvector(size_type n, const Type &value, TypeHandle type_handle = pvector_type_handle) : base_class(n, value, allocator()) { }
  epvector(const Type *begin, const Type *end, TypeHandle type_handle = pvector_type_handle) : base_class(begin, end, allocator()) { }
  epvector(std::initializer_list<Type> init, TypeHandle type_handle = pvector_type_handle) : base_class(std::move(init), allocator()) { }
};

#else  // HAVE_EIGEN
#define epvector pvector
#endif  // HAVE_EIGEN

#endif
