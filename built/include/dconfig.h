/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dconfig.h
 * @author cary
 * @date 1998-07-14
 */

#ifndef DCONFIG_H
#define DCONFIG_H

#include "dtoolbase.h"
#include "notifyCategoryProxy.h"

// These macros are used in each directory to call an initialization function
// at static-init time.  These macros may eventually be phased out in favor of
// a simpler interface that does not require static init.

// NOTE: Having a macro called Configure proved to be problematic with some
// DX9 headers.  To avoid that in the future we provide a new family of macros
// prefixed by DTool and deprecate the old ones, to be removed from the
// codebase sometime in the future.

// This macro should appear in the config_*.h file.

#define ConfigureDecl(name, expcl, exptp)
#define DToolConfigureDecl(name, expcl, exptp)

// This macro defines the actual declaration of the object defined above; it
// should appear in the config_*.cxx file.

#define ConfigureDef(name) \
  class StaticInitializer_ ## name { \
  public: \
    StaticInitializer_ ## name(); \
  }; \
  static StaticInitializer_ ## name name;
#define DToolConfigureDef(name) \
  class StaticInitializer_ ## name { \
  public: \
    StaticInitializer_ ## name(); \
  }; \
  static StaticInitializer_ ## name name;

// This macro can be used in lieu of the above two when the Configure object
// does not need to be visible outside of the current C file.

#define Configure(name) ConfigureDef(name)
#define DToolConfigure(name) DToolConfigureDef(name)

// This one defines a block of code that will be executed at static init time.
// It must always be defined (in the C file), even if no code is to be
// executed.

#define ConfigureFn(name) \
  StaticInitializer_ ## name::StaticInitializer_ ## name()
#define DToolConfigureFn(name) \
  StaticInitializer_ ## name::StaticInitializer_ ## name()

#endif /* __CONFIG_H__ */
