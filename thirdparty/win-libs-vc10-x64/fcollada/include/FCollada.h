/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@mainpage FCollada Documentation
	
	@section intro_sec Introduction
	The FCollada classes are designed to contain COLLADA documents.

	@section install_sec Installation

	@subsection step1 Step 1: Download
	You can download the FCollada libraries from our website: http://www.feelingsoftware.com
	
	@section copyright Copyright
	Copyright (C) 2005-2007 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FCOLLADA_H_
#define _FCOLLADA_H_

/**
	FCollada exception handling.
	Force this #define to 0 to disallow exception handling within the FCollada library.
	By default, a debug library will not handle exceptions so that your debugger can.
	In release, all exceptions should be handled so that your users receive a meaningful message,
	rather than crash your application. Force this #define to 0 only if your platform does not
	support exception handling.
*/
#ifdef _DEBUG
#define	FCOLLADA_EXCEPTION 0
#define _FTRY 
#define _FCATCH_ALL for (int x = 0; x != 0;)
#else
#define FCOLLADA_EXCEPTION 1
#define _FTRY try
#define _FCATCH_ALL catch (...) 
#endif

#define PREMIUM

#ifndef _F_UTILS_H_
#include "FUtils/FUtils.h"
#endif // _F_UTILS_H_

/** 
	FCollada version number.
	You should verify that you have the correct version, if you use the FCollada library as a DLLs.
	For a history of version, check the Changes.txt file.

	Most significant 16 bits represents the major version number and least
	significant 16 bits represents the minor number of the major version. 

	@ingroup FCollada
*/
#define FCOLLADA_VERSION 0x00030005 /* MMMM.NNNN */

/** FCollada build string.
	This string is meant to be appended to all user interface messages.
	@ingroup FCollada
*/
#define FCOLLADA_BUILDSTR "B"

// The main FCollada class: the document object.
class FCDocument;
class FUTestBed;
class FColladaPluginManager;
typedef fm::pvector<FCDocument> FCDocumentList;
class FUPlugin;
typedef IFunctor0<bool>* CancelLoadingCallback;

/** 
	This namespace contains FCollada global functions and member variables
*/
namespace FCollada
{
	/** Retrieves the FCollada version number.
		Used for DLL-versions of the FCollada library: verify that you have
		a compatible version of the FCollada library using this function.
		@return The FCollada version number. */
	FCOLLADA_EXPORT unsigned long GetVersion();

	/** Initializes the FCollada library.
		This call is necessary for static library versions of FCollada,
		as it initializes the plug-in structure. The initialization count
		is referenced counters, so you can safely call this function multiple times.
		You will need to call the FCollada::Release function as many times as it
		is initialized to correctly detach the plug-ins.
		In the DLL versions of FCollada, the DllMain function initializes the library.*/
	FCOLLADA_EXPORT void Initialize();

	/** Releases the FCollada library.
		This call will detach the FCollada plug-ins and release all the top-level
		FCollada documents. This call is referenced counted and the number
		of initializations must match the number of releases. Only the last release
		will detach the plug-ins and release the top-level documents.
		In the DLL versions of FCollada, the DllMain function should
		do the last release of the library.
		@return The number of initialization calls left to release. */
	FCOLLADA_EXPORT size_t Release();

	/**	Creates a new top FCDocument object.
		You must used this function to create your top-level FCDocument object if you use external references.
		Note: in order to safely delete a FCollada top document, you must call the document's Release() method.
		Otherwise, the top document object container won't be notified, and you could assert in
		FCollada::Release().
		@return A new top document object. */
	FCOLLADA_EXPORT FCDocument* NewTopDocument();

	/**	Creates a new FCDocument object.
		Use this function to create an unmanaged FCDocument.
		Any documents created using this method must either be xreffed
		onto the scene or deleted manually using FUObject::Release
		@return A new document object. */
	FCOLLADA_EXPORT FCDocument* NewDocument();

	/** Retrieves the number of top documents.
		@return The number of top documents. */
	FCOLLADA_EXPORT size_t GetTopDocumentCount();

