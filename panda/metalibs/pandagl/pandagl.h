/**
 * @file pandagl.h
 * @author drose
 * @date 2001-01-02
 */

#ifndef PANDAGL_H
#define PANDAGL_H

#include "pandabase.h"

EXPCL_PANDAGL void init_libpandagl();
extern "C" EXPCL_PANDAGL int get_pipe_type_pandagl();

#if defined(HAVE_EGL) && !defined(USE_X11)
extern "C" EXPCL_PANDAGL int get_pipe_type_p3headlessgl();
#endif

#endif
