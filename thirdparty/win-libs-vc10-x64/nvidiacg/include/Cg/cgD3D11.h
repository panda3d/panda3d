/*
 *
 * Copyright (c) 2002-2012, NVIDIA Corporation.
 *
 *
 *
 * NVIDIA Corporation("NVIDIA") supplies this software to you in consideration
 * of your agreement to the following terms, and your use, installation,
 * modification or redistribution of this NVIDIA software constitutes
 * acceptance of these terms.  If you do not agree with these terms, please do
 * not use, install, modify or redistribute this NVIDIA software.
 *
 *
 *
 * In consideration of your agreement to abide by the following terms, and
 * subject to these terms, NVIDIA grants you a personal, non-exclusive license,
 * under NVIDIA's copyrights in this original NVIDIA software (the "NVIDIA
 * Software"), to use, reproduce, modify and redistribute the NVIDIA
 * Software, with or without modifications, in source and/or binary forms;
 * provided that if you redistribute the NVIDIA Software, you must retain the
 * copyright notice of NVIDIA, this notice and the following text and
 * disclaimers in all such redistributions of the NVIDIA Software. Neither the
 * name, trademarks, service marks nor logos of NVIDIA Corporation may be used
 * to endorse or promote products derived from the NVIDIA Software without
 * specific prior written permission from NVIDIA.  Except as expressly stated
 * in this notice, no other rights or licenses express or implied, are granted
 * by NVIDIA herein, including but not limited to any patent rights that may be
 * infringed by your derivative works or by other works in which the NVIDIA
 * Software may be incorporated. No hardware is licensed hereunder.
 *
 *
 *
 * THE NVIDIA SOFTWARE IS BEING PROVIDED ON AN "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING
 * WITHOUT LIMITATION, WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT,
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR ITS USE AND OPERATION
 * EITHER ALONE OR IN COMBINATION WITH OTHER PRODUCTS.
 *
 *
 *
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * EXEMPLARY, CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, LOST
 * PROFITS; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) OR ARISING IN ANY WAY OUT OF THE USE,
 * REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE NVIDIA SOFTWARE,
 * HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING
 * NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF NVIDIA HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __CGD3D11_H__
#define __CGD3D11_H__

#ifdef _WIN32

#pragma once

#include <windows.h>
#include <d3d11.h>

#include "Cg/cg.h"

/* Set up for either Win32 import/export/lib. */

#ifdef CGD3D11DLL_EXPORTS
# define CGD3D11DLL_API __declspec(dllexport)
#elif defined(CG_LIB)
# define CGD3D11DLL_API
#else
# define CGD3D11DLL_API __declspec(dllimport)
#endif

#ifndef CGD3D11ENTRY
# ifdef _WIN32
#  define CGD3D11ENTRY __cdecl
# else
#  define CGD3D11ENTRY
# endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CGD3D11_EXPLICIT

CGD3D11DLL_API ID3D11Device * CGD3D11ENTRY cgD3D11GetDevice(CGcontext Context);
CGD3D11DLL_API HRESULT CGD3D11ENTRY cgD3D11SetDevice(CGcontext Context, ID3D11Device *pDevice);
CGD3D11DLL_API void CGD3D11ENTRY cgD3D11SetTextureParameter(CGparameter Parameter, ID3D11Resource *pTexture);
CGD3D11DLL_API void CGD3D11ENTRY cgD3D11SetSamplerStateParameter(CGparameter Parameter, ID3D11SamplerState *pSamplerState);
CGD3D11DLL_API void CGD3D11ENTRY cgD3D11SetTextureSamplerStateParameter(CGparameter Parameter, ID3D11Resource *pTexture, ID3D11SamplerState *pSamplerState);
CGD3D11DLL_API HRESULT CGD3D11ENTRY cgD3D11LoadProgram(CGprogram Program, UINT Flags);
CGD3D11DLL_API ID3D10Blob * CGD3D11ENTRY cgD3D11GetCompiledProgram(CGprogram Program);
CGD3D11DLL_API ID3D10Blob * CGD3D11ENTRY cgD3D11GetProgramErrors(CGprogram Program);
CGD3D11DLL_API CGbool CGD3D11ENTRY cgD3D11IsProgramLoaded(CGprogram Program);
CGD3D11DLL_API HRESULT CGD3D11ENTRY cgD3D11BindProgram(CGprogram Program);
CGD3D11DLL_API void CGD3D11ENTRY cgD3D11UnloadProgram(CGprogram Program);
CGD3D11DLL_API ID3D11Buffer * CGD3D11ENTRY cgD3D11GetBufferByIndex(CGprogram Program, UINT Index);
CGD3D11DLL_API void CGD3D11ENTRY cgD3D11RegisterStates(CGcontext Context);
CGD3D11DLL_API void CGD3D11ENTRY cgD3D11SetManageTextureParameters(CGcontext Context, CGbool Flag);
CGD3D11DLL_API CGbool CGD3D11ENTRY cgD3D11GetManageTextureParameters(CGcontext Context);
CGD3D11DLL_API ID3D10Blob * CGD3D11ENTRY cgD3D11GetIASignatureByPass(CGpass Pass);
CGD3D11DLL_API CGprofile CGD3D11ENTRY cgD3D11GetLatestVertexProfile(void);
CGD3D11DLL_API CGprofile CGD3D11ENTRY cgD3D11GetLatestGeometryProfile(void);
CGD3D11DLL_API CGprofile CGD3D11ENTRY cgD3D11GetLatestPixelProfile(void);
CGD3D11DLL_API CGprofile CGD3D11ENTRY cgD3D11GetLatestHullProfile(void);
CGD3D11DLL_API CGprofile CGD3D11ENTRY cgD3D11GetLatestDomainProfile(void);
CGD3D11DLL_API CGbool CGD3D11ENTRY cgD3D11IsProfileSupported(CGprofile Profile);
CGD3D11DLL_API DWORD CGD3D11ENTRY cgD3D11TypeToSize(CGtype Type);
CGD3D11DLL_API HRESULT CGD3D11ENTRY cgD3D11GetLastError(void);
CGD3D11DLL_API const char ** CGD3D11ENTRY cgD3D11GetOptimalOptions(CGprofile Profile);
CGD3D11DLL_API const char * CGD3D11ENTRY cgD3D11TranslateCGerror(CGerror Error);
CGD3D11DLL_API const char * CGD3D11ENTRY cgD3D11TranslateHRESULT(HRESULT hr);
CGD3D11DLL_API void CGD3D11ENTRY cgD3D11UnbindProgram(CGprogram Program);
CGD3D11DLL_API CGbuffer CGD3D11ENTRY cgD3D11CreateBuffer(CGcontext Context, int size, const void *data, D3D11_USAGE bufferUsage);
CGD3D11DLL_API CGbuffer CGD3D11ENTRY cgD3D11CreateBufferFromObject(CGcontext Context, ID3D11Buffer *obj, CGbool manageObject);
CGD3D11DLL_API ID3D11Buffer * CGD3D11ENTRY cgD3D11GetBufferObject(CGbuffer buffer);

#endif /* CGD3D11_EXPLICIT */

#ifdef __cplusplus
}
#endif

#endif /* _WIN32 */

#endif
