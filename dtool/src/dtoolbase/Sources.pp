#define SELECT_TAU select.tau

#begin lib_target
  #define TARGET dtoolbase

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx
  
  #define SOURCES \
    addHash.I addHash.h \
    atomicAdjust.h \
    atomicAdjustDummyImpl.h atomicAdjustDummyImpl.I \
    atomicAdjustI386Impl.h atomicAdjustI386Impl.I \
    atomicAdjustNsprImpl.h atomicAdjustNsprImpl.I \
    atomicAdjustPosixImpl.h atomicAdjustPosixImpl.I \
    atomicAdjustWin32Impl.h atomicAdjustWin32Impl.I \
    cmath.I cmath.h \
    dallocator.T dallocator.h \
    deletedChain.h deletedChain.T \
    dtoolbase.h dtoolbase_cc.h dtoolsymbols.h \
    fakestringstream.h \
    indent.I indent.h indent.cxx \
    memoryBase.h \
    mutexImpl.h \
    mutexDummyImpl.h mutexDummyImpl.I \
    mutexNsprImpl.h mutexNsprImpl.I \
    mutexLinuxImpl.h mutexLinuxImpl.I \
    mutexPosixImpl.h mutexPosixImpl.I \
    mutexWin32Impl.h mutexWin32Impl.I \
    mutexSpinlockImpl.h mutexSpinlockImpl.I \
    nearly_zero.h \
    numeric_types.h \
    selectThreadImpl.h \
    stl_compares.I stl_compares.h \
    pallocator.T pallocator.h \
    pdeque.h plist.h pmap.h pset.h pvector.h \
    dlmalloc.c lookup3.h lookup3.c

 #define INCLUDED_SOURCES  \
    addHash.cxx \
    atomicAdjustDummyImpl.cxx \
    atomicAdjustI386Impl.cxx \
    atomicAdjustNsprImpl.cxx \
    atomicAdjustPosixImpl.cxx \
    atomicAdjustWin32Impl.cxx \
    dtoolbase.cxx \
    mutexDummyImpl.cxx \
    mutexNsprImpl.cxx \
    mutexLinuxImpl.cxx \
    mutexPosixImpl.cxx \
    mutexWin32Impl.cxx \
    mutexSpinlockImpl.cxx

  #define INSTALL_HEADERS \
    addHash.I addHash.h \
    atomicAdjust.h \
    atomicAdjustDummyImpl.h atomicAdjustDummyImpl.I \
    atomicAdjustI386Impl.h atomicAdjustI386Impl.I \
    atomicAdjustNsprImpl.h atomicAdjustNsprImpl.I \
    atomicAdjustPosixImpl.h atomicAdjustPosixImpl.I \
    atomicAdjustWin32Impl.h atomicAdjustWin32Impl.I \
    cmath.I cmath.h \
    dallocator.T dallocator.h \
    deletedChain.h deletedChain.T \
    dtoolbase.h dtoolbase_cc.h dtoolsymbols.h fakestringstream.h \
    indent.I indent.h \
    memoryBase.h \
    mutexImpl.h \
    mutexDummyImpl.h mutexDummyImpl.I \
    mutexNsprImpl.h mutexNsprImpl.I \
    mutexLinuxImpl.h mutexLinuxImpl.I \
    mutexPosixImpl.h mutexPosixImpl.I \
    mutexWin32Impl.h mutexWin32Impl.I \
    mutexSpinlockImpl.h mutexSpinlockImpl.I \
    nearly_zero.h \
    numeric_types.h \
    selectThreadImpl.h \
    stl_compares.I stl_compares.h \
    pallocator.T pallocator.h \
    pdeque.h plist.h pmap.h pset.h pvector.h \
    lookup3.h

#end lib_target