	/** Retrieves a top document.
		@param index The index of the top document.
		@return The top document at the given index. */
	FCOLLADA_EXPORT FCDocument* GetTopDocument(size_t index);

	/** Retrieves whether a document is a top document.
		@param document The document to verify.
		@return Whether the document is a top document. */
	FCOLLADA_EXPORT bool IsTopDocument(FCDocument* document);

	/** Retrieves the list of all the document currently loaded by FCollada.
		@param documents The list of documents to fill in.
			Thist list is cleared of all its content at the beginning of the function. */
	FCOLLADA_EXPORT void GetAllDocuments(FCDocumentList& documents);

	/** Load document.
		@param filename the string of the file to load from
		@return the loaded FCDocument. NULL is returned if any error occurs. */
	FCOLLADA_EXPORT bool LoadDocumentFromFile(FCDocument* document, const fchar* filename);
	DEPRECATED(3.05A, LoadDocumentFromFile) inline bool LoadDocument(FCDocument* document, const fchar* filename) { return LoadDocumentFromFile(document, filename); }
	DEPRECATED(3.05A, NewTopDocument and LoadDocumentFromFile) FCOLLADA_EXPORT FCDocument* LoadDocument(const fchar* filename);

	/** Load Document from memory address.
		@param document An empty document to load imported content into.
		@param contents The content to load into the document, in a format suitable
			to be read by this plugin.
		@param length The length of the content buffer.
		@return true if the file is imported successfully. */
	FCOLLADA_EXPORT bool LoadDocumentFromMemory(const fchar* filename, FCDocument* document, void* data, size_t length);

	/** Save document.
		@param document The FCollada document to be written on to the disk.
		@param filename the string of the file name to which the content is saved.
		@return Whether the file is saved correctly. */
	FCOLLADA_EXPORT bool SaveDocument(FCDocument* document, const fchar* filename);

	/** Retrieves the global dereferencing flag.
		Setting this flag will force all the entity instance to automatically
		attempt to open the externally-referenced documents when needed.
		The default behavior is to always dereference when needed.
		@return Whether to automatically dereference the entity instances. */
	FCOLLADA_EXPORT bool GetDereferenceFlag();

	/** Sets the global dereferencing flag.
		Setting this flag will force all the entity instance to automatically
		attempt to open the externally-referenced documents when needed.
		The default behavior is to always dereference when needed.
		@param flag Whether to automatically dereference the entity instances. */
	FCOLLADA_EXPORT void SetDereferenceFlag(bool flag);

	/**	Registers a new FUPlugin plug-in to the FColladaPluginManager.
		@deprecated Use GetPluginManager()->AddPlugin() instead.
		@param plugin The new plugin to register. */
	DEPRECATED(3.05A, GetPluginManager()->RegisterPlugin) FCOLLADA_EXPORT bool RegisterPlugin(FUPlugin* plugin);

	/** Retrieves the FCollada plug-ins manager.
		@return The plug-in manager. */
	FCOLLADA_EXPORT FColladaPluginManager* GetPluginManager();

#ifndef RETAIL
	/** Runs the FCollada-specific automated tests and sets the
		results within the given test bed.
		@param testBed A test-bed against which the tests will be run. */
	FCOLLADA_EXPORT void RunTests(FUTestBed& testBed);
#endif // RETAIL

	/** Registers a callback function that will be called to check if 
		we want to cancel the loading of the FCollada document.
		@param callback The callback functor that will tell us if we cancel or not */
	FCOLLADA_EXPORT void SetCancelLoadingCallback(CancelLoadingCallback callback);

	/** Check if we should cancel the loading of the FCollada document.
		@return whether we should cancel the loading. */
	FCOLLADA_EXPORT bool CancelLoading();
}

/** @defgroup FCollada FCollada Library Classes.
	These classes, namespaces and functions are top-level and contain information
	and control processes that affects all documents. */

#endif // _FCOLLADA_H_
