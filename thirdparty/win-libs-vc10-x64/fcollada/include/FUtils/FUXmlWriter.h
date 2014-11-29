/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
@file FUXmlWriter.h
This file defines the FUXmlWriter namespace.
*/

#ifndef _FU_XML_WRITER_H_
#define _FU_XML_WRITER_H_

#ifdef HAS_LIBXML

/**
Common XML writing utility functions.
Based on top of the LibXML2 library.
This whole namespace is considered internal and should only be used
by the FCollada library.

@ingroup FUtils
*/
namespace FUXmlWriter
{
	/** Creates a dangling XML tree node.
		@param name The name of the new XML tree node.
		@return The new XML tree node. This pointer should never be NULL. */
	FCOLLADA_EXPORT xmlNode* CreateNode(const char* name);

	/** Appends a dangling XML tree child node to a XML tree node.
		The child XML tree node is added at the end of the parent XML tree node children list.
		@param parent The parent XML tree node.
		@param child The child XML tree node. */
	FCOLLADA_EXPORT void AddChild(xmlNode* parent, xmlNode* child);

	/** Rename the given node to the new name
		@param node The XML tree node to rename
		@param newName The new name for the XML tree node */
	FCOLLADA_EXPORT void RenameNode(xmlNode* node, const char* newName);

	/** Creates a child XML tree node within a XML tree node.
		The child XML tree node is added at the end of the parent XML tree node children list.
		@param parent The parent XML tree node.
		@param name The name of the new child node.
		@return The new child XML tree node. */
	FCOLLADA_EXPORT xmlNode* AddChild(xmlNode* parent, const char* name);

	/** Creates a child XML tree node within a XML tree node.
		The child XML tree node is added at the end of the parent XML tree node children list.
		The given content string is added to the returned child XML tree node.
		@param parent The parent XML tree node.
		@param name The name of the new child XML tree node.
		@param content The content to add to the new child XML tree node.
		@return The new child XML tree node. */
	FCOLLADA_EXPORT xmlNode* AddChild(xmlNode* parent, const char* name, const char* content);
#ifdef UNICODE
	FCOLLADA_EXPORT xmlNode* AddChild(xmlNode* parent, const char* name, const fstring& content); /**< See above. */
#endif
	inline xmlNode* AddChild(xmlNode* parent, const char* name, const fm::string& content) { return AddChild(parent, name, content.c_str()); } /**< See above. */
	inline xmlNode* AddChild(xmlNode* parent, const char* name, FUSStringBuilder& content) { return AddChild(parent, name, content.ToCharPtr()); } /**< See above. */

	/** Appends an unprocessed content string to a XML tree node.
			The content string is added at the end of the XML tree node's content, with no special characters added.
			@param node The XML tree node.
			@param content The content to add to the XML tree node. */
	FCOLLADA_EXPORT void AddContentUnprocessed(xmlNode* node, const char* content);
	inline void AddContentUnprocessed(xmlNode* node, const fm::string& content) { return AddContentUnprocessed(node, content.c_str()); } /**< See above. */
	inline void AddContentUnprocessed(xmlNode* node, FUSStringBuilder& content) { return AddContentUnprocessed(node, content.ToCharPtr()); } /**< See above. */

	/** Creates a child XML tree node within a XML tree node.
		The child XML tree node is added at the end of the parent XML tree node children list.
		The given content value is added, in string-form, to the returned child XML tree node.
		@param parent The parent XML tree node.
		@param name The name of the new child XML tree node.
		@param value A primitive value. This value is stringified and added, as content, to the new child XML tree node.
		@return The new child XML tree node. */
	template <typename T> inline xmlNode* AddChild(xmlNode* parent, const char* name, const T& value) { xmlNode* child = AddChild(parent, name); FUSStringBuilder builder; builder.set(value); AddContentUnprocessed(child, builder.ToCharPtr()); return child; }

	/** Appends a dangling XML tree node as a sibling of a XML tree node.
		Two sibling XML tree nodes have the same parent XML tree node.
		The dangling XML tree node is added at the end of the parent XML tree node children list.
		@param sibling The sibling XML tree node. It must have a valid parent XML tree node.
		@param dangling The dangling XML tree node. */
	 FCOLLADA_EXPORT void AddSibling(xmlNode* sibling, xmlNode* dangling);

	/** Creates a XML tree node as a sibling of a XML tree node.
		Two sibling XML tree nodes have the same parent XML tree node.
		The new XML tree node is added at the end of the parent XML tree node children list.
		@param sibling The sibling XML tree node. It must have a valid parent XML tree node.
		@param name The name of the new XML tree node.
		@return The new sibling XML tree node. */
	 FCOLLADA_EXPORT xmlNode* AddSibling(xmlNode* sibling, const char* name);

	/** Inserts a XML tree node before the given XML tree child node.
		@param parent The parent XML tree node.
		@param sibling The sibling XML tree node. It must be parented to the given parent XML tree node.
		@param name The name of the new XML tree node.
		@return The inserted sibling XML tree node. */
	 FCOLLADA_EXPORT xmlNode* InsertChild(xmlNode* parent, xmlNode* sibling, const char* name);

