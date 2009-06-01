// Filename: physx_enumerations.h
// Created by: pratt (Apr 20, 2006)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PHYSX_ENUMERATIONS_H
#define PHYSX_ENUMERATIONS_H

#ifdef HAVE_PHYSX

#include "NoMinMax.h"
#include "NxPhysics.h"

enum PhysxActorDescType {
  physx_adt_shapeless = NX_ADT_SHAPELESS,
  physx_adt_default = NX_ADT_DEFAULT,
  physx_adt_allocator = NX_ADT_ALLOCATOR,
  physx_adt_list = NX_ADT_LIST,
  physx_adt_pointer = NX_ADT_POINTER,
};

enum PhysxActorFlag {
  physx_af_fluid_disable_collision = NX_AF_FLUID_DISABLE_COLLISION,
  physx_af_contact_modification = NX_AF_CONTACT_MODIFICATION,
  physx_af_disable_collision = NX_AF_DISABLE_COLLISION,
  physx_af_disable_response = NX_AF_DISABLE_RESPONSE,
  physx_af_lock_com = NX_AF_LOCK_COM,
  physx_af_user_actor_pair_filtering = NX_AF_USER_ACTOR_PAIR_FILTERING,
  physx_af_force_cone_friction = NX_AF_FORCE_CONE_FRICTION,
};

enum PhysxAssertResponse {
  physx_ar_continue = NX_AR_CONTINUE,
  physx_ar_ignore = NX_AR_IGNORE,
  physx_ar_breakpoint = NX_AR_BREAKPOINT,
};

enum PhysxAxisType {
  physx_axis_plus_x = NX_AXIS_PLUS_X,
  physx_axis_minus_x = NX_AXIS_MINUS_X,
  physx_axis_plus_y = NX_AXIS_PLUS_Y,
  physx_axis_minus_y = NX_AXIS_MINUS_Y,
  physx_axis_plus_z = NX_AXIS_PLUS_Z,
  physx_axis_minus_z = NX_AXIS_MINUS_Z,
  physx_axis_arbitrary = NX_AXIS_ARBITRARY,
};

enum PhysxBSphereMethod {
  physx_bs_none = NX_BS_NONE,
  physx_bs_gems = NX_BS_GEMS,
  physx_bs_miniball = NX_BS_MINIBALL,
  physx_bs_force_dword = NX_BS_FORCE_DWORD,
};

enum PhysxBodyFlag {
  physx_bf_frozen_rot_z = NX_BF_FROZEN_ROT_Z,
  physx_bf_frozen_rot_y = NX_BF_FROZEN_ROT_Y,
  physx_bf_frozen_rot_x = NX_BF_FROZEN_ROT_X,
  physx_bf_kinematic = NX_BF_KINEMATIC,
  physx_bf_pose_sleep_test = NX_BF_POSE_SLEEP_TEST,
  physx_bf_disable_gravity = NX_BF_DISABLE_GRAVITY,
  physx_bf_frozen_pos_z = NX_BF_FROZEN_POS_Z,
  physx_bf_frozen_pos_x = NX_BF_FROZEN_POS_X,
  physx_bf_frozen_pos_y = NX_BF_FROZEN_POS_Y,
  physx_bf_visualization = NX_BF_VISUALIZATION,
  physx_bf_filter_sleep_vel = NX_BF_FILTER_SLEEP_VEL,
  physx_bf_energy_sleep_test = NX_BF_ENERGY_SLEEP_TEST,
};

enum PhysxCapsuleShapeFlag {
  physx_swept_shape = NX_SWEPT_SHAPE,
};

enum PhysxClothAttachmentFlag {
  physx_cloth_attachment_tearable = NX_CLOTH_ATTACHMENT_TEARABLE,
  physx_cloth_attachment_twoway = NX_CLOTH_ATTACHMENT_TWOWAY,
};

enum PhysxClothFlag {
  physx_clf_disable_collision = NX_CLF_DISABLE_COLLISION,
  physx_clf_bending_ortho = NX_CLF_BENDING_ORTHO,
  physx_clf_bending = NX_CLF_BENDING,
  physx_clf_pressure = NX_CLF_PRESSURE,
  physx_clf_damping = NX_CLF_DAMPING,
  physx_clf_fluid_collision = NX_CLF_FLUID_COLLISION,
  physx_clf_triangle_collision = NX_CLF_TRIANGLE_COLLISION,
  physx_clf_visualization = NX_CLF_VISUALIZATION,
  physx_clf_collision_twoway = NX_CLF_COLLISION_TWOWAY,
  physx_clf_hardware = NX_CLF_HARDWARE,
  physx_clf_gravity = NX_CLF_GRAVITY,
  physx_clf_selfcollision = NX_CLF_SELFCOLLISION,
  physx_clf_tearable = NX_CLF_TEARABLE,
  physx_clf_validbounds = NX_CLF_VALIDBOUNDS,
  physx_clf_static = NX_CLF_STATIC,
  physx_clf_comdamping = NX_CLF_COMDAMPING,
};

enum PhysxClothMeshFlags {
  physx_cloth_mesh_tearable = NX_CLOTH_MESH_TEARABLE,
};

enum PhysxClothVertexAttachmentStatus {
  physx_cloth_vertex_attachment_none = NX_CLOTH_VERTEX_ATTACHMENT_NONE,
  physx_cloth_vertex_attachment_global = NX_CLOTH_VERTEX_ATTACHMENT_GLOBAL,
  physx_cloth_vertex_attachment_shape = NX_CLOTH_VERTEX_ATTACHMENT_SHAPE,
};

enum PhysxClothVertexFlags {
  physx_cloth_vertex_attached = NX_CLOTH_VERTEX_ATTACHED,
  physx_cloth_vertex_tearable = NX_CLOTH_VERTEX_TEARABLE,
};

