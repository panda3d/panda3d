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
	@file FCDCamera.h
	This file contains the FCDCamera class.
*/

#ifndef _FCD_CAMERA_H_
#define _FCD_CAMERA_H_

#ifndef _FCD_TARGETED_ENTITY_H_
#include "FCDocument/FCDTargetedEntity.h"
#endif // _FCD_TARGETED_ENTITY_H_
#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#include "FCDocument/FCDParameterAnimatable.h"
#endif // _FCD_PARAMETER_ANIMATABLE_H_

class FCDocument;
class FCDSceneNode;

/**
	A COLLADA camera.
	Based on the FCDTargetedEntity class to support aimed cameras.
	COLLADA defines two types of cameras: perspective and orthographic.
	Both types are fully handled by this class.

	A COLLADA perspective camera defines two of the three following parameters: horizontal field of view,
	vertical field of view and aspect ratio. The missing parameter can be calculated using the following formulae:
	aspect ratio = vertical field of view / horizontal field of view. The vertical and horizontal field
	of view are in degrees.

	A COLLADA orthographic camera defines two of the three following parameters: horizontal magnification,
	vertical magnification and aspect ratio. The missing parameter can be calculated using the following formulae:
	aspect ratio = vertical magnification / horizontal magnification. You can calculate the viewport width
	and height using the following formulas: viewport width = horizontal magnification * 2, viewport height
	= vertical magnification * 2.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDCamera : public FCDTargetedEntity
{
public:
	/** The types of projection supported. */
	enum ProjectionType
	{
		PERSPECTIVE, /**< A perspective projection. Uses a truncated rectangular pyramid frustrum. */
		ORTHOGRAPHIC /**< An orthographic projection. Uses a rectangular prism frustrum. */
	};

