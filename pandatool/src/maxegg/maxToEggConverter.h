// Filename: maxToEggConverter.h
// Created by Corey Revilla and Ken Strickland (6/22/03)
// from mayaToEggConverter.cxx created by drose (10Nov99)
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

#ifndef __maxToEggConverter__H
#define __maxToEggConverter__H

#pragma conform(forScope, off)

/* Error-Reporting Includes
 */
#define MTEC Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM4

/* Helpful Defintions and Casts
 */
#define null 0
#define PHYSIQUE_CLASSID Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B)

/* External Helper Functions for UI
 */
// *** Figure out why this is causing link errors
//DWORD WINAPI ProgressBarFunction(LPVOID arg);

////////////////////////////////////////////////////////////////////
//       Class : MaxToEggConverter
// Description : This class supervises the construction of an EggData
//               structure from a Max model
////////////////////////////////////////////////////////////////////
class MaxToEggConverter {
 public:
    MaxToEggConverter();
    ~MaxToEggConverter();

    bool convert(MaxEggOptions *options);
    
 private:
    struct PandaMaterial {
        std::vector<EggTexture*> _texture_list;
        Colorf _color;
    };
    typedef std::map<Mtl*,PandaMaterial> MaterialMap;
    MaxEggOptions    *_options;
    int               _current_frame;
    PT(EggData)       _egg_data;
    string            _program_name;
    MaxNodeTree       _tree;
    int               _cur_tref;
    EggTextureCollection _textures;
    MaterialMap       _material_map;
    
    void reset();

    bool convert_char_model();
    bool convert_char_chan(double start_frame, double end_frame, 
                           double frame_inc, double output_frame_rate);
    bool convert_hierarchy(EggGroupNode *egg_root);
    bool process_model_node(MaxNodeDesc *node_desc);
    
    void get_transform(INode *max_node, EggGroup *egg_group);
    LMatrix4d get_object_transform(INode *max_node);
    void get_joint_transform(INode *max_node, EggGroup *egg_group);
    void get_joint_transform(INode *max_node, INode *parent_node, 
                             EggGroup *egg_group);
    
    bool make_nurbs_curve(NURBSCVCurve *curve, const string &name,
                          TimeValue time, EggGroup *egg_group);
    void make_polyset(INode *max_node,
                      Mesh *mesh,
                      EggGroup *egg_group,
                      Shader *default_shader = NULL);

    //Gets the vertex normal for a given face and vertex. Go figure.
    Point3 get_max_vertex_normal(Mesh *mesh, int faceNo, int vertNo);
    
    void get_vertex_weights(INode *max_node, EggVertexPool *vpool);

    void set_material_attributes(EggPrimitive &primitive, INode *max_node);
    
    void set_material_attributes(EggPrimitive &primitive, Mtl *maxMaterial, Face *face);
    
    const PandaMaterial &get_panda_material(Mtl *mtl, MtlID id);

    void apply_texture_properties(EggTexture &tex, 
                                  StdMat *maxMaterial);
    bool reparent_decals(EggGroupNode *egg_parent);
    
 public:
    
    Modifier* FindSkinModifier (INode* node, const Class_ID &type);
};


#endif
