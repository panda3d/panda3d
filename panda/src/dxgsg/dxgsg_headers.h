// Filename: dxgsg-headers.h
// Created by:  georges (30May01)
//
// local headers for dxgsg .cxx files
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include <directRenderTraverser.h>
#include <cullTraverser.h>
#include <displayRegion.h>
#include <projectionNode.h>
#include <camera.h>
#include <renderBuffer.h>
#include <geom.h>
#include <geomSphere.h>
#include <geomIssuer.h>
#include <graphicsWindow.h>
#include <graphicsChannel.h>
#include <projection.h>
#include <get_rel_pos.h>
#include <perspectiveProjection.h>
#include <ambientLight.h>
#include <directionalLight.h>
#include <pointLight.h>
#include <spotlight.h>
#include <projectionNode.h>
#include <transformAttribute.h>
#include <transformTransition.h>
#include <colorAttribute.h>
#include <colorTransition.h>
#include <lightAttribute.h>
#include <lightTransition.h>
#include <textureAttribute.h>
#include <textureTransition.h>
#include <renderModeAttribute.h>
#include <renderModeTransition.h>
#include <materialAttribute.h>
#include <materialTransition.h>
#include <colorBlendAttribute.h>
#include <colorBlendTransition.h>
#include <colorMaskAttribute.h>
#include <colorMaskTransition.h>
#include <texMatrixAttribute.h>
#include <texMatrixTransition.h>
#include <texGenAttribute.h>
#include <texGenTransition.h>
#include <textureApplyAttribute.h>
#include <textureApplyTransition.h>
#include <clipPlaneAttribute.h>
#include <clipPlaneTransition.h>
#include <transparencyAttribute.h>
#include <transparencyTransition.h>
#include <fogAttribute.h>
#include <fogTransition.h>
#include <linesmoothAttribute.h>
#include <linesmoothTransition.h>
#include <depthTestAttribute.h>
#include <depthTestTransition.h>
#include <depthWriteAttribute.h>
#include <depthWriteTransition.h>
#include <cullFaceAttribute.h>
#include <cullFaceTransition.h>
#include <stencilAttribute.h>
#include <stencilTransition.h>
#include <throw_event.h>
#include <pStatTimer.h>
#include <pStatCollector.h>
#include <dconfig.h>
#include <assert.h>
#include <time.h>
#include <pnmImage.h>

#include "config_dxgsg.h"
#include "dxTextureContext.h"
#include "dxSavedFrameBuffer.h"
#include "dxGraphicsStateGuardian.h"

#include <mmsystem.h>

#pragma hdrstop

