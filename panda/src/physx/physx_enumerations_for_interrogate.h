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
  physx_adt_shapeless,
  physx_adt_default,
  physx_adt_allocator,
  physx_adt_list,
  physx_adt_pointer,
};

enum PhysxActorFlag {
  physx_af_fluid_disable_collision = (1<<3),
  physx_af_contact_modification = (1<<4),
  physx_af_disable_collision = (1<<0),
  physx_af_disable_response = (1<<1),
  physx_af_lock_com = (1<<2),
  physx_af_user_actor_pair_filtering = (1<<6),
  physx_af_force_cone_friction = (1<<5),
};

enum PhysxAssertResponse {
  physx_ar_continue,
  physx_ar_ignore,
  physx_ar_breakpoint,
};

enum PhysxAxisType {
  physx_axis_plus_x,
  physx_axis_minus_x,
  physx_axis_plus_y,
  physx_axis_minus_y,
  physx_axis_plus_z,
  physx_axis_minus_z,
  physx_axis_arbitrary,
};

enum PhysxBSphereMethod {
  physx_bs_none,
  physx_bs_gems,
  physx_bs_miniball,
  physx_bs_force_dword = 0x7fffffff,
};

enum PhysxBodyFlag {
  physx_bf_frozen_rot_z = (1<<6),
  physx_bf_frozen_rot_y = (1<<5),
  physx_bf_frozen_rot_x = (1<<4),
  physx_bf_kinematic = (1<<7),
  physx_bf_pose_sleep_test = (1<<9),
  physx_bf_disable_gravity = (1<<0),
  physx_bf_frozen_pos_z = (1<<3),
  physx_bf_frozen_pos_x = (1<<1),
  physx_bf_frozen_pos_y = (1<<2),
  physx_bf_visualization = (1<<8),
  physx_bf_filter_sleep_vel = (1<<10),
  physx_bf_energy_sleep_test = (1<<11),
};

enum PhysxCapsuleShapeFlag {
  physx_swept_shape = (1<<0),
};

enum PhysxClothAttachmentFlag {
  physx_cloth_attachment_tearable = (1<<1),
  physx_cloth_attachment_twoway = (1<<0),
};

enum PhysxClothFlag {
  physx_clf_disable_collision = (1<<2),
  physx_clf_bending_ortho = (1<<7),
  physx_clf_bending = (1<<6),
  physx_clf_pressure = (1<<0),
  physx_clf_damping = (1<<8),
  physx_clf_fluid_collision = (1<<16),
  physx_clf_triangle_collision = (1<<11),
  physx_clf_visualization = (1<<4),
  physx_clf_collision_twoway = (1<<9),
  physx_clf_hardware = (1<<13),
  physx_clf_gravity = (1<<5),
  physx_clf_selfcollision = (1<<3),
  physx_clf_tearable = (1<<12),
  physx_clf_validbounds = (1<<15),
  physx_clf_static = (1<<1),
  physx_clf_comdamping = (1<<14),
};

enum PhysxClothMeshFlags {
  physx_cloth_mesh_tearable = (1<<8),
};

enum PhysxClothVertexAttachmentStatus {
  physx_cloth_vertex_attachment_none,
  physx_cloth_vertex_attachment_global,
  physx_cloth_vertex_attachment_shape,
};

enum PhysxClothVertexFlags {
  physx_cloth_vertex_attached = (1<<0),
  physx_cloth_vertex_tearable = (1<<7),
};

enum PhysxCombineMode {
  physx_cm_multiply = 2,
  physx_cm_average = 0,
  physx_cm_max = 3,
  physx_cm_pad_32 = 0xffffffff,
  physx_cm_n_values = 4,
  physx_cm_min = 1,
};

enum PhysxCompartmentFlag {
  physx_cf_sleep_notification = (1<<0),
  physx_cf_continuous_cd = (1<<1),
  physx_cf_inherit_settings = (1<<3),
  physx_cf_restricted_scene = (1<<2),
};