enum PhysxCombineMode {
  physx_cm_multiply = NX_CM_MULTIPLY,
  physx_cm_average = NX_CM_AVERAGE,
  physx_cm_max = NX_CM_MAX,
  physx_cm_pad_32 = NX_CM_PAD_32,
  physx_cm_n_values = NX_CM_N_VALUES,
  physx_cm_min = NX_CM_MIN,
};

enum PhysxCompartmentFlag {
  physx_cf_sleep_notification = NX_CF_SLEEP_NOTIFICATION,
  physx_cf_continuous_cd = NX_CF_CONTINUOUS_CD,
  physx_cf_inherit_settings = NX_CF_INHERIT_SETTINGS,
  physx_cf_restricted_scene = NX_CF_RESTRICTED_SCENE,
};

enum PhysxCompartmentType {
  physx_sct_cloth = NX_SCT_CLOTH,
  physx_sct_rigidbody = NX_SCT_RIGIDBODY,
  physx_sct_softbody = NX_SCT_SOFTBODY,
  physx_sct_fluid = NX_SCT_FLUID,
};

enum PhysxContactPairFlag {
  physx_ignore_pair = NX_IGNORE_PAIR,
  physx_notify_on_touch = NX_NOTIFY_ON_TOUCH,
  physx_notify_on_end_touch = NX_NOTIFY_ON_END_TOUCH,
  physx_notify_forces = NX_NOTIFY_FORCES,
  physx_notify_on_slide = NX_NOTIFY_ON_SLIDE,
  physx_notify_on_impact = NX_NOTIFY_ON_IMPACT,
  physx_notify_contact_modification = NX_NOTIFY_CONTACT_MODIFICATION,
  physx_notify_on_roll = NX_NOTIFY_ON_ROLL,
  physx_notify_on_start_touch = NX_NOTIFY_ON_START_TOUCH,
};

enum PhysxConvexFlags {
  physx_cf_use_uncompressed_normals = NX_CF_USE_UNCOMPRESSED_NORMALS,
  physx_cf_flipnormals = NX_CF_FLIPNORMALS,
  physx_cf_use_legacy_cooker = NX_CF_USE_LEGACY_COOKER,
  physx_cf_inflate_convex = NX_CF_INFLATE_CONVEX,
  physx_cf_16_bit_indices = NX_CF_16_BIT_INDICES,
  physx_cf_compute_convex = NX_CF_COMPUTE_CONVEX,
};

enum PhysxCookingValue {
  physx_cooking_convex_version_pc = NX_COOKING_CONVEX_VERSION_PC,
  physx_cooking_mesh_version_pc = NX_COOKING_MESH_VERSION_PC,
  physx_cooking_convex_version_xenon = NX_COOKING_CONVEX_VERSION_XENON,
  physx_cooking_mesh_version_xenon = NX_COOKING_MESH_VERSION_XENON,
  physx_cooking_convex_version_playstation3 = NX_COOKING_CONVEX_VERSION_PLAYSTATION3,
  physx_cooking_mesh_version_playstation3 = NX_COOKING_MESH_VERSION_PLAYSTATION3,
};

enum PhysxD6JointDriveType {
  physx_d6joint_drive_velocity = NX_D6JOINT_DRIVE_VELOCITY,
  physx_d6joint_drive_position = NX_D6JOINT_DRIVE_POSITION,
};

enum PhysxD6JointFlag {
  physx_d6joint_slerp_drive = NX_D6JOINT_SLERP_DRIVE,
  physx_d6joint_gear_enabled = NX_D6JOINT_GEAR_ENABLED,
};

enum PhysxD6JointMotion {
  physx_d6joint_motion_locked = NX_D6JOINT_MOTION_LOCKED,
  physx_d6joint_motion_limited = NX_D6JOINT_MOTION_LIMITED,
  physx_d6joint_motion_free = NX_D6JOINT_MOTION_FREE,
};

enum PhysxDebugColor {
  physx_argb_white = NX_ARGB_WHITE,
  physx_argb_green = NX_ARGB_GREEN,
  physx_argb_red = NX_ARGB_RED,
  physx_argb_yellow = NX_ARGB_YELLOW,
  physx_argb_magenta = NX_ARGB_MAGENTA,
  physx_argb_blue = NX_ARGB_BLUE,
  physx_argb_cyan = NX_ARGB_CYAN,
  physx_argb_black = NX_ARGB_BLACK,
};

enum PhysxDeviceCode {
  physx_dc_ppu_0 = NX_DC_PPU_0,
  physx_dc_ppu_1 = NX_DC_PPU_1,
  physx_dc_ppu_2 = NX_DC_PPU_2,
  physx_dc_ppu_3 = NX_DC_PPU_3,
  physx_dc_ppu_4 = NX_DC_PPU_4,
  physx_dc_ppu_5 = NX_DC_PPU_5,
  physx_dc_ppu_6 = NX_DC_PPU_6,
  physx_dc_ppu_7 = NX_DC_PPU_7,
  physx_dc_ppu_8 = NX_DC_PPU_8,
  physx_dc_cpu = NX_DC_CPU,
  physx_dc_ppu_auto_assign = NX_DC_PPU_AUTO_ASSIGN,
};

enum PhysxDistanceJointFlag {
  physx_djf_max_distance_enabled = NX_DJF_MAX_DISTANCE_ENABLED,
  physx_djf_min_distance_enabled = NX_DJF_MIN_DISTANCE_ENABLED,
  physx_djf_spring_enabled = NX_DJF_SPRING_ENABLED,
};

enum PhysxEffectorType {
  physx_effector_spring_and_damper = NX_EFFECTOR_SPRING_AND_DAMPER,
};

enum PhysxEmitterShape {
  physx_fe_ellipse = NX_FE_ELLIPSE,
  physx_fe_rectangular = NX_FE_RECTANGULAR,
};

enum PhysxEmitterType {
  physx_fe_constant_flow_rate = NX_FE_CONSTANT_FLOW_RATE,
  physx_fe_constant_pressure = NX_FE_CONSTANT_PRESSURE,
};

