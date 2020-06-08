
#ifndef CONFIG_RECASTDETOUR_H
#define CONFIG_RECASTDETOUR_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableInt.h"

NotifyCategoryDecl(recastdetour, EXPCL_RECASTDETOUR, EXPTP_RECASTDETOUR);

extern ConfigVariableInt    recastdetour_sample_config_variable;

extern EXPCL_RECASTDETOUR void init_librecastdetour();

#endif
