#if $[OSX_PLATFORM]
#output extensions_darwin.py
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
################################# DO NOT EDIT ###########################

# This defines the shared-library filename extension that is built and
# imported on OSX.  It's normally .dylib, but in certain Python and
# OSX versions (for instance, Python 2.4 on OSX 10.4), it appears that
# we need to generate and import .so files instead, since Python won't
# import the .dylibs directly.  This is controlled via the BUNDLE_EXT
# variable defined in Config.pp.
#if $[BUNDLE_EXT]
dll_ext = "$[BUNDLE_EXT]"
#else
dll_ext = ".dylib"
#endif

#end extensions_darwin.py

#endif  // OSX_PLATFORM
