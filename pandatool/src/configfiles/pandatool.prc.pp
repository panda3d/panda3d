//
// pandatool.prc.pp
//
// This file defines the script to auto-generate _pandatool.prc at
// ppremake time.
//

#output 30_pandatool.prc
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################


# Since we have compiled pandatool, it follows that the ptloader
# module is available.  Request that it be loaded.  This module allows
# direct loading of third-party model files like .flt, .mb, or .dxf
# into Panda, for instance via the loadModel() call or on the pview
# command line.

load-file-type ptloader

#end 30_pandatool.prc
