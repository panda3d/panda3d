###########################################################
###                                                     ###
### Panda3D Configuration File -  User-Editable Portion ###
###                                                     ###
###########################################################

# Uncomment one of the following lines to choose whether you should
# run using OpenGL, DirectX or TinyPanda (software) rendering.
# There can only be one load-display line, but you can use
# multiple aux-display lines to specify fallback modules.
# When the module indicated by load-display fails, it will fall
# back to the next display module indicated by aux-display,
# when that fails, the next aux-display line, and so on.

load-display pandagl
#load-display pandadx9
#load-display pandagles
#load-display p3tinydisplay

# These control the placement and size of the default rendering window.
# A value of -2 for the origin means to center it on the screen,
# while -1 lets the window manager choose the position.

win-origin -2 -2
win-size 800 600

# Uncomment this line if you want to run Panda fullscreen instead of
# in a window.

fullscreen #f

# The framebuffer-hardware flag forces it to use an accelerated driver.
# The framebuffer-software flag forces it to use a software renderer.
# If you set both to false, it will use whatever's available.

framebuffer-hardware #t
framebuffer-software #f

# These set the minimum requirements for the framebuffer.
# A value of 1 means: get as many bits as possible,
# consistent with the other framebuffer requirements.

depth-bits 1
color-bits 1 1 1
alpha-bits 0
stencil-bits 0
multisamples 0

# These control the amount of output Panda gives for some various
# categories.  The severity levels, in order, are "spam", "debug",
# "info", "warning", and "error"; the default is "info".  Uncomment
# one (or define a new one for the particular category you wish to
# change) to control this output.

notify-level warning
default-directnotify-level warning

# These specify where model files may be loaded from.  You probably
# want to set this to a sensible path for yourself.  $THIS_PRC_DIR is
# a special variable that indicates the same directory as this
# particular Config.prc file.

model-path    $MAIN_DIR
model-path    $THIS_PRC_DIR/..
model-path    $THIS_PRC_DIR/../models

# This enable the automatic creation of a TK window when running
# Direct.

want-directtools  #f
want-tk           #f

# Enable/disable performance profiling tool and frame-rate meter

want-pstats            #f
show-frame-rate-meter  #f

# Enable audio using the OpenAL audio library by default:

audio-library-name p3openal_audio

# Enable the use of the new movietexture class.

use-movietexture #t

# The new version of panda supports hardware vertex animation, but it's not quite ready

hardware-animated-vertices #f

# Enable the model-cache, but only for models, not textures.

model-cache-dir $XDG_CACHE_HOME/panda3d
model-cache-textures #f

# This option specifies the default profiles for Cg shaders.
# Setting it to #t makes them arbvp1 and arbfp1, since these
# seem to be most reliable. Setting it to #f makes Panda use
# the latest profile available.

basic-shaders-only #f