private:
	DeclareObjectType(FCDTargetedEntity);

	// Camera flags
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, projection, FC("Projection Type")); // ProjectionType enumerated-type.

	// Camera parameters
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, viewY, FC("Vertical View"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, viewX, FC("Horizontal View"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, aspectRatio, FC("Aspect Ratio"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, nearZ, FC("Near-Z Plane Distance"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, farZ, FC("Far-Z Plane Distance"));

public:
	DeclareFlag(HasHorizontalView, 0);
	DeclareFlag(HasVerticalView, 1);
	DeclareFlagCount(2);

public:
	/** Constructor: do not use directly. Create new cameras using the FCDLibrary::AddEntity function.
		@param document The COLLADA document that contains this camera entity.*/
	FCDCamera(FCDocument* document);

	/** Destructor. */
	virtual ~FCDCamera();

	/** Retrieves the entity type for this class. This function is part of the FCDEntity interface.
		@return The entity type: CAMERA. */
	virtual Type GetType() const { return CAMERA; }

	/** Retrieves the type of projection of this camera.
		@return The projection type. */
	inline ProjectionType GetProjectionType() const { return (ProjectionType) *projection; }

	/** Sets the type of projections for this camera.
		@param type The projection type for this camera. */
	inline void SetProjectionType(ProjectionType type) { projection = type; SetDirtyFlag(); }

	/** Retrieves whether this camera is a perspective camera.
		This is the default type of camera.
		@return Whether this camera is a perspective camera.*/
	DEPRECATED(3.05A, GetProjectionType() == FCDCamera::PERSPECTIVE) inline bool IsPerspective() const { return projection == PERSPECTIVE; }

	/** Sets the type of this camera to perspective. */
	DEPRECATED(3.05A, SetProjectionType(FCDCamera::PERSPECTIVE)) inline void SetPerspective() { projection = PERSPECTIVE; SetDirtyFlag(); }

	/** Retrieves whether the perspective camera defines an horizontal field of view.
		If the camera does not define the horizontal field of view, you can calculate
		it using the following formula: horizontal field of view = vertical field of view / aspect ratio.
		@return Whether the perspective camera defines an horizontal field of view. */
	inline bool HasHorizontalFov() const { return GetHasHorizontalViewFlag(); }

	/** Retrieves whether the perspective camera defines a vertical field of view.
		If the camera does not define the vertical field of view, you can calculate
		it using the following formula: vertical field of view = aspect ratio * horizontal field of view.
		@return Whether the perspective camera defines a vertical field of view. */
	inline bool HasVerticalFov() const { return GetHasVerticalViewFlag(); }

	/** Retrieves the horizontal field of view. Before retrieving this value, 
		check whether the camera defines the horizontal field of view using the
		HasHorizontalFov function.
		@return The horizontal field of view, in degrees. */
	inline FCDParameterAnimatableFloat& GetFovX() { return viewX; }
	inline const FCDParameterAnimatableFloat& GetFovX() const { return viewX; } /**< See above. */

	/** Retrieves the vertical field of view. Before retrieving this value, 
		check whether the camera defines the vertical field of view using the
		HasVerticalFov function.
		@return The horizontal field of view, in degrees. */
	inline FCDParameterAnimatableFloat& GetFovY() { return viewY; }
	inline const FCDParameterAnimatableFloat& GetFovY() const { return viewY; } /**< See above. */

	/** Sets the horizontal field of view value for this camera.
		@param fovX The new horizontal field of view, in degrees. */
	void SetFovX(float fovX);

	/** Sets the vertical field of view value for this camera.
		@param fovY The new vertical field of view, in degrees. */
	void SetFovY(float fovY);

	/** Retrieves whether this camera is an orthographic camera.
		@return Whether this camera is an orthographic camera. */
	DEPRECATED(3.05A, GetProjectionType() == FCDCamera::ORTHOGRAPHIC) inline bool IsOrthographic() const { return projection == ORTHOGRAPHIC; }

	/** Sets the type of this camera to orthographic. */
	DEPRECATED(3.05A, SetProjectionType(FCDCamera::ORTHOGRAPHIC)) inline void SetOrthographic() { projection = ORTHOGRAPHIC; SetDirtyFlag(); }

	/** Retrieves whether the orthographic camera defines an horizontal magnification.
		If the camera does not define the horizontal magnification, you can calculate
		it using the following formula: horizontal magnification = vertical magnification / aspect ratio.
		@return Whether the orthographic camera defines an horizontal magnification. */
	inline bool HasHorizontalMag() const { return GetHasHorizontalViewFlag(); }

	/** Retrieves whether the perspective camera defines a vertical magnification.
		If the camera does not define the vertical magnification, you can calculate
		it using the following formula: vertical magnification = aspect ratio * horizontal magnification.
		@return Whether the perspective camera defines a vertical magnification. */
	inline bool HasVerticalMag() const { return GetHasVerticalViewFlag(); }

	/** Retrieves the horizontal magnification. Before retrieving this value, 
		check whether the camera defines the horizontal magnification using the
		HasHorizontalMag function.
		@return The horizontal magnification. */
	inline FCDParameterAnimatableFloat& GetMagX() { return viewX; }
	inline const FCDParameterAnimatableFloat& GetMagX() const { return viewX; } /**< See above. */

	/** Retrieves the vertical magnification. Before retrieving this value, 
		check whether the camera defines the vertical magnification using the
		HasVerticalMag function.
		@return The vertical magnification. */
	inline FCDParameterAnimatableFloat& GetMagY() { return viewY; }
	inline const FCDParameterAnimatableFloat& GetMagY() const { return viewY; } /**< See above. */

	/** Sets the horizontal magnification for this camera.
		@param magX The new horizontal magnification, in degrees. */
	inline void SetMagX(float magX) { return SetFovX(magX); }

	/** Sets the vertical magnification value for this camera.
		@param magY The new vertical magnification, in degrees. */
	inline void SetMagY(float magY) { return SetFovY(magY); }

	/** Retrieves the near-z value for this camera.
		The near-z value represent how close the near-clip
		plane of the view frustum is. For orthographic cameras,
		this value is solely used for depth-buffering.
		@return The near-z value for this camera. */
	inline FCDParameterAnimatableFloat& GetNearZ() { return nearZ; }
	inline const FCDParameterAnimatableFloat& GetNearZ() const { return nearZ; } /**< See above. */

	/** Retrieves the far-z value for this camera. The far-z value represent
		how close the far-clip plane of the view frustum is.
		For orthographic cameras, this value is solely used for depth-buffering.
		@return The far-z value for this camera. */
	inline FCDParameterAnimatableFloat& GetFarZ() { return farZ; }
	inline const FCDParameterAnimatableFloat& GetFarZ() const { return farZ; } /**< See above. */

	/** Retrieves whether the camera defines an aspect ratio.
		@return Whether the camera defines an aspect ratio. */
	inline bool HasAspectRatio() const { return !(GetHasVerticalViewFlag() && GetHasHorizontalViewFlag()); }

	/** Retrieves the aspect ratio for the view of this camera. Before using this value,
		check if there are only one of the horizontal and vertical view ratios.
		If there are both of the view ratios provided for the camera, you will
		need to calculate the aspect ratio using the following formula:
		aspect ratio = vertical field of view / horizontal field of view.
		@return The view aspect ratio. */
	inline FCDParameterAnimatableFloat& GetAspectRatio() { return aspectRatio; }
	inline const FCDParameterAnimatableFloat& GetAspectRatio() const { return aspectRatio; } /**< See above. */

	/** Sets the near-z value for this camera.
		The near-z value represent how close the near-clip
		plane of the view frustum is. For orthographic cameras,
		this value is solely used for depth-buffering.
		@param _nearZ A valid near-z value. No check is made to verify that the
		near-z value is greater than the far-z value.*/
	inline void SetNearZ(float _nearZ) { nearZ = _nearZ; SetDirtyFlag(); }

	/** Sets the far-z value for this camera. The far-z value represent
		how close the far-clip plane of the view frustum is.
		For orthographic cameras, this value is solely used for depth-buffering.
		@param _farZ A valid far-z value. No check is made to verify that the
		far-z value is greater than the near-z value.*/
	inline void SetFarZ(float _farZ) { farZ = _farZ; SetDirtyFlag(); }

	/** Sets the aspect ratio for the view of this camera.
		@param aspectRatio The new view aspect ratio. */
	void SetAspectRatio(float aspectRatio);
};

#endif // _FCD_CAMERA_H_

