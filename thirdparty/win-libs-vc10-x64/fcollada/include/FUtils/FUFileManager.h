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

#ifndef _FU_FILE_MANAGER_H_
#define _FU_FILE_MANAGER_H_

#ifndef _FU_URI_H_
#include "FUtils/FUUri.h"
#endif //_FU_URI_H_

#ifndef _FM_ARRAY_H_
#include "FMath/FMArray.h"
#endif //_FM_ARRAY_H_

class FUFile;

/** A scheme callback to load remote files.
	Takes the remote file FUUri as parameter, and returns the absolute file
	path of a local, possibly cached file.*/
typedef IFunctor1<const FUUri&, fstring> SchemeLoadCallback;

/** A scheme callback to test the existence of remote files.
	Takes the remote file FUUri as parameter, and returns True if the remote
	file exists, and False otherwise.*/
typedef IFunctor1<const FUUri&, bool> SchemeExistsCallback;

/** Provides pre-processing of files before opening.
	Takes inFilename as an argument.  If the file is processed,
	then outFilename should be set to the filename of the new file,
	and the funtion returns true. The current document URI is then
	overriden by the new filename. */
typedef IFunctor2<const fstring&, fstring&, bool>	SchemePreProcessCallback;

/** A scheme callback that will be called when the file is complete.
	Sends the absolute path of a local, possibly cached file and the id. */
typedef IFunctor2<const fstring&, size_t, void> SchemeOnCompleteCallback;

/** A scheme callback that will request the file to be fetched
	Sends the absolute path of a remote file, a callback to call on completion
	and a user data field which can be used to identify the request. */
typedef IFunctor3<const FUUri&, SchemeOnCompleteCallback*, size_t, void> SchemeRequestFileCallback;

/** An helper structure to organize file scheme callbacks within the
	FUFileManager class.
	Use the NewSchemeCallbacks method to get a new SchemeCallbacks object.
	Use the NewFUFunctor* methods to assign callbacks.*/
struct FCOLLADA_EXPORT SchemeCallbacks
{
	SchemeLoadCallback* load;		/**< The file open callback.*/
	SchemeExistsCallback* exists;	/**< The file existence test callback.*/
	fm::vector<SchemePreProcessCallback*> openers; /**< Callbacks to process files before opening */
	SchemeRequestFileCallback* request;

	/** [INTERNAL] Default constructor.*/
	SchemeCallbacks();
	/** [INTERNAL] Copy constructor.*/
	SchemeCallbacks(const SchemeCallbacks& copy);
	/** [INTERNAL] Destructor.*/
	~SchemeCallbacks();
};

/** A function to instantiate SchemeCallbacks within the FCollada DLL.
	@return The new scheme callbacks instance.*/
FCOLLADA_EXPORT SchemeCallbacks* NewSchemeCallbacks();

/** Handles most file system related operations.
	Is useful mostly for platform-abstraction and to handle the relative paths
	within COLLADA documents. */
class FCOLLADA_EXPORT FUFileManager
{
private:
	typedef fm::map<FUUri::Scheme, SchemeCallbacks*> SchemeCallbackMap;

	FUUriList pathStack;
	bool forceAbsolute;

	SchemeCallbackMap schemeCallbackMap;
public:
	/** Constructor.
		When creating a new file manager, the file system's current file
		path is retrieved and placed on the file path stack. */
	FUFileManager();

	/** Destructor. */
	~FUFileManager();

	/** Retrieves the current file path.
		This is the file path used when creating all relative paths.
		@return The current file path. */
	inline const FUUri& GetCurrentUri() const { return pathStack.back(); }

	/** Sets a new current file path.
		Files paths are placed on a stack in order to easily return
		to previous file paths.
		@param path The new file path. */
	void PushRootPath(const fstring& path);

	/** Sets a new current file path using a document filename.
		Files paths are placed on a stack in order to easily return
		to previous file paths.
		@param filename A document filename. */
	void PushRootFile(const fstring& filename);

	/** Removes the current file path from the stack. */
	void PopRootPath();
	void PopRootFile(); /**< See above. */

	/** Opens a file.
		@see FUFile.
		@param filename A file path with a filename.
		@param write Whether to open the file for writing as opposed
			to opening the file for reading.
		@return The file handle. */
	FUFile* OpenFile(const fstring& filename, bool write=false, SchemeOnCompleteCallback* onComplete=NULL, size_t userData = 0);

	/** Makes the directory.
		Currently, the directory must be one level from a directory that exists
		@param directory The path to the directory.
		@return True if the directory exists after this call, whether it was created here or not. False if the 
			directory does not exist. */
	static bool MakeDirectory(const fstring& directory);

	/** Determines whether a file exists or not.
		@param filename A file path.
		@return True if the file exists, false otherwise.*/
	bool FileExists(const fstring& filename);

	/** Strips the filename from the full file path.
		@param filename The full file path, including the filename.
		@return The file path without the filename. */
	static fstring StripFileFromPath(const fstring& filename);

	/** Retrieves the extension of a filename
		@param filename A filename, may include a file path.
		@return The extension of the filename. */
	static fstring GetFileExtension(const fstring& filename);

	/** Extracts the network hostname for a URI and returns it.
		@param filename A file URI.
		@return The network hostname. */
	static fstring ExtractNetworkHostname(fstring& filename);

	/** Sets an internal flag that will force all returned file paths
		and URIs to be absolute, rather than relative to the current file path.
		@param _forceAbsolute Whether all file paths should be absolute. */
	void SetForceAbsoluteFlag(bool _forceAbsolute) { forceAbsolute = _forceAbsolute; }

	/** Prepares a URI for export.
		@param uri A file URI.
		@return The patched URI string. */
	fstring CleanUri(const FUUri& uri);

	/** Retrieves the absolute folder name where the application resides.
		@return The absolute folder name of the application. */
	static fstring GetApplicationFolderName();

	/** Retrieves the absolute folder name where the FCollada library resides.
		@return The absolute folder name of the FCollada library. */
	static fstring GetModuleFolderName();

	/** Retrieves the folder name from a full path name. 
		For example, C:/dirname/file.ext will return C:/dirname
		@param folder The folder name.
		@param path The path name will be returned to this parameter.
	*/
	static void GetFolderFromPath(const fstring& folder, fstring& path);

	/** Sets the callbacks overriding the actions of a specified scheme.
		The provided callbacks are owned by the FUFileManager, and thus deleted if
		modified or released.
		@see NewSchemeCallbacks.
		@param scheme The scheme to override.
		@param callbacks The callbacks structure. */
	void SetSchemeCallbacks(FUUri::Scheme scheme, SchemeCallbacks* callbacks);

	/** Removes the callbacks that override actions of a specified scheme.
		@param scheme The scheme to remove the callback.
		@return The callback, if any, that was associated to this scheme. */
	void RemoveSchemeCallbacks(FUUri::Scheme scheme);

	/** Removes all the scheme callbacks registered on this file manager.*/
	void RemoveAllSchemeCallbacks();

	/** [INTERNAL] Clone the scheme callbacks off the passed in file manager
		@param srcFileManager The File manager to clone our schemes off */
	void CloneSchemeCallbacks(const FUFileManager* srcFileManager);
};

#endif // _FU_FILE_MANAGER_H_

