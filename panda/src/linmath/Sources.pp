#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET linmath
  #define LOCAL_LIBS \
    putil

  #define SOURCES \
    compose_matrix.I compose_matrix.cxx compose_matrix.h \
    config_linmath.cxx config_linmath.h coordinateSystem.cxx \
    coordinateSystem.h deg_2_rad.h \
    ioPtaDatagramLinMath.I ioPtaDatagramLinMath.cxx \
    ioPtaDatagramLinMath.h lmatrix.cxx lmatrix.h \
    lmatrix3.I lmatrix3.h lmatrix4.I lmatrix4.h \
    luse.I luse.N luse.cxx \
    luse.h lquaternion.I lquaternion.h lrotation.I lrotation.h \
    lvec2_ops.I lvec2_ops.h lvec3_ops.I lvec3_ops.h lvec4_ops.I \
    lvec4_ops.h lvecBase2.I lvecBase2.h lvecBase3.I lvecBase3.h \
    lvecBase4.I lvecBase4.h lvector2.I lvector2.h lvector3.I lvector3.h \
    lvector4.I lvector4.h \
    mathNumbers.cxx mathNumbers.h nearly_zero.h \
    pta_Colorf.cxx pta_Colorf.h \
    pta_Normalf.cxx pta_Normalf.h pta_TexCoordf.cxx pta_TexCoordf.h \
    pta_Vertexf.cxx pta_Vertexf.h vector_Colorf.cxx vector_Colorf.h \
    vector_LPoint2f.cxx vector_LPoint2f.h \
    vector_LVecBase3f.cxx vector_LVecBase3f.h \
    vector_Normalf.cxx \
    vector_Normalf.h vector_Vertexf.cxx vector_Vertexf.h

  #define INSTALL_HEADERS \
    cmath.I cmath.h compose_matrix.I compose_matrix.h config_linmath.h \
    coordinateSystem.h deg_2_rad.h ioPtaDatagramLinMath.I \
    ioPtaDatagramLinMath.h lmat_ops.I lmat_ops.h lmatrix.h lmatrix3.I \
    lmatrix3.h lmatrix4.I lmatrix4.h lorientation.I lorientation.h \
    lpoint2.I lpoint2.h lpoint3.I lpoint3.h lpoint4.I lpoint4.h \
    lquaternion.I lquaternion.h lrotation.I lrotation.h luse.I luse.h \
    lvec2_ops.I lvec2_ops.h lvec3_ops.I lvec3_ops.h lvec4_ops.I \
    lvec4_ops.h lvecBase2.I lvecBase2.h lvecBase3.I lvecBase3.h \
    lvecBase4.I lvecBase4.h lvector2.I lvector2.h lvector3.I lvector3.h \
    lvector4.I lvector4.h mathNumbers.h nearly_zero.h pta_Colorf.h \
    pta_Normalf.h pta_TexCoordf.h pta_Vertexf.h vector_Colorf.h \
    vector_LPoint2f.h vector_LVecBase3f.h \
    vector_Normalf.h vector_TexCoordf.h \
    vector_Vertexf.h

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

