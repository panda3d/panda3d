/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUSingleton.h
	This file contains the FUSingleton template class.
*/

#ifndef _FU_SINGLETON_H_
#define _FU_SINGLETON_H_

/**
	A Singleton class.

	Use these macros to easily implement singletons.
	A singleton is a class which has only one object of this class.
	The advantage of a singleton over a static class is that the
	application controls when and how the singleton is created and
	destroyed. The disadvantage of a singleton is that you have one
	extra memory lookup to do.

	There are four functions created by the macro:

	CreateSingleton: initializes the singleton.
		You will need to call this function before any
		attempts to access the singleton are made.

	DestroySingleton: release the singleton.
		As with the singleton creation, this function
		needs to be called in the expected order.

	GetSingleton: retrieves the singleton.

	@ingroup FUtils
*/
#define DeclareSingletonClass(_SingletonClass) \
private: \
	static _SingletonClass* singleton; \
public: \
	static _SingletonClass* CreateSingleton(); \
	static void DestroySingleton(); \
	static inline _SingletonClass* GetSingleton() { return singleton; } \
private:

/** Implements once the singleton pointer container.
	@param _SingletonClass The class name for the singleton class. */
#define ImplementSingletonClass(_SingletonClass) \
	_SingletonClass* _SingletonClass::CreateSingleton() { \
    	FUAssert(singleton == NULL, return singleton); \
		return singleton = new _SingletonClass(); } \
	void _SingletonClass::DestroySingleton() { \
		FUAssert(singleton != NULL, return); \
		SAFE_DELETE(singleton); } \
	_SingletonClass* _SingletonClass::singleton = NULL

#endif // _FU_SINGLETON_H_
