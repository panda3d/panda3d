#define LOCAL_LIBS express pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_NSPR yes
#define USE_IPC yes
#define DIRECTORY_IF_IPC yes

#begin lib_target
  #define TARGET ipc
  
  #define SOURCES							\
    ipc.cxx ipc_atomics.h ipc_condition.h ipc_file.I ipc_file.h \
    ipc_library.h ipc_mach_traits.h ipc_mutex.h ipc_nspr_traits.h \
    ipc_nt_traits.h ipc_posix_traits.h \
    ipc_ps2_traits.h ipc_semaphore.h ipc_solaris_traits.h ipc_thread.h \
    ipc_traits.cxx ipc_traits.h

  #define INSTALL_HEADERS					\
    ipc_mutex.h ipc_condition.h ipc_semaphore.h ipc_thread.h \
    ipc_traits.h ipc_mach_traits.h ipc_nt_traits.h ipc_posix_traits.h \
    ipc_solaris_traits.h ipc_nspr_traits.h ipc_ps2_traits.h \
    ipc_atomics.h ipc_library.h ipc_file.h ipc_file.I

#end lib_target

// There needs to be a way to compile this into the ipc library only
// when appropriate.  Perhaps as simple as compiling it all the time,
// but protecting the code itself within #ifdefs.
#define EXTRA_DIST ipc_nt_traits.cxx

// test_lib_target is not implemented yet.  This is a bogus
// test_bin_target until then.
#begin test_bin_target
  #define TARGET loom
  
  #define SOURCES							\
    loom.cxx loom.h loom_internal.h

  #define LOCAL_LIBS ipc $[LOCAL_LIBS]
#end test_bin_target


#begin test_bin_target
  #define TARGET test_diners
  #define SOURCES test_diners.cxx 
  #define LOCAL_LIBS ipc $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_priority
  #define SOURCES test_priority.cxx 
  #define LOCAL_LIBS ipc $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_prodcons
  #define SOURCES test_prodcons.cxx 
  #define LOCAL_LIBS ipc $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_threaddata
  #define SOURCES test_threaddata.cxx 
  #define LOCAL_LIBS ipc $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET loom_main
  #define SOURCES loom_main.cxx 
  #define LOCAL_LIBS loom ipc $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_file
  #define SOURCES test_file.cxx 
  #define LOCAL_LIBS loom ipc $[LOCAL_LIBS]
#end test_bin_target

// Oops, these are test .so's
#begin test_bin_target
  #define TARGET loom_test1
  #define SOURCES loom_test1.cxx 
  #define LOCAL_LIBS loom ipc $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET loom_test2
  #define SOURCES loom_test2.cxx 
  #define LOCAL_LIBS loom ipc $[LOCAL_LIBS]
#end test_bin_target
