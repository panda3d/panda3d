set(P3DTOOLBASE_HEADERS
    addHash.I addHash.h
    atomicAdjust.h
    atomicAdjustDummyImpl.h atomicAdjustDummyImpl.I
    atomicAdjustI386Impl.h atomicAdjustI386Impl.I
    atomicAdjustPosixImpl.h atomicAdjustPosixImpl.I
    atomicAdjustWin32Impl.h atomicAdjustWin32Impl.I
    cmath.I cmath.h
    deletedBufferChain.h deletedBufferChain.I
    deletedChain.h deletedChain.T
    dtoolbase.h dtoolbase_cc.h dtoolsymbols.h
    dtool_platform.h
    fakestringstream.h
    indent.I indent.h
    memoryBase.h
    memoryHook.h memoryHook.I
    mutexImpl.h
    mutexDummyImpl.h mutexDummyImpl.I
    mutexPosixImpl.h mutexPosixImpl.I
    mutexWin32Impl.h mutexWin32Impl.I
    mutexSpinlockImpl.h mutexSpinlockImpl.I
    nearly_zero.h
    neverFreeMemory.h neverFreeMemory.I
    numeric_types.h
    pdtoa.h
    pstrtod.h
    register_type.I register_type.h
    selectThreadImpl.h
    stl_compares.I stl_compares.h
    typeHandle.I typeHandle.h
    typeRegistry.I typeRegistry.h
    typeRegistryNode.I typeRegistryNode.h
    typedObject.I typedObject.h
    pallocator.T pallocator.h
    pdeque.h plist.h pmap.h pset.h
    pvector.h epvector.h
    lookup3.h
)

set(P3DTOOLBASE_SOURCES
    addHash.cxx
    atomicAdjustDummyImpl.cxx
    atomicAdjustI386Impl.cxx
    atomicAdjustPosixImpl.cxx
    atomicAdjustWin32Impl.cxx
    deletedBufferChain.cxx
    dtoolbase.cxx
    indent.cxx
    lookup3.c
    memoryBase.cxx
    memoryHook.cxx
    mutexDummyImpl.cxx
    mutexPosixImpl.cxx
    mutexWin32Impl.cxx
    mutexSpinlockImpl.cxx
    neverFreeMemory.cxx
    pdtoa.cxx
    pstrtod.cxx
    register_type.cxx
    typeHandle.cxx
    typeRegistry.cxx typeRegistryNode.cxx
    typedObject.cxx
)

set(P3DTOOLBASE_IGATEEXT
    typeHandle_ext.cxx
    typeHandle_ext.h
)

composite_sources(p3dtoolbase P3DTOOLBASE_SOURCES)

add_library(p3dtoolbase	${P3DTOOLBASE_HEADERS} ${P3DTOOLBASE_SOURCES})
set_target_properties(p3dtoolbase PROPERTIES DEFINE_SYMBOL BUILDING_DTOOL_DTOOLBASE)
# The extensions need py_panda.h and extension.h from interrogatedb
target_include_directories(p3dtoolbase PUBLIC
  $<TARGET_PROPERTY:p3interrogatedb,INTERFACE_INCLUDE_DIRECTORIES>)
target_use_packages(p3dtoolbase THREADS EIGEN)
target_interrogate(p3dtoolbase ${P3DTOOLBASE_SOURCES} EXTENSIONS ${P3DTOOLBASE_IGATEEXT})

install(TARGETS p3dtoolbase DESTINATION lib)
install(FILES ${P3DTOOLBASE_HEADERS} DESTINATION include/panda3d)