enum PhysxCompartmentType {
  physx_sct_cloth = 2,
  physx_sct_rigidbody = 0,
  physx_sct_softbody = 2,
  physx_sct_fluid = 1,
};

enum PhysxContactPairFlag {
  physx_ignore_pair = (1<<0),
  physx_notify_on_touch = (1<<3),
  physx_notify_on_end_touch = (1<<2),
  physx_notify_forces = (1<<7),
  physx_notify_on_slide = (1<<6),
  physx_notify_on_impact = (1<<4),
  physx_notify_contact_modification = (1<<16),
  physx_notify_on_roll = (1<<5),
  physx_notify_on_start_touch = (1<<1),
};

enum PhysxConvexFlags {
  physx_cf_use_uncompressed_normals = (1<<5),
  physx_cf_flipnormals = (1<<0),
  physx_cf_use_legacy_cooker = (1<<4),
  physx_cf_inflate_convex = (1<<3),
  physx_cf_16_bit_indices = (1<<1),
  physx_cf_compute_convex = (1<<2),
};

enum PhysxCookingValue {
  physx_cooking_convex_version_pc,
  physx_cooking_mesh_version_pc,
  physx_cooking_convex_version_xenon,
  physx_cooking_mesh_version_xenon,
  physx_cooking_convex_version_playstation3,
  physx_cooking_mesh_version_playstation3,
};

enum PhysxD6JointDriveType {
  physx_d6joint_drive_velocity = 1<<1,
  physx_d6joint_drive_position = 1<<0,
};

enum PhysxD6JointFlag {
  physx_d6joint_slerp_drive = 1<<0,
  physx_d6joint_gear_enabled = 1<<1,
};

enum PhysxD6JointMotion {
  physx_d6joint_motion_locked,
  physx_d6joint_motion_limited,
  physx_d6joint_motion_free,
};

enum PhysxDebugColor {
  physx_argb_white = 0xffffffff,
  physx_argb_green = 0xff00ff00,
  physx_argb_red = 0xffff0000,
  physx_argb_yellow = 0xffffff00,
  physx_argb_magenta = 0xffff00ff,
  physx_argb_blue = 0xff0000ff,
  physx_argb_cyan = 0xff00ffff,
  physx_argb_black = 0xff000000,
};

enum PhysxDeviceCode {
  physx_dc_ppu_0 = 0,
  physx_dc_ppu_1 = 1,
  physx_dc_ppu_2 = 2,
  physx_dc_ppu_3 = 3,
  physx_dc_ppu_4 = 4,
  physx_dc_ppu_5 = 5,
  physx_dc_ppu_6 = 6,
  physx_dc_ppu_7 = 7,
  physx_dc_ppu_8 = 8,
  physx_dc_cpu = 0xffff0000,
  physx_dc_ppu_auto_assign = 0xffff0001,
};

enum PhysxDistanceJointFlag {
  physx_djf_max_distance_enabled = 1 << 0,
  physx_djf_min_distance_enabled = 1 << 1,
  physx_djf_spring_enabled = 1 << 2,
};

enum PhysxEffectorType {
  physx_effector_spring_and_damper,
};

enum PhysxEmitterShape {
  physx_fe_ellipse = (1<<1),
  physx_fe_rectangular = (1<<0),
};

enum PhysxEmitterType {
  physx_fe_constant_flow_rate = (1<<1),
  physx_fe_constant_pressure = (1<<0),
};

enum PhysxErrorCode {
  physxe_invalid_parameter = 1,
  physxe_db_warning = 206,
  physxe_out_of_memory = 3,
  physxe_internal_error = 4,
  physxe_invalid_operation = 2,
  physxe_db_info = 205,
  physxe_assertion = 107,
  physxe_no_error = 0,
  physxe_db_print = 208,
};

enum PhysxFilterOp {
  physx_filterop_and,
  physx_filterop_or,
  physx_filterop_xor,
  physx_filterop_nand,
  physx_filterop_nor,
  physx_filterop_nxor,
  physx_filterop_swap_and,
};

