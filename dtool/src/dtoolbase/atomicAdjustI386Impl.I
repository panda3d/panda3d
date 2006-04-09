// Filename: atomicAdjustI386Impl.I
// Created by:  drose (01Apr06)
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


////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustI386Impl::inc
//       Access: Public, Static
//  Description: Atomically increments the indicated variable.
////////////////////////////////////////////////////////////////////
INLINE void AtomicAdjustI386Impl::
inc(volatile PN_int32 &var) {
#ifndef __EDG__
  __asm__ __volatile__("lock; incl %0"
                       :"=m" (var)
                       :"m" (&var));
#endif  // __EDG__
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustI386Impl::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns true if the new value is nonzero, false if it
//               is zero.
////////////////////////////////////////////////////////////////////
INLINE bool AtomicAdjustI386Impl::
dec(volatile PN_int32 &var) {
  unsigned char c;
#ifndef __EDG__
  __asm__ __volatile__("lock; decl %0; sete %1"
                       :"=m" (var), "=qm" (c)
                       :"m" (&var) : "memory");
#endif  // __EDG__
  return (c == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustI386Impl::set
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustI386Impl::
set(PN_int32 &var, PN_int32 new_value) {
  PN_int32 orig_value = var;
  var = new_value;
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustI386Impl::get
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustI386Impl::
get(const PN_int32 &var) {
  return var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustI386Impl::set_ptr
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustI386Impl::
set_ptr(void *&var, void *new_value) {
  void *orig_value = var;
  var = new_value;
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustI386Impl::get_ptr
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustI386Impl::
get_ptr(void * const &var) {
  return var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustI386Impl::compare_and_exchange
//       Access: Public, Static
//  Description: Atomic compare and exchange.  
//
//               If mem is equal to old_value, store new_value in mem.
//               In either case, return the original value of mem.
//               The caller can test for success by comparing
//               return_value == old_value.
//
//               The atomic function expressed in pseudo-code:
//
//                 orig_value = mem;
//                 if (mem == old_value) {
//                   mem = new_value;
//                 }
//                 return orig_value;
//
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustI386Impl::
compare_and_exchange(volatile PN_int32 &mem, PN_int32 old_value,
                     PN_int32 new_value) {
  PN_int32 prev;
#ifndef __EDG__
  __asm__ __volatile__("lock; cmpxchgl %1,%2"
                       : "=a"(prev)
                       : "r"(new_value), "m"(mem), "0"(old_value)
                       : "memory");
#endif  // __EDG__
  return prev;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustI386Impl::compare_and_exchange_ptr
//       Access: Public, Static
//  Description: Atomic compare and exchange.  
//
//               As above, but works on pointers instead of integers.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustI386Impl::
compare_and_exchange_ptr(void * volatile &mem, void *old_value,
                         void *new_value) {
  void *prev;
#ifndef __EDG__
  __asm__ __volatile__("lock; cmpxchgl %1,%2"
                       : "=a"(prev)
                       : "r"(new_value), "m"(mem), "0"(old_value)
                       : "memory");
#endif  // __EDG__
  return prev;
}