enum PhysxErrorCode {
  physxe_invalid_parameter = NXE_INVALID_PARAMETER,
  physxe_db_warning = NXE_DB_WARNING,
  physxe_out_of_memory = NXE_OUT_OF_MEMORY,
  physxe_internal_error = NXE_INTERNAL_ERROR,
  physxe_invalid_operation = NXE_INVALID_OPERATION,
  physxe_db_info = NXE_DB_INFO,
  physxe_assertion = NXE_ASSERTION,
  physxe_no_error = NXE_NO_ERROR,
  physxe_db_print = NXE_DB_PRINT,
};

enum PhysxFilterOp {
  physx_filterop_and = NX_FILTEROP_AND,
  physx_filterop_or = NX_FILTEROP_OR,
  physx_filterop_xor = NX_FILTEROP_XOR,
  physx_filterop_nand = NX_FILTEROP_NAND,
  physx_filterop_nor = NX_FILTEROP_NOR,
  physx_filterop_nxor = NX_FILTEROP_NXOR,
  physx_filterop_swap_and = NX_FILTEROP_SWAP_AND,
};

enum PhysxFluidCollisionMethod {
  physx_f_static = NX_F_STATIC,
  physx_f_dynamic = NX_F_DYNAMIC,
};

enum PhysxFluidDescType {
  physx_fdt_default = NX_FDT_DEFAULT,
  physx_fdt_allocator = NX_FDT_ALLOCATOR,
};

enum PhysxFluidEmitterEventType {
  physx_feet_emitter_empty = NX_FEET_EMITTER_EMPTY,
};

enum PhysxFluidEmitterFlag {
  physx_fef_visualization = NX_FEF_VISUALIZATION,
  physx_fef_add_body_velocity = NX_FEF_ADD_BODY_VELOCITY,
  physx_fef_enabled = NX_FEF_ENABLED,
  physx_fef_force_on_body = NX_FEF_FORCE_ON_BODY,
};

enum PhysxFluidEventType {
  physx_fet_no_particles_left = NX_FET_NO_PARTICLES_LEFT,
};

enum PhysxFluidFlag {
  physx_ff_visualization = NX_FF_VISUALIZATION,
  physx_ff_disable_gravity = NX_FF_DISABLE_GRAVITY,
  physx_ff_hardware = NX_FF_HARDWARE,
  physx_ff_priority_mode = NX_FF_PRIORITY_MODE,
  physx_ff_collision_twoway = NX_FF_COLLISION_TWOWAY,
  physx_ff_enabled = NX_FF_ENABLED,
};

enum PhysxFluidSimulationMethod {
  physx_f_sph = NX_F_SPH,
  physx_f_mixed_mode = NX_F_MIXED_MODE,
  physx_f_no_particle_interaction = NX_F_NO_PARTICLE_INTERACTION,
};

enum PhysxForceFieldCoordinates {
  physx_ffc_cartesian = NX_FFC_CARTESIAN,
  physx_ffc_spherical = NX_FFC_SPHERICAL,
  physx_ffc_cylindrical = NX_FFC_CYLINDRICAL,
  physx_ffc_toroidal = NX_FFC_TOROIDAL,
};

enum PhysxForceFieldFlags {
  physx_fff_ignore_rigidbody_mass = NX_FFF_IGNORE_RIGIDBODY_MASS,
  physx_fff_ignore_fluid_mass = NX_FFF_IGNORE_FLUID_MASS,
  physx_fff_legacy_force = NX_FFF_LEGACY_FORCE,
  physx_fff_ignore_cloth_mass = NX_FFF_IGNORE_CLOTH_MASS,
  physx_fff_ignore_softbody_mass = NX_FFF_IGNORE_SOFTBODY_MASS,
};

enum PhysxForceFieldShapeFlags {
  physx_ffs_exclude = NX_FFS_EXCLUDE,
};

enum PhysxForceFieldType {
  physx_ff_type_force = NX_FF_TYPE_FORCE,
  physx_ff_type_acceleration = NX_FF_TYPE_ACCELERATION,
};

enum PhysxForceMode {
  physx_force = NX_FORCE,
  physx_impulse = NX_IMPULSE,
  physx_velocity_change = NX_VELOCITY_CHANGE,
  physx_smooth_impulse = NX_SMOOTH_IMPULSE,
  physx_smooth_velocity_change = NX_SMOOTH_VELOCITY_CHANGE,
  physx_acceleration = NX_ACCELERATION,
};

enum PhysxHWVersion {
  physx_hw_version_none = NX_HW_VERSION_NONE,
  physx_hw_version_athena_1_0 = NX_HW_VERSION_ATHENA_1_0,
};

enum PhysxHeightFieldAxis {
  physx_not_heightfield = NX_NOT_HEIGHTFIELD,
  physx_z = NX_Z,
  physx_y = NX_Y,
  physx_x = NX_X,
};

enum PhysxHeightFieldFlags {
  physx_hf_no_boundary_edges = NX_HF_NO_BOUNDARY_EDGES,
};

enum PhysxHeightFieldFormat {
  physx_hf_s16_tm = NX_HF_S16_TM,
};

enum PhysxHeightFieldTessFlag {
  physx_hf_0th_vertex_shared = NX_HF_0TH_VERTEX_SHARED,
};

enum PhysxInternalArray {
  physx_array_triangles = NX_ARRAY_TRIANGLES,
  physx_array_vertices = NX_ARRAY_VERTICES,
  physx_array_normals = NX_ARRAY_NORMALS,
  physx_array_hull_vertices = NX_ARRAY_HULL_VERTICES,
  physx_array_hull_polygons = NX_ARRAY_HULL_POLYGONS,
};

enum PhysxInternalFormat {
  physx_format_nodata = NX_FORMAT_NODATA,
  physx_format_float = NX_FORMAT_FLOAT,
  physx_format_byte = NX_FORMAT_BYTE,
  physx_format_short = NX_FORMAT_SHORT,
  physx_format_int = NX_FORMAT_INT,
};