enum PhysxFluidCollisionMethod {
  physx_f_static = (1<<0),
  physx_f_dynamic = (1<<1),
};

enum PhysxFluidDescType {
  physx_fdt_default,
  physx_fdt_allocator,
};

enum PhysxFluidEmitterEventType {
  physx_feet_emitter_empty,
};

enum PhysxFluidEmitterFlag {
  physx_fef_visualization = (1<<0),
  physx_fef_add_body_velocity = (1<<3),
  physx_fef_enabled = (1<<4),
  physx_fef_force_on_body = (1<<2),
};

enum PhysxFluidEventType {
  physx_fet_no_particles_left,
};

enum PhysxFluidFlag {
  physx_ff_visualization = (1<<0),
  physx_ff_disable_gravity = (1<<1),
  physx_ff_hardware = (1<<4),
  physx_ff_priority_mode = (1<<5),
  physx_ff_collision_twoway = (1<<2),
  physx_ff_enabled = (1<<3),
};

enum PhysxFluidSimulationMethod {
  physx_f_sph = (1<<0),
  physx_f_mixed_mode = (1<<2),
  physx_f_no_particle_interaction = (1<<1),
};

enum PhysxForceFieldCoordinates {
  physx_ffc_cartesian,
  physx_ffc_spherical,
  physx_ffc_cylindrical,
  physx_ffc_toroidal,
};

enum PhysxForceFieldFlags {
  physx_fff_ignore_rigidbody_mass = (1<<3),
  physx_fff_ignore_fluid_mass = (1<<0),
  physx_fff_legacy_force = (1<<4),
  physx_fff_ignore_cloth_mass = (1<<1),
  physx_fff_ignore_softbody_mass = (1<<2),
};

enum PhysxForceFieldShapeFlags {
  physx_ffs_exclude = (1<<0),
};

enum PhysxForceFieldType {
  physx_ff_type_force,
  physx_ff_type_acceleration,
};

enum PhysxForceMode {
  physx_force,
  physx_impulse,
  physx_velocity_change,
  physx_smooth_impulse,
  physx_smooth_velocity_change,
  physx_acceleration,
};

enum PhysxHWVersion {
  physx_hw_version_none = 0,
  physx_hw_version_athena_1_0 = 1,
};

enum PhysxHeightFieldAxis {
  physx_not_heightfield = 0xff,
  physx_z = 2,
  physx_y = 1,
  physx_x = 0,
};

enum PhysxHeightFieldFlags {
  physx_hf_no_boundary_edges = (1 << 0),
};

enum PhysxHeightFieldFormat {
  physx_hf_s16_tm = (1 << 0),
};

enum PhysxHeightFieldTessFlag {
  physx_hf_0th_vertex_shared = (1 << 0),
};

enum PhysxInternalArray {
  physx_array_triangles,
  physx_array_vertices,
  physx_array_normals,
  physx_array_hull_vertices,
  physx_array_hull_polygons,
};

enum PhysxInternalFormat {
  physx_format_nodata,
  physx_format_float,
  physx_format_byte,
  physx_format_short,
  physx_format_int,
};

enum PhysxJointFlag {
  physx_jf_collision_enabled = (1<<0),
  physx_jf_visualization = (1<<1),
};

enum PhysxJointProjectionMode {
  physx_jpm_linear_mindist = 2,
  physx_jpm_point_mindist = 1,
  physx_jpm_none = 0,
};

enum PhysxJointState {
  physx_js_unbound,
  physx_js_simulating,
  physx_js_broken,
};

enum PhysxJointType {
  physx_joint_prismatic,
  physx_joint_revolute,
  physx_joint_cylindrical,
  physx_joint_spherical,
  physx_joint_point_on_line,
  physx_joint_point_in_plane,
  physx_joint_distance,
  physx_joint_pulley,
  physx_joint_fixed,
  physx_joint_d6,
  physx_joint_count,
  physx_joint_force_dword = 0x7fffffff,
};

