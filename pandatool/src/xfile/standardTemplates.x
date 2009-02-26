xof 0303txt 0032

# This file contains the standard template definitions for Direct3D
# Retained Mode.  I extracted these from the DirectX API via something
# like the following code:
#
#  #include <rmxftmpl.h>
#
#  LPDIRECTXFILE dx_file;
#  LPDIRECTXFILESAVEOBJECT dx_file_save;
#  HRESULT hr;
#
#  hr = DirectXFileCreate(&dx_file);
#  hr = dx_file->RegisterTemplates(D3DRM_XTEMPLATES, D3DRM_XTEMPLATE_BYTES);
#  hr = dx_file->CreateSaveObject("filename.x", DXFILEFORMAT_TEXT,
#                                  &dx_file_save);
#  static const GUID *temps[] = {
#    &TID_D3DRMInfo,
#    &TID_D3DRMMesh,
#    &TID_D3DRMVector,
#    ...
#  };
#  static const int num_temps = sizeof(temps) / sizeof(temps[0]);
#  hr = dx_file_save->SaveTemplates(num_temps, temps);
#


template Header {
 <3d82ab43-62da-11cf-ab39-0020af71e433>
 WORD major;
 WORD minor;
 DWORD flags;
}

template Vector {
 <3d82ab5e-62da-11cf-ab39-0020af71e433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template MeshFace {
 <3d82ab5f-62da-11cf-ab39-0020af71e433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template Mesh {
 <3d82ab44-62da-11cf-ab39-0020af71e433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

template ColorRGBA {
 <35ff44e0-6c7c-11cf-8f52-0040333594a3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
 FLOAT alpha;
}

template ColorRGB {
 <d3e16e81-7835-11cf-8f52-0040333594a3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
}

template Material {
 <3d82ab4d-62da-11cf-ab39-0020af71e433>
 ColorRGBA faceColor;
 FLOAT power;
 ColorRGB specularColor;
 ColorRGB emissiveColor;
 [...]
}

template Frame {
 <3d82ab46-62da-11cf-ab39-0020af71e433>
 [...]
}

template Matrix4x4 {
 <f6f23f45-7686-11cf-8f52-0040333594a3>
 array FLOAT matrix[16];
}

template FrameTransformMatrix {
 <f6f23f41-7686-11cf-8f52-0040333594a3>
 Matrix4x4 frameMatrix;
}

template MeshMaterialList {
 <f6f23f42-7686-11cf-8f52-0040333594a3>
 DWORD nMaterials;
 DWORD nFaceIndexes;
 array DWORD faceIndexes[nFaceIndexes];
 [Material <3d82ab4d-62da-11cf-ab39-0020af71e433>]
}

template Coords2d {
 <f6f23f44-7686-11cf-8f52-0040333594a3>
 FLOAT u;
 FLOAT v;
}

template MeshTextureCoords {
 <f6f23f40-7686-11cf-8f52-0040333594a3>
 DWORD nTextureCoords;
 array Coords2d textureCoords[nTextureCoords];
}

template MeshNormals {
 <f6f23f43-7686-11cf-8f52-0040333594a3>
 DWORD nNormals;
 array Vector normals[nNormals];
 DWORD nFaceNormals;
 array MeshFace faceNormals[nFaceNormals];
}

template Animation {
 <3d82ab4f-62da-11cf-ab39-0020af71e433>
 [...]
}

template AnimationSet {
 <3d82ab50-62da-11cf-ab39-0020af71e433>
 [Animation <3d82ab4f-62da-11cf-ab39-0020af71e433>]
}

template FloatKeys {
 <10dd46a9-775b-11cf-8f52-0040333594a3>
 DWORD nValues;
 array FLOAT values[nValues];
}

template TimedFloatKeys {
 <f406b180-7b3b-11cf-8f52-0040333594a3>
 DWORD time;
 FloatKeys tfkeys;
}

template AnimationKey {
 <10dd46a8-775b-11cf-8f52-0040333594a3>
 DWORD keyType;
 DWORD nKeys;
 array TimedFloatKeys keys[nKeys];
}

template Guid {
 <a42790e0-7810-11cf-8f52-0040333594a3>
 DWORD data1;
 WORD data2;
 WORD data3;
 array UCHAR data4[8];
}

template TextureFilename {
 <a42790e1-7810-11cf-8f52-0040333594a3>
 STRING filename;
}

template IndexedColor {
 <1630b820-7842-11cf-8f52-0040333594a3>
 DWORD index;
 ColorRGBA indexColor;
}

template MeshVertexColors {
 <1630b821-7842-11cf-8f52-0040333594a3>
 DWORD nVertexColors;
 array IndexedColor vertexColors[nVertexColors];
}

template Boolean {
 <537da6a0-ca37-11d0-941c-0080c80cfa7b>
 DWORD truefalse;
}

template MaterialWrap {
 <4885ae60-78e8-11cf-8f52-0040333594a3>
 Boolean u;
 Boolean v;
}

template Boolean2d {
 <4885ae63-78e8-11cf-8f52-0040333594a3>
 Boolean u;
 Boolean v;
}

template MeshFaceWraps {
 <ed1ec5c0-c0a8-11d0-941c-0080c80cfa7b>
 DWORD nFaceWrapValues;
 array Boolean2d faceWrapValues[nFaceWrapValues];
}

template AnimationOptions {
 <e2bf56c0-840f-11cf-8f52-0040333594a3>
 DWORD openclosed;
 DWORD positionquality;
}

# Since I don't have documentation on the precise semantic meaning of
# the BINARY keyword, I can't parse the following yet.

#template InlineData {
# <3a23eea0-94b1-11d0-ab39-0020af71e433>
# [BINARY]
#}
#
#template Url {
# <3a23eea1-94b1-11d0-ab39-0020af71e433>
# DWORD nUrls;
# array STRING urls[nUrls];
#}
#
#template ProgressiveMesh {
# <8a63c360-997d-11d0-941c-0080c80cfa7b>
# [Url <3a23eea1-94b1-11d0-ab39-0020af71e433>, InlineData <3a23eea0-94b1-11d0-ab39-0020af71e433>]
#}

template ExternalVisual {
 <98116aa0-bdba-11d1-82c0-00a0c9697271>
 Guid guidExternalVisual;
 [...]
}

template StringProperty {
 <7f0f21e0-bfe1-11d1-82c0-00a0c9697271>
 STRING key;
 STRING value;
}

template PropertyBag {
 <7f0f21e1-bfe1-11d1-82c0-00a0c9697271>
 [StringProperty <7f0f21e0-bfe1-11d1-82c0-00a0c9697271>]
}

template RightHanded {
 <7f5d5ea0-d53a-11d1-82c0-00a0c9697271>
 DWORD bRightHanded;
}

template XSkinMeshHeader {
 <3cf169ce-ff7c-44ab-93c0-f78f62d172e2>
 WORD nMaxSkinWeightsPerVertex;
 WORD nMaxSkinWeightsPerFace;
 WORD nBones;
}

template VertexDuplicationIndices {
 <b8d65549-d7c9-4995-89cf-53a9a8b031e3>
 DWORD nIndices;
 DWORD nOriginalVertices;
 array DWORD indices[nIndices];
}

template SkinWeights {
 <6f0d123b-bad2-4167-a0d0-80224f25fabb>
 STRING transformNodeName;
 DWORD nWeights;
 array DWORD vertexIndices[nWeights];
 array FLOAT weights[nWeights];
 Matrix4x4 matrixOffset;
}

template AnimTicksPerSecond {
 <9E415A43-7BA6-4a73-8743-B73D47E88476>
 DWORD AnimTicksPerSecond;
} 
