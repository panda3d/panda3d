/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDocumentTools.h
	This file contains the FCDocumentTools namespace.
*/

#ifndef _FC_DOCUMENT_TOOLS_H_
#define _FC_DOCUMENT_TOOLS_H_

/**
	Contains pre-processing tools or post-processing tools used to
	modify whole COLLADA documents.
	@ingroup FCDocument
*/
namespace FCDocumentTools
{
	/** Standardizes the up-axis and the length units accross the whole document.
		Regardless of whether your application can handle different up-axis values or
		different length units, you should call this function at least once
		to ensure that the whole document has only one up-axis and one length unit.
		@param document The COLLADA document to process.
		@param upAxis The wanted up-axis for the whole document. If the
			zero-vector is given, the up-axis of the document-level asset information
			structure is used.
		@param unitInMeters The wanted length unit. If this value is zero, the length
			unit of the document-level asset information structure is used.
		@param handleTargets This flag is very specific to ColladaMax which handles the
			specificities of targeted cameras and lights: the pivots cannot be modified
			simply, since the re-targeting will happen before the pivot transform is done. */
	void FCOLLADA_EXPORT StandardizeUpAxisAndLength(FCDocument* document, const FMVector3& upAxis = FMVector3::Origin, float unitInMeters = 0.0f, bool handleTargets=false);

};

#endif // _FC_DOCUMENT_TOOLS_H_