enum PhysxMaterialFlag {
  physx_mf_disable_friction = 1 << 4,
  physx_mf_disable_strong_friction = 1 << 5,
  physx_mf_anisotropic = 1 << 0,
};

enum PhysxMatrixType {
  physx_zero_matrix,
  physx_identity_matrix,
};

enum PhysxMeshDataDirtyBufferFlags {
  physx_mdf_vertices_pos_dirty = 1 << 0,
  physx_mdf_parent_indices_dirty = 1 << 3,
  physx_mdf_vertices_normal_dirty = 1 << 1,
  physx_mdf_indices_dirty = 1 << 2,
};

enum PhysxMeshDataFlags {
  physx_mdf_16_bit_indices = 1 << 0,
};

enum PhysxMeshFlags {
  physx_mf_16_bit_indices = (1<<1),
  physx_mf_flipnormals = (1<<0),
  physx_mf_hardware_mesh = (1<<2),
};

enum PhysxMeshPagingMode {
  physx_mesh_paging_manual,
  physx_mesh_paging_fallback,
  physx_mesh_paging_auto,
};

enum PhysxMeshShapeFlag {
  physx_mesh_double_sided = (1<<1),
  physx_mesh_smooth_sphere_collisions = (1<<0),
};

enum PhysxParameter {
  physx_visualize_collision_edges = 44,
  physx_params_num_values = 97,
  physx_visualize_collision_dynamic = 48,
  physx_visualize_contact_error = 35,
  physx_visualize_fluid_motion_limit = 58,
  physx_visualize_cloth_workpackets = 66,
  physx_visualize_fluid_velocity = 54,
  physx_visualize_fluid_mesh_packets = 61,
  physx_visualize_body_axes = 11,
  physx_visualize_softbody_sleep = 86,
  physx_visualize_softbody_attachment = 89,
  physx_visualize_cloth_collisions = 64,
  physx_continuous_cd = 8,
  physx_visualize_collision_aabbs = 38,
  physx_visualize_joint_world_axes = 28,
  physx_visualize_fluid_drains = 62,
  physx_default_sleep_energy = 77,
  physx_visualize_cloth_mesh = 63,
  physx_penalty_force = 0,
  physx_visualize_contact_normal = 34,
  physx_implicit_sweep_cache_size = 76,
  physx_visualize_softbody_tearing = 88,
  physx_visualize_collision_ccd = 50,
  physx_bounce_threshold = 4,
  physx_visualize_collision_spheres = 45,
  physx_visualize_collision_axes = 40,
  physx_visualize_fluid_bounds = 56,
  physx_visualize_joint_limits = 29,
  physx_visualize_force_fields = 91,
  physx_visualize_cloth_validbounds = 92,
  physx_default_sleep_lin_vel_squared = 2,
  physx_visualize_cloth_selfcollisions = 65,
  physx_visualize_softbody_collisions = 84,
  physx_constant_fluid_max_particles_per_step = 79,
  physx_visualize_contact_force = 36,
  physx_sta_frict_scaling = 6,
  physx_default_sleep_ang_vel_squared = 3,
  physx_ccd_epsilon = 73,
  physx_asynchronous_mesh_creation = 96,
  physx_visualize_world_axes = 10,
  physx_visualize_active_vertices = 72,
  physx_select_hw_algo = 71,
  physx_visualize_cloth_sleep_vertex = 94,
  physx_visualize_cloth_attachment = 82,
  physx_visualize_fluid_stc_collision = 60,
  physx_visualization_scale = 9,
  physx_visualize_cloth_tearable_vertices = 80,
  physx_visualize_cloth_sleep = 67,
  physx_visualize_collision_fnormals = 43,
  physx_visualize_body_mass_axes = 12,
  physx_visualize_body_ang_velocity = 14,
  physx_visualize_fluid_packet_data = 90,
  physx_visualize_fluid_kernel_radius = 55,
  physx_bbox_noise_level = 75,
  physx_skin_width = 1,
  physx_visualize_body_lin_velocity = 13,
  physx_visualize_fluid_dyn_collision = 59,
  physx_visualize_actor_axes = 37,
  physx_visualize_collision_free = 49,
  physx_visualize_cloth_tearing = 81,
  physx_coll_veto_jointed = 69,
  physx_visualize_softbody_tearable_vertices = 87,
  physx_visualize_collision_shapes = 39,
  physx_trigger_trigger_callback = 70,
  physx_visualize_fluid_packets = 57,
  physx_visualize_contact_point = 33,
  physx_max_angular_velocity = 7,
  physx_dyn_frict_scaling = 5,
  physx_visualize_softbody_validbounds = 93,
  physx_visualize_softbody_sleep_vertex = 95,
  physx_visualize_softbody_workpackets = 85,
  physx_adaptive_force = 68,
  physx_solver_convergence_threshold = 74,
  physx_visualize_fluid_position = 53,
  physx_visualize_softbody_mesh = 83,
  physx_visualize_joint_local_axes = 27,
  physx_visualize_body_joint_groups = 22,
  physx_visualize_collision_sap = 46,
  physx_visualize_collision_static = 47,
  physx_visualize_fluid_emitters = 52,
  physx_constant_fluid_max_packets = 78,
  physx_visualize_collision_vnormals = 42,
  physx_params_force_dword = 0x7fffffff,
  physx_visualize_collision_skeletons = 51,
  physx_visualize_collision_compounds = 41,
};

