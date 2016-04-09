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

NotifyCategoryDecl(vulkandisplay, EXPCL_VULKANDISPLAY, EXPTP_VULKANDISPLAY);

extern ConfigVariableInt vulkan_color_palette_size;

extern EXPCL_VULKANDISPLAY void init_libvulkandisplay();
extern "C" EXPCL_VULKANDISPLAY int get_pipe_type_p3vulkandisplay();

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

void vulkan_error(VkResult result, const char *message);

#endif
