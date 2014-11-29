/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDImage.h
	This file contains the FCDImage class.
*/

#ifndef _FCD_IMAGE_H_
#define _FCD_IMAGE_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDocument;

/**
	A COLLADA image.

	A COLLADA image encapsulates an image file and are contained
	within the image library. FCollada doesn't support inlined image bits.

	An image is solely defined by its filename. For some
	image types and optionally, the width, height and depth of
	the image may be required and valid.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDImage : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, filename, FC("Filename"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, width, FC("Width"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, height, FC("Height"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, depth, FC("Depth"));

public:
	DeclareFlag(Video, 0); /**< Flags this image as a video stream.*/
	DeclareFlagCount(1);

public:
	/** Constructor: do not use directly.
		Instead, use the FCDLibrary::AddEntity function.
		@param document The COLLADA document that owns the image. */
	FCDImage(FCDocument* document);

	/** Destructor. */
	virtual ~FCDImage();

	/** Retrieves the entity type for this class. This function is part
		of the FCDEntity class interface.
		@return The entity type: IMAGE. */
	virtual Type GetType() const { return IMAGE; }

	/** Retrieves the filename of the image file.
		This is the image file that you should load when attempting
		to use the image bits. FCollada will deal with the filename
		internally and provide an absolute filename.
		@return The filename of the image file. */
	const fstring& GetFilename() const { return filename; }

	/** Sets the filename of the image file.
		This is the image file that you should load when attempting
		to use the image bits. FCollada will deal with the filename
		internally and export a relative filename.
		@param _filename The filename of the image file. */
	void SetFilename(const fstring& _filename);

	/** Retrieves the width of the image.
		This parameter is useful for off-screen render targets and is
		optional for texture image files.
		@return The width of the image. This value will be zero to indicate
			that you should retrieve the width of the image from the image file. */
	const uint32& GetWidth() const { return width; }

	/** Sets the width of the image.
		This parameter is useful for off-screen render targets and is
		optional for texture image files.
		@param _width The width of the image. This value can be zero to indicate
			that you should retrieve the width of the image from the image file. */
	void SetWidth(uint32 _width) { width = _width; SetDirtyFlag(); }

	/** Retrieves the height of the image.
		This parameter is useful for off-screen render targets and is
		optional for texture image files.
		@return The height of the image. This value will be zero to indicate
			that you should retrieve the height of the image from the image file. */
	const uint32& GetHeight() const { return height; }

	/** Sets the height of the image.
		This parameter is useful for off-screen render targets and is
		optional for texture image files.
		@param _height The height of the image. This value can be zero to indicate
			that you should retrieve the width of the image from the image file. */
	void SetHeight(uint32 _height) { height = _height; SetDirtyFlag(); }

	/** Retrieves the depth of the 3D image.
		This parameter is optional for texture image files.
		@return The depth of the image. This value will be zero to indicate
			that you should retrieve the depth of the image from the image file. */
	const uint32& GetDepth() const { return depth; }

	/** Sets the depth of the 3D image.
		This parameter is optional for texture image files.
		@param _depth The depth of the image. This value can be zero to indicate
			that you should retrieve the depth of the image from the image file. */
	void SetDepth(uint32 _depth) { depth = _depth; SetDirtyFlag(); }

	/** Copies the image entity into a clone.
		The clone may reside in another document.
		@param clone The empty clone. If this pointer is NULL, a new image
			will be created and you will need to release the returned pointer manually.
		@param cloneChildren Whether to recursively clone this entity's children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;
};

#endif // _FCD_IMAGE_H_
