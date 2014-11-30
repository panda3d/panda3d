/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FU_PLUGIN_MANAGER_H_
#define _FU_PLUGIN_MANAGER_H_

class FUObjectType;
class FUPlugin;

/**
	A generic plug-in manager.
	Use this structure to attach and detach plug-in libraries
	as well as load and unload plug-ins.
	For a plug-in library to be valid, it must expose three functions:
	- GetPluginCount
	- GetPluginType
	- CreatePlugin
	The implementation of these functions and their exposition is highly OS-dependant.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUPluginManager
{
public:
	/** Callback to retrieve the number of plug-ins contained within a library.
		Necessary plugin function definition.
		Under Win32, this function MUST be a DLL export and called "GetPluginCount". */
	typedef uint32 (*GetPluginCount)(void);

	/** Callback to retrieve the type of a given library plug-in.
		Necessary plugin function definition.
		Under Win32, this function MUST be a DLL export and called "GetPluginType". */
	typedef const FUObjectType* (*GetPluginType)(uint32 index);

	/**	Callback to create a given library plug-in.
		Necessary plugin function definition.
		Under Win32, this function MUST be a DLL export and called "CreatePlugin". */
	typedef FUPlugin* (*CreatePlugin)(uint32 index);

private:
	struct PluginLibrary
	{
		fstring filename;
#if defined(WIN32)
		HMODULE module;
#elif defined(__APPLE__) || defined(LINUX)
		void* module;
#endif // WIN32

		GetPluginCount getPluginCount;
		GetPluginType getPluginType;
		CreatePlugin createPlugin;
	};
	typedef fm::pvector<PluginLibrary> PluginLibraryList;

	fstring pluginFolderName;
	PluginLibraryList loadedLibraries;
	FUObjectContainer<FUPlugin> loadedPlugins;

public:
	/** Constructor.
		Look for the "plugins" folder within the given application filepath
		and attaches itself to the plug-in libraries it finds within the given file filter.
		@param pluginLibraryFilter A filter string for plug-in library files.
			An example of a valid filter on Win32 is "*.fcp" or "*.fcp|*.dll". */
	FUPluginManager(const fchar* pluginLibraryFilter);

	/** Destructor.
		Releases all the loaded plug-ins and detaches all
		the plug-in libraries. */
	virtual ~FUPluginManager();
	
	/** Add one custom plug-in library.
		This function is useful when dealing with static plug-in
		libraries.
		@param fnGetPluginCount The library's plugin count retrieval function.
		@param fnGetPluginType The library's plugin type retrieval function.
		@param fnCreatePlugin The library's plugin creation function. */
	void AddPluginLibrary(FUPluginManager::GetPluginCount fnGetPluginCount, FUPluginManager::GetPluginType fnGetPluginType, FUPluginManager::CreatePlugin fnCreatePlugin);

	/** Loads the plug-ins from the loaded plug-in libraries.
		@param pluginType The minimum object type for the plug-ins
			that must be loaded. This filters all the plug-ins within
			the plug-in libraries in order to creates only wanted
			plug-ins. When the given type is FUObject::GetClassType,
			then all plug-ins will always be loaded. */
	void LoadPlugins(const FUObjectType& pluginType);

	/** Unloads all the loaded plug-ins.
		This function releases all the loaded plug-ins,
		but does not detach all the plug-in libraries. */
	void UnloadPlugins();

	/** Retrieves the number of loaded plug-ins.
		@return The number of loaded plug-ins. */
	inline size_t GetLoadedPluginCount() { return loadedPlugins.size(); }
	
	/** Retrieves all the loaded plug-ins.
		@return The loaded plug-ins list. */
	inline FUPlugin** GetLoadedPlugins() { return loadedPlugins.begin(); }
	inline const FUPlugin** GetLoadedPlugins() const { return loadedPlugins.begin(); } /**< See above. */
	
	/** Retrieves a loaded plug-in.
		@param index The index of the loaded plug-in.
		@return The plug-in at the given index. */
	inline FUPlugin* GetLoadedPlugin(size_t index) { FUAssert(index < loadedPlugins.size(), return NULL); return loadedPlugins.at(index); }
	inline const FUPlugin* GetLoadedPlugin(size_t index) const { FUAssert(index < loadedPlugins.size(), return NULL); return loadedPlugins.at(index); } /**< See above. */

private:
	void LoadPluginsInFolderName(const fstring& folderName, const fchar* pluginLibraryFilter);
};

#endif // _FU_PLUGIN_MANAGER_H_
