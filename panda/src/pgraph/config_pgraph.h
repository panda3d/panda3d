/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pgraph.h
 * @author drose
 * @date 2002-02-21
 */

#ifndef CONFIG_PGRAPH_H
#define CONFIG_PGRAPH_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableList.h"
#include "configVariableString.h"

class DSearchPath;

ConfigureDecl(config_pgraph, EXPCL_PANDA_PGRAPH, EXPTP_PANDA_PGRAPH);
NotifyCategoryDecl(pgraph, EXPCL_PANDA_PGRAPH, EXPTP_PANDA_PGRAPH);
NotifyCategoryDecl(loader, EXPCL_PANDA_PGRAPH, EXPTP_PANDA_PGRAPH);
NotifyCategoryDecl(portal, EXPCL_PANDA_PGRAPH, EXPTP_PANDA_PGRAPH);

extern ConfigVariableBool fake_view_frustum_cull;
extern ConfigVariableBool clip_plane_cull;
extern ConfigVariableBool allow_portal_cull;
extern ConfigVariableBool debug_portal_cull;
extern ConfigVariableBool show_occluder_volumes;
extern ConfigVariableBool unambiguous_graph;
extern ConfigVariableBool detect_graph_cycles;
extern ConfigVariableBool no_unsupported_copy;
extern ConfigVariableBool allow_unrelated_wrt;
extern ConfigVariableBool paranoid_compose;
extern ConfigVariableBool compose_componentwise;
extern ConfigVariableBool paranoid_const;
extern ConfigVariableBool auto_break_cycles;
extern EXPCL_PANDA_PGRAPH ConfigVariableBool garbage_collect_states;
extern ConfigVariableDouble garbage_collect_states_rate;
extern ConfigVariableBool transform_cache;
extern ConfigVariableBool state_cache;
extern ConfigVariableBool uniquify_transforms;
extern EXPCL_PANDA_PGRAPH ConfigVariableBool uniquify_states;
extern ConfigVariableBool uniquify_attribs;
extern ConfigVariableBool retransform_sprites;
extern ConfigVariableBool depth_offset_decals;
extern ConfigVariableInt max_collect_vertices;
extern ConfigVariableInt max_collect_indices;
extern EXPCL_PANDA_PGRAPH ConfigVariableBool premunge_data;
extern ConfigVariableBool preserve_geom_nodes;
extern ConfigVariableBool flatten_geoms;
extern EXPCL_PANDA_PGRAPH ConfigVariableInt max_lenses;

extern ConfigVariableBool polylight_info;

extern ConfigVariableBool show_vertex_animation;
extern ConfigVariableBool show_transparency;

extern ConfigVariableBool m_dual;
extern ConfigVariableBool m_dual_opaque;
extern ConfigVariableBool m_dual_transparent;
extern ConfigVariableBool m_dual_flash;

extern ConfigVariableList load_file_type;
extern ConfigVariableString default_model_extension;

extern ConfigVariableBool allow_live_flatten;

extern EXPCL_PANDA_PGRAPH void init_libpgraph();

#endif
