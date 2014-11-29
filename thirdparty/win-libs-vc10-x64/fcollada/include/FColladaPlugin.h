/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FColladaPlugin.h
	This file contains the various FCollada plug-in classes
	and the internal FColladaPluginManager class.
*/

#ifndef _FCOLLADA_PLUGIN_H_
#define _FCOLLADA_PLUGIN_H_

#ifndef _FU_PLUGIN_H_
#include "FUtils/FUPlugin.h"
#endif // _FU_PLUGIN_H_

class FUPluginManager;
class FCDObject;
class FCDENode;
class FCDETechnique;

/**
	A FCPExtraTechnique plug-in.
	We use the plug-in in FCollada to parse technique-specific informations.
	There are three techniques that FCollada handles directly but can be overriden
	using one or more plug-ins: "MAYA", "MAX3D" and "FCOLLADA".

	@see FUPlugin
	@ingroup FCollada
*/
class FCOLLADA_EXPORT FCPExtraTechnique : public FUPlugin
{
private:
	DeclareObjectType(FUPlugin);

public:
	/** Retrieves the name of the technique profile that this plug-in handles.
		@return The name of the technique profile. */
	virtual const char* GetProfileName() = 0;

	/** Replaces the extra tree node and generates the profile-specific object for it.
		A temporary extra tree is generated for archiving purposes and will be kept
		after this call only if this function returns NULL.
		@param techniqueNode The extra tree node for the profile-specific technique.
		@param parent The object that is creating this profile-specific object.
		@return An handle to the profile-specific object. When this handle is NULL,
			the FCollada extra tree will be kept. When the extra tree that contains
			this handle is released, the handle will also be released. If you use this
			handle elsewhere, make sure you correctly use the FUTrackedPtr, FUObjectRef,
			FUTrackedList and FUObjectContainer templates. */
	virtual FUTrackable* ReadFromArchive(FCDETechnique* techniqueNode, FUObject* parent) = 0;

	/** Generate a temporary extra tree for the given handle for archiving purposes.
		After archiving is complete, all the temporary extra trees are released.
		@param techniqueNode The extra tree node to contain the profile-specific object.
			This node is in the form: \<technique profile="X"\>
		@param handle A custom object created by the ReadFromArchive function. */
	virtual void WriteToArchive(FCDETechnique* techniqueNode, const FUTrackable* handle) const = 0;

protected:
	/** Destructor.
		Don't destroy directly. Use the Release function. */
	virtual ~FCPExtraTechnique() {}
};

/**
	A FCollada content archiving plugin.
	FCollada utilizes these plugins to import from and export to
	different 3D formats. Aside from supporting the standard XML format, binary fomrats
	can also be supported(E.g. X3D, IFF, OpenFlight).

	@see FUPlugin
	@ingroup FCollada
*/
class FCOLLADA_EXPORT FCPArchive: public FUPlugin
{
private:
	DeclareObjectType(FUPlugin);

public:
	/**	Determine if this plug-in supports import into FCollada.
		@return Whether this plug-in supports import. */
	virtual bool IsImportSupported() = 0;

	/**	Determine if this plug-in supports export from FCollada.
		@return Whether this plug-in supports export. */
	virtual bool IsExportSupported() = 0;

	/** Determine if this plug-in supports exporting part of the FCollada assets.
		@return Whether partial export is supported. */
	virtual bool IsPartialExportSupported() = 0;

	/** Determine if this plug-in supports the given file extension.
		@param ext string for the file extension
		@return 'true' if the given file extension is supported by this plug-in. */
	virtual bool IsExtensionSupported(const char* ext) = 0;

	/** Determine the number of file extension this plug-in supports
		@return number of supported file extensions. */
	virtual int GetSupportedExtensionsCount() = 0;

	/** Retrieve one supported extension.
		@param index the index at which the extension is to be retrieved. 
			Ideally to be less then that returned by GetSupportedExtensionsCount().
		@return the string of the file extension at the given index. NULL is returned
			if the given index is invalid. */
	virtual const char* GetSupportedExtensionAt(int index) = 0;

	/** Adds an extra extension to the list of supported extension by this plugin.
		@param ext The new extension to support.
		@return Whether the extension was added.*/
	virtual bool AddExtraExtension(const char* /*ext*/){ return false; }

	/** Removes an extension that has previously been added to the extra
		extension list.
		@param ext The extension to remove.
		@return Whether the extension was supported and can be removed. */
	virtual bool RemoveExtraExtension(const char* /*ext*/){ return false; }

	/** Import a file into FCollada.
		@param filePath full file path to the file to be imported.
		@param document an empty document to be filled with imported content.
		@return Whether the file is imported successfully. */
	virtual bool ImportFile(const fchar* filePath, FCDocument* document) = 0;

	/** Import a file from the given memory address.  It is the plugin managers
		duty to ensure that the appropriate plugin is selected based on the 
		content of this memory address.
		@param document An empty document to load imported content into.
		@param contents The content to load into the document, in a format suitable
			to be read by this plugin.
		@param length The length of the content buffer.
		@return Whether the file is imported successfully. */
	virtual bool ImportFileFromMemory(const fchar* filePath, FCDocument* document, const void* contents, size_t length) = 0;

	/** Export a file from FCollada.
		@param document a document to be be exported.
		@param filePath full file path to the file to be exported.
		@return Whether the file is exported successfully. */
	virtual bool ExportFile(FCDocument* document, const fchar* filePath) = 0;

