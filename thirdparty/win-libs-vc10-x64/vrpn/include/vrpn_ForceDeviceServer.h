// vrpn_ForceDeviceServer.h: interface for the vrpn_ForceDeviceServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VRPN_FORCEDEVICESERVER_H__E5B0D6FA_E426_45E5_97C7_B2169BEBF90A__INCLUDED_)
#define AFX_VRPN_FORCEDEVICESERVER_H__E5B0D6FA_E426_45E5_97C7_B2169BEBF90A__INCLUDED_

#include "vrpn_ForceDevice.h"
#include "vrpn_HashST.h"


typedef vrpn_Hash<unsigned int, void*> vrpn_VoidHash;
typedef  struct _vrpn_DisplayableObject 
{
	int m_ObjectType;
	void *m_pObject;
	void *m_pObjectMesh;

} vrpn_DISPLAYABLEOBJECT;

typedef vrpn_Hash<unsigned int, vrpn_DISPLAYABLEOBJECT*> vrpn_DISPLAYABLEHASH;

class VRPN_API vrpn_ForceDeviceServer : public vrpn_ForceDevice  
{
public:
	vrpn_ForceDeviceServer(const char * name, vrpn_Connection *c);
	virtual ~vrpn_ForceDeviceServer();
protected:
	
	static int VRPN_CALLBACK handle_addObject_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_addObjectExScene_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setVertex_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setNormal_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setTriangle_message(void *userdata, 
				       vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_removeTriangle_message(void *userdata, 
				       vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_updateTrimeshChanges_message(void *userdata, 
					 vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_transformTrimesh_message(void *userdata, 
					 vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setTrimeshType_message(void *userdata, 
					 vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setObjectPosition_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setObjectOrientation_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setObjectScale_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_removeObject_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_moveToParent_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setHapticOrigin_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setHapticScale_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setSceneOrigin_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setObjectIsTouchable_message(void *userdata, 
				     vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_clearTrimesh_message(void *userdata, 
					 vrpn_HANDLERPARAM p);
	// Add an object to the haptic scene as root (parent -1 = default) or as child (ParentNum =the number of the parent)
	virtual bool addObject(vrpn_int32 objNum, vrpn_int32 ParentNum=-1)=0; 
	// Add an object next to the haptic scene as root 
	virtual bool addObjectExScene(vrpn_int32 objNum)=0; 
	// vertNum normNum and triNum start at 0
	virtual bool setVertex(vrpn_int32 objNum, vrpn_int32 vertNum,vrpn_float32 x,vrpn_float32 y,vrpn_float32 z)=0;
    // NOTE: ghost dosen't take normals, 
    //       and normals still aren't implemented for Hcollide
    virtual bool setNormal(vrpn_int32 objNum, vrpn_int32 normNum,vrpn_float32 x,vrpn_float32 y,vrpn_float32 z)=0;
    virtual bool setTriangle(vrpn_int32 objNum, vrpn_int32 triNum,vrpn_int32 vert0,vrpn_int32 vert1,vrpn_int32 vert2,
		  vrpn_int32 norm0=-1,vrpn_int32 norm1=-1,vrpn_int32 norm2=-1)=0;
    virtual bool removeTriangle(vrpn_int32 objNum, vrpn_int32 triNum)=0; 
    // should be called to incorporate the above changes into the 
    // displayed trimesh 
    virtual bool updateTrimeshChanges(vrpn_int32 objNum,vrpn_float32 kspring, vrpn_float32 kdamp, vrpn_float32 fdyn, vrpn_float32 fstat)=0;
	// set trimesh type
	virtual bool setTrimeshType(vrpn_int32 objNum,vrpn_int32 type)=0;
    // set the trimesh's homogen transform matrix (in row major order)
    virtual bool setTrimeshTransform(vrpn_int32 objNum, vrpn_float32 homMatrix[16])=0;
	// set position of an object
	virtual bool setObjectPosition(vrpn_int32 objNum, vrpn_float32 Pos[3])=0;
	// set orientation of an object
	virtual bool setObjectOrientation(vrpn_int32 objNum, vrpn_float32 axis[3], vrpn_float32 angle)=0;
	// set Scale of an object
	virtual bool setObjectScale(vrpn_int32 objNum, vrpn_float32 Scale[3])=0;
	// remove an object from the scene
	virtual bool removeObject(vrpn_int32 objNum)=0;
    virtual bool clearTrimesh(vrpn_int32 objNum)=0;
  
	/** Functions to organize the scene	**********************************************************/
	// Change The parent of an object
	virtual bool moveToParent(vrpn_int32 objNum, vrpn_int32 ParentNum)=0;
	// Set the Origin of the haptic scene
	virtual bool setHapticOrigin(vrpn_float32 Pos[3], vrpn_float32 axis[3], vrpn_float32 angle)=0;    
	// Set the Scale factor of the haptic scene
	virtual bool setHapticScale(vrpn_float32 Scale)=0;
	// Set the Origin of the haptic scene
	virtual bool setSceneOrigin(vrpn_float32 Pos[3], vrpn_float32 axis[3], vrpn_float32 angle)=0;    
	// make an object touchable or not
	virtual bool setObjectIsTouchable(vrpn_int32 objNum, vrpn_bool IsTouchable=true)=0;
protected:
	vrpn_DISPLAYABLEHASH m_hObjectList;
};

#endif // !defined(AFX_VRPN_FORCEDEVICESERVER_H__E5B0D6FA_E426_45E5_97C7_B2169BEBF90A__INCLUDED_)