enum PhysxJointFlag {
  physx_jf_collision_enabled = NX_JF_COLLISION_ENABLED,
  physx_jf_visualization = NX_JF_VISUALIZATION,
};

enum PhysxJointProjectionMode {
  physx_jpm_linear_mindist = NX_JPM_LINEAR_MINDIST,
  physx_jpm_point_mindist = NX_JPM_POINT_MINDIST,
  physx_jpm_none = NX_JPM_NONE,
};

enum PhysxJointState {
  physx_js_unbound = NX_JS_UNBOUND,
  physx_js_simulating = NX_JS_SIMULATING,
  physx_js_broken = NX_JS_BROKEN,
};

enum PhysxJointType {
  physx_joint_prismatic = NX_JOINT_PRISMATIC,
  physx_joint_revolute = NX_JOINT_REVOLUTE,
  physx_joint_cylindrical = NX_JOINT_CYLINDRICAL,
  physx_joint_spherical = NX_JOINT_SPHERICAL,
  physx_joint_point_on_line = NX_JOINT_POINT_ON_LINE,
  physx_joint_point_in_plane = NX_JOINT_POINT_IN_PLANE,
  physx_joint_distance = NX_JOINT_DISTANCE,
  physx_joint_pulley = NX_JOINT_PULLEY,
  physx_joint_fixed = NX_JOINT_FIXED,
  physx_joint_d6 = NX_JOINT_D6,
  physx_joint_count = NX_JOINT_COUNT,
  physx_joint_force_dword = NX_JOINT_FORCE_DWORD,
};

enum PhysxMaterialFlag {
  physx_mf_disable_friction = NX_MF_DISABLE_FRICTION,
  physx_mf_disable_strong_friction = NX_MF_DISABLE_STRONG_FRICTION,
  physx_mf_anisotropic = NX_MF_ANISOTROPIC,
};

enum PhysxMatrixType {
  physx_zero_matrix = NX_ZERO_MATRIX,
  physx_identity_matrix = NX_IDENTITY_MATRIX,
};

enum PhysxMeshDataDirtyBufferFlags {
  physx_mdf_vertices_pos_dirty = NX_MDF_VERTICES_POS_DIRTY,
  physx_mdf_parent_indices_dirty = NX_MDF_PARENT_INDICES_DIRTY,
  physx_mdf_vertices_normal_dirty = NX_MDF_VERTICES_NORMAL_DIRTY,
  physx_mdf_indices_dirty = NX_MDF_INDICES_DIRTY,
};

enum PhysxMeshDataFlags {
  physx_mdf_16_bit_indices = NX_MDF_16_BIT_INDICES,
};

enum PhysxMeshFlags {
  physx_mf_16_bit_indices = NX_MF_16_BIT_INDICES,
  physx_mf_flipnormals = NX_MF_FLIPNORMALS,
  physx_mf_hardware_mesh = NX_MF_HARDWARE_MESH,
};

enum PhysxMeshPagingMode {
  physx_mesh_paging_manual = NX_MESH_PAGING_MANUAL,
  physx_mesh_paging_fallback = NX_MESH_PAGING_FALLBACK,
  physx_mesh_paging_auto = NX_MESH_PAGING_AUTO,
};

enum PhysxMeshShapeFlag {
  physx_mesh_double_sided = NX_MESH_DOUBLE_SIDED,
  physx_mesh_smooth_sphere_collisions = NX_MESH_SMOOTH_SPHERE_COLLISIONS,
};