	/** Returns a child XML tree node within a XML tree node.
		If the child XML tree node with the given name does not exists, it is created and
		the given content is added to the new XML tree node.
		@param parent The parent XML tree node.
		@param name The name of the child XML tree node.
		@param content The content to add to the child XML tree node, if it must be created.
		@return The child XML tree node. */
	FCOLLADA_EXPORT xmlNode* AddChildOnce(xmlNode* parent, const char* name, const char* content=NULL);
	inline xmlNode* AddChildOnce(xmlNode* parent, const char* name, const fm::string& content) { return AddChildOnce(parent, name, content.c_str()); } /**< See above. */
	inline xmlNode* AddChildOnce(xmlNode* parent, const char* name, FUSStringBuilder& content) { return AddChildOnce(parent, name, content.ToCharPtr()); } /**< See above. */

	/** Returns a child XML tree node within a XML tree node.
		If the child XML tree node with the given name does not exists, it is created and
		the given content is added to the new XML tree node.
		@param parent The parent XML tree node.
		@param name The name of the child XML tree node.
		@param value A primitive value. If the child XML tree node must be created: this value is stringified and added as content.
		@return The child XML tree node. */
	template <typename T> inline xmlNode* AddChildOnce(xmlNode* parent, const char* name, const T& value) { xmlNode* child = AddChildOnce(parent, name); FUSStringBuilder builder(value); AddContentUnprocessed(child, builder.ToCharPtr()); return child; }

	/** Appends a content string to a XML tree node.
		The content string is added at the end of the XML tree node's content, with no special characters added.
		The content string will be processed for non-UTF8 characters.
		@param node The XML tree node.
		@param content The content to add to the XML tree node. */
	FCOLLADA_EXPORT void AddContent(xmlNode* node, const char* content);
#ifdef UNICODE
	FCOLLADA_EXPORT void AddContent(xmlNode* node, const fstring& content); /**< See above. */
#endif
	inline void AddContent(xmlNode* node, const fm::string& content) { AddContent(node, content.c_str()); } /**< See above. */
	inline void AddContent(xmlNode* node, FUSStringBuilder& content) { AddContent(node, content.ToCharPtr()); } /**< See above. */

	/** Converts all the spaces and other characters not acceptable in filenames,
		into their hex-equivalent. Example: %20 for spaces.
		@param str The string with the spaces to convert. */
	 FCOLLADA_EXPORT void ConvertFilename(fstring& str);

	/** Appends a primitive value to a XML tree node.
		The primitive value is added at the end of the XML tree node's content, with no special characters added.
		@param node The XML tree node.
		@param value A primitive value. The value is stringified and added as content to the XML tree node. */
	template <typename T> inline void AddContent(xmlNode* node, const T& value) { FUSStringBuilder builder(value); return AddContentUnprocessed(node, builder.ToCharPtr()); }

	/** Appends a XML attribute to a XML tree node.
		A XML attribute appears in the form @<node name="value"/@>.
		@param node The XML tree node.
		@param attributeName The name of the XML attribute.
		@param attributeValue The value of the XML attribute. */
	FCOLLADA_EXPORT void AddAttribute(xmlNode* node, const char* attributeName, const char* attributeValue);
#ifdef UNICODE
	FCOLLADA_EXPORT void AddAttribute(xmlNode* node, const char* attributeName, const fstring& attributeValue); /**< See above. */
#endif
	inline void AddAttribute(xmlNode* node, const char* attributeName, FUSStringBuilder& attributeValue) { AddAttribute(node, attributeName, attributeValue.ToCharPtr()); } /**< See above. */
	inline void AddAttribute(xmlNode* node, const char* attributeName, const fm::string& attributeValue) { AddAttribute(node, attributeName, attributeValue.c_str()); } /**< See above. */

	/** Appends a XML attribute to a XML tree node.
		A XML attribute appears in the form @<node name="value"@/@>.
		@param node The XML tree node.
		@param attributeName The name of the XML attribute.
		@param attributeValue A primitive value. The value is stringified and set as the value of the XML attribute. */
	template <typename T> inline void AddAttribute(xmlNode* node, const char* attributeName, const T& attributeValue) { FUSStringBuilder builder; builder.set(attributeValue); AddAttribute(node, attributeName, builder.ToCharPtr()); }

	/** Removes a XML attribute from a XML tree node.
		@param node The XML tree node with the unwanted attribute.
		@param attributeName The name of the XML attribute to remove. */
	FCOLLADA_EXPORT void RemoveAttribute(xmlNode* node, const char* attributeName);

	/** Appends a dangling XML tree node to a XML tree node
		The dangling XML tree node is inserted in lexical order,
		after all the sibling XML tree node with the same name.
		@param parent The XML tree node onto which the dangling XML node tree is appended.
		@param child The dangling XML tree node. */
	FCOLLADA_EXPORT void AddChildSorted(xmlNode* parent, xmlNode* child);

	/** Creates a new child XML tree node of a XML tree node
		The new child XML tree node is inserted in lexical order,
		after all the sibling XML tree node with the same name.
		@param parent The XML tree node onto which the new XML node tree is created.
		@param name The name of the new child XML tree node.
		@param content A content string to be added to the child XML tree node.
		@return The new child XML tree node. */
	FCOLLADA_EXPORT xmlNode* AddChildSorted(xmlNode* parent, const char* name, const char* content=NULL);

	/** Detaches a XML tree node from its current parent and adds it
		as the child of another XML tree node.
		@param node The XML tree node to reparent.
		@param newParent The new parent XML tree node. */
	FCOLLADA_EXPORT void ReParentNode(xmlNode* node, xmlNode* newParent);
};

#endif // HAS_LIBXML

#endif // _FU_XML_WRITER_H_
