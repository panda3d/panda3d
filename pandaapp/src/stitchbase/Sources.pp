#define YACC_PREFIX stitchyy
#define LFLAGS -i

#begin ss_lib_target
  #define TARGET stitchbase
  #define LOCAL_LIBS pandaappbase
  #define OTHER_LIBS \
    progbase \
    putil:c express:c mathutil:c linmath:c pnmimage:c panda:m
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    config_stitch.cxx config_stitch.h \
    fadeImagePool.I fadeImagePool.cxx fadeImagePool.h \
    fixedPoint.h \
    layeredImage.cxx layeredImage.h \
    morphGrid.cxx morphGrid.h stitchCommand.cxx stitchCommand.h \
    stitchCommandReader.cxx stitchCommandReader.h \
    stitchCylindricalLens.cxx stitchCylindricalLens.h stitchFile.cxx \
    stitchFile.h stitchFisheyeLens.cxx stitchFisheyeLens.h \
    stitchImage.cxx stitchImage.h stitchImageCommandOutput.cxx \
    stitchImageCommandOutput.h stitchImageOutputter.cxx \
    stitchImageOutputter.h stitchImageRasterizer.cxx \
    stitchImageRasterizer.h stitchLens.cxx stitchLens.h \
    stitchPSphereLens.cxx stitchPSphereLens.h stitchPerspectiveLens.cxx \
    stitchPerspectiveLens.h stitchPoint.cxx stitchPoint.h stitcher.cxx \
    stitcher.h triangle.cxx triangle.h triangleRasterizer.cxx \
    triangleRasterizer.h \
    stitchParserDefs.h stitchParser.yxx stitchLexerDefs.h stitchLexer.lxx \
    stitchCylindricalScreen.cxx stitchCylindricalScreen.h \
    stitchFlatScreen.cxx stitchFlatScreen.h \
    stitchMultiScreen.cxx stitchMultiScreen.h \
    stitchScreen.cxx stitchScreen.h

  #define INSTALL_HEADERS \
    config_stitch.h \
    fadeImagePool.I fadeImagePool.h \
    fixedPoint.h layeredImage.h morphGrid.h stitchCommand.h \
    stitchCommandReader.h stitchCylindricalLens.h stitchFile.h \
    stitchFisheyeLens.h stitchImage.h stitchImageCommandOutput.h \
    stitchImageOutputter.h stitchImageRasterizer.h stitchLens.h \
    stitchLexerDefs.h stitchPSphereLens.h stitchParser.h \
    stitchParserDefs.h stitchPerspectiveLens.h stitchPoint.h \
    stitcher.h triangle.h triangleRasterizer.h \
    stitchCylindricalScreen.h \
    stitchFlatScreen.h \
    stitchMultiScreen.h \
    stitchScreen.h


#end ss_lib_target
