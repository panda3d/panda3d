
#ifndef CONFIG_NAVIGATION_H
#define CONFIG_NAVIGATION_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableInt.h"

NotifyCategoryDecl(navigation, EXPCL_NAVIGATION, EXPTP_NAVIGATION);

extern ConfigVariableInt    navigation_sample_config_variable;

extern EXPCL_NAVIGATION void init_libnavigation();

#endif