enum PhysxParticleDataFlag {
  physx_fp_delete = (1<<0),
};

enum PhysxParticleFlag {
  physx_fp_separated = (1<<2),
  physx_fp_motion_limit_reached = (1<<3),
  physx_fp_collision_with_dynamic = (1<<1),
  physx_fp_collision_with_static = (1<<0),
};

enum PhysxProfileZoneName {
  physx_pz_client_frame,
  physx_pz_cpu_simulate,
  physx_pz_ppu0_simulate,
  physx_pz_ppu1_simulate,
  physx_pz_ppu2_simulate,
  physx_pz_ppu3_simulate,
  physx_pz_total_simulation = 0x10,
};

enum PhysxPruningStructure {
  physx_pruning_none,
  physx_pruning_octree,
  physx_pruning_quadtree,
  physx_pruning_dynamic_aabb_tree,
  physx_pruning_static_aabb_tree,
};

enum PhysxPulleyJointFlag {
  physx_pjf_is_rigid = 1 << 0,
  physx_pjf_motor_enabled = 1 << 1,
};

enum PhysxQueryFlags {
  physx_query_world_space = (1<<0),
  physx_query_first_contact = (1<<1),
};

enum PhysxQueryReportResult {
  physx_sqr_continue,
  physx_sqr_abort_query,
  physx_sqr_abort_all_queries,
  physx_sqr_force_dword = 0x7fffffff,
};

enum PhysxRaycastBit {
  physx_raycast_impact = (1<<1),
  physx_raycast_distance = (1<<4),
  physx_raycast_face_index = (1<<3),
  physx_raycast_shape = (1<<0),
  physx_raycast_normal = (1<<2),
  physx_raycast_uv = (1<<5),
  physx_raycast_material = (1<<7),
  physx_raycast_face_normal = (1<<6),
};

enum PhysxRemoteDebuggerObjectType {
  physx_dbg_objecttype_contact = 11,
  physx_dbg_objecttype_actor = 1,
  physx_dbg_objecttype_boundingbox = 12,
  physx_dbg_objecttype_cloth = 15,
  physx_dbg_objecttype_capsule = 5,
  physx_dbg_objecttype_cylinder = 6,
  physx_dbg_objecttype_plane = 2,
  physx_dbg_objecttype_mesh = 8,
  physx_dbg_objecttype_box = 3,
  physx_dbg_objecttype_fluid = 17,
  physx_dbg_objecttype_convex = 7,
  physx_dbg_objecttype_generic = 0,
  physx_dbg_objecttype_sphere = 4,
  physx_dbg_objecttype_joint = 10,
  physx_dbg_objecttype_softbody = 16,
  physx_dbg_objecttype_vector = 13,
  physx_dbg_objecttype_wheel = 9,
  physx_dbg_objecttype_camera = 14,
};

