// Filename: config_notify.C
// Created by:  drose (29Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "config_notify.h"
#include "dconfig.h"

ConfigureDef(config_notify);

ConfigureFn(config_notify) {
}

// We have to declare this as a function instead of a static const,
// because its value might be needed at static init time.
bool
get_assert_abort() {
  static bool *assert_abort = (bool *)NULL;
  if (assert_abort == (bool *)NULL) {
    assert_abort = new bool;
    *assert_abort = config_notify.GetBool("assert-abort", false);
  }
  return *assert_abort;
}
