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

#ifndef _FU_URI_H_
#define _FU_URI_H_

/**
	A simple URI structure.

	This structure is quite incomplete but covers all the necessary cases for now.
	Possible upgrades to support all five parts:
	SCHEME://HOSTNAME/FILENAME@@ARGUMENTS@#DAE_ID

	[scheme:][schemeDelimiter][userInfo@][host][:port][/path][?query][#fragment]

	Right now, SCHEME must always be "file://".
	HOSTNAME, on Windows, can be a UNC computer name. No other
	hostname types are supported. ARGUMENTS are not supported.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUUri
{
public:
	enum Scheme
	{
		NONE,
		FILE,
		FTP,
		HTTP,
		HTTPS
	};

private:
	/** The URI scheme */
	Scheme scheme;
	/** The URI scheme specific part */
	fstring schemeDelimiter;
	/** The URI user used to connect to the host */
	fstring username;
	/** The URI password used to connect to the host */
	fstring password;
	/** The URI the host */
	fstring hostname;
	/** The URI port used to connect to the host */
	uint32 port;
	/** The URI path represent the name of the filename. */
	fstring path;
	/** The URI query */
	fstring query;
	/** The URI fragment represent the COLLADA id of the element targeted. */
	fstring fragment;

	static bool IsAlpha(fchar fc);
	static bool IsDigit(fchar fc);
	static bool IsAlphaNumeric(fchar fc);
	static bool IsMark(fchar fc);
	static bool IsHex(fchar fc);
	static bool IsReserved(fchar fc);

	/** For a relative path, extract the list of the individual paths that must be traversed to get to the file.
		@param filename A file path.
		@param list The returned list of paths to traverse.
		@param includeFilename Whether the filename should be pushed at the back of the returned list. */
	void ExtractPathStack(const fstring& name, FStringList& list, bool includeFilename = false) const;
public:
	/** Constructor. */
	FUUri();

	/** Constructor.
		@param uri The string value for the URI.
		@param escape Whether to escape the strings.*/
	FUUri(const fstring& uri, bool escape = false);

	/** Constructor.
		@param scheme The scheme to use in the construction of the URI.
		@param username The username to use in the construction of the URI.
		@param passwd The password to use in the construction of the URI.
		@param host The host to use in the construction of the URI.
		@param port The port to use in the construction of the URI.
		@param path The path to use in the construction of the URI.
		@param query The query to use in the construction of the URI.
		@param fragment The fragment to use in the construction of the URI. */
	FUUri(Scheme scheme, const fstring& username, const fstring& passwd, const fstring& host, uint32 port, const fstring& path = FC(""), const fstring& query = FC(""), const fstring& fragment = FC(""));

	/** Constructor.
		@param scheme The scheme to use in the construction of the URI.
		@param host The host to use in the construction of the URI.
		@param path The path to use in the construction of the URI.
		@param fragment The fragment to use in the construction of the URI. */
	FUUri(Scheme scheme, const fstring& host, const fstring& path = FC(""), const fstring& fragment = FC(""));

	/** Constructor.
		@param path The path to use in the construction of the URI.
		@param fragment The fragment to use in the construction of the URI. */
	FUUri(const fstring& path, const fstring& fragment);

	/** Retrieves the scheme from the URI.
		@return The URI scheme. */
	inline Scheme GetScheme() const { return scheme; }

	/** Retrieves the scheme delimiter from the URI.
		@return The URI scheme delimiter. */
	inline const fstring& GetSchemeDelimiter() const { return schemeDelimiter; }

	/** Retrieves the user information from the URI.
		@return The URI user information. */
	fstring GetUserInformations() const;

	/** Retrieves the host information from the URI.
		@return The URI host. */
	inline const fstring& GetHostname() const { return hostname; }

	/** Retrieves the port number from the URI.
		@return The URI port number. */
	inline uint32 GetPort() const { return port; }

	/** Sets the port number of the URI.
		@param _port A valid port number. */
	inline void SetPort(uint32 _port) { port = _port; }

	/** Retrieves the path from the URI.
		@return The URI path. */
	inline const fstring& GetPath() const { return path; }

	/** Retrieves the query from the URI.
		@return The URI query. */
	inline const fstring& GetQuery() const { return query; }

	/** Sets the query of the URI.
		@param _query A URI fragment. */
	inline void SetQuery(const fstring& _query) { query = _query; }

	/** Retrieves the fragment from the URI.
		@return The URI query. */
	inline const fstring& GetFragment() const { return fragment; }

	/** Sets the fragment of the URI.
		@param _fragment A URI fragment. */
	inline void SetFragment(const fstring& _fragment) { fragment = _fragment; }

	/** Retrieves the authority string from the URI. ("[userInfo@]host[:port]")
		@return The URI authority string. */
	fstring GetAuthority() const;

	/** Retrieves an absolute path from the URI.
		@return The URI absolute path. */
	fstring GetAbsolutePath() const;

	/** Retrieves an absolute URI string from the URI.
		@param fragment Whether to return a string with the fragment
		@return The URI string. */
	fstring GetAbsoluteUri(bool fragment = true) const;

	/** Retrieves an relative URI string from the URI.
		@return The URI string. */
	fstring GetRelativeUri(const FUUri& uri) const;

	/** Makes a relative path from a uri
		@param	uri	The uri representing the path.
		@return The relative path. */
	fstring MakeRelative(const fstring& path) const;

	/** Makes an absolute path from a relative path and this URI
		@param	relpath	The relative path
		@return The absolute path. */
	fstring MakeAbsolute(const fstring& relativePath) const;

	/** Makes the passed in URI relative to this URI
		@param uri The relative or absolute URI */
	void MakeAbsolute(FUUri& uri) const;

	/** Resolves a URI from a relative path against this URI
		@param	relpath	The relative path.
		@return The newly contructed URI. */
	FUUri Resolve(const fstring& relativePath) const;

	/** Retrieves whether this URI points to a file.
		@return Whether this URI points to a file. */
	bool IsFile() const;

	/** Escapes a path
		@param path A path.
		@return The escaped path. */
	static fstring Escape(const fstring& path);
};

typedef fm::vector<FUUri> FUUriList; /**< A dynamically-sized array of URIs. */

#endif // _FU_URI_H_

