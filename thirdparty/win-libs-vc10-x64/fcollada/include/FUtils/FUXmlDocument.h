/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _XML_DOCUMENT_H_
#define _XML_DOCUMENT_H_

class FUFileManager;
struct _xmlDoc;
typedef struct _xmlDoc xmlDoc;

#ifdef HAS_LIBXML

/** Simple container for a XML document.
	When this container is released, it will automatically release the XML document. */
class FCOLLADA_EXPORT FUXmlDocument
{
private:
	bool isParsing;
	fstring filename;
	xmlDoc* xmlDocument;

public:
	/** Constructor.
		Opens the XML document for the given filename.
		@param manager To handle non-file system opens and to handle relative paths.
		@param filename The filename of the XML document to open.
		@param isParsing Whether the document should be opened from file or created and written out. */
	FUXmlDocument(FUFileManager* manager, const fchar* filename, bool isParsing);

	/** Creates an XML document from a data string.
		@param data The data buffer containing the XML document.
		@param length The length of the data. If the length is -1,
			the data buffer must be NULL-terminated. */
	FUXmlDocument(const char* data, size_t length = (size_t) ~0);

	/** Destructor.
		Releases the XML document. */
	~FUXmlDocument();

	/** Creates the root XML tree node for the document.
		@param name The name of the root XML tree node.
		@return The newly created root XML tree node. */
	xmlNode* CreateRootNode(const char* name);

	/** Releases the stored XML data, if any */
	void ReleaseXmlData();

	/** Retrieves the root XML tree node for the document.
		@return The root XML tree node. This pointer will be NULL
			if the document did not load successfully. */
	xmlNode* GetRootNode();

	/** Writes out the XML document.
		@param encoding The format encoding string.
		@return Whether the XML document was written out successfully. */
	bool Write(const char* encoding = "utf-8");
};

#endif // HAS_LIBXML

#endif // _XML_DOCUMENT_H_