enum PhysxRevoluteJointFlag {
  physx_rjf_motor_enabled = 1 << 1,
  physx_rjf_limit_enabled = 1 << 0,
  physx_rjf_spring_enabled = 1 << 2,
};

enum PhysxSDKCreateError {
  physxce_wrong_version = 2,
  physxce_bundle_error = 7,
  physxce_physx_not_found = 1,
  physxce_reset_error = 5,
  physxce_in_use_error = 6,
  physxce_connection_error = 4,
  physxce_no_error = 0,
  physxce_descriptor_invalid = 3,
};

enum PhysxSDKCreationFlag {
  physx_sdkf_no_hardware = (1<<0),
};

enum PhysxSceneFlags {
  physx_sf_force_cone_friction = 0x80,
  physx_sf_disable_collisions = 0x2,
  physx_sf_restricted_scene = 0x20,
  physx_sf_fluid_performance_hint = 0x80*4,
  physx_sf_simulate_separate_thread = 0x4,
  physx_sf_sequential_primary = 0x80*2,
  physx_sf_disable_scene_mutex = 0x40,
  physx_sf_enable_multithread = 0x8,
  physx_sf_disable_sse = 0x1,
  physx_sf_enable_activetransforms = 0x10,
};

enum PhysxSceneQueryExecuteMode {
  physx_sqe_synchronous,
  physx_sqe_asynchronous,
  physx_sqe_force_dword = 0x7fffffff,
};

enum PhysxSepAxis {
  physx_sep_axis_overlap,
  physx_sep_axis_a0,
  physx_sep_axis_a1,
  physx_sep_axis_a2,
  physx_sep_axis_b0,
  physx_sep_axis_b1,
  physx_sep_axis_b2,
  physx_sep_axis_a0_cross_b0,
  physx_sep_axis_a0_cross_b1,
  physx_sep_axis_a0_cross_b2,
  physx_sep_axis_a1_cross_b0,
  physx_sep_axis_a1_cross_b1,
  physx_sep_axis_a1_cross_b2,
  physx_sep_axis_a2_cross_b0,
  physx_sep_axis_a2_cross_b1,
  physx_sep_axis_a2_cross_b2,
  physx_sep_axis_force_dword = 0x7fffffff,
};

enum PhysxShapeFlag {
  physx_sf_disable_response = (1<<12),
  physx_sf_softbody_drain = (1<<18),
  physx_sf_cloth_drain = (1<<15),
  physx_trigger_on_enter = (1<<0),
  physx_sf_point_contact_force = (1<<7),
  physx_sf_softbody_disable_collision = (1<<19),
  physx_sf_feature_indices = (1<<5),
  physx_sf_cloth_twoway = (1<<17),
  physx_sf_visualization = (1<<3),
  physx_trigger_on_stay = (1<<2),
  physx_sf_cloth_disable_collision = (1<<16),
  physx_sf_fluid_twoway = (1<<11),
  physx_trigger_on_leave = (1<<1),
  physx_sf_disable_scene_queries = (1<<14),
  physx_sf_disable_raycasting = (1<<6),
  physx_sf_dynamic_dynamic_ccd = (1<<13),
  physx_sf_disable_collision = (1<<4),
  physx_sf_softbody_twoway = (1<<20),
  physx_sf_fluid_disable_collision = (1<<10),
  physx_sf_fluid_drain = (1<<8),
};

enum PhysxShapePairStreamFlags {
  physx_sf_has_mats_per_point = (1<<0),
  physx_sf_has_features_per_point = (1<<2),
};

enum PhysxShapeType {
  physx_shape_plane,
  physx_shape_sphere,
  physx_shape_box,
  physx_shape_capsule,
  physx_shape_wheel,
  physx_shape_convex,
  physx_shape_mesh,
  physx_shape_heightfield,
  physx_shape_raw_mesh,
  physx_shape_compound,
  physx_shape_count,
  physx_shape_force_dword = 0x7fffffff,
};