enum PhysxParameter {
  physx_visualize_collision_edges = NX_VISUALIZE_COLLISION_EDGES,
  physx_params_num_values = NX_PARAMS_NUM_VALUES,
  physx_visualize_collision_dynamic = NX_VISUALIZE_COLLISION_DYNAMIC,
  physx_visualize_contact_error = NX_VISUALIZE_CONTACT_ERROR,
  physx_visualize_fluid_motion_limit = NX_VISUALIZE_FLUID_MOTION_LIMIT,
  physx_visualize_cloth_workpackets = NX_VISUALIZE_CLOTH_WORKPACKETS,
  physx_visualize_fluid_velocity = NX_VISUALIZE_FLUID_VELOCITY,
  physx_visualize_fluid_mesh_packets = NX_VISUALIZE_FLUID_MESH_PACKETS,
  physx_visualize_body_axes = NX_VISUALIZE_BODY_AXES,
  physx_visualize_softbody_sleep = NX_VISUALIZE_SOFTBODY_SLEEP,
  physx_visualize_softbody_attachment = NX_VISUALIZE_SOFTBODY_ATTACHMENT,
  physx_visualize_cloth_collisions = NX_VISUALIZE_CLOTH_COLLISIONS,
  physx_continuous_cd = NX_CONTINUOUS_CD,
  physx_visualize_collision_aabbs = NX_VISUALIZE_COLLISION_AABBS,
  physx_visualize_joint_world_axes = NX_VISUALIZE_JOINT_WORLD_AXES,
  physx_visualize_fluid_drains = NX_VISUALIZE_FLUID_DRAINS,
  physx_default_sleep_energy = NX_DEFAULT_SLEEP_ENERGY,
  physx_visualize_cloth_mesh = NX_VISUALIZE_CLOTH_MESH,
  physx_penalty_force = NX_PENALTY_FORCE,
  physx_visualize_contact_normal = NX_VISUALIZE_CONTACT_NORMAL,
  physx_implicit_sweep_cache_size = NX_IMPLICIT_SWEEP_CACHE_SIZE,
  physx_visualize_softbody_tearing = NX_VISUALIZE_SOFTBODY_TEARING,
  physx_visualize_collision_ccd = NX_VISUALIZE_COLLISION_CCD,
  physx_bounce_threshold = NX_BOUNCE_THRESHOLD,
  physx_visualize_collision_spheres = NX_VISUALIZE_COLLISION_SPHERES,
  physx_visualize_collision_axes = NX_VISUALIZE_COLLISION_AXES,
  physx_visualize_fluid_bounds = NX_VISUALIZE_FLUID_BOUNDS,
  physx_visualize_joint_limits = NX_VISUALIZE_JOINT_LIMITS,
  physx_visualize_force_fields = NX_VISUALIZE_FORCE_FIELDS,
  physx_visualize_cloth_validbounds = NX_VISUALIZE_CLOTH_VALIDBOUNDS,
  physx_default_sleep_lin_vel_squared = NX_DEFAULT_SLEEP_LIN_VEL_SQUARED,
  physx_visualize_cloth_selfcollisions = NX_VISUALIZE_CLOTH_SELFCOLLISIONS,
  physx_visualize_softbody_collisions = NX_VISUALIZE_SOFTBODY_COLLISIONS,
  physx_constant_fluid_max_particles_per_step = NX_CONSTANT_FLUID_MAX_PARTICLES_PER_STEP,
  physx_visualize_contact_force = NX_VISUALIZE_CONTACT_FORCE,
  physx_sta_frict_scaling = NX_STA_FRICT_SCALING,
  physx_default_sleep_ang_vel_squared = NX_DEFAULT_SLEEP_ANG_VEL_SQUARED,
  physx_ccd_epsilon = NX_CCD_EPSILON,
  physx_asynchronous_mesh_creation = NX_ASYNCHRONOUS_MESH_CREATION,
  physx_visualize_world_axes = NX_VISUALIZE_WORLD_AXES,
  physx_visualize_active_vertices = NX_VISUALIZE_ACTIVE_VERTICES,
  physx_select_hw_algo = NX_SELECT_HW_ALGO,
  physx_visualize_cloth_sleep_vertex = NX_VISUALIZE_CLOTH_SLEEP_VERTEX,
  physx_visualize_cloth_attachment = NX_VISUALIZE_CLOTH_ATTACHMENT,
  physx_visualize_fluid_stc_collision = NX_VISUALIZE_FLUID_STC_COLLISION,
  physx_visualization_scale = NX_VISUALIZATION_SCALE,
  physx_visualize_cloth_tearable_vertices = NX_VISUALIZE_CLOTH_TEARABLE_VERTICES,
  physx_visualize_cloth_sleep = NX_VISUALIZE_CLOTH_SLEEP,
  physx_visualize_collision_fnormals = NX_VISUALIZE_COLLISION_FNORMALS,
  physx_visualize_body_mass_axes = NX_VISUALIZE_BODY_MASS_AXES,
  physx_visualize_body_ang_velocity = NX_VISUALIZE_BODY_ANG_VELOCITY,
  physx_visualize_fluid_packet_data = NX_VISUALIZE_FLUID_PACKET_DATA,
  physx_visualize_fluid_kernel_radius = NX_VISUALIZE_FLUID_KERNEL_RADIUS,
  physx_bbox_noise_level = NX_BBOX_NOISE_LEVEL,
  physx_skin_width = NX_SKIN_WIDTH,
  physx_visualize_body_lin_velocity = NX_VISUALIZE_BODY_LIN_VELOCITY,
  physx_visualize_fluid_dyn_collision = NX_VISUALIZE_FLUID_DYN_COLLISION,
  physx_visualize_actor_axes = NX_VISUALIZE_ACTOR_AXES,
  physx_visualize_collision_free = NX_VISUALIZE_COLLISION_FREE,
  physx_visualize_cloth_tearing = NX_VISUALIZE_CLOTH_TEARING,
  physx_coll_veto_jointed = NX_COLL_VETO_JOINTED,
  physx_visualize_softbody_tearable_vertices = NX_VISUALIZE_SOFTBODY_TEARABLE_VERTICES,
  physx_visualize_collision_shapes = NX_VISUALIZE_COLLISION_SHAPES,
  physx_trigger_trigger_callback = NX_TRIGGER_TRIGGER_CALLBACK,
  physx_visualize_fluid_packets = NX_VISUALIZE_FLUID_PACKETS,
  physx_visualize_contact_point = NX_VISUALIZE_CONTACT_POINT,
  physx_max_angular_velocity = NX_MAX_ANGULAR_VELOCITY,
  physx_dyn_frict_scaling = NX_DYN_FRICT_SCALING,
  physx_visualize_softbody_validbounds = NX_VISUALIZE_SOFTBODY_VALIDBOUNDS,
  physx_visualize_softbody_sleep_vertex = NX_VISUALIZE_SOFTBODY_SLEEP_VERTEX,
  physx_visualize_softbody_workpackets = NX_VISUALIZE_SOFTBODY_WORKPACKETS,
  physx_adaptive_force = NX_ADAPTIVE_FORCE,
  physx_solver_convergence_threshold = NX_SOLVER_CONVERGENCE_THRESHOLD,
  physx_visualize_fluid_position = NX_VISUALIZE_FLUID_POSITION,
  physx_visualize_softbody_mesh = NX_VISUALIZE_SOFTBODY_MESH,
  physx_visualize_joint_local_axes = NX_VISUALIZE_JOINT_LOCAL_AXES,
  physx_visualize_body_joint_groups = NX_VISUALIZE_BODY_JOINT_GROUPS,
  physx_visualize_collision_sap = NX_VISUALIZE_COLLISION_SAP,
  physx_visualize_collision_static = NX_VISUALIZE_COLLISION_STATIC,
  physx_visualize_fluid_emitters = NX_VISUALIZE_FLUID_EMITTERS,
  physx_constant_fluid_max_packets = NX_CONSTANT_FLUID_MAX_PACKETS,
  physx_visualize_collision_vnormals = NX_VISUALIZE_COLLISION_VNORMALS,
  physx_params_force_dword = NX_PARAMS_FORCE_DWORD,
  physx_visualize_collision_skeletons = NX_VISUALIZE_COLLISION_SKELETONS,
  physx_visualize_collision_compounds = NX_VISUALIZE_COLLISION_COMPOUNDS,
};

