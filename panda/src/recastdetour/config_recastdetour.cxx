

#include "config_recastdetour.h"

#include "pandaSystem.h"
#include "dconfig.h"
#include "InputGeom.h"
#include "MeshLoaderObj.h"
#include "NavMeshSample.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_RECASTDETOUR)
  #error Buildsystem error: BUILDING_RECASTDETOUR not defined
#endif

Configure(config_recastdetour);
NotifyCategoryDef(recastdetour, "");

ConfigureFn(config_recastdetour) {
  init_librecastdetour();
}

ConfigVariableInt recastdetour_sample_config_variable
("recastdetour-sample-config-variable", 25);

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_librecastdetour() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}
