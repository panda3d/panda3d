#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET linmath
  #define LOCAL_LIBS \
    putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
     compose_matrix.h compose_matrix_src.I  \
     compose_matrix_src.cxx compose_matrix_src.h config_linmath.h  \
     coordinateSystem.h dbl2fltnames.h dblnames.h deg_2_rad.h  \
     flt2dblnames.h fltnames.h ioPtaDatagramLinMath.I  \
     ioPtaDatagramLinMath.h lcast_to.h lcast_to_src.h  \
     lcast_to_src.I lmatrix.h lmatrix3.h lmatrix3_src.I  \
     lmatrix3_src.cxx lmatrix3_src.h lmatrix4.h lmatrix4_src.I  \
     lmatrix4_src.cxx lmatrix4_src.h lorientation.h  \
     lorientation_src.I lorientation_src.cxx lorientation_src.h  \
     lpoint2.h lpoint2_src.I lpoint2_src.cxx lpoint2_src.h  \
     lpoint3.h lpoint3_src.I lpoint3_src.cxx lpoint3_src.h  \
     lpoint4.h lpoint4_src.I lpoint4_src.cxx lpoint4_src.h  \
     lquaternion.h lquaternion_src.I lquaternion_src.cxx  \
     lquaternion_src.h lrotation.h lrotation_src.I  \
     lrotation_src.cxx lrotation_src.h luse.I luse.N luse.h  \
     lvec2_ops.h lvec2_ops_src.I lvec2_ops_src.h lvec3_ops.h  \
     lvec3_ops_src.I lvec3_ops_src.h lvec4_ops.h lvec4_ops_src.I  \
     lvec4_ops_src.h lvecBase2.h lvecBase2_src.I  \
     lvecBase2_src.cxx lvecBase2_src.h lvecBase3.h  \
     lvecBase3_src.I lvecBase3_src.cxx lvecBase3_src.h  \
     lvecBase4.h lvecBase4_src.I lvecBase4_src.cxx  \
     lvecBase4_src.h lvector2.h lvector2_src.I lvector2_src.cxx  \
     lvector2_src.h lvector3.h lvector3_src.I lvector3_src.cxx  \
     lvector3_src.h lvector4.h lvector4_src.I lvector4_src.cxx  \
     lvector4_src.h mathNumbers.h pta_Colorf.h  \
     pta_Normalf.h pta_TexCoordf.h pta_Vertexf.h vector_Colorf.h  \
     vector_LPoint2f.h vector_LVecBase3f.h vector_Normalf.h  \
     vector_TexCoordf.h vector_Vertexf.h
    
   #define INCLUDED_SOURCES \
     compose_matrix.cxx config_linmath.cxx coordinateSystem.cxx  \
     ioPtaDatagramLinMath.cxx lmatrix.cxx lmatrix3.cxx  \
     lmatrix4.cxx lorientation.cxx lorientation lpoint2.cxx  \
     lpoint3.cxx lpoint4.cxx lquaternion.cxx lrotation.cxx  \
     lrotation luse.cxx lvecBase2.cxx lvecBase3.cxx lvecBase4.cxx  \
     lvector2.cxx lvector3.cxx lvector4.cxx mathNumbers.cxx  \
     pta_Colorf.cxx pta_Normalf.cxx pta_TexCoordf.cxx  \
     pta_Vertexf.cxx vector_Colorf.cxx vector_LPoint2f.cxx  \
     vector_LVecBase3f.cxx vector_Normalf.cxx vector_Vertexf.cxx  \

  #define INSTALL_HEADERS \
    compose_matrix.h compose_matrix_src.I \
    compose_matrix_src.h config_linmath.h coordinateSystem.h \
    dbl2fltnames.h dblnames.h deg_2_rad.h \
    flt2dblnames.h fltnames.h ioPtaDatagramLinMath.I \
    ioPtaDatagramLinMath.h lcast_to.h lcast_to_src.I lcast_to_src.h \
    lmat_ops.h lmat_ops_src.I lmat_ops_src.h lmatrix.h lmatrix3.h \
    lmatrix3_src.I lmatrix3_src.h lmatrix4.h lmatrix4_src.I \
    lmatrix4_src.h lorientation.h lorientation_src.I \
    lorientation_src.h lpoint2.h lpoint2_src.I lpoint2_src.h lpoint3.h \
    lpoint3_src.I lpoint3_src.h lpoint4.h lpoint4_src.I lpoint4_src.h \
    lquaternion.h lquaternion_src.I lquaternion_src.h lrotation.h \
    lrotation_src.I lrotation_src.h luse.I luse.h lvec2_ops.h \
    lvec2_ops_src.I lvec2_ops_src.h lvec3_ops.h lvec3_ops_src.I \
    lvec3_ops_src.h lvec4_ops.h lvec4_ops_src.I lvec4_ops_src.h \
    lvecBase2.h lvecBase2_src.I lvecBase2_src.h lvecBase3.h \
    lvecBase3_src.I lvecBase3_src.h lvecBase4.h lvecBase4_src.I \
    lvecBase4_src.h lvector2.h lvector2_src.I lvector2_src.h \
    lvector3.h lvector3_src.I lvector3_src.h lvector4.h lvector4_src.I \
    lvector4_src.h mathNumbers.h pta_Colorf.h \
    pta_Normalf.h pta_TexCoordf.h pta_Vertexf.h vector_Colorf.h \
    vector_LPoint2f.h vector_LVecBase3f.h vector_Normalf.h \
    vector_TexCoordf.h vector_Vertexf.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_math
  #define LOCAL_LIBS \
    linmath
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_math.cxx

#end test_bin_target

