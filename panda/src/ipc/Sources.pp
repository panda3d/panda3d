#define LOCAL_LIBS express pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES nspr ipc
#define BUILD_DIRECTORY $[HAVE_IPC]

#begin lib_target
  #define TARGET ipc
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  
  
  #define SOURCES                                                       \
    ipc_atomics.h ipc_condition.h ipc_file.I ipc_file.h \
    ipc_library.h ipc_mach_traits.h ipc_mutex.h ipc_nspr_traits.h \
    ipc_nt_traits.h ipc_posix_traits.h \
    ipc_ps2_traits.h ipc_semaphore.h ipc_solaris_traits.h ipc_thread.h \
    ipc_traits.h
    
  #define INCLUDED_SOURCES                                                       \
    ipc.cxx ipc_traits.cxx

  #define INSTALL_HEADERS                                       \
    ipc_mutex.h ipc_condition.h ipc_semaphore.h ipc_thread.h \
    ipc_traits.h ipc_mach_traits.h ipc_nt_traits.h ipc_posix_traits.h \
    ipc_solaris_traits.h ipc_nspr_traits.h ipc_ps2_traits.h \
    ipc_atomics.h ipc_library.h ipc_file.h ipc_file.I

#end lib_target

// There needs to be a way to compile this into the ipc library only
// when appropriate.  Perhaps as simple as compiling it all the time,
// but protecting the code itself within #ifdefs.
#define EXTRA_DIST ipc_nt_traits.cxx
