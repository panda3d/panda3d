#!python

import re

files=(
    "config_%(n)sgsg.cxx",
    "config_%(n)sgsg.h",
    "%(n)sGeomNodeContext.I",
    "%(n)sGeomNodeContext.cxx",
    "%(n)sGeomNodeContext.h",
    "%(n)sGraphicsStateGuardian.I",
    "%(n)sGraphicsStateGuardian.cxx",
    "%(n)sGraphicsStateGuardian.h",
    "%(n)sSavedFrameBuffer.I",
    "%(n)sSavedFrameBuffer.cxx",
    "%(n)sSavedFrameBuffer.h",
    "%(n)sTextureContext.I",
    "%(n)sTextureContext.cxx",
    "%(n)sTextureContext.h",
    "%(n)sext.h",
    "%(n)sgsg_composite1.cxx",
    #"Sources.pp",
)

conversion=(
    ("\"gl([A-Z])", "\"cr\\1"),
    ("\\bgl([A-Z])", "chromium.\\1"),
    ("\\bGL([A-TV-Z])", "CR\\1"),
    ("glgsg", "crgsg"),
    ("gl_", "cr_"),
    ("gl-", "cr-"),
    #("GL_", "CR_"),
    ("PANDAGL", "PANDACR"),
    ("#include <GL/gl.h>", """
#include <GL/gl.h>
// Chromium specific
#ifdef WIN32_VC // [
#define WINDOWS 1
#endif //]
#include "cr_glwrapper.h"
#include "cr_applications.h"
#include "cr_spu.h"
extern SPUDispatchTable chromium;
"""),
    ("(TypeHandle CRGraphicsStateGuardian::_type_handle;)", "\\1\nSPUDispatchTable chromium;"),
)

for fn in files:
    destPath=fn%({"n":"cr"})
    print destPath
    # Read the file:
    f=open("../glgsg/"+(fn)%({"n":"gl"}), "rb")
    text=f.read()
    f.close()
    
    # Convert the text:
    for i in conversion:
        text=re.sub(i[0], i[1], text)
    
    # Write the file:
    dest=open(destPath, "wb")
    dest.write(text)
    dest.close()





