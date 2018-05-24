/**
 * @file pandadx9.cxx
 * @author masad
 * @date 2004-01-15
 */

#include "pandadx9.h"

#include "config_dxgsg9.h"
#include "wdxGraphicsPipe9.h"

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpandadx9() {
  init_libdxgsg9();
}

/**
 * Returns the TypeHandle index of the recommended graphics pipe type defined
 * by this module.
 */
int
get_pipe_type_pandadx9() {
  return wdxGraphicsPipe9::get_class_type().get_index();
}
