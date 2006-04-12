// Filename: atomicAdjustNsprImpl.I
// Created by:  drose (09Aug02)
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
//     Function: AtomicAdjustNsprImpl::inc
//       Access: Public, Static
//  Description: Atomically increments the indicated variable.
////////////////////////////////////////////////////////////////////
INLINE void AtomicAdjustNsprImpl::
inc(TVOLATILE PN_int32 &var) {
  PR_AtomicIncrement((PRInt32 *)&var);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustNsprImpl::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns true if the new value is nonzero, false if it
//               is zero.
////////////////////////////////////////////////////////////////////
INLINE bool AtomicAdjustNsprImpl::
dec(TVOLATILE PN_int32 &var) {
  return (PR_AtomicDecrement((PRInt32 *)&var) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustNsprImpl::set
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustNsprImpl::
set(TVOLATILE PN_int32 &var, PN_int32 new_value) {
  return PR_AtomicSet((PRInt32 *)&var, new_value);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustNsprImpl::get
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustNsprImpl::
get(const TVOLATILE PN_int32 &var) {
  return var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustNsprImpl::set_ptr
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustNsprImpl::
set_ptr(void * TVOLATILE &var, void *new_value) {
  return (void *)PR_AtomicSet((PRInt32 *)&var, (PRInt32)new_value);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustNsprImpl::get_ptr
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustNsprImpl::
get_ptr(void * const TVOLATILE &var) {
  return var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustNsprImpl::compare_and_exchange
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
INLINE PN_int32 AtomicAdjustNsprImpl::
compare_and_exchange(TVOLATILE PN_int32 &mem, PN_int32 old_value,
                     PN_int32 new_value) {
  PR_Lock(_mutex);
  PN_int32 orig_value = mem;
  if (mem == old_value) {
    mem = new_value;
  }
  PR_Unlock(_mutex);
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustNsprImpl::compare_and_exchange_ptr
//       Access: Public, Static
//  Description: Atomic compare and exchange.  
//
//               As above, but works on pointers instead of integers.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustNsprImpl::
compare_and_exchange_ptr(void * TVOLATILE &mem, void *old_value,
                         void *new_value) {
  PR_Lock(_mutex);
  void *orig_value = mem;
  if (mem == old_value) {
    mem = new_value;
  }
  PR_Unlock(_mutex);
  return orig_value;
}
