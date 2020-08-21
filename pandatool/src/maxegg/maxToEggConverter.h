/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file maxToEggConverter.h
 * @author Corey Revilla and Ken Strickland
 * @date 2003-06-22
 * from mayaToEggConverter.cxx created by drose (10Nov99)
 */

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
// *** Figure out why this is causing link errors DWORD WINAPI
// ProgressBarFunction(LPVOID arg);

/**
 * This class supervises the construction of an EggData structure from a Max
 * model
 */
class MaxToEggConverter {
 public:
    MaxToEggConverter();
    ~MaxToEggConverter();

    bool convert(MaxEggOptions *options);

 private:
    struct PandaMaterial {
        std::vector<PT(EggTexture)> _texture_list;
        LColor _color;
        std::vector<int> _map_channels;
        bool _any_diffuse;
        bool _any_opacity;
        bool _any_gloss;
        bool _any_normal;
    };
    typedef std::map<Mtl*,PandaMaterial> MaterialMap;
    MaxEggOptions    *_options;
    int               _current_frame;
    PT(EggData)       _egg_data;
    std::string            _program_name;
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

    bool make_nurbs_curve(INode *max_node, NURBSCVCurve *curve,
                          TimeValue time, EggGroup *egg_group);
    void make_polyset(INode *max_node,
                      Mesh *mesh,
                      EggGroup *egg_group,
                      Shader *default_shader = nullptr);

    Point3 get_max_vertex_normal(Mesh *mesh, int faceNo, int vertNo);
    VertColor get_max_vertex_color(Mesh *mesh, int FaceNo, int VertexNo);
    VertColor get_max_vertex_color(Mesh *mesh,int FaceNo,int VertexNo, int channel);
    UVVert get_max_vertex_texcoord(Mesh *mesh, int faceNo, int vertNo, int channel);

    void get_vertex_weights(INode *max_node, EggVertexPool *vpool);

    const PandaMaterial &get_panda_material(Mtl *mtl, MtlID id);
    void analyze_diffuse_maps(PandaMaterial &pandaMat, Texmap *m);
    void analyze_opacity_maps(PandaMaterial &pandaMat, Texmap *m);
    void analyze_gloss_maps(PandaMaterial &pandaMat, Texmap *m);
    void analyze_glow_maps(PandaMaterial &pandaMat, Texmap *m);
    void analyze_normal_maps(PandaMaterial &pandaMat, Texmap *m);
    void add_map_channel(PandaMaterial &pandaMat, int channel);
    void apply_texture_properties(EggTexture &tex, int channel);
    std::string generate_tex_name();
    std::string get_uv_name(int n);
    bool reparent_decals(EggGroupNode *egg_parent);

 public:

    Modifier* FindSkinModifier (INode* node, const Class_ID &type);
};


#endif
