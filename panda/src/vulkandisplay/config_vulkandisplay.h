/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_vulkandisplay.h
 * @author rdb
 * @date 2016-02-16
 */

#ifndef CONFIG_VULKANDISPLAY_H
#define CONFIG_VULKANDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableInt.h"
#include "configVariableInt64.h"

NotifyCategoryDecl(vulkandisplay, EXPCL_VULKANDISPLAY, EXPTP_VULKANDISPLAY);

extern ConfigVariableInt vulkan_color_palette_size;
extern ConfigVariableInt64 vulkan_memory_page_size;

extern EXPCL_VULKANDISPLAY void init_libvulkandisplay();
extern "C" EXPCL_VULKANDISPLAY int get_pipe_type_p3vulkandisplay();

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR

#elif defined(HAVE_X11)
// Make sure Eigen is included first to avoid conflict
#include "lsimpleMatrix.h"
#include "pre_x11_include.h"
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.h>

#ifdef HAVE_X11
#include "post_x11_include.h"
#endif

void vulkan_error(VkResult result, const char *message);

#endif
