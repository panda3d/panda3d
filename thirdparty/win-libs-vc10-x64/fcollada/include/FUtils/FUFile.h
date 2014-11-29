/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUFile.h
	This file contains the FUFile class.
*/

#ifndef _FU_FILE_H_
#define _FU_FILE_H_

/**
	A cross-platform file.
	This class encapsulates synchronous file I/O across different
	platforms and supports Unicode where the platform does.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUFile
{
public:
	/** The file open mode. */
	enum Mode
	{
		READ, /**< Open for reading only. */
		WRITE /**< Open for writing only. */
	};

private:
	FILE* filePtr;
	fstring filepath;
	
public:
	/** Constructor.
		@param filename The filename of the file to open.
		@param mode The opening mode. */
	FUFile(const fstring& filename, Mode mode);
	FUFile(const fchar* filename, Mode mode); /**< See above. */

	/** Default constructor.
		The file object will not be attached: use the Open function
		to attach the file object to a file on the file system. */
	FUFile();

	/** Destructor.
		Automatically flushes/closes any file left opened. */
	~FUFile();

	/** Retrieves the absolute, OS-specific path used to open this file.
		@return The path.*/
	inline const fstring& GetFilePath() const { return filepath; }

	/** Opens a file on the file system.
		@param filename The filename of the file to open.
		@param mode The opening mode.
		@return Whether the open command was successful. */
	inline bool Open(const fstring& filename, Mode mode) { return Open(filename.c_str(), mode); }
	bool Open(const fchar* filename, Mode mode); /**< See above. */

	/** Retrieves whether a file is attached to this object.
		@return Whether a file is attached to this object. */
	bool IsOpen() { return filePtr != NULL; }

	/** Retrieves the length, in bytes, of the attached file.
		@return The length of the file, in bytes. */
	size_t GetLength();
	
	/** @deprecated Retrieves the OS-specific handle to the file.
		Using this function is not recommended.
		@return The OS-specific file handle. */
	FILE* GetHandle() { return filePtr; }

	/** Reads length bytes of data from this file.
		This function will fail if the file was not opened in READ mode.
		This function will advance the FILE pointer length bytes ahead.
		@param buffer A buffer large enough to accept the wanted data.
		@param length The length, in bytes, of the data to read.
		@return True on success, false otherwise. */
	bool Read(void* buffer, size_t length);
	
	/** Writes length bytes of data to this file.
		This function will fail if the file was not opened in WRITE mode.
		@param buffer A buffer contained the data to write.
		@param length The length, in bytes, of the data to write.
		@return True on success, false otherwise.*/
	bool Write(const void* buffer, size_t length);

	/** Flushes the data written to the file.
		This function will do nothing if the file was not opened in WRITE mode. */
	void Flush();
	
	/** Closes and detaches the file. */
	void Close();
};

#endif // _FU_FILE_H_