enum PhysxShapesType {
  physx_dynamic_shapes = (1<<1),
  physx_static_shapes = (1<<0),
};

enum PhysxSimulationStatus {
  physx_rigid_body_finished = (1<<0),
  physx_all_finished = (1<<0),
  physx_primary_finished = (1<<1),
};

enum PhysxSimulationType {
  physx_simulation_hw = 1,
  physx_sty_force_dword = 0x7fffffff,
  physx_simulation_sw = 0,
};

enum PhysxSoftBodyAttachmentFlag {
  physx_softbody_attachment_tearable = (1<<1),
  physx_softbody_attachment_twoway = (1<<0),
};

enum PhysxSoftBodyFlag {
  physx_sbf_visualization = (1<<4),
  physx_sbf_fluid_collision = (1<<13),
  physx_sbf_static = (1<<1),
  physx_sbf_collision_twoway = (1<<8),
  physx_sbf_disable_collision = (1<<2),
  physx_sbf_validbounds = (1<<12),
  physx_sbf_tearable = (1<<9),
  physx_sbf_volume_conservation = (1<<6),
  physx_sbf_selfcollision = (1<<3),
  physx_sbf_gravity = (1<<5),
  physx_sbf_comdamping = (1<<11),
  physx_sbf_hardware = (1<<10),
  physx_sbf_damping = (1<<7),
};

enum PhysxSoftBodyMeshFlags {
  physx_softbody_mesh_16_bit_indices = (1<<1),
  physx_softbody_mesh_tearable = (1<<2),
};

enum PhysxSoftBodyVertexFlags {
  physx_softbody_vertex_tearable = (1<<7),
};

enum PhysxSphericalJointFlag {
  physx_sjf_swing_spring_enabled = 1 << 3,
  physx_sjf_joint_spring_enabled = 1 << 4,
  physx_sjf_twist_limit_enabled = 1 << 0,
  physx_sjf_twist_spring_enabled = 1 << 2,
  physx_sjf_swing_limit_enabled = 1 << 1,
};

enum PhysxStandardFences {
  physx_fence_run_finished,
  physx_num_standard_fences,
};

enum PhysxSweepFlags {
  physx_sf_async = (1<<2),
  physx_sf_debug_sm = (1<<5),
  physx_sf_dynamics = (1<<1),
  physx_sf_statics = (1<<0),
  physx_sf_debug_et = (1<<6),
  physx_sf_all_hits = (1<<3),
};

enum PhysxThreadPollResult {
  physx_thread_shutdown = 3,
  physx_thread_force_dword = 0x7fffffff,
  physx_thread_nowork = 0,
  physx_thread_simulation_end = 2,
  physx_thread_morework = 1,
};

enum PhysxThreadPriority {
  physx_tp_above_normal = 1,
  physx_tp_normal = 2,
  physx_tp_high = 0,
  physx_tp_low = 4,
  physx_tp_force_dword = 0xffFFffFF,
  physx_tp_below_normal = 3,
};

enum PhysxThreadWait {
  physx_wait_force_dword = 0x7fffffff,
  physx_wait_shutdown = 2,
  physx_wait_simulation_end = 1,
  physx_wait_none = 0,
};

enum PhysxTimeStepMethod {
  physx_timestep_fixed,
  physx_timestep_variable,
  physx_timestep_inherit,
  physx_num_timestep_methods,
  physx_tsm_force_dword = 0x7fffffff,
};

enum PhysxTriangleFlags {
  physx_boundary_edge20 = (1<<6),
  physx_double_sided = (1<<3),
  physx_active_edge12 = (1<<1),
  physx_active_edge01 = (1<<0),
  physx_boundary_edge01 = (1<<4),
  physx_boundary_edge12 = (1<<5),
  physx_active_edge20 = (1<<2),
};



#endif // HAVE_PHYSX

#endif // PHYSX_ENUMERATIONS_H
