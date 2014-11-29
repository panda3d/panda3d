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

#ifndef _FU_XML_PARSER_H_
#define _FU_XML_PARSER_H_

#ifdef HAS_LIBXML

typedef fm::pvector<struct _xmlNode> xmlNodeList; /**< A dynamically-sized array of XML nodes. */

namespace FUXmlParser
{
	// Parse an XML compatable string for the std representation
	FCOLLADA_EXPORT fm::string XmlToString(const char* s);
#ifdef UNICODE
	FCOLLADA_EXPORT fstring XmlToString(const fchar* s);
#endif // UNICODE

	// Retrieve specific child nodes
	FCOLLADA_EXPORT xmlNode* FindChildByType(xmlNode* parent, const char* type);
	FCOLLADA_EXPORT xmlNode* FindChildByName(xmlNode* parent, const char* name);
	FCOLLADA_EXPORT void FindChildrenByType(xmlNode* parent, const char* type, xmlNodeList& nodes);
	FCOLLADA_EXPORT xmlNode* FindChildByProperty(xmlNode* parent, const char* prop, const char* val);
	FCOLLADA_EXPORT xmlNode* FindNodeInListByProperty(xmlNodeList list, const char* property, const char* prop);

	// Retrieve node property and content
	FCOLLADA_EXPORT bool HasNodeProperty(xmlNode* node, const char* property);
	FCOLLADA_EXPORT fm::string ReadNodeProperty(xmlNode* node, const char* property);
	FCOLLADA_EXPORT FUCrc32::crc32 ReadNodePropertyCRC(xmlNode* node, const char* property);
	FCOLLADA_EXPORT const char* ReadNodeContentDirect(xmlNode* node);
	FCOLLADA_EXPORT fm::string ReadNodeContentFull(xmlNode* node);
};

inline bool IsEquivalent(const xmlChar* sz1, const char* sz2) { return IsEquivalent((const char*) sz1, sz2); }

#endif // HAS_LIBXML

#endif //_FU_XML_PARSER_H_