enum PhysxParticleDataFlag {
  physx_fp_delete = NX_FP_DELETE,
};

enum PhysxParticleFlag {
  physx_fp_separated = NX_FP_SEPARATED,
  physx_fp_motion_limit_reached = NX_FP_MOTION_LIMIT_REACHED,
  physx_fp_collision_with_dynamic = NX_FP_COLLISION_WITH_DYNAMIC,
  physx_fp_collision_with_static = NX_FP_COLLISION_WITH_STATIC,
};

enum PhysxProfileZoneName {
  physx_pz_client_frame = NX_PZ_CLIENT_FRAME,
  physx_pz_cpu_simulate = NX_PZ_CPU_SIMULATE,
  physx_pz_ppu0_simulate = NX_PZ_PPU0_SIMULATE,
  physx_pz_ppu1_simulate = NX_PZ_PPU1_SIMULATE,
  physx_pz_ppu2_simulate = NX_PZ_PPU2_SIMULATE,
  physx_pz_ppu3_simulate = NX_PZ_PPU3_SIMULATE,
  physx_pz_total_simulation = NX_PZ_TOTAL_SIMULATION,
};

enum PhysxPruningStructure {
  physx_pruning_none = NX_PRUNING_NONE,
  physx_pruning_octree = NX_PRUNING_OCTREE,
  physx_pruning_quadtree = NX_PRUNING_QUADTREE,
  physx_pruning_dynamic_aabb_tree = NX_PRUNING_DYNAMIC_AABB_TREE,
  physx_pruning_static_aabb_tree = NX_PRUNING_STATIC_AABB_TREE,
};

enum PhysxPulleyJointFlag {
  physx_pjf_is_rigid = NX_PJF_IS_RIGID,
  physx_pjf_motor_enabled = NX_PJF_MOTOR_ENABLED,
};

enum PhysxQueryFlags {
  physx_query_world_space = NX_QUERY_WORLD_SPACE,
  physx_query_first_contact = NX_QUERY_FIRST_CONTACT,
};

enum PhysxQueryReportResult {
  physx_sqr_continue = NX_SQR_CONTINUE,
  physx_sqr_abort_query = NX_SQR_ABORT_QUERY,
  physx_sqr_abort_all_queries = NX_SQR_ABORT_ALL_QUERIES,
  physx_sqr_force_dword = NX_SQR_FORCE_DWORD,
};

enum PhysxRaycastBit {
  physx_raycast_impact = NX_RAYCAST_IMPACT,
  physx_raycast_distance = NX_RAYCAST_DISTANCE,
  physx_raycast_face_index = NX_RAYCAST_FACE_INDEX,
  physx_raycast_shape = NX_RAYCAST_SHAPE,
  physx_raycast_normal = NX_RAYCAST_NORMAL,
  physx_raycast_uv = NX_RAYCAST_UV,
  physx_raycast_material = NX_RAYCAST_MATERIAL,
  physx_raycast_face_normal = NX_RAYCAST_FACE_NORMAL,
};

enum PhysxRemoteDebuggerObjectType {
  physx_dbg_objecttype_contact = NX_DBG_OBJECTTYPE_CONTACT,
  physx_dbg_objecttype_actor = NX_DBG_OBJECTTYPE_ACTOR,
  physx_dbg_objecttype_boundingbox = NX_DBG_OBJECTTYPE_BOUNDINGBOX,
  physx_dbg_objecttype_cloth = NX_DBG_OBJECTTYPE_CLOTH,
  physx_dbg_objecttype_capsule = NX_DBG_OBJECTTYPE_CAPSULE,
  physx_dbg_objecttype_cylinder = NX_DBG_OBJECTTYPE_CYLINDER,
  physx_dbg_objecttype_plane = NX_DBG_OBJECTTYPE_PLANE,
  physx_dbg_objecttype_mesh = NX_DBG_OBJECTTYPE_MESH,
  physx_dbg_objecttype_box = NX_DBG_OBJECTTYPE_BOX,
  physx_dbg_objecttype_fluid = NX_DBG_OBJECTTYPE_FLUID,
  physx_dbg_objecttype_convex = NX_DBG_OBJECTTYPE_CONVEX,
  physx_dbg_objecttype_generic = NX_DBG_OBJECTTYPE_GENERIC,
  physx_dbg_objecttype_sphere = NX_DBG_OBJECTTYPE_SPHERE,
  physx_dbg_objecttype_joint = NX_DBG_OBJECTTYPE_JOINT,
  physx_dbg_objecttype_softbody = NX_DBG_OBJECTTYPE_SOFTBODY,
  physx_dbg_objecttype_vector = NX_DBG_OBJECTTYPE_VECTOR,
  physx_dbg_objecttype_wheel = NX_DBG_OBJECTTYPE_WHEEL,
  physx_dbg_objecttype_camera = NX_DBG_OBJECTTYPE_CAMERA,
};

enum PhysxRevoluteJointFlag {
  physx_rjf_motor_enabled = NX_RJF_MOTOR_ENABLED,
  physx_rjf_limit_enabled = NX_RJF_LIMIT_ENABLED,
  physx_rjf_spring_enabled = NX_RJF_SPRING_ENABLED,
};

enum PhysxSDKCreateError {
  physxce_wrong_version = NXCE_WRONG_VERSION,
  physxce_bundle_error = NXCE_BUNDLE_ERROR,
  physxce_physx_not_found = NXCE_PHYSX_NOT_FOUND,
  physxce_reset_error = NXCE_RESET_ERROR,
  physxce_in_use_error = NXCE_IN_USE_ERROR,
  physxce_connection_error = NXCE_CONNECTION_ERROR,
  physxce_no_error = NXCE_NO_ERROR,
  physxce_descriptor_invalid = NXCE_DESCRIPTOR_INVALID,
};