	/** Start exporting parts of the FCollada assets.
		Only valid if 'IsPartlyExportSupported()' returns 'true'.
		@param filePath A full file path specifying the location of the target path
			This is in order to process relative file paths correctly.  If this parameter
			is NULL then all paths will be exported absolutely.
		@return Whether the operation succeeded. */
	virtual bool StartExport(const fchar* absoluteFilePath) = 0;

	/** Export one object in FCollada.
		Only valid if 'IsPartlyExportSupported()' returns 'true'.
		@param document the FCDocument that contains the object to be exported.
		@param object the FCDObject to be exported to the file specified in 'StartExport()'
		@return Whether the given object is successfully exported. */
	virtual bool ExportObject(FCDObject* object) = 0;

	/** End the export of the current document started by 'StartExport()'.
		Only valid if 'IsPartlyExportSupported()' returns 'true'.
		This function will release any resources associated with the export.
		@param outData [out] Void pointer to recieve a pointer to the memory chunk to write
		@return Whether the request is valid, and the outData has been filled. */
	virtual bool EndExport(fm::vector<uint8>& outData) = 0;

	/** End the export of the current document started by 'StartExport()'.
		Only valid if 'IsPartlyExportSupported()' returns 'true'.
		This function will release any resources associated with the export.
		@param filePath A full file path specifying the location to write the currently exported
			document too.
		@return Whether the call is valid, and the file is written to disk without error. */
	virtual bool EndExport(const fchar* filePath) = 0;

	/** Load the save data onto an object.  This allows for loads of partial documents 
		@param object The object to recieve the loaded data
		@param data The data to load
		@return Whether the load was successful */
	virtual bool ImportObject(FCDObject* object, const fm::vector<uint8>& data) = 0;

protected:
	/** Destructor.
		Don't destroy directly, use the Release function.*/
	virtual ~FCPArchive() {}
};

/**
	The FCollada plug-ins manager.
*/
class FCOLLADA_EXPORT FColladaPluginManager : public FUObject
{
private:
	DeclareObjectType(FColladaPluginManager);

	typedef FUObjectContainer<FCPExtraTechnique> FCPExtraList;
	typedef FUObjectContainer<FCPArchive> FCPArchiveList;

	FCPExtraList extraTechniquePlugins;
	FCPArchiveList archivePlugins;

	FUPluginManager* loader;

public:
	/** Constructor. */
	FColladaPluginManager();

	/** Retrieve the number of archive plugins that are loaded.
		@return The number of archive plugins loaded. */
	size_t GetArchivePluginsCount() { return archivePlugins.size(); }

	/** Retrieves the archive plugin specified by the given index.
		@param index The archive plugin index.
		@return The plugin pointer on success, NULL otherwise.*/
	FCPArchive* GetArchivePlugin(size_t index){ FUAssert(index < archivePlugins.size(), return NULL); return archivePlugins[index]; }

	/** Manually registers a plugin.
		To manually un-register a plugin, use the plugin->Release() function.
		@param plugin The plugin to manually add to the plugin map.
		@return ?. */
	bool RegisterPlugin(FUPlugin* plugin);
	DEPRECATED(3.05A, RegisterPlugin) inline bool AddPlugin(FCPExtraTechnique* plugin) { return RegisterPlugin(plugin); } /**< See above. */
	DEPRECATED(3.05A, RegisterPlugin) inline bool AddArchivePlugin(FCPArchive* plugin) { return RegisterPlugin(plugin); } /**< See above. */

	/**	Load document to the given file.
		@param document the FCDocument which will be filled with loaded contents.
		@param filename The file name of the file to load.
		@return 'true' if the operation is successful. */
	bool LoadDocumentFromFile(FCDocument* document, const fchar* filename);
	DEPRECATED(3.05A, LoadDocumentFromFile) inline bool LoadDocument(FCDocument* document, const fchar* filename) { return LoadDocumentFromFile(document, filename); } /**< See above. */

	/** Load the given file from the provided memory address.
		@param filename The absolute filename for the data. Used to manage relative paths.
		@param document The FCollada document to fill in.
		@param data The memory buffer containing the raw document.
		@param length The length of the memory buffer. */
	bool LoadDocumentFromMemory(const fchar* filename, FCDocument* document, void* data, size_t length);

	/**	Save document to the given file.
		@param document the FCDocument whose contents are to be writtern in the file.
		@param filename the full path of the file to write.
		@return 'true' if the operation is successful. */
	bool SaveDocumentToFile(FCDocument* document, const fchar* filename);
	DEPRECATED(3.05A, SaveDocumentToFile) inline bool SaveDocument(FCDocument* document, const fchar* filename) { return SaveDocumentToFile(document, filename); } /**< See above. */

private:
	/** Don't allow direct destruction.
		Call the Release() function instead. */
	virtual ~FColladaPluginManager(); 

	/** [INTERNAL] Find the correct plug-in to the document according to the file extension. */
	FCPArchive* FindArchivePlugin(const fchar* filename);

	// Extra Technique plug-ins support
	typedef fm::pvector<FCDETechnique> FCDETechniqueList;
	void PostImportDocument(FCDocument* document);
	void PreExportDocument(FCDocument* document, FCDETechniqueList& techniques);
	void PostExportDocument(FCDocument* document, FCDETechniqueList& techniques);

	/** Make a processed map of CRC32 profile names vs plug-ins to handle them. */
	typedef fm::map<uint32, FCPExtraTechnique*> FCPExtraMap;
	void CreateExtraTechniquePluginMap(FCPExtraMap& map);
};

#endif // _FCOLLADA_PLUGIN_H_
