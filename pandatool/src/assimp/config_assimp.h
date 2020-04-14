/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_assimp.h
 * @author rdb
 * @date 2011-03-29
 */

#ifndef CONFIG_ASSIMP_H
#define CONFIG_ASSIMP_H

#include "pandatoolbase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableDouble.h"
#include "dconfig.h"

ConfigureDecl(config_assimp, EXPCL_ASSIMP, EXPTP_ASSIMP);
NotifyCategoryDecl(assimp, EXPCL_ASSIMP, EXPTP_ASSIMP);

extern ConfigVariableBool assimp_calc_tangent_space;
extern ConfigVariableBool assimp_join_identical_vertices;
extern ConfigVariableBool assimp_improve_cache_locality;
extern ConfigVariableBool assimp_remove_redundant_materials;
extern ConfigVariableBool assimp_fix_infacing_normals;
extern ConfigVariableBool assimp_optimize_meshes;
extern ConfigVariableBool assimp_optimize_graph;
extern ConfigVariableBool assimp_flip_winding_order;
extern ConfigVariableBool assimp_gen_normals;
extern ConfigVariableDouble assimp_smooth_normal_angle;

extern EXPCL_ASSIMP void init_libassimp();

#endif