enum PhysxSDKCreationFlag {
  physx_sdkf_no_hardware = NX_SDKF_NO_HARDWARE,
};

enum PhysxSceneFlags {
  physx_sf_force_cone_friction = NX_SF_FORCE_CONE_FRICTION,
  physx_sf_disable_collisions = NX_SF_DISABLE_COLLISIONS,
  physx_sf_restricted_scene = NX_SF_RESTRICTED_SCENE,
  physx_sf_fluid_performance_hint = NX_SF_FLUID_PERFORMANCE_HINT,
  physx_sf_simulate_separate_thread = NX_SF_SIMULATE_SEPARATE_THREAD,
  physx_sf_sequential_primary = NX_SF_SEQUENTIAL_PRIMARY,
  physx_sf_disable_scene_mutex = NX_SF_DISABLE_SCENE_MUTEX,
  physx_sf_enable_multithread = NX_SF_ENABLE_MULTITHREAD,
  physx_sf_disable_sse = NX_SF_DISABLE_SSE,
  physx_sf_enable_activetransforms = NX_SF_ENABLE_ACTIVETRANSFORMS,
};

enum PhysxSceneQueryExecuteMode {
  physx_sqe_synchronous = NX_SQE_SYNCHRONOUS,
  physx_sqe_asynchronous = NX_SQE_ASYNCHRONOUS,
  physx_sqe_force_dword = NX_SQE_FORCE_DWORD,
};

enum PhysxSepAxis {
  physx_sep_axis_overlap = NX_SEP_AXIS_OVERLAP,
  physx_sep_axis_a0 = NX_SEP_AXIS_A0,
  physx_sep_axis_a1 = NX_SEP_AXIS_A1,
  physx_sep_axis_a2 = NX_SEP_AXIS_A2,
  physx_sep_axis_b0 = NX_SEP_AXIS_B0,
  physx_sep_axis_b1 = NX_SEP_AXIS_B1,
  physx_sep_axis_b2 = NX_SEP_AXIS_B2,
  physx_sep_axis_a0_cross_b0 = NX_SEP_AXIS_A0_CROSS_B0,
  physx_sep_axis_a0_cross_b1 = NX_SEP_AXIS_A0_CROSS_B1,
  physx_sep_axis_a0_cross_b2 = NX_SEP_AXIS_A0_CROSS_B2,
  physx_sep_axis_a1_cross_b0 = NX_SEP_AXIS_A1_CROSS_B0,
  physx_sep_axis_a1_cross_b1 = NX_SEP_AXIS_A1_CROSS_B1,
  physx_sep_axis_a1_cross_b2 = NX_SEP_AXIS_A1_CROSS_B2,
  physx_sep_axis_a2_cross_b0 = NX_SEP_AXIS_A2_CROSS_B0,
  physx_sep_axis_a2_cross_b1 = NX_SEP_AXIS_A2_CROSS_B1,
  physx_sep_axis_a2_cross_b2 = NX_SEP_AXIS_A2_CROSS_B2,
  physx_sep_axis_force_dword = NX_SEP_AXIS_FORCE_DWORD,
};

enum PhysxShapeFlag {
  physx_sf_disable_response = NX_SF_DISABLE_RESPONSE,
  physx_sf_softbody_drain = NX_SF_SOFTBODY_DRAIN,
  physx_sf_cloth_drain = NX_SF_CLOTH_DRAIN,
  physx_trigger_on_enter = NX_TRIGGER_ON_ENTER,
  physx_sf_point_contact_force = NX_SF_POINT_CONTACT_FORCE,
  physx_sf_softbody_disable_collision = NX_SF_SOFTBODY_DISABLE_COLLISION,
  physx_sf_feature_indices = NX_SF_FEATURE_INDICES,
  physx_sf_cloth_twoway = NX_SF_CLOTH_TWOWAY,
  physx_sf_visualization = NX_SF_VISUALIZATION,
  physx_trigger_on_stay = NX_TRIGGER_ON_STAY,
  physx_sf_cloth_disable_collision = NX_SF_CLOTH_DISABLE_COLLISION,
  physx_sf_fluid_twoway = NX_SF_FLUID_TWOWAY,
  physx_trigger_on_leave = NX_TRIGGER_ON_LEAVE,
  physx_sf_disable_scene_queries = NX_SF_DISABLE_SCENE_QUERIES,
  physx_sf_disable_raycasting = NX_SF_DISABLE_RAYCASTING,
  physx_sf_dynamic_dynamic_ccd = NX_SF_DYNAMIC_DYNAMIC_CCD,
  physx_sf_disable_collision = NX_SF_DISABLE_COLLISION,
  physx_sf_softbody_twoway = NX_SF_SOFTBODY_TWOWAY,
  physx_sf_fluid_disable_collision = NX_SF_FLUID_DISABLE_COLLISION,
  physx_sf_fluid_drain = NX_SF_FLUID_DRAIN,
};

enum PhysxShapePairStreamFlags {
  physx_sf_has_mats_per_point = NX_SF_HAS_MATS_PER_POINT,
  physx_sf_has_features_per_point = NX_SF_HAS_FEATURES_PER_POINT,
};

enum PhysxShapeType {
  physx_shape_plane = NX_SHAPE_PLANE,
  physx_shape_sphere = NX_SHAPE_SPHERE,
  physx_shape_box = NX_SHAPE_BOX,
  physx_shape_capsule = NX_SHAPE_CAPSULE,
  physx_shape_wheel = NX_SHAPE_WHEEL,
  physx_shape_convex = NX_SHAPE_CONVEX,
  physx_shape_mesh = NX_SHAPE_MESH,
  physx_shape_heightfield = NX_SHAPE_HEIGHTFIELD,
  physx_shape_raw_mesh = NX_SHAPE_RAW_MESH,
  physx_shape_compound = NX_SHAPE_COMPOUND,
  physx_shape_count = NX_SHAPE_COUNT,
  physx_shape_force_dword = NX_SHAPE_FORCE_DWORD,
};

