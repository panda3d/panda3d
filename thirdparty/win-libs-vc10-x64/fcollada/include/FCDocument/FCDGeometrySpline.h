/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/**
	@file FCDGeometrySpline.h
	This file contains the FCDGeometrySpline class.
	The FCDGeometrySpline class hold the information for one COLLADA geometric spline.
*/
#ifndef _FCD_GEOMETRY_SPLINE_H_
#define _FCD_GEOMETRY_SPLINE_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

class FCDocument;
class FCDGeometry;
class FCDBezierSpline;
class FCDNURBSSpline;

/** Represents a generic spline.
	A FCSpline contains a list of control vertices and a closed attribute which defaults to false.*/
class FCOLLADA_EXPORT FCDSpline : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FUDaeSplineForm::Form form;
	fm::string name;

protected:
	FMVector3List cvs; /**< The list of control vertices. */

public:
	/** Constructor.
		@param document The FCollada document that owns this spline. */
	FCDSpline(FCDocument* document);

	/** Destructor. */
	virtual ~FCDSpline();

	/** Retrieves the type of the spline. This is the only method of the FCDSpline interface.
		@return FUDaeSplineType of the spline.*/
	virtual FUDaeSplineType::Type GetSplineType() const = 0;

	/** Gets the name of the spline.
		@return The spline name.*/
	inline fm::string& GetName() { return name; }
	inline const fm::string& GetName() const { return name; } /**< See above.*/

	/** Sets the name of the spline.
		@param _name The new name.*/
	inline void SetName(const fm::string& _name) { name = _name; }

	/** Retrieves if the spline is closed or not.
		@return The closed boolean value.*/
	inline bool IsClosed() const { return form == FUDaeSplineForm::CLOSED; }

	/** Sets if the spline is closed or not.
		@param closed The closed attribute value.*/
	inline void SetClosed(bool closed) { form = (closed) ? FUDaeSplineForm::CLOSED : FUDaeSplineForm::OPEN; }

	/** Retrieves the number of CVs in the spline.
		@return The number of control vertices.*/
	inline size_t GetCVCount() const { return cvs.size(); }

	/** Retrieves a pointer to the control vertex specified by the given index.
		@param index The index, must be higher or equal to 0 and lower than GetCVCount().
		@return The control vertex.*/
	inline FMVector3* GetCV(size_t index) { FUAssert(index < GetCVCount(), return NULL); return &(cvs.at(index)); }
	inline const FMVector3* GetCV(size_t index) const { FUAssert(index < GetCVCount(), return NULL); return &(cvs.at(index)); } /**< See above. */

	/** Retrieves a reference to the CVs list.
		@return The reference to the control vertices. */
	inline FMVector3List& GetCVs() { return cvs; }
	inline const FMVector3List& GetCVs() const { return cvs; } /**< See above. */

	/** Empty the spline's control vertex list. */
	inline void ClearCVs() { cvs.clear(); }

	/** [INTERNAL] Copies the spline into a clone.
		The clone may reside in another document.
		@param clone The empty clone. This pointer cannot be NULL.
		@return The clone. */
	virtual FCDSpline* Clone(FCDSpline* clone) const;
};

typedef fm::pvector<FCDSpline> FCDSplineList; /**< A dynamically-sized array of FCSpline.*/
typedef FUObjectContainer<FCDSpline> FCDSplineContainer; /**< A dynamically-sized containment array of FCSpline.*/
typedef fm::pvector<FCDBezierSpline> FCDBezierSplineList; /**< A dynamically-sized array of FCDBezierSpline. @ingroup FCDGeometry */
typedef fm::pvector<FCDNURBSSpline> FCDNURBSSplineList; /**< A dynamically-sized array of FCDNURBSSpline. @ingroup FCDGeometry */

/** Represents a Linear spline.

	Linear splines are, like the Bezier splines, represented as an array of
	adjacent linear segment. Each segment consisting of 2 control vertices, the
	last one being reused as the first vertex of the next segment. If the spline
	is closed, the first vertex is also reused for the last vertex of the last segment.
*/
class FCOLLADA_EXPORT FCDLinearSpline : public FCDSpline
{
private:
	DeclareObjectType(FCDSpline);

public:
	/** Constructor.
		@param document The FCollada document that owns this spline. */
	FCDLinearSpline(FCDocument* document);

