//
// _panda.prc.pp
//
// This file defines the script to auto-generate _panda.prc at
// ppremake time.  This is intended to fill in some of the default
// parameters, in particular the default display types.
//

#output 80_panda.prc
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

# Let's set up a default window size of 800x600.  The user can
# override this in his or her personal prc file.
win-width 800
win-height 600

# Define the display types that have been compiled in.  Panda will
# pick one of these by going through the list in this order until one
# is found that works, unless the user specifically requests a
# particular display type with the load-display directive.

#if $[HAVE_DX]
aux-display pandadx9
#endif
#if $[HAVE_GL]
aux-display pandagl
#endif
#if $[HAVE_DX]
aux-display pandadx8
aux-display pandadx7
#endif

# The egg loader is handy to have available by default.  This allows
# clients to load egg files.  (The bam loader is built-in so bam files
# are always loadable).

# By qualifying with the extension "egg", we indicate the egg loader
# should be made available only if you explicitly name a file with an
# .egg extension.

# Also see ptloader, which is built as part of pandatool; it allows
# files of more exotic types (like .flt, .mb, .lwo, and .dxf) to be
# loaded directly into Panda.

load-file-type egg pandaegg



# The following lines define some handy object types to use within the
# egg syntax.  This remaps <ObjectType> { name } into whatever egg
# syntax is given by egg-object-type-name, which makes a handy
# abbreviation for modeling packages (like Maya) to insert
# sophisticated egg syntax into the generated egg file, using a single
# object type string.

egg-object-type-portal          <Scalar> portal { 1 }
egg-object-type-polylight       <Scalar> polylight { 1 }

#end 80_panda.prc
