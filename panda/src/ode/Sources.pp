#define BUILD_DIRECTORY $[HAVE_ODE]
#define BUILDING_DLL BUILDING_PANDAODE

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3ode
  #define LOCAL_LIBS \
    p3pgraph p3physics

  #define USE_PACKAGES ode
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx \
    $[TARGET]_composite2.cxx $[TARGET]_composite3.cxx \
    $[TARGET]_ext_composite.cxx

  #define SOURCES \
    ode_includes.h config_ode.h \
    odeWorld.I odeWorld.h \
    odeMass.I odeMass.h \
    odeBody.I odeBody.h odeBody_ext.h \
    odeJointGroup.I odeJointGroup.h \
    odeJoint.I odeJoint.h odeJoint_ext.h \
    odeUtil.h odeUtil_ext.h \
    odeSpace.I odeSpace.h \
    odeSpace_ext.I odeSpace_ext.h \
    odeGeom.I odeGeom.h \
    odeGeom_ext.I odeGeom_ext.h \
    odeSurfaceParameters.I odeSurfaceParameters.h \
    odeContactGeom.I odeContactGeom.h \
    odeContact.I odeContact.h \
    odeAMotorJoint.I odeAMotorJoint.h \
    odeBallJoint.I odeBallJoint.h \
    odeContactJoint.I odeContactJoint.h \
    odeFixedJoint.I odeFixedJoint.h \
    odeHingeJoint.I odeHingeJoint.h \
    odeHinge2Joint.I odeHinge2Joint.h \
    odeLMotorJoint.I odeLMotorJoint.h \
    odeNullJoint.I odeNullJoint.h \
    odePlane2dJoint.I odePlane2dJoint.h \
    odeSliderJoint.I odeSliderJoint.h \
    odeUniversalJoint.I odeUniversalJoint.h \
    odeJointCollection.I odeJointCollection.h \
    odeSimpleSpace.I odeSimpleSpace.h \
    odeHashSpace.I odeHashSpace.h \
    odeQuadTreeSpace.I odeQuadTreeSpace.h \
    odeSphereGeom.I odeSphereGeom.h \
    odeBoxGeom.I odeBoxGeom.h \
    odePlaneGeom.I odePlaneGeom.h \
    odeCappedCylinderGeom.I odeCappedCylinderGeom.h \
    odeCylinderGeom.I odeCylinderGeom.h \
    odeRayGeom.I odeRayGeom.h \
    odeTriMeshData.I odeTriMeshData.h \
    odeTriMeshGeom.I odeTriMeshGeom.h \
    odeCollisionEntry.I odeCollisionEntry.h \
    odeHelperStructs.h

  #define INCLUDED_SOURCES \
    config_ode.cxx \
    odeWorld.cxx odeMass.cxx \
    odeBody.cxx odeBody_ext.cxx \
    odeJointGroup.cxx \
    odeJoint.cxx odeJoint_ext.cxx \
    odeUtil.cxx odeUtil_ext.cxx \
    odeSpace.cxx odeSpace_ext.cxx \
    odeGeom.cxx odeGeom_ext.cxx \
    odeSurfaceParameters.cxx \
    odeContactGeom.cxx odeContact.cxx \
    odeAMotorJoint.cxx odeBallJoint.cxx \
    odeContactJoint.cxx odeFixedJoint.cxx \
    odeHingeJoint.cxx odeHinge2Joint.cxx \
    odeLMotorJoint.cxx odeNullJoint.cxx \
    odePlane2dJoint.cxx odeSliderJoint.cxx \
    odeUniversalJoint.cxx odeJointCollection.cxx\
    odeSimpleSpace.cxx \
    odeHashSpace.cxx odeQuadTreeSpace.cxx \
    odeSphereGeom.cxx odeBoxGeom.cxx \
    odePlaneGeom.cxx odeCappedCylinderGeom.cxx \
    odeCylinderGeom.cxx odeRayGeom.cxx \
    odeTriMeshData.cxx  odeTriMeshGeom.cxx \
    odeCollisionEntry.cxx


  #define INSTALL_HEADERS \
    ode_includes.h config_ode.h \
    odeWorld.I odeWorld.h \
    odeMass.I odeMass.h \
    odeBody.I odeBody.h \
    odeJointGroup.I odeJointGroup.h \
    odeJoint.I odeJoint.h \
    odeUtil.h \
    odeSpace.I odeSpace.h \
    odeGeom.I odeGeom.h \
    odeSurfaceParameters.I odeSurfaceParameters.h \
    odeContactGeom.I odeContactGeom.h \
    odeContact.I odeContact.h \
    odeAMotorJoint.I odeAMotorJoint.h \
    odeBallJoint.I odeBallJoint.h \
    odeContactJoint.I odeContactJoint.h \
    odeFixedJoint.I odeFixedJoint.h \
    odeHingeJoint.I odeHingeJoint.h \
    odeHinge2Joint.I odeHinge2Joint.h \
    odeLMotorJoint.I odeLMotorJoint.h \
    odeNullJoint.I odeNullJoint.h \
    odePlane2dJoint.I odePlane2dJoint.h \
    odeSliderJoint.I odeSliderJoint.h \
    odeUniversalJoint.I odeUniversalJoint.h \
    odeJointCollection.I odeJointCollection.h \
    odeSimpleSpace.I odeSimpleSpace.h \
    odeHashSpace.I odeHashSpace.h \
    odeQuadTreeSpace.I odeQuadTreeSpace.h \
    odeSphereGeom.I odeSphereGeom.h \
    odeBoxGeom.I odeBoxGeom.h \
    odePlaneGeom.I odePlaneGeom.h \
    odeCappedCylinderGeom.I odeCappedCylinderGeom.h \
    odeCylinderGeom.I odeCylinderGeom.h \
    odeRayGeom.I odeRayGeom.h \
    odeTriMeshData.I odeTriMeshData.h \
    odeTriMeshGeom.I odeTriMeshGeom.h  \
    odeCollisionEntry.I odeCollisionEntry.h

  #define IGATESCAN all

#end lib_target