	/** Destructor. */
	virtual ~FCDLinearSpline();

	/** FCDSpline method implementation.
		@return The LINEAR spline type.*/
	virtual FUDaeSplineType::Type GetSplineType() const { return FUDaeSplineType::LINEAR; }

	/** Adds a CV to a Linear spline
		@param cv 3D position of the CV.*/
	bool AddCV(const FMVector3& cv){ cvs.push_back(cv); return true; }

	/** Convert the linear segments contained inside this linear spline into Bezier segments.
		@param toFill The Bezier spline to fill with linear information.*/
	void ToBezier(FCDBezierSpline& toFill);

	/** Determines if the spline is valid.
		@return True is the spline is valid, false otherwise.*/
	virtual bool IsValid() const;
};

/** Represents a Bezier spline.
	
	The Bezier spline is represented as an array of adjacent cubic Bezier segments.
	Each segment consists of 4 control vertices, the last one being reused as the
	first vertex of the next segment. If the spline is closed, the first control vertex
	is also reused for the last vertex of the last segment.
*/
class FCOLLADA_EXPORT FCDBezierSpline : public FCDSpline
{
private:
	DeclareObjectType(FCDSpline);

public:
	/** Constructor.
		@param document The FCollada document that owns this spline. */
	FCDBezierSpline(FCDocument* document);
	
	/** Destructor. */
	virtual ~FCDBezierSpline();

	/** FCDSpline method implementation.
		@return The BEZIER spline type.*/
	virtual FUDaeSplineType::Type GetSplineType() const { return FUDaeSplineType::BEZIER; }

	/** Adds a CV to a Bezier spline
		@param cv 3D position of the CV.*/
	bool AddCV(const FMVector3& cv){ cvs.push_back(cv); return true; }

	/** Creates one NURB per Bezier segment and appends it to the provided NURB list.
		@param toFill The NURB list to fill.*/
	void ToNURBs(FCDNURBSSplineList &toFill) const;

	/** Determines if the spline is valid.
		@return True is the spline is valid, false otherwise.*/
	virtual bool IsValid() const;
};

/**
	Represents a NURBS spline.

	This is a typical NURBS spline. It uses a list of FMVector3 for its control vertices,
	along with a list of float to represent each vertex weight. A knot vector (uniform or not,
	clamped or not), together with a degree, complete the specification.
*/
class FCOLLADA_EXPORT FCDNURBSSpline : public FCDSpline
{
private:
	DeclareObjectType(FCDSpline);

	FloatList weights;
	FloatList knots;
	uint32 degree;

public:
	/** Constructor.
		@param document The FCollada document that owns this spline. */
	FCDNURBSSpline(FCDocument* document);
	
	/** Destructor. */
	virtual ~FCDNURBSSpline();

	/** FCDSpline method implementation.
		@return The NURBS spline type.*/
	virtual FUDaeSplineType::Type GetSplineType() const { return FUDaeSplineType::NURBS; }

	/** Get the degree for this NURBS.
		@return The degree.*/
	inline uint32 GetDegree() const { return degree; }

	/** Set the degree for this NURBS.
		@param deg The wanted degree.*/
	inline void SetDegree(uint32 deg){ degree = deg; }

	/** Add a control vertex as a 3D position and a weight attribute specific to this CV.
		@param cv The 3D position.
		@param weight The weight attribute.*/
	bool AddCV(const FMVector3& cv, float weight);

	/** Retrieves a reference to the weight specified by the index.
		@param index The index.
		@return The address of the weight value, NULL if index is invalid.*/
	inline float* GetWeight(size_t index) { FUAssert(index < GetCVCount(), return NULL); return &(weights.at(index)); }
	inline const float* GetWeight(size_t index) const { FUAssert(index < GetCVCount(), return NULL); return &(weights.at(index)); } /**< See above. */

	/** Retrieves the knot count in this NURB.
		@return The knot count.*/
	inline size_t GetKnotCount() const { return knots.size(); }

	/** Add a knot to this NURB.
		@param knot The knot value.*/
	inline void AddKnot(float knot) { knots.push_back(knot); }

	/** Retrieves a reference to the knot specified by the index.
		@param index The index.
		@return The address of the knot value, NULL if index is invalid.*/
	inline float* GetKnot(size_t index) { FUAssert(index < GetKnotCount(), return NULL); return &(knots.at(index));}
	inline const float* GetKnot(size_t index) const { FUAssert(index < GetKnotCount(), return NULL); return &(knots.at(index));} /**< See above. */

