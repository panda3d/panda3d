#define BUILD_DIRECTORY $[HAVE_SSL]

#begin bin_target
  #define TARGET httpbackup
  #define LOCAL_LIBS pandaappbase
  #define USE_PACKAGES ssl

  #define OTHER_LIBS \
    progbase pandatoolbase \
    express:c downloader:c pandaexpress:m \
    panda:m dtool:m dtoolconfig:m \
    pystub

  #define SOURCES \
    backupCatalog.I backupCatalog.cxx backupCatalog.h \
    httpBackup.cxx httpBackup.h

#end bin_target