enum PhysxShapesType {
  physx_dynamic_shapes = NX_DYNAMIC_SHAPES,
  physx_static_shapes = NX_STATIC_SHAPES,
};

enum PhysxSimulationStatus {
  physx_rigid_body_finished = NX_RIGID_BODY_FINISHED,
  physx_all_finished = NX_ALL_FINISHED,
  physx_primary_finished = NX_PRIMARY_FINISHED,
};

enum PhysxSimulationType {
  physx_simulation_hw = NX_SIMULATION_HW,
  physx_sty_force_dword = NX_STY_FORCE_DWORD,
  physx_simulation_sw = NX_SIMULATION_SW,
};

enum PhysxSoftBodyAttachmentFlag {
  physx_softbody_attachment_tearable = NX_SOFTBODY_ATTACHMENT_TEARABLE,
  physx_softbody_attachment_twoway = NX_SOFTBODY_ATTACHMENT_TWOWAY,
};

enum PhysxSoftBodyFlag {
  physx_sbf_visualization = NX_SBF_VISUALIZATION,
  physx_sbf_fluid_collision = NX_SBF_FLUID_COLLISION,
  physx_sbf_static = NX_SBF_STATIC,
  physx_sbf_collision_twoway = NX_SBF_COLLISION_TWOWAY,
  physx_sbf_disable_collision = NX_SBF_DISABLE_COLLISION,
  physx_sbf_validbounds = NX_SBF_VALIDBOUNDS,
  physx_sbf_tearable = NX_SBF_TEARABLE,
  physx_sbf_volume_conservation = NX_SBF_VOLUME_CONSERVATION,
  physx_sbf_selfcollision = NX_SBF_SELFCOLLISION,
  physx_sbf_gravity = NX_SBF_GRAVITY,
  physx_sbf_comdamping = NX_SBF_COMDAMPING,
  physx_sbf_hardware = NX_SBF_HARDWARE,
  physx_sbf_damping = NX_SBF_DAMPING,
};

enum PhysxSoftBodyMeshFlags {
  physx_softbody_mesh_16_bit_indices = NX_SOFTBODY_MESH_16_BIT_INDICES,
  physx_softbody_mesh_tearable = NX_SOFTBODY_MESH_TEARABLE,
};

enum PhysxSoftBodyVertexFlags {
  physx_softbody_vertex_tearable = NX_SOFTBODY_VERTEX_TEARABLE,
};

enum PhysxSphericalJointFlag {
  physx_sjf_swing_spring_enabled = NX_SJF_SWING_SPRING_ENABLED,
  physx_sjf_joint_spring_enabled = NX_SJF_JOINT_SPRING_ENABLED,
  physx_sjf_twist_limit_enabled = NX_SJF_TWIST_LIMIT_ENABLED,
  physx_sjf_twist_spring_enabled = NX_SJF_TWIST_SPRING_ENABLED,
  physx_sjf_swing_limit_enabled = NX_SJF_SWING_LIMIT_ENABLED,
};

enum PhysxStandardFences {
  physx_fence_run_finished = NX_FENCE_RUN_FINISHED,
  physx_num_standard_fences = NX_NUM_STANDARD_FENCES,
};

enum PhysxSweepFlags {
  physx_sf_async = NX_SF_ASYNC,
  physx_sf_debug_sm = NX_SF_DEBUG_SM,
  physx_sf_dynamics = NX_SF_DYNAMICS,
  physx_sf_statics = NX_SF_STATICS,
  physx_sf_debug_et = NX_SF_DEBUG_ET,
  physx_sf_all_hits = NX_SF_ALL_HITS,
};

enum PhysxThreadPollResult {
  physx_thread_shutdown = NX_THREAD_SHUTDOWN,
  physx_thread_force_dword = NX_THREAD_FORCE_DWORD,
  physx_thread_nowork = NX_THREAD_NOWORK,
  physx_thread_simulation_end = NX_THREAD_SIMULATION_END,
  physx_thread_morework = NX_THREAD_MOREWORK,
};

enum PhysxThreadPriority {
  physx_tp_above_normal = NX_TP_ABOVE_NORMAL,
  physx_tp_normal = NX_TP_NORMAL,
  physx_tp_high = NX_TP_HIGH,
  physx_tp_low = NX_TP_LOW,
  physx_tp_force_dword = NX_TP_FORCE_DWORD,
  physx_tp_below_normal = NX_TP_BELOW_NORMAL,
};

enum PhysxThreadWait {
  physx_wait_force_dword = NX_WAIT_FORCE_DWORD,
  physx_wait_shutdown = NX_WAIT_SHUTDOWN,
  physx_wait_simulation_end = NX_WAIT_SIMULATION_END,
  physx_wait_none = NX_WAIT_NONE,
};

enum PhysxTimeStepMethod {
  physx_timestep_fixed = NX_TIMESTEP_FIXED,
  physx_timestep_variable = NX_TIMESTEP_VARIABLE,
  physx_timestep_inherit = NX_TIMESTEP_INHERIT,
  physx_num_timestep_methods = NX_NUM_TIMESTEP_METHODS,
  physx_tsm_force_dword = NX_TSM_FORCE_DWORD,
};

enum PhysxTriangleFlags {
  physx_boundary_edge20 = NXTF_BOUNDARY_EDGE20,
  physx_double_sided = NXTF_DOUBLE_SIDED,
  physx_active_edge12 = NXTF_ACTIVE_EDGE12,
  physx_active_edge01 = NXTF_ACTIVE_EDGE01,
  physx_boundary_edge01 = NXTF_BOUNDARY_EDGE01,
  physx_boundary_edge12 = NXTF_BOUNDARY_EDGE12,
  physx_active_edge20 = NXTF_ACTIVE_EDGE20,
};



#endif // HAVE_PHYSX

#endif // PHYSX_ENUMERATIONS_H