"""
    ("\\bglAccum", "chromium.Accum"),
    ("\\bglActiveTextureARB", "chromium.ActiveTextureARB"),
    ("\\bglAlphaFunc", "chromium.AlphaFunc"),
    ("\\bglAreTexturesResident", "chromium.AreTexturesResident"),
    ("\\bglArrayElement", "chromium.ArrayElement"),
    ("\\bglBarrierCreate", "chromium.BarrierCreate"),
    ("\\bglBarrierDestroy", "chromium.BarrierDestroy"),
    ("\\bglBarrierExec", "chromium.BarrierExec"),
    ("\\bglBegin", "chromium.Begin"),
    ("\\bglBindTexture", "chromium.BindTexture"),
    ("\\bglBitmap", "chromium.Bitmap"),
    ("\\bglBlendColorEXT", "chromium.BlendColorEXT"),
    ("\\bglBlendEquationEXT", "chromium.BlendEquationEXT"),
    ("\\bglBlendFunc", "chromium.BlendFunc"),
    ("\\bglBoundsInfo", "chromium.BoundsInfo"),
    ("\\bglCallList", "chromium.CallList"),
    ("\\bglCallLists", "chromium.CallLists"),
    ("\\bglChromiumParameterfCR", "chromium.ChromiumParameterfCR"),
    ("\\bglChromiumParameteriCR", "chromium.ChromiumParameteriCR"),
    ("\\bglChromiumParametervCR", "chromium.ChromiumParametervCR"),
    ("\\bglClear", "chromium.Clear"),
    ("\\bglClearAccum", "chromium.ClearAccum"),
    ("\\bglClearColor", "chromium.ClearColor"),
    ("\\bglClearDepth", "chromium.ClearDepth"),
    ("\\bglClearIndex", "chromium.ClearIndex"),
    ("\\bglClearStencil", "chromium.ClearStencil"),
    ("\\bglClientActiveTextureARB", "chromium.ClientActiveTextureARB"),
    ("\\bglClipPlane", "chromium.ClipPlane"),
    ("\\bglColor3b", "chromium.Color3b"),
    ("\\bglColor3bv", "chromium.Color3bv"),
    ("\\bglColor3d", "chromium.Color3d"),
    ("\\bglColor3dv", "chromium.Color3dv"),
    ("\\bglColor3f", "chromium.Color3f"),
    ("\\bglColor3fv", "chromium.Color3fv"),
    ("\\bglColor3i", "chromium.Color3i"),
    ("\\bglColor3iv", "chromium.Color3iv"),
    ("\\bglColor3s", "chromium.Color3s"),
    ("\\bglColor3sv", "chromium.Color3sv"),
    ("\\bglColor3ub", "chromium.Color3ub"),
    ("\\bglColor3ubv", "chromium.Color3ubv"),
    ("\\bglColor3ui", "chromium.Color3ui"),
    ("\\bglColor3uiv", "chromium.Color3uiv"),
    ("\\bglColor3us", "chromium.Color3us"),
    ("\\bglColor3usv", "chromium.Color3usv"),
    ("\\bglColor4b", "chromium.Color4b"),
    ("\\bglColor4bv", "chromium.Color4bv"),
    ("\\bglColor4d", "chromium.Color4d"),
    ("\\bglColor4dv", "chromium.Color4dv"),
    ("\\bglColor4f", "chromium.Color4f"),
    ("\\bglColor4fv", "chromium.Color4fv"),
    ("\\bglColor4i", "chromium.Color4i"),
    ("\\bglColor4iv", "chromium.Color4iv"),
    ("\\bglColor4s", "chromium.Color4s"),
    ("\\bglColor4sv", "chromium.Color4sv"),
    ("\\bglColor4ub", "chromium.Color4ub"),
    ("\\bglColor4ubv", "chromium.Color4ubv"),
    ("\\bglColor4ui", "chromium.Color4ui"),
    ("\\bglColor4uiv", "chromium.Color4uiv"),
    ("\\bglColor4us", "chromium.Color4us"),
    ("\\bglColor4usv", "chromium.Color4usv"),
    ("\\bglColorMask", "chromium.ColorMask"),
    ("\\bglColorMaterial", "chromium.ColorMaterial"),
    ("\\bglColorPointer", "chromium.ColorPointer"),
    ("\\bglCombinerInputNV", "chromium.CombinerInputNV"),
    ("\\bglCombinerOutputNV", "chromium.CombinerOutputNV"),
    ("\\bglCombinerParameterfNV", "chromium.CombinerParameterfNV"),
    ("\\bglCombinerParameterfvNV", "chromium.CombinerParameterfvNV"),
    ("\\bglCombinerParameteriNV", "chromium.CombinerParameteriNV"),
    ("\\bglCombinerParameterivNV", "chromium.CombinerParameterivNV"),
    ("\\bglCombinerStageParameterfvNV", "chromium.CombinerStageParameterfvNV"),
    ("\\bglCopyPixels", "chromium.CopyPixels"),
    ("\\bglCopyTexImage1D", "chromium.CopyTexImage1D"),
    ("\\bglCopyTexImage2D", "chromium.CopyTexImage2D"),
    ("\\bglCopyTexSubImage1D", "chromium.CopyTexSubImage1D"),
    ("\\bglCopyTexSubImage2D", "chromium.CopyTexSubImage2D"),
    ("\\bglCreateContext", "chromium.CreateContext"),
    ("\\bglCullFace", "chromium.CullFace"),
    ("\\bglDeleteLists", "chromium.DeleteLists"),
    ("\\bglDeleteTextures", "chromium.DeleteTextures"),
    ("\\bglDepthFunc", "chromium.DepthFunc"),
    ("\\bglDepthMask", "chromium.DepthMask"),
    ("\\bglDepthRange", "chromium.DepthRange"),
    ("\\bglDestroyContext", "chromium.DestroyContext"),
    ("\\bglDisable", "chromium.Disable"),
    ("\\bglDisableClientState", "chromium.DisableClientState"),
    ("\\bglDrawArrays", "chromium.DrawArrays"),
    ("\\bglDrawBuffer", "chromium.DrawBuffer"),
    ("\\bglDrawElements", "chromium.DrawElements"),
    ("\\bglDrawPixels", "chromium.DrawPixels"),
    ("\\bglDrawRangeElements", "chromium.DrawRangeElements"),
    ("\\bglEdgeFlag", "chromium.EdgeFlag"),
    ("\\bglEdgeFlagPointer", "chromium.EdgeFlagPointer"),
    ("\\bglEdgeFlagv", "chromium.EdgeFlagv"),
    ("\\bglEnable", "chromium.Enable"),
    ("\\bglEnableClientState", "chromium.EnableClientState"),
    ("\\bglEnd", "chromium.End"),
    ("\\bglEndList", "chromium.EndList"),
    ("\\bglEvalCoord1d", "chromium.EvalCoord1d"),
    ("\\bglEvalCoord1dv", "chromium.EvalCoord1dv"),
    ("\\bglEvalCoord1f", "chromium.EvalCoord1f"),
    ("\\bglEvalCoord1fv", "chromium.EvalCoord1fv"),
    ("\\bglEvalCoord2d", "chromium.EvalCoord2d"),
    ("\\bglEvalCoord2dv", "chromium.EvalCoord2dv"),
    ("\\bglEvalCoord2f", "chromium.EvalCoord2f"),
    ("\\bglEvalCoord2fv", "chromium.EvalCoord2fv"),
    ("\\bglEvalMesh1", "chromium.EvalMesh1"),
    ("\\bglEvalMesh2", "chromium.EvalMesh2"),
    ("\\bglEvalPoint1", "chromium.EvalPoint1"),
    ("\\bglEvalPoint2", "chromium.EvalPoint2"),
    ("\\bglFeedbackBuffer", "chromium.FeedbackBuffer"),
    ("\\bglFinalCombinerInputNV", "chromium.FinalCombinerInputNV"),
    ("\\bglFinish", "chromium.Finish"),
    ("\\bglFlush", "chromium.Flush"),
    ("\\bglFogf", "chromium.Fogf"),
    ("\\bglFogfv", "chromium.Fogfv"),
    ("\\bglFogi", "chromium.Fogi"),
    ("\\bglFogiv", "chromium.Fogiv"),
    ("\\bglFrontFace", "chromium.FrontFace"),
    ("\\bglFrustum", "chromium.Frustum"),
    ("\\bglGenLists", "chromium.GenLists"),
    ("\\bglGenTextures", "chromium.GenTextures"),
    ("\\bglGetBooleanv", "chromium.GetBooleanv"),
    ("\\bglGetChromiumParametervCR", "chromium.GetChromiumParametervCR"),
    ("\\bglGetClipPlane", "chromium.GetClipPlane"),
    ("\\bglGetCombinerInputParameterfvNV", "chromium.GetCombinerInputParameterfvNV"),
    ("\\bglGetCombinerInputParameterivNV", "chromium.GetCombinerInputParameterivNV"),
    ("\\bglGetCombinerOutputParameterfvNV", "chromium.GetCombinerOutputParameterfvNV"),
    ("\\bglGetCombinerOutputParameterivNV", "chromium.GetCombinerOutputParameterivNV"),
    ("\\bglGetCombinerStageParameterfvNV", "chromium.GetCombinerStageParameterfvNV"),
    ("\\bglGetDoublev", "chromium.GetDoublev"),
    ("\\bglGetError", "chromium.GetError"),
    ("\\bglGetFinalCombinerInputParameterfvNV", "chromium.GetFinalCombinerInputParameterfvNV"),
    ("\\bglGetFinalCombinerInputParameterivNV", "chromium.GetFinalCombinerInputParameterivNV"),
    ("\\bglGetFloatv", "chromium.GetFloatv"),
    ("\\bglGetIntegerv", "chromium.GetIntegerv"),
    ("\\bglGetLightfv", "chromium.GetLightfv"),
    ("\\bglGetLightiv", "chromium.GetLightiv"),
    ("\\bglGetMapdv", "chromium.GetMapdv"),
    ("\\bglGetMapfv", "chromium.GetMapfv"),
    ("\\bglGetMapiv", "chromium.GetMapiv"),
    ("\\bglGetMaterialfv", "chromium.GetMaterialfv"),
    ("\\bglGetMaterialiv", "chromium.GetMaterialiv"),
    ("\\bglGetPixelMapfv", "chromium.GetPixelMapfv"),
    ("\\bglGetPixelMapuiv", "chromium.GetPixelMapuiv"),
    ("\\bglGetPixelMapusv", "chromium.GetPixelMapusv"),
    ("\\bglGetPointerv", "chromium.GetPointerv"),
    ("\\bglGetPolygonStipple", "chromium.GetPolygonStipple"),
    ("\\bglGetString", "chromium.GetString"),
    ("\\bglGetTexEnvfv", "chromium.GetTexEnvfv"),
    ("\\bglGetTexEnviv", "chromium.GetTexEnviv"),
    ("\\bglGetTexGendv", "chromium.GetTexGendv"),
    ("\\bglGetTexGenfv", "chromium.GetTexGenfv"),
    ("\\bglGetTexGeniv", "chromium.GetTexGeniv"),
    ("\\bglGetTexImage", "chromium.GetTexImage"),
    ("\\bglGetTexLevelParameterfv", "chromium.GetTexLevelParameterfv"),
    ("\\bglGetTexLevelParameteriv", "chromium.GetTexLevelParameteriv"),
    ("\\bglGetTexParameterfv", "chromium.GetTexParameterfv"),
    ("\\bglGetTexParameteriv", "chromium.GetTexParameteriv"),
    ("\\bglHint", "chromium.Hint"),
    ("\\bglIndexMask", "chromium.IndexMask"),
    ("\\bglIndexPointer", "chromium.IndexPointer"),
    ("\\bglIndexd", "chromium.Indexd"),
    ("\\bglIndexdv", "chromium.Indexdv"),
    ("\\bglIndexf", "chromium.Indexf"),
    ("\\bglIndexfv", "chromium.Indexfv"),
    ("\\bglIndexi", "chromium.Indexi"),
    ("\\bglIndexiv", "chromium.Indexiv"),
    ("\\bglIndexs", "chromium.Indexs"),
    ("\\bglIndexsv", "chromium.Indexsv"),
    ("\\bglIndexub", "chromium.Indexub"),
    ("\\bglIndexubv", "chromium.Indexubv"),
    ("\\bglInitNames", "chromium.InitNames"),
    ("\\bglInterleavedArrays", "chromium.InterleavedArrays"),
    ("\\bglIsEnabled", "chromium.IsEnabled"),
    ("\\bglIsList", "chromium.IsList"),
    ("\\bglIsTexture", "chromium.IsTexture"),
    ("\\bglLightModelf", "chromium.LightModelf"),
    ("\\bglLightModelfv", "chromium.LightModelfv"),
    ("\\bglLightModeli", "chromium.LightModeli"),
    ("\\bglLightModeliv", "chromium.LightModeliv"),
    ("\\bglLightf", "chromium.Lightf"),
    ("\\bglLightfv", "chromium.Lightfv"),
    ("\\bglLighti", "chromium.Lighti"),
    ("\\bglLightiv", "chromium.Lightiv"),
    ("\\bglLineStipple", "chromium.LineStipple"),
    ("\\bglLineWidth", "chromium.LineWidth"),
    ("\\bglListBase", "chromium.ListBase"),
    ("\\bglLoadIdentity", "chromium.LoadIdentity"),
    ("\\bglLoadMatrixd", "chromium.LoadMatrixd"),
    ("\\bglLoadMatrixf", "chromium.LoadMatrixf"),
    ("\\bglLoadName", "chromium.LoadName"),
    ("\\bglLogicOp", "chromium.LogicOp"),
    ("\\bglMakeCurrent", "chromium.MakeCurrent"),
    ("\\bglMap1d", "chromium.Map1d"),
    ("\\bglMap1f", "chromium.Map1f"),
    ("\\bglMap2d", "chromium.Map2d"),
    ("\\bglMap2f", "chromium.Map2f"),
    ("\\bglMapGrid1d", "chromium.MapGrid1d"),
    ("\\bglMapGrid1f", "chromium.MapGrid1f"),
    ("\\bglMapGrid2d", "chromium.MapGrid2d"),
    ("\\bglMapGrid2f", "chromium.MapGrid2f"),
    ("\\bglMaterialf", "chromium.Materialf"),
    ("\\bglMaterialfv", "chromium.Materialfv"),
    ("\\bglMateriali", "chromium.Materiali"),
    ("\\bglMaterialiv", "chromium.Materialiv"),
    ("\\bglMatrixMode", "chromium.MatrixMode"),
    ("\\bglMultMatrixd", "chromium.MultMatrixd"),
    ("\\bglMultMatrixf", "chromium.MultMatrixf"),
    ("\\bglMultiTexCoord1dARB", "chromium.MultiTexCoord1dARB"),
    ("\\bglMultiTexCoord1dvARB", "chromium.MultiTexCoord1dvARB"),
    ("\\bglMultiTexCoord1fARB", "chromium.MultiTexCoord1fARB"),
    ("\\bglMultiTexCoord1fvARB", "chromium.MultiTexCoord1fvARB"),
    ("\\bglMultiTexCoord1iARB", "chromium.MultiTexCoord1iARB"),
    ("\\bglMultiTexCoord1ivARB", "chromium.MultiTexCoord1ivARB"),
    ("\\bglMultiTexCoord1sARB", "chromium.MultiTexCoord1sARB"),
    ("\\bglMultiTexCoord1svARB", "chromium.MultiTexCoord1svARB"),
    ("\\bglMultiTexCoord2dARB", "chromium.MultiTexCoord2dARB"),
    ("\\bglMultiTexCoord2dvARB", "chromium.MultiTexCoord2dvARB"),
    ("\\bglMultiTexCoord2fARB", "chromium.MultiTexCoord2fARB"),
    ("\\bglMultiTexCoord2fvARB", "chromium.MultiTexCoord2fvARB"),
    ("\\bglMultiTexCoord2iARB", "chromium.MultiTexCoord2iARB"),
    ("\\bglMultiTexCoord2ivARB", "chromium.MultiTexCoord2ivARB"),
    ("\\bglMultiTexCoord2sARB", "chromium.MultiTexCoord2sARB"),
    ("\\bglMultiTexCoord2svARB", "chromium.MultiTexCoord2svARB"),
    ("\\bglMultiTexCoord3dARB", "chromium.MultiTexCoord3dARB"),
    ("\\bglMultiTexCoord3dvARB", "chromium.MultiTexCoord3dvARB"),
    ("\\bglMultiTexCoord3fARB", "chromium.MultiTexCoord3fARB"),
    ("\\bglMultiTexCoord3fvARB", "chromium.MultiTexCoord3fvARB"),
    ("\\bglMultiTexCoord3iARB", "chromium.MultiTexCoord3iARB"),
    ("\\bglMultiTexCoord3ivARB", "chromium.MultiTexCoord3ivARB"),
    ("\\bglMultiTexCoord3sARB", "chromium.MultiTexCoord3sARB"),
    ("\\bglMultiTexCoord3svARB", "chromium.MultiTexCoord3svARB"),
    ("\\bglMultiTexCoord4dARB", "chromium.MultiTexCoord4dARB"),
    ("\\bglMultiTexCoord4dvARB", "chromium.MultiTexCoord4dvARB"),
    ("\\bglMultiTexCoord4fARB", "chromium.MultiTexCoord4fARB"),
    ("\\bglMultiTexCoord4fvARB", "chromium.MultiTexCoord4fvARB"),
    ("\\bglMultiTexCoord4iARB", "chromium.MultiTexCoord4iARB"),
    ("\\bglMultiTexCoord4ivARB", "chromium.MultiTexCoord4ivARB"),
    ("\\bglMultiTexCoord4sARB", "chromium.MultiTexCoord4sARB"),
    ("\\bglMultiTexCoord4svARB", "chromium.MultiTexCoord4svARB"),
    ("\\bglNewList", "chromium.NewList"),
    ("\\bglNormal3b", "chromium.Normal3b"),
    ("\\bglNormal3bv", "chromium.Normal3bv"),
    ("\\bglNormal3d", "chromium.Normal3d"),
    ("\\bglNormal3dv", "chromium.Normal3dv"),
    ("\\bglNormal3f", "chromium.Normal3f"),
    ("\\bglNormal3fv", "chromium.Normal3fv"),
    ("\\bglNormal3i", "chromium.Normal3i"),
    ("\\bglNormal3iv", "chromium.Normal3iv"),
    ("\\bglNormal3s", "chromium.Normal3s"),
    ("\\bglNormal3sv", "chromium.Normal3sv"),
    ("\\bglNormalPointer", "chromium.NormalPointer"),
    ("\\bglOrtho", "chromium.Ortho"),
    ("\\bglPassThrough", "chromium.PassThrough"),
    ("\\bglPixelMapfv", "chromium.PixelMapfv"),
    ("\\bglPixelMapuiv", "chromium.PixelMapuiv"),
    ("\\bglPixelMapusv", "chromium.PixelMapusv"),
    ("\\bglPixelStoref", "chromium.PixelStoref"),
    ("\\bglPixelStorei", "chromium.PixelStorei"),
    ("\\bglPixelTransferf", "chromium.PixelTransferf"),
    ("\\bglPixelTransferi", "chromium.PixelTransferi"),
    ("\\bglPixelZoom", "chromium.PixelZoom"),
    ("\\bglPointSize", "chromium.PointSize"),
    ("\\bglPolygonMode", "chromium.PolygonMode"),
    ("\\bglPolygonOffset", "chromium.PolygonOffset"),
    ("\\bglPolygonStipple", "chromium.PolygonStipple"),
    ("\\bglPopAttrib", "chromium.PopAttrib"),
    ("\\bglPopClientAttrib", "chromium.PopClientAttrib"),
    ("\\bglPopMatrix", "chromium.PopMatrix"),
    ("\\bglPopName", "chromium.PopName"),
    ("\\bglPrioritizeTextures", "chromium.PrioritizeTextures"),
    ("\\bglPushAttrib", "chromium.PushAttrib"),
    ("\\bglPushClientAttrib", "chromium.PushClientAttrib"),
    ("\\bglPushMatrix", "chromium.PushMatrix"),
    ("\\bglPushName", "chromium.PushName"),
    ("\\bglRasterPos2d", "chromium.RasterPos2d"),
    ("\\bglRasterPos2dv", "chromium.RasterPos2dv"),
    ("\\bglRasterPos2f", "chromium.RasterPos2f"),
    ("\\bglRasterPos2fv", "chromium.RasterPos2fv"),
    ("\\bglRasterPos2i", "chromium.RasterPos2i"),
    ("\\bglRasterPos2iv", "chromium.RasterPos2iv"),
    ("\\bglRasterPos2s", "chromium.RasterPos2s"),
    ("\\bglRasterPos2sv", "chromium.RasterPos2sv"),
    ("\\bglRasterPos3d", "chromium.RasterPos3d"),
    ("\\bglRasterPos3dv", "chromium.RasterPos3dv"),
    ("\\bglRasterPos3f", "chromium.RasterPos3f"),
    ("\\bglRasterPos3fv", "chromium.RasterPos3fv"),
    ("\\bglRasterPos3i", "chromium.RasterPos3i"),
    ("\\bglRasterPos3iv", "chromium.RasterPos3iv"),
    ("\\bglRasterPos3s", "chromium.RasterPos3s"),
    ("\\bglRasterPos3sv", "chromium.RasterPos3sv"),
    ("\\bglRasterPos4d", "chromium.RasterPos4d"),
    ("\\bglRasterPos4dv", "chromium.RasterPos4dv"),
    ("\\bglRasterPos4f", "chromium.RasterPos4f"),
    ("\\bglRasterPos4fv", "chromium.RasterPos4fv"),
    ("\\bglRasterPos4i", "chromium.RasterPos4i"),
    ("\\bglRasterPos4iv", "chromium.RasterPos4iv"),
    ("\\bglRasterPos4s", "chromium.RasterPos4s"),
    ("\\bglRasterPos4sv", "chromium.RasterPos4sv"),
    ("\\bglReadBuffer", "chromium.ReadBuffer"),
    ("\\bglReadPixels", "chromium.ReadPixels"),
    ("\\bglRectd", "chromium.Rectd"),
    ("\\bglRectdv", "chromium.Rectdv"),
    ("\\bglRectf", "chromium.Rectf"),
    ("\\bglRectfv", "chromium.Rectfv"),
    ("\\bglRecti", "chromium.Recti"),
    ("\\bglRectiv", "chromium.Rectiv"),
    ("\\bglRects", "chromium.Rects"),
    ("\\bglRectsv", "chromium.Rectsv"),
    ("\\bglRenderMode", "chromium.RenderMode"),
    ("\\bglRotated", "chromium.Rotated"),
    ("\\bglRotatef", "chromium.Rotatef"),
    ("\\bglScaled", "chromium.Scaled"),
    ("\\bglScalef", "chromium.Scalef"),
    ("\\bglScissor", "chromium.Scissor"),
    ("\\bglSecondaryColor3bEXT", "chromium.SecondaryColor3bEXT"),
    ("\\bglSecondaryColor3bvEXT", "chromium.SecondaryColor3bvEXT"),
    ("\\bglSecondaryColor3dEXT", "chromium.SecondaryColor3dEXT"),
    ("\\bglSecondaryColor3dvEXT", "chromium.SecondaryColor3dvEXT"),
    ("\\bglSecondaryColor3fEXT", "chromium.SecondaryColor3fEXT"),
    ("\\bglSecondaryColor3fvEXT", "chromium.SecondaryColor3fvEXT"),
    ("\\bglSecondaryColor3iEXT", "chromium.SecondaryColor3iEXT"),
    ("\\bglSecondaryColor3ivEXT", "chromium.SecondaryColor3ivEXT"),
    ("\\bglSecondaryColor3sEXT", "chromium.SecondaryColor3sEXT"),
    ("\\bglSecondaryColor3svEXT", "chromium.SecondaryColor3svEXT"),
    ("\\bglSecondaryColor3ubEXT", "chromium.SecondaryColor3ubEXT"),
    ("\\bglSecondaryColor3ubvEXT", "chromium.SecondaryColor3ubvEXT"),
    ("\\bglSecondaryColor3uiEXT", "chromium.SecondaryColor3uiEXT"),
    ("\\bglSecondaryColor3uivEXT", "chromium.SecondaryColor3uivEXT"),
    ("\\bglSecondaryColor3usEXT", "chromium.SecondaryColor3usEXT"),
    ("\\bglSecondaryColor3usvEXT", "chromium.SecondaryColor3usvEXT"),
    ("\\bglSecondaryColorPointerEXT", "chromium.SecondaryColorPointerEXT"),
    ("\\bglSelectBuffer", "chromium.SelectBuffer"),
    ("\\bglSemaphoreCreate", "chromium.SemaphoreCreate"),
    ("\\bglSemaphoreDestroy", "chromium.SemaphoreDestroy"),
    ("\\bglSemaphoreP", "chromium.SemaphoreP"),
    ("\\bglSemaphoreV", "chromium.SemaphoreV"),
    ("\\bglShadeModel", "chromium.ShadeModel"),
    ("\\bglStencilFunc", "chromium.StencilFunc"),
    ("\\bglStencilMask", "chromium.StencilMask"),
    ("\\bglStencilOp", "chromium.StencilOp"),
    ("\\bglSwapBuffers", "chromium.SwapBuffers"),
    ("\\bglTexCoord1d", "chromium.TexCoord1d"),
    ("\\bglTexCoord1dv", "chromium.TexCoord1dv"),
    ("\\bglTexCoord1f", "chromium.TexCoord1f"),
    ("\\bglTexCoord1fv", "chromium.TexCoord1fv"),
    ("\\bglTexCoord1i", "chromium.TexCoord1i"),
    ("\\bglTexCoord1iv", "chromium.TexCoord1iv"),
    ("\\bglTexCoord1s", "chromium.TexCoord1s"),
    ("\\bglTexCoord1sv", "chromium.TexCoord1sv"),
    ("\\bglTexCoord2d", "chromium.TexCoord2d"),
    ("\\bglTexCoord2dv", "chromium.TexCoord2dv"),
    ("\\bglTexCoord2f", "chromium.TexCoord2f"),
    ("\\bglTexCoord2fv", "chromium.TexCoord2fv"),
    ("\\bglTexCoord2i", "chromium.TexCoord2i"),
    ("\\bglTexCoord2iv", "chromium.TexCoord2iv"),
    ("\\bglTexCoord2s", "chromium.TexCoord2s"),
    ("\\bglTexCoord2sv", "chromium.TexCoord2sv"),
    ("\\bglTexCoord3d", "chromium.TexCoord3d"),
    ("\\bglTexCoord3dv", "chromium.TexCoord3dv"),
    ("\\bglTexCoord3f", "chromium.TexCoord3f"),
    ("\\bglTexCoord3fv", "chromium.TexCoord3fv"),
    ("\\bglTexCoord3i", "chromium.TexCoord3i"),
    ("\\bglTexCoord3iv", "chromium.TexCoord3iv"),
    ("\\bglTexCoord3s", "chromium.TexCoord3s"),
    ("\\bglTexCoord3sv", "chromium.TexCoord3sv"),
    ("\\bglTexCoord4d", "chromium.TexCoord4d"),
    ("\\bglTexCoord4dv", "chromium.TexCoord4dv"),
    ("\\bglTexCoord4f", "chromium.TexCoord4f"),
    ("\\bglTexCoord4fv", "chromium.TexCoord4fv"),
    ("\\bglTexCoord4i", "chromium.TexCoord4i"),
    ("\\bglTexCoord4iv", "chromium.TexCoord4iv"),
    ("\\bglTexCoord4s", "chromium.TexCoord4s"),
    ("\\bglTexCoord4sv", "chromium.TexCoord4sv"),
    ("\\bglTexCoordPointer", "chromium.TexCoordPointer"),
    ("\\bglTexEnvf", "chromium.TexEnvf"),
    ("\\bglTexEnvfv", "chromium.TexEnvfv"),
    ("\\bglTexEnvi", "chromium.TexEnvi"),
    ("\\bglTexEnviv", "chromium.TexEnviv"),
    ("\\bglTexGend", "chromium.TexGend"),
    ("\\bglTexGendv", "chromium.TexGendv"),
    ("\\bglTexGenf", "chromium.TexGenf"),
    ("\\bglTexGenfv", "chromium.TexGenfv"),
    ("\\bglTexGeni", "chromium.TexGeni"),
    ("\\bglTexGeniv", "chromium.TexGeniv"),
    ("\\bglTexImage1D", "chromium.TexImage1D"),
    ("\\bglTexImage2D", "chromium.TexImage2D"),
    ("\\bglTexParameterf", "chromium.TexParameterf"),
    ("\\bglTexParameterfv", "chromium.TexParameterfv"),
    ("\\bglTexParameteri", "chromium.TexParameteri"),
    ("\\bglTexParameteriv", "chromium.TexParameteriv"),
    ("\\bglTexSubImage1D", "chromium.TexSubImage1D"),
    ("\\bglTexSubImage2D", "chromium.TexSubImage2D"),
    ("\\bglTranslated", "chromium.Translated"),
    ("\\bglTranslatef", "chromium.Translatef"),
    ("\\bglVertex2d", "chromium.Vertex2d"),
    ("\\bglVertex2dv", "chromium.Vertex2dv"),
    ("\\bglVertex2f", "chromium.Vertex2f"),
    ("\\bglVertex2fv", "chromium.Vertex2fv"),
    ("\\bglVertex2i", "chromium.Vertex2i"),
    ("\\bglVertex2iv", "chromium.Vertex2iv"),
    ("\\bglVertex2s", "chromium.Vertex2s"),
    ("\\bglVertex2sv", "chromium.Vertex2sv"),
    ("\\bglVertex3d", "chromium.Vertex3d"),
    ("\\bglVertex3dv", "chromium.Vertex3dv"),
    ("\\bglVertex3f", "chromium.Vertex3f"),
    ("\\bglVertex3fv", "chromium.Vertex3fv"),
    ("\\bglVertex3i", "chromium.Vertex3i"),
    ("\\bglVertex3iv", "chromium.Vertex3iv"),
    ("\\bglVertex3s", "chromium.Vertex3s"),
    ("\\bglVertex3sv", "chromium.Vertex3sv"),
    ("\\bglVertex4d", "chromium.Vertex4d"),
    ("\\bglVertex4dv", "chromium.Vertex4dv"),
    ("\\bglVertex4f", "chromium.Vertex4f"),
    ("\\bglVertex4fv", "chromium.Vertex4fv"),
    ("\\bglVertex4i", "chromium.Vertex4i"),
    ("\\bglVertex4iv", "chromium.Vertex4iv"),
    ("\\bglVertex4s", "chromium.Vertex4s"),
    ("\\bglVertex4sv", "chromium.Vertex4sv"),
    ("\\bglVertexPointer", "chromium.VertexPointer"),
    ("\\bglViewport", "chromium.Viewport"),
    ("\\bglWriteback", "chromium.Writeback"),


#define glAccum chromium.Accum
#define glActiveTextureARB chromium.ActiveTextureARB
#define glAlphaFunc chromium.AlphaFunc
#define glAreTexturesResident chromium.AreTexturesResident
#define glArrayElement chromium.ArrayElement
#define glBarrierCreate chromium.BarrierCreate
#define glBarrierDestroy chromium.BarrierDestroy
#define glBarrierExec chromium.BarrierExec
#define glBegin chromium.Begin
#define glBindTexture chromium.BindTexture
#define glBitmap chromium.Bitmap
#define glBlendColorEXT chromium.BlendColorEXT
#define glBlendEquationEXT chromium.BlendEquationEXT
#define glBlendFunc chromium.BlendFunc
#define glBoundsInfo chromium.BoundsInfo
#define glCallList chromium.CallList
#define glCallLists chromium.CallLists
#define glChromiumParameterfCR chromium.ChromiumParameterfCR
#define glChromiumParameteriCR chromium.ChromiumParameteriCR
#define glChromiumParametervCR chromium.ChromiumParametervCR
#define glClear chromium.Clear
#define glClearAccum chromium.ClearAccum
#define glClearColor chromium.ClearColor
#define glClearDepth chromium.ClearDepth
#define glClearIndex chromium.ClearIndex
#define glClearStencil chromium.ClearStencil
#define glClientActiveTextureARB chromium.ClientActiveTextureARB
#define glClipPlane chromium.ClipPlane
#define glColor3b chromium.Color3b
#define glColor3bv chromium.Color3bv
#define glColor3d chromium.Color3d
#define glColor3dv chromium.Color3dv
#define glColor3f chromium.Color3f
#define glColor3fv chromium.Color3fv
#define glColor3i chromium.Color3i
#define glColor3iv chromium.Color3iv
#define glColor3s chromium.Color3s
#define glColor3sv chromium.Color3sv
#define glColor3ub chromium.Color3ub
#define glColor3ubv chromium.Color3ubv
#define glColor3ui chromium.Color3ui
#define glColor3uiv chromium.Color3uiv
#define glColor3us chromium.Color3us
#define glColor3usv chromium.Color3usv
#define glColor4b chromium.Color4b
#define glColor4bv chromium.Color4bv
#define glColor4d chromium.Color4d
#define glColor4dv chromium.Color4dv
#define glColor4f chromium.Color4f
#define glColor4fv chromium.Color4fv
#define glColor4i chromium.Color4i
#define glColor4iv chromium.Color4iv
#define glColor4s chromium.Color4s
#define glColor4sv chromium.Color4sv
#define glColor4ub chromium.Color4ub
#define glColor4ubv chromium.Color4ubv
#define glColor4ui chromium.Color4ui
#define glColor4uiv chromium.Color4uiv
#define glColor4us chromium.Color4us
#define glColor4usv chromium.Color4usv
#define glColorMask chromium.ColorMask
#define glColorMaterial chromium.ColorMaterial
#define glColorPointer chromium.ColorPointer
#define glCombinerInputNV chromium.CombinerInputNV
#define glCombinerOutputNV chromium.CombinerOutputNV
#define glCombinerParameterfNV chromium.CombinerParameterfNV
#define glCombinerParameterfvNV chromium.CombinerParameterfvNV
#define glCombinerParameteriNV chromium.CombinerParameteriNV
#define glCombinerParameterivNV chromium.CombinerParameterivNV
#define glCombinerStageParameterfvNV chromium.CombinerStageParameterfvNV
#define glCopyPixels chromium.CopyPixels
#define glCopyTexImage1D chromium.CopyTexImage1D
#define glCopyTexImage2D chromium.CopyTexImage2D
#define glCopyTexSubImage1D chromium.CopyTexSubImage1D
#define glCopyTexSubImage2D chromium.CopyTexSubImage2D
#define glCreateContext chromium.CreateContext
#define glCullFace chromium.CullFace
#define glDeleteLists chromium.DeleteLists
#define glDeleteTextures chromium.DeleteTextures
#define glDepthFunc chromium.DepthFunc
#define glDepthMask chromium.DepthMask
#define glDepthRange chromium.DepthRange
#define glDestroyContext chromium.DestroyContext
#define glDisable chromium.Disable
#define glDisableClientState chromium.DisableClientState
#define glDrawArrays chromium.DrawArrays
#define glDrawBuffer chromium.DrawBuffer
#define glDrawElements chromium.DrawElements
#define glDrawPixels chromium.DrawPixels
#define glDrawRangeElements chromium.DrawRangeElements
#define glEdgeFlag chromium.EdgeFlag
#define glEdgeFlagPointer chromium.EdgeFlagPointer
#define glEdgeFlagv chromium.EdgeFlagv
#define glEnable chromium.Enable
#define glEnableClientState chromium.EnableClientState
#define glEnd chromium.End
#define glEndList chromium.EndList
#define glEvalCoord1d chromium.EvalCoord1d
#define glEvalCoord1dv chromium.EvalCoord1dv
#define glEvalCoord1f chromium.EvalCoord1f
#define glEvalCoord1fv chromium.EvalCoord1fv
#define glEvalCoord2d chromium.EvalCoord2d
#define glEvalCoord2dv chromium.EvalCoord2dv
#define glEvalCoord2f chromium.EvalCoord2f
#define glEvalCoord2fv chromium.EvalCoord2fv
#define glEvalMesh1 chromium.EvalMesh1
#define glEvalMesh2 chromium.EvalMesh2
#define glEvalPoint1 chromium.EvalPoint1
#define glEvalPoint2 chromium.EvalPoint2
#define glFeedbackBuffer chromium.FeedbackBuffer
#define glFinalCombinerInputNV chromium.FinalCombinerInputNV
#define glFinish chromium.Finish
#define glFlush chromium.Flush
#define glFogf chromium.Fogf
#define glFogfv chromium.Fogfv
#define glFogi chromium.Fogi
#define glFogiv chromium.Fogiv
#define glFrontFace chromium.FrontFace
#define glFrustum chromium.Frustum
#define glGenLists chromium.GenLists
#define glGenTextures chromium.GenTextures
#define glGetBooleanv chromium.GetBooleanv
#define glGetChromiumParametervCR chromium.GetChromiumParametervCR
#define glGetClipPlane chromium.GetClipPlane
#define glGetCombinerInputParameterfvNV chromium.GetCombinerInputParameterfvNV
#define glGetCombinerInputParameterivNV chromium.GetCombinerInputParameterivNV
#define glGetCombinerOutputParameterfvNV chromium.GetCombinerOutputParameterfvNV
#define glGetCombinerOutputParameterivNV chromium.GetCombinerOutputParameterivNV
#define glGetCombinerStageParameterfvNV chromium.GetCombinerStageParameterfvNV
#define glGetDoublev chromium.GetDoublev
#define glGetError chromium.GetError
#define glGetFinalCombinerInputParameterfvNV chromium.GetFinalCombinerInputParameterfvNV
#define glGetFinalCombinerInputParameterivNV chromium.GetFinalCombinerInputParameterivNV
#define glGetFloatv chromium.GetFloatv
#define glGetIntegerv chromium.GetIntegerv
#define glGetLightfv chromium.GetLightfv
#define glGetLightiv chromium.GetLightiv
#define glGetMapdv chromium.GetMapdv
#define glGetMapfv chromium.GetMapfv
#define glGetMapiv chromium.GetMapiv
#define glGetMaterialfv chromium.GetMaterialfv
#define glGetMaterialiv chromium.GetMaterialiv
#define glGetPixelMapfv chromium.GetPixelMapfv
#define glGetPixelMapuiv chromium.GetPixelMapuiv
#define glGetPixelMapusv chromium.GetPixelMapusv
#define glGetPointerv chromium.GetPointerv
#define glGetPolygonStipple chromium.GetPolygonStipple
#define glGetString chromium.GetString
#define glGetTexEnvfv chromium.GetTexEnvfv
#define glGetTexEnviv chromium.GetTexEnviv
#define glGetTexGendv chromium.GetTexGendv
#define glGetTexGenfv chromium.GetTexGenfv
#define glGetTexGeniv chromium.GetTexGeniv
#define glGetTexImage chromium.GetTexImage
#define glGetTexLevelParameterfv chromium.GetTexLevelParameterfv
#define glGetTexLevelParameteriv chromium.GetTexLevelParameteriv
#define glGetTexParameterfv chromium.GetTexParameterfv
#define glGetTexParameteriv chromium.GetTexParameteriv
#define glHint chromium.Hint
#define glIndexMask chromium.IndexMask
#define glIndexPointer chromium.IndexPointer
#define glIndexd chromium.Indexd
#define glIndexdv chromium.Indexdv
#define glIndexf chromium.Indexf
#define glIndexfv chromium.Indexfv
#define glIndexi chromium.Indexi
#define glIndexiv chromium.Indexiv
#define glIndexs chromium.Indexs
#define glIndexsv chromium.Indexsv
#define glIndexub chromium.Indexub
#define glIndexubv chromium.Indexubv
#define glInitNames chromium.InitNames
#define glInterleavedArrays chromium.InterleavedArrays
#define glIsEnabled chromium.IsEnabled
#define glIsList chromium.IsList
#define glIsTexture chromium.IsTexture
#define glLightModelf chromium.LightModelf
#define glLightModelfv chromium.LightModelfv
#define glLightModeli chromium.LightModeli
#define glLightModeliv chromium.LightModeliv
#define glLightf chromium.Lightf
#define glLightfv chromium.Lightfv
#define glLighti chromium.Lighti
#define glLightiv chromium.Lightiv
#define glLineStipple chromium.LineStipple
#define glLineWidth chromium.LineWidth
#define glListBase chromium.ListBase
#define glLoadIdentity chromium.LoadIdentity
#define glLoadMatrixd chromium.LoadMatrixd
#define glLoadMatrixf chromium.LoadMatrixf
#define glLoadName chromium.LoadName
#define glLogicOp chromium.LogicOp
#define glMakeCurrent chromium.MakeCurrent
#define glMap1d chromium.Map1d
#define glMap1f chromium.Map1f
#define glMap2d chromium.Map2d
#define glMap2f chromium.Map2f
#define glMapGrid1d chromium.MapGrid1d
#define glMapGrid1f chromium.MapGrid1f
#define glMapGrid2d chromium.MapGrid2d
#define glMapGrid2f chromium.MapGrid2f
#define glMaterialf chromium.Materialf
#define glMaterialfv chromium.Materialfv
#define glMateriali chromium.Materiali
#define glMaterialiv chromium.Materialiv
#define glMatrixMode chromium.MatrixMode
#define glMultMatrixd chromium.MultMatrixd
#define glMultMatrixf chromium.MultMatrixf
#define glMultiTexCoord1dARB chromium.MultiTexCoord1dARB
#define glMultiTexCoord1dvARB chromium.MultiTexCoord1dvARB
#define glMultiTexCoord1fARB chromium.MultiTexCoord1fARB
#define glMultiTexCoord1fvARB chromium.MultiTexCoord1fvARB
#define glMultiTexCoord1iARB chromium.MultiTexCoord1iARB
#define glMultiTexCoord1ivARB chromium.MultiTexCoord1ivARB
#define glMultiTexCoord1sARB chromium.MultiTexCoord1sARB
#define glMultiTexCoord1svARB chromium.MultiTexCoord1svARB
#define glMultiTexCoord2dARB chromium.MultiTexCoord2dARB
#define glMultiTexCoord2dvARB chromium.MultiTexCoord2dvARB
#define glMultiTexCoord2fARB chromium.MultiTexCoord2fARB
#define glMultiTexCoord2fvARB chromium.MultiTexCoord2fvARB
#define glMultiTexCoord2iARB chromium.MultiTexCoord2iARB
#define glMultiTexCoord2ivARB chromium.MultiTexCoord2ivARB
#define glMultiTexCoord2sARB chromium.MultiTexCoord2sARB
#define glMultiTexCoord2svARB chromium.MultiTexCoord2svARB
#define glMultiTexCoord3dARB chromium.MultiTexCoord3dARB
#define glMultiTexCoord3dvARB chromium.MultiTexCoord3dvARB
#define glMultiTexCoord3fARB chromium.MultiTexCoord3fARB
#define glMultiTexCoord3fvARB chromium.MultiTexCoord3fvARB
#define glMultiTexCoord3iARB chromium.MultiTexCoord3iARB
#define glMultiTexCoord3ivARB chromium.MultiTexCoord3ivARB
#define glMultiTexCoord3sARB chromium.MultiTexCoord3sARB
#define glMultiTexCoord3svARB chromium.MultiTexCoord3svARB
#define glMultiTexCoord4dARB chromium.MultiTexCoord4dARB
#define glMultiTexCoord4dvARB chromium.MultiTexCoord4dvARB
#define glMultiTexCoord4fARB chromium.MultiTexCoord4fARB
#define glMultiTexCoord4fvARB chromium.MultiTexCoord4fvARB
#define glMultiTexCoord4iARB chromium.MultiTexCoord4iARB
#define glMultiTexCoord4ivARB chromium.MultiTexCoord4ivARB
#define glMultiTexCoord4sARB chromium.MultiTexCoord4sARB
#define glMultiTexCoord4svARB chromium.MultiTexCoord4svARB
#define glNewList chromium.NewList
#define glNormal3b chromium.Normal3b
#define glNormal3bv chromium.Normal3bv
#define glNormal3d chromium.Normal3d
#define glNormal3dv chromium.Normal3dv
#define glNormal3f chromium.Normal3f
#define glNormal3fv chromium.Normal3fv
#define glNormal3i chromium.Normal3i
#define glNormal3iv chromium.Normal3iv
#define glNormal3s chromium.Normal3s
#define glNormal3sv chromium.Normal3sv
#define glNormalPointer chromium.NormalPointer
#define glOrtho chromium.Ortho
#define glPassThrough chromium.PassThrough
#define glPixelMapfv chromium.PixelMapfv
#define glPixelMapuiv chromium.PixelMapuiv
#define glPixelMapusv chromium.PixelMapusv
#define glPixelStoref chromium.PixelStoref
#define glPixelStorei chromium.PixelStorei
#define glPixelTransferf chromium.PixelTransferf
#define glPixelTransferi chromium.PixelTransferi
#define glPixelZoom chromium.PixelZoom
#define glPointSize chromium.PointSize
#define glPolygonMode chromium.PolygonMode
#define glPolygonOffset chromium.PolygonOffset
#define glPolygonStipple chromium.PolygonStipple
#define glPopAttrib chromium.PopAttrib
#define glPopClientAttrib chromium.PopClientAttrib
#define glPopMatrix chromium.PopMatrix
#define glPopName chromium.PopName
#define glPrioritizeTextures chromium.PrioritizeTextures
#define glPushAttrib chromium.PushAttrib
#define glPushClientAttrib chromium.PushClientAttrib
#define glPushMatrix chromium.PushMatrix
#define glPushName chromium.PushName
#define glRasterPos2d chromium.RasterPos2d
#define glRasterPos2dv chromium.RasterPos2dv
#define glRasterPos2f chromium.RasterPos2f
#define glRasterPos2fv chromium.RasterPos2fv
#define glRasterPos2i chromium.RasterPos2i
#define glRasterPos2iv chromium.RasterPos2iv
#define glRasterPos2s chromium.RasterPos2s
#define glRasterPos2sv chromium.RasterPos2sv
#define glRasterPos3d chromium.RasterPos3d
#define glRasterPos3dv chromium.RasterPos3dv
#define glRasterPos3f chromium.RasterPos3f
#define glRasterPos3fv chromium.RasterPos3fv
#define glRasterPos3i chromium.RasterPos3i
#define glRasterPos3iv chromium.RasterPos3iv
#define glRasterPos3s chromium.RasterPos3s
#define glRasterPos3sv chromium.RasterPos3sv
#define glRasterPos4d chromium.RasterPos4d
#define glRasterPos4dv chromium.RasterPos4dv
#define glRasterPos4f chromium.RasterPos4f
#define glRasterPos4fv chromium.RasterPos4fv
#define glRasterPos4i chromium.RasterPos4i
#define glRasterPos4iv chromium.RasterPos4iv
#define glRasterPos4s chromium.RasterPos4s
#define glRasterPos4sv chromium.RasterPos4sv
#define glReadBuffer chromium.ReadBuffer
#define glReadPixels chromium.ReadPixels
#define glRectd chromium.Rectd
#define glRectdv chromium.Rectdv
#define glRectf chromium.Rectf
#define glRectfv chromium.Rectfv
#define glRecti chromium.Recti
#define glRectiv chromium.Rectiv
#define glRects chromium.Rects
#define glRectsv chromium.Rectsv
#define glRenderMode chromium.RenderMode
#define glRotated chromium.Rotated
#define glRotatef chromium.Rotatef
#define glScaled chromium.Scaled
#define glScalef chromium.Scalef
#define glScissor chromium.Scissor
#define glSecondaryColor3bEXT chromium.SecondaryColor3bEXT
#define glSecondaryColor3bvEXT chromium.SecondaryColor3bvEXT
#define glSecondaryColor3dEXT chromium.SecondaryColor3dEXT
#define glSecondaryColor3dvEXT chromium.SecondaryColor3dvEXT
#define glSecondaryColor3fEXT chromium.SecondaryColor3fEXT
#define glSecondaryColor3fvEXT chromium.SecondaryColor3fvEXT
#define glSecondaryColor3iEXT chromium.SecondaryColor3iEXT
#define glSecondaryColor3ivEXT chromium.SecondaryColor3ivEXT
#define glSecondaryColor3sEXT chromium.SecondaryColor3sEXT
#define glSecondaryColor3svEXT chromium.SecondaryColor3svEXT
#define glSecondaryColor3ubEXT chromium.SecondaryColor3ubEXT
#define glSecondaryColor3ubvEXT chromium.SecondaryColor3ubvEXT
#define glSecondaryColor3uiEXT chromium.SecondaryColor3uiEXT
#define glSecondaryColor3uivEXT chromium.SecondaryColor3uivEXT
#define glSecondaryColor3usEXT chromium.SecondaryColor3usEXT
#define glSecondaryColor3usvEXT chromium.SecondaryColor3usvEXT
#define glSecondaryColorPointerEXT chromium.SecondaryColorPointerEXT
#define glSelectBuffer chromium.SelectBuffer
#define glSemaphoreCreate chromium.SemaphoreCreate
#define glSemaphoreDestroy chromium.SemaphoreDestroy
#define glSemaphoreP chromium.SemaphoreP
#define glSemaphoreV chromium.SemaphoreV
#define glShadeModel chromium.ShadeModel
#define glStencilFunc chromium.StencilFunc
#define glStencilMask chromium.StencilMask
#define glStencilOp chromium.StencilOp
#define glSwapBuffers chromium.SwapBuffers
#define glTexCoord1d chromium.TexCoord1d
#define glTexCoord1dv chromium.TexCoord1dv
#define glTexCoord1f chromium.TexCoord1f
#define glTexCoord1fv chromium.TexCoord1fv
#define glTexCoord1i chromium.TexCoord1i
#define glTexCoord1iv chromium.TexCoord1iv
#define glTexCoord1s chromium.TexCoord1s
#define glTexCoord1sv chromium.TexCoord1sv
#define glTexCoord2d chromium.TexCoord2d
#define glTexCoord2dv chromium.TexCoord2dv
#define glTexCoord2f chromium.TexCoord2f
#define glTexCoord2fv chromium.TexCoord2fv
#define glTexCoord2i chromium.TexCoord2i
#define glTexCoord2iv chromium.TexCoord2iv
#define glTexCoord2s chromium.TexCoord2s
#define glTexCoord2sv chromium.TexCoord2sv
#define glTexCoord3d chromium.TexCoord3d
#define glTexCoord3dv chromium.TexCoord3dv
#define glTexCoord3f chromium.TexCoord3f
#define glTexCoord3fv chromium.TexCoord3fv
#define glTexCoord3i chromium.TexCoord3i
#define glTexCoord3iv chromium.TexCoord3iv
#define glTexCoord3s chromium.TexCoord3s
#define glTexCoord3sv chromium.TexCoord3sv
#define glTexCoord4d chromium.TexCoord4d
#define glTexCoord4dv chromium.TexCoord4dv
#define glTexCoord4f chromium.TexCoord4f
#define glTexCoord4fv chromium.TexCoord4fv
#define glTexCoord4i chromium.TexCoord4i
#define glTexCoord4iv chromium.TexCoord4iv
#define glTexCoord4s chromium.TexCoord4s
#define glTexCoord4sv chromium.TexCoord4sv
#define glTexCoordPointer chromium.TexCoordPointer
#define glTexEnvf chromium.TexEnvf
#define glTexEnvfv chromium.TexEnvfv
#define glTexEnvi chromium.TexEnvi
#define glTexEnviv chromium.TexEnviv
#define glTexGend chromium.TexGend
#define glTexGendv chromium.TexGendv
#define glTexGenf chromium.TexGenf
#define glTexGenfv chromium.TexGenfv
#define glTexGeni chromium.TexGeni
#define glTexGeniv chromium.TexGeniv
#define glTexImage1D chromium.TexImage1D
#define glTexImage2D chromium.TexImage2D
#define glTexParameterf chromium.TexParameterf
#define glTexParameterfv chromium.TexParameterfv
#define glTexParameteri chromium.TexParameteri
#define glTexParameteriv chromium.TexParameteriv
#define glTexSubImage1D chromium.TexSubImage1D
#define glTexSubImage2D chromium.TexSubImage2D
#define glTranslated chromium.Translated
#define glTranslatef chromium.Translatef
#define glVertex2d chromium.Vertex2d
#define glVertex2dv chromium.Vertex2dv
#define glVertex2f chromium.Vertex2f
#define glVertex2fv chromium.Vertex2fv
#define glVertex2i chromium.Vertex2i
#define glVertex2iv chromium.Vertex2iv
#define glVertex2s chromium.Vertex2s
#define glVertex2sv chromium.Vertex2sv
#define glVertex3d chromium.Vertex3d
#define glVertex3dv chromium.Vertex3dv
#define glVertex3f chromium.Vertex3f
#define glVertex3fv chromium.Vertex3fv
#define glVertex3i chromium.Vertex3i
#define glVertex3iv chromium.Vertex3iv
#define glVertex3s chromium.Vertex3s
#define glVertex3sv chromium.Vertex3sv
#define glVertex4d chromium.Vertex4d
#define glVertex4dv chromium.Vertex4dv
#define glVertex4f chromium.Vertex4f
#define glVertex4fv chromium.Vertex4fv
#define glVertex4i chromium.Vertex4i
#define glVertex4iv chromium.Vertex4iv
#define glVertex4s chromium.Vertex4s
#define glVertex4sv chromium.Vertex4sv
#define glVertexPointer chromium.VertexPointer
#define glViewport chromium.Viewport
#define glWriteback chromium.Writeback 


#define glgsg crgsg
#define NotifyCategoryGetCategory_glgsg NotifyCategoryGetCategory_crgsg

#define gl_show_transforms cr_show_transforms
#define gl_cheap_textures cr_cheap_textures
#define gl_cull_traversal cr_cull_traversal
#define gl_ignore_mipmaps cr_ignore_mipmaps
#define gl_force_mipmaps cr_force_mipmaps
#define gl_show_mipmaps cr_show_mipmaps
#define gl_save_mipmaps cr_save_mipmaps
#define gl_auto_normalize_lighting cr_auto_normalize_lighting
#define gl_supports_bgr cr_supports_bgr

#define GLDecalType CRDecalType
#define gl_decal_type cr_decal_type

#define init_libglgsg init_libcrgsg


#define glgsg_cat crgsg_cat

#define GLGraphicsStateGuardian CRGraphicsStateGuardian
#define GLTextureContext CRTextureContext
#define GLGeomNodeContext CRGeomNodeContext
#define GLSavedFrameBuffer CRSavedFrameBuffer

#define make_GlGraphicsStateGuardian make_CRGraphicsStateGuardian

"""
