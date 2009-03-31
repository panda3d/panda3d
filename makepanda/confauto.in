###########################################################
###                                                     ###
### Panda3D Configuration File - Auto-Generated Portion ###
###                                                     ###
### Editing this file is not recommended. Most of these ###
### directives can be overriden in Config.prc           ###
###                                                     ###
###########################################################

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
egg-object-type-seq24           <Switch> { 1 } <Scalar> fps { 24 }
egg-object-type-seq12           <Switch> { 1 } <Scalar> fps { 12 }
egg-object-type-indexed         <Scalar> indexed { 1 }

egg-object-type-binary          <Scalar> alpha { binary }
egg-object-type-dual            <Scalar> alpha { dual }
egg-object-type-glass           <Scalar> alpha { blend_no_occlude }

# These are just shortcuts to define the Model and DCS flags, which
# indicate nodes that should not be flattened out of the hierarchy
# during the conversion process.  DCS goes one step further and
# indicates that the node's transform is important and should be
# preserved (DCS stands for Dynamic Coordinate System).  Notouch is
# even stronger, and means not to do any flattening below the node at
# all.
egg-object-type-model           <Model> { 1 }
egg-object-type-dcs             <DCS> { 1 }
egg-object-type-notouch         <DCS> { no_touch }

# The following define various kinds of collision geometry.  These
# mark the geometry at this level and below as invisible collision
# polygons, which can be used by Panda's collision system to detect
# collisions more optimally than regular visible polygons.
egg-object-type-barrier         <Collide> { Polyset descend }
egg-object-type-sphere          <Collide> { Sphere descend }
egg-object-type-invsphere       <Collide> { InvSphere descend }
egg-object-type-tube            <Collide> { Tube descend }

# As above, but these are flagged to be "intangible", so that they
# will trigger an event but not stop an object from passing through.
egg-object-type-trigger         <Collide> { Polyset descend intangible }
egg-object-type-trigger-sphere  <Collide> { Sphere descend intangible }

# "floor" and "dupefloor" define the nodes in question as floor
# polygons.  dupefloor means to duplicate the geometry first so that
# the same polygons serve both as visible geometry and as collision
# polygons.
egg-object-type-floor           <Collide> { Polyset descend level }
egg-object-type-dupefloor       <Collide> { Polyset keep descend level }

# "bubble" puts an invisible bubble around an object, but does not
# otherwise remove the geometry.
egg-object-type-bubble          <Collide> { Sphere keep descend }

# "ghost" turns off the normal collide bit that is set on visible
# geometry by default, so that if you are using visible geometry for
# collisions, this particular geometry will not be part of those
# collisions--it is ghostlike.
egg-object-type-ghost           <Scalar> collide-mask { 0 }

# "glow" is useful for halo effects and things of that ilk.  It
# renders the object in add mode instead of the normal opaque mode.
egg-object-type-glow            <Scalar> blend { add }

# This module allows direct loading of formats like .flt, .mb, or .dxf

load-file-type p3ptloader

# Define a new egg object type.  See the comments in _panda.prc about this.

egg-object-type-direct-widget   <Scalar> collide-mask { 0x80000000 } <Collide> { Polyset descend }

# Define a new cull bin that will render on top of everything else.

cull-bin gui-popup 60 unsorted

# The following two lines are a fix for flaky hardware clocks.

lock-to-one-cpu #t
paranoid-clock 1

# This default only comes into play if you try to load a model
# and don't specify an extension.

default-model-extension .egg
