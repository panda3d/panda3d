/************************************************************************************

Filename    :   Util_Render_Stereo.cpp
Content     :   Stereo rendering configuration implementation
Created     :   October 22, 2012
Authors     :   Michael Antonov, Andrew Reisse

Copyright   :   Copyright 2012 Oculus, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus Inc license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "Util_Render_Stereo.h"

namespace OVR { namespace Util { namespace Render {


//-----------------------------------------------------------------------------------

// DistortionFnInverse computes the inverse of the distortion function on an argument.
float DistortionConfig::DistortionFnInverse(float r)
{    
    OVR_ASSERT((r <= 10.0f));

    float s, d;
    float delta = r * 0.25f;

    s = r * 0.5f;
    d = fabs(r - DistortionFn(s));

    for (int i = 0; i < 20; i++)
    {
        float sUp   = s + delta;
        float sDown = s - delta;
        float dUp   = fabs(r - DistortionFn(sUp));
        float dDown = fabs(r - DistortionFn(sDown));

        if (dUp < d)
        {
            s = sUp;
            d = dUp;
        }
        else if (dDown < d)
        {
            s = sDown;
            d = dDown;
        }
        else
        {
            delta *= 0.5f;
        }
    }

    return s;
}


//-----------------------------------------------------------------------------------
// **** StereoConfig Implementation

StereoConfig::StereoConfig(StereoMode mode, const Viewport& vp)
    : Mode(mode),
      InterpupillaryDistance(0.064f), AspectMultiplier(1.0f),
      FullView(vp), DirtyFlag(true), IPDOverride(false),
      YFov(0), Aspect(vp.w / float(vp.h)), ProjectionCenterOffset(0),
      OrthoPixelOffset(0)
{
    // And default distortion for it.
    Distortion.SetCoefficients(1.0f, 0.22f, 0.24f);
    Distortion.Scale = 1.0f; // Will be computed later.

    // Fit left of the image.
    DistortionFitX = -1.0f;
    DistortionFitY = 0.0f;

    // Initialize "fake" default HMD values for testing without HMD plugged in.
    // These default values match those returned by the HMD.
    HMD.HResolution            = 1280;
    HMD.VResolution            = 800;
    HMD.HScreenSize            = 0.14976f;
    HMD.VScreenSize            = HMD.HScreenSize / (1280.0f / 800.0f);
    HMD.InterpupillaryDistance = InterpupillaryDistance;
    HMD.LensSeparationDistance = 0.0635f;
    HMD.EyeToScreenDistance    = 0.041f;
    HMD.DistortionK[0]         = Distortion.K[0];
    HMD.DistortionK[1]         = Distortion.K[1];
    HMD.DistortionK[2]         = Distortion.K[2];
    HMD.DistortionK[3]         = 0;

    Set2DAreaFov(DegreeToRad(85.0f));
}

void StereoConfig::SetFullViewport(const Viewport& vp)
{
    if (vp != FullView)
    { 
        FullView = vp;
        DirtyFlag = true;
    }
}

void StereoConfig::SetHMDInfo(const HMDInfo& hmd)
{
    HMD = hmd;
    Distortion.K[0] = hmd.DistortionK[0];
    Distortion.K[1] = hmd.DistortionK[1];
    Distortion.K[2] = hmd.DistortionK[2];
    Distortion.K[3] = hmd.DistortionK[3];

    Distortion.SetChromaticAberration(hmd.ChromaAbCorrection[0], hmd.ChromaAbCorrection[1],
                                      hmd.ChromaAbCorrection[2], hmd.ChromaAbCorrection[3]);

    if (!IPDOverride)
        InterpupillaryDistance = HMD.InterpupillaryDistance;

    DirtyFlag = true;
}

void StereoConfig::SetDistortionFitPointVP(float x, float y)
{
    DistortionFitX = x;
    DistortionFitY = y;
    DirtyFlag = true;
}

void StereoConfig::SetDistortionFitPointPixels(float x, float y)
{
    DistortionFitX = (4 * x / float(FullView.w)) - 1.0f;
    DistortionFitY = (2 * y / float(FullView.h)) - 1.0f;
    DirtyFlag = true;
}

void StereoConfig::Set2DAreaFov(float fovRadians)
{
    Area2DFov = fovRadians;
    DirtyFlag = true;
}


const StereoEyeParams& StereoConfig::GetEyeRenderParams(StereoEye eye)
{
    static const UByte eyeParamIndices[3] = { 0, 0, 1 };

    updateIfDirty();
    OVR_ASSERT(eye < sizeof(eyeParamIndices));
    return EyeRenderParams[eyeParamIndices[eye]];
}


void StereoConfig::updateComputedState()
{
    // Need to compute all of the following:
    //   - Aspect Ratio
    //   - FOV
    //   - Projection offsets for 3D
    //   - Distortion XCenterOffset
    //   - Update 2D
    //   - Initialize EyeRenderParams

    // Compute aspect ratio. Stereo mode cuts width in half.
    Aspect = float(FullView.w) / float(FullView.h);
    Aspect *= (Mode == Stereo_None) ? 1.0f : 0.5f;
    Aspect *= AspectMultiplier; 

    updateDistortionOffsetAndScale();

    // Compute Vertical FOV based on distance, distortion, etc.
    // Distance from vertical center to render vertical edge perceived through the lens.
    // This will be larger then normal screen size due to magnification & distortion.
    //
    // This percievedHalfRTDistance equation should hold as long as the render target
    // and display have the same aspect ratios. What we'd like to know is where the edge
    // of the render target will on the perceived screen surface. With NO LENS,
    // the answer would be:
    //
    //  halfRTDistance = (VScreenSize / 2) * aspect *
    //                   DistortionFn_Inverse( DistortionScale / aspect )
    //
    // To model the optical lens we eliminates DistortionFn_Inverse. Aspect ratios
    // cancel out, so we get:
    //
    //  halfRTDistance = (VScreenSize / 2) * DistortionScale
    //
    if (Mode == Stereo_None)
    {
        YFov = DegreeToRad(80.0f);
    }
    else
    {
        float percievedHalfRTDistance = (HMD.VScreenSize / 2) * Distortion.Scale;    
        YFov = 2.0f * atan(percievedHalfRTDistance/HMD.EyeToScreenDistance);
    }
    
    updateProjectionOffset();
    update2D();
    updateEyeParams();

    DirtyFlag = false;
}

void StereoConfig::updateDistortionOffsetAndScale()
{
    // Distortion center shift is stored separately, since it isn't affected
    // by the eye distance.
    float lensOffset        = HMD.LensSeparationDistance * 0.5f;
    float lensShift         = HMD.HScreenSize * 0.25f - lensOffset;
    float lensViewportShift = 4.0f * lensShift / HMD.HScreenSize;
    Distortion.XCenterOffset= lensViewportShift;

    // Compute distortion scale from DistortionFitX & DistortionFitY.
    // Fit value of 0.0 means "no fit".
    if ((fabs(DistortionFitX) < 0.0001f) &&  (fabs(DistortionFitY) < 0.0001f))
    {
        Distortion.Scale = 1.0f;
    }
    else
    {
        // Convert fit value to distortion-centered coordinates before fit radius
        // calculation.
        float stereoAspect = 0.5f * float(FullView.w) / float(FullView.h);
        float dx           = DistortionFitX - Distortion.XCenterOffset;
        float dy           = DistortionFitY / stereoAspect;
        float fitRadius    = sqrt(dx * dx + dy * dy);
        Distortion.Scale   = Distortion.DistortionFn(fitRadius)/fitRadius;
    }
}

void StereoConfig::updateProjectionOffset()
{
    // Post-projection viewport coordinates range from (-1.0, 1.0), with the
    // center of the left viewport falling at (1/4) of horizontal screen size.
    // We need to shift this projection center to match with the lens center;
    // note that we don't use the IPD here due to collimated light property of the lens.
    // We compute this shift in physical units (meters) to
    // correct for different screen sizes and then rescale to viewport coordinates.    
    float viewCenter         = HMD.HScreenSize * 0.25f;
    float eyeProjectionShift = viewCenter - HMD.LensSeparationDistance*0.5f;
    ProjectionCenterOffset   = 4.0f * eyeProjectionShift / HMD.HScreenSize;
}

void StereoConfig::update2D()
{
    // Orthographic projection fakes a screen at a distance of 0.8m from the
    // eye, where hmd screen projection surface is at 0.05m distance.
    // This introduces an extra off-center pixel projection shift based on eye distance.
    // This offCenterShift is the pixel offset of the other camera's center
    // in your reference camera based on surface distance.    
    float metersToPixels          = (HMD.HResolution / HMD.HScreenSize);
    float lensDistanceScreenPixels= metersToPixels * HMD.LensSeparationDistance;
    float eyeDistanceScreenPixels = metersToPixels * InterpupillaryDistance;
    float offCenterShiftPixels    = (HMD.EyeToScreenDistance / 0.8f) * eyeDistanceScreenPixels;
    float leftPixelCenter         = (HMD.HResolution / 2) - lensDistanceScreenPixels * 0.5f;
    float rightPixelCenter        = lensDistanceScreenPixels * 0.5f;
    float pixelDifference         = leftPixelCenter - rightPixelCenter;
    
    // This computes the number of pixels that fit within specified 2D FOV (assuming
    // distortion scaling will be done).
    float percievedHalfScreenDistance = tan(Area2DFov * 0.5f) * HMD.EyeToScreenDistance;
    float vfovSize = 2.0f * percievedHalfScreenDistance / Distortion.Scale;
    FovPixels = HMD.VResolution * vfovSize / HMD.VScreenSize;
    
    // Create orthographic matrix.   
    Matrix4f& m      = OrthoCenter;
    m.SetIdentity();
    m.M[0][0] = FovPixels / (FullView.w * 0.5f);
    m.M[1][1] = -FovPixels / FullView.h;
    m.M[0][3] = 0;
    m.M[1][3] = 0;
    m.M[2][2] = 0;

    float orthoPixelOffset = (pixelDifference + offCenterShiftPixels/Distortion.Scale) * 0.5f;
    OrthoPixelOffset = orthoPixelOffset * 2.0f / FovPixels;
}

void StereoConfig::updateEyeParams()
{
    // Projection matrix for the center eye, which the left/right matrices are based on.
    Matrix4f projCenter = Matrix4f::PerspectiveRH(YFov, Aspect, 0.01f, 2000.0f);
   
    switch(Mode)
    {
    case Stereo_None:
        {
            EyeRenderParams[0].Init(StereoEye_Center, FullView, 0, projCenter, OrthoCenter);
        }
        break;

    case Stereo_LeftRight_Multipass:
        {
            Matrix4f projLeft  = Matrix4f::Translation(ProjectionCenterOffset, 0, 0) * projCenter,
                     projRight = Matrix4f::Translation(-ProjectionCenterOffset, 0, 0) * projCenter;

            EyeRenderParams[0].Init(StereoEye_Left,
                Viewport(FullView.x, FullView.y, FullView.w/2, FullView.h),
                         +InterpupillaryDistance * 0.5f,  // World view shift.                       
                         projLeft, OrthoCenter * Matrix4f::Translation(OrthoPixelOffset, 0, 0),
                         &Distortion);
            EyeRenderParams[1].Init(StereoEye_Right,
                Viewport(FullView.x + FullView.w/2, FullView.y, FullView.w/2, FullView.h),
                         -InterpupillaryDistance * 0.5f,                         
                         projRight, OrthoCenter * Matrix4f::Translation(-OrthoPixelOffset, 0, 0),
                         &Distortion);
        }
        break;
    }

}


}}}  // OVR::Util::Render