	/** Retrieves a const reference to the weight list.
		@return The weights' const reference.*/
	inline FloatList& GetWeights() { return weights; }
	inline const FloatList& GetWeights() const { return weights; } /**< See above. */

	/** Retrieves a const reference to the knot list.
		@return The knots' const reference.*/
	inline FloatList& GetKnots() { return knots; }
	inline const FloatList& GetKnots() const { return knots; } /**< See above. */

	/** Determines if the spline is valid.
		@return True is the spline is valid, false otherwise.*/
	virtual bool IsValid() const;

	/** [INTERNAL] Copies the spline into a clone.
		The clone may reside in another document.
		@param clone The empty clone. This pointer cannot be NULL.
		@return The clone. */
	virtual FCDSpline* Clone(FCDSpline* clone) const;
};

/**
	A COLLADA geometric spline.

	A COLLADA spline contains an array of FCDSpline of the same type.

	@todo: Insert the mathematical formula to calculate the spline position.

	@ingroup FCDGeometry
*/
class FCOLLADA_EXPORT FCDGeometrySpline : public FCDObject
{
private:
	DeclareObjectType(FCDObject);
	FCDGeometry* parent;

	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, type, FC("Spline Type")); // FUDaeSplineType::Type;
	DeclareParameterContainer(FCDSpline, splines, FC("Splines"));

public:
	/** Constructor: do not use directly. Use the FCDGeometry::CreateMesh function instead.
		@param document The COLLADA document that owns the new spline.
		@param parent The geometry entity that contains the new spline. */
	FCDGeometrySpline(FCDocument* document, FCDGeometry* parent);

	/** Destructor. */
	virtual ~FCDGeometrySpline();

	/** Retrieves the parent of this geometric spline: the geometry entity.
		@return The geometry entity that this spline belongs to. */
	FCDGeometry* GetParent() { return parent; }
	const FCDGeometry* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the type of this geometry spline.
		@return The type.*/
	FUDaeSplineType::Type GetType() const { return (FUDaeSplineType::Type) *type; }

	/** Sets the spline type for this geometry spline.
		Changing the type of a geometry spline that contains sub-splines will clear all its sub-splines.
		@param _type The type. */
	bool SetType(FUDaeSplineType::Type _type);

	/** Retrieves the number of splines in this geometry spline.
		@return The spline count.*/
	size_t GetSplineCount() const { return splines.size(); }

	/** Retrieves the total amount of control vertices in the spline array.
		@return The total CV count.*/
	size_t GetTotalCVCount();

	/** Retrieves a pointer to the spline specified by the given index.
		@param index The index, higher or equal to 0 and lower than GetSplineCount().
		@return The FCDSpline pointer, or NULL if index is invalid.*/
	FCDSpline* GetSpline(size_t index){ FUAssert(index < GetSplineCount(), return NULL); return splines[index]; }
	const FCDSpline* GetSpline(size_t index) const { FUAssert(index < GetSplineCount(), return NULL); return splines[index]; }/**< see above */

	/** Adds a spline to this geometry spline.
		@param type The type of spline to create.
			Set the type to FUDaeSplineType::UNKNOWN to create a spline that has
			the same type as this geometry spline.
		@return The new spline. This pointer will be NULL if the requested spline
			type is different from existing splines. This pointer will be NULL
			if the requested type is FUDaeSplineType::UNKNOWN and this geometry spline
			doesn't yet have a type assigned. */
	FCDSpline* AddSpline(FUDaeSplineType::Type type = FUDaeSplineType::UNKNOWN);

	/** Converts the Bezier splines in this geometry to a list of NURBS splines.
		@param toFill The list of NURBS to fill. An empty list if the type of this geometry is not BEZIER.*/
	void ConvertBezierToNURBS(FCDNURBSSplineList& toFill);

	/** Copies the spline into a clone.
		The clone may reside in another document.
		@param clone The empty clone. If this pointer is NULL, a new spline
			will be created and you will need to release the returned pointer manually.
		@return The clone. */
	FCDGeometrySpline* Clone(FCDGeometrySpline* clone = NULL) const;
};

#endif // _FCD_GEOMETRY_SPLINE_H_
