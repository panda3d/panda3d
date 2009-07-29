/*
  maxEggExpOptions.h 
  Created by Phillip Saltzman, 2/15/05
  Carnegie Mellon University, Entetainment Technology Center

  This file contains a class that allows users to specify
  export options, and then execute the export
*/

#ifndef __maxEggExpOptions__H
#define __maxEggExpOptions__H

#pragma conform(forScope, off)

/* Externed Globals */
extern HINSTANCE hInstance;

//Save/load chunk definitions
#define CHUNK_OVERWRITE_FLAG  0x1000
#define CHUNK_PVIEW_FLAG      0x1001
#define CHUNK_LOG_OUTPUT      0x1002
#define CHUNK_EGG_EXP_OPTIONS 0x1100
#define CHUNK_ANIM_TYPE       0x1101
#define CHUNK_EGG_CHECKED     0x1102
#define CHUNK_DBL_SIDED       0x1103
#define CHUNK_SF              0x1104
#define CHUNK_EF              0x1105
#define CHUNK_FILENAME        0x1106
#define CHUNK_SHORTNAME       0x1107
#define CHUNK_EXPORT_FULL     0x1108
#define CHUNK_ALL_FRAMES      0x1109
#define CHUNK_NODE_LIST       0x1200
#define CHUNK_NODE_HANDLE     0x1201

#define CHUNK_ADD_COLLISION   0x1202
#define CHUNK_CS_TYPE         0x1203
#define CHUNK_CF_TYPE         0x1204

//Global functions
void ChunkSave(ISave *isave, int chunkid, int value);
void ChunkSave(ISave *isave, int chunkid, bool value);
void ChunkSave(ISave *isave, int chunkid, char *value);
char *ChunkLoadString(ILoad *iload, char *buffer, int maxBytes);
int ChunkLoadInt(ILoad *iload);
bool ChunkLoadBool(ILoad *iload);
void SetICustEdit(HWND wnd, int nIDDlgItem, char *text);
INT_PTR CALLBACK MaxOptionsDialogProc( HWND hWnd, UINT message, 
                                    WPARAM wParam, LPARAM lParam );

struct MaxEggOptions
{
    MaxEggOptions();

    enum Anim_Type {
        AT_none,
        AT_model,
        AT_chan,
        AT_pose,
        AT_strobe,
        AT_both
    };

    enum CS_Type{
        CS_none                 = 0x00000000,
        CS_plane                = 0x00010000,
        CS_polygon              = 0x00020000,
        CS_polyset              = 0x00030000,
        CS_sphere               = 0x00040000,
        CS_tube                 = 0x00050000,
        CS_insphere             = 0x00060000,
        CS_floormesh            = 0x00080000
    };

    enum CF_Type{
        CF_none                  = 0x00000000,
        CF_descend               = 0x00100000,
        CF_event                 = 0x00200000,
        CF_keep                  = 0x00400000,
        CF_solid                 = 0x00800000,
        CF_center                = 0x01000000,
        CF_turnstile             = 0x02000000,
        CF_level                 = 0x04000000,
        CF_intangible            = 0x08000000,
    };

    IObjParam *_max_interface;
    Anim_Type _anim_type;
    int _start_frame;
    int _end_frame;
    bool _double_sided;

    bool _add_collision;
    CS_Type _cs_type;
    CF_Type _cf_type;

    bool _successful;
    bool _export_whole_scene;
    bool _export_all_frames;
    char _file_name[2048];
    char _short_name[256];
    PT(PathReplace) _path_replace;
    std::vector<ULONG> _node_list;
};

class MaxOptionsDialog : public MaxEggOptions
{
    friend class MaxEggPlugin;
    
  public:
    int _min_frame, _max_frame;
    bool _checked;
    bool _choosing_nodes;
    MaxEggOptions::Anim_Type _prev_type;

    MaxOptionsDialog();
    ~MaxOptionsDialog();
    
    void SetMaxInterface(IObjParam *iface) { _max_interface = iface; }
    void UpdateUI(HWND hWnd);
    bool UpdateFromUI(HWND hWnd);
    void RefreshNodeList(HWND hWnd);
    void SetAnimRange();
    
    bool FindNode(ULONG INodeHandle); //returns true if the node is already in the list
    void AddNode(ULONG INodeHandle);
    void RemoveNode(int i);
    void RemoveNodeByHandle(ULONG INodeHandle);
    void ClearNodeList(HWND hWnd);
    void CullBadNodes();
    
    ULONG GetNode(int i) { return (i >= 0 && i < _node_list.size()) ? _node_list[i] : ULONG_MAX; }
    
    IOResult Load(ILoad *iload);
    IOResult Save(ISave *isave);
};

#endif // __MaxEggExpOptions__H
