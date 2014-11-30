/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDExtra.h
	This file contains the FCDExtra class and its sub-classes:
	FCDENode, FCDETechnique and FCDEAttribute.
*/

#ifndef _FCD_EXTRA_H_
#define _FCD_EXTRA_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

class FCDAnimated;
class FCDAnimatedCustom;
class FCDEAttribute;
class FCDETechnique;
class FCDEType;
class FCDENode;

typedef fm::pvector<FCDENode> FCDENodeList; /**< A dynamically-sized list of extra tree nodes. */

/**
	A COLLADA extra tree.

	An extra tree contains the user-defined COLLADA information
	contained within \<extra\> elements. For this, the extra tree
	root simply contains a list of techniques. Each technique
	belongs to a different application-specific profile.
*/
class FCOLLADA_EXPORT FCDExtra : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FUObject* parent;
	DeclareParameterContainer(FCDEType, types, FC("Extra Types"));

public:
	/** Constructor.
		Only structures that contain extra trees should create them.
		@param document The COLLADA document that owns the extra tree.
		@param parent The object that contains this extra tree. This parameter
			is used only for plug-in support. */
	FCDExtra(FCDocument* document, FUObject* parent);

	/** Destructor. */
	virtual ~FCDExtra();

	/** Retrieves the parent object for the extra tree.
		@return The parent object pointer. */
	inline FUObject* GetParent() { return parent; }
	inline const FUObject* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the list of types contained by this extra tree.
		@return The list of types. */
	DEPRECATED(3.05A, GetTypeCount and GetType(index)) inline void GetTypes() const {}

	/** Retrieves the number of types contained by this extra tree.
		@return The number of types. */
	size_t GetTypeCount() const { return types.size(); }

	/** Retrieves the default extra type.
		The default extra type has an empty typename and is always created by default.
		The default extra type will NOT be exported if it is empty.
		@return The default extra type. */
	inline FCDEType* GetDefaultType() { return const_cast<FCDEType*>(const_cast<const FCDExtra*>(this)->GetDefaultType()); }
	inline const FCDEType* GetDefaultType() const { return FindType(""); }  /**< See above. */

	/** Retrieves a specific type contained by this extra tree.
		@param index The index of the type.
		@return The type. This pointer will be NULL if the index is out-of-bounds. */
	inline FCDEType* GetType(size_t index) { FUAssert(index < types.size(), return NULL); return types.at(index); }
	inline const FCDEType* GetType(size_t index) const { FUAssert(index < types.size(), return NULL); return types.at(index); } /**< See above. */

	/** Adds a new application-specific type to the extra tree.
		If the given application-specific type already exists
		within the extra tree, the old type will be returned.
		@param name The application-specific name.
		@return A type for this application-specific name. */
	FCDEType* AddType(const char* name);
	inline FCDEType* AddType(const fm::string& name) { return AddType(name.c_str()); } /**< See above. */

	/** Retrieves a specific type contained by this extra tree.
		@param name The application-specific name of the type.
		@return The type that matches the name. This pointer may
			be NULL if no type matches the name. */
	inline FCDEType* FindType(const char* name) { return const_cast<FCDEType*>(const_cast<const FCDExtra*>(this)->FindType(name)); }
	const FCDEType* FindType(const char* name) const; /**< See above. */
	inline FCDEType* FindType(const fm::string& name) { return FindType(name.c_str()); } /**< See above. */
	inline const FCDEType* FindType(const fm::string& name) const { return FindType(name.c_str()); } /**< See above. */

	/** Determines whether this structure is empty or not.
		Basically, if there is an extra type, and that this type contains at
		least one extra technique, content exists.
		@return True if non-empty, false otherwise.*/
	bool HasContent() const;

	/** [INTERNAL] Clones the extra tree information.
		@param clone The extra tree that will take in this extra tree's information.
			If this pointer is NULL, a new extra tree will be created and you will
			need to release the returned pointer manually.
		@return The clone. */
	FCDExtra* Clone(FCDExtra* clone = NULL) const;
};

/**
	A COLLADA typed extra node.

	The 'type' attribute of the extra nodes allow us to bucket techniques
	to allow for different data for the same idea.

	Therefore, a typed extra node contains a type name and a list of techniques.
*/
class FCOLLADA_EXPORT FCDEType : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FCDExtra* parent;
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, name, FC("Type name"));
	DeclareParameterContainer(FCDETechnique, techniques, FC("Profile-specific Techniques"));

public:
	/** Constructor: do not use directly.
		Use the FCDExtra::AddType function instead.
		@param document The COLLADA document that owns the extra tree.
		@param parent The parent extra tree structure.
		@param type The name of the type for this typed extra. */
	FCDEType(FCDocument* document, FCDExtra* parent, const char* type);

	/** Destructor. */
	virtual ~FCDEType();

	/** Retrieves the extra tree that contains this typed extra.
		@return The parent extra tree. */
	inline FCDExtra* GetParent() { return parent; }
	inline const FCDExtra* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the name of the type of the typed extra.
		@return The name of the type. */
	inline const fm::string& GetName() const { return name; }

	/** Modifies the name of the type of the typed extra.
		Be careful when modifying the name of a type. The extra tree
		assumes no duplicate type names within its typed extras.
		@param _name The new name of the type. */
	inline void SetName(const fm::string& _name) { name = _name; }

	/** Retrieves the list of techniques contained by this extra tree.
		@return The list of techniques. */
	DEPRECATED(3.05A, GetTechniqueCount and GetTechnique(index)) inline void GetTechniques() const {}

	/** Retrieves the number of techniques contained by this extra tree.
		@return The number of techniques. */
	inline size_t GetTechniqueCount() const { return techniques.size(); }

	/** Retrieves a specific technique contained by this extra tree.
		@param index The index of the technique.
		@return The technique. This pointer will be NULL if the
			index is out-of-bounds. */
	inline FCDETechnique* GetTechnique(size_t index) { FUAssert(index < techniques.size(), return NULL); return techniques.at(index); }
	inline const FCDETechnique* GetTechnique(size_t index) const { FUAssert(index < techniques.size(), return NULL); return techniques.at(index); } /**< See above. */

	/** Adds a new application-specific profile technique to the extra tree.
		If the given application-specific profile already exists
		within the extra tree, the old technique will be returned.
		@param profile The application-specific profile name.
		@return A technique for this application-specific profile. */
	FCDETechnique* AddTechnique(const char* profile);
	inline FCDETechnique* AddTechnique(const fm::string& profile) { return AddTechnique(profile.c_str()); } /**< See above. */

	/** Retrieves a specific technique contained by this extra tree.
		@param profile The application-specific profile name of the technique.
		@return The technique that matches the profile name. This pointer may
			be NULL if no technique matches the profile name. */
	FCDETechnique* FindTechnique(const char* profile) { return const_cast<FCDETechnique*>(const_cast<const FCDEType*>(this)->FindTechnique(profile)); }
	const FCDETechnique* FindTechnique(const char* profile) const; /**< See above. */
	inline FCDETechnique* FindTechnique(const fm::string& profile) { return FindTechnique(profile.c_str()); } /**< See above. */
	inline const FCDETechnique* FindTechnique(const fm::string& profile) const { return FindTechnique(profile.c_str()); } /**< See above. */

	/** Retrieves the extra tree node that has a given element name.
		This function searches for the extra tree node within all the
		techniques.
		@param name An element name.
		@return The extra tree node that matches the element name. This pointer
			will be NULL if no extra tree node matches the element name. */
	inline FCDENode* FindRootNode(const char* name) { return const_cast<FCDENode*>(const_cast<const FCDEType*>(this)->FindRootNode(name)); }
	const FCDENode* FindRootNode(const char* name) const; /**< See above. */
	inline FCDENode* FindRootNode(const fm::string& name) { return FindRootNode(name.c_str()); } /**< See above. */
	inline const FCDENode* FindRootNode(const fm::string& name) const { return FindRootNode(name.c_str()); } /**< See above. */

	/** [INTERNAL] Clones the extra tree information.
		@param clone The extra tree that will take in this extra tree's information.
			If this pointer is NULL, a new extra tree will be created and you will
			need to release the returned pointer manually.
		@return The clone. */
	FCDEType* Clone(FCDEType* clone = NULL) const;
};

/**
	A COLLADA extra tree node.

	The extra tree node is a hierarchical structure that contains child
	extra tree nodes as well as attributes. If the extra tree node is a leaf
	of the tree, it may contain textual content.

	The extra tree node leaf may be animated, if it has the 'sid' attribute.
*/
class FCOLLADA_EXPORT FCDENode : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FCDENode* parent;
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, name, FC("Node name"));
	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, content, FC("Node content"));

	DeclareParameterContainer(FCDENode, children, FC("Children"));
	DeclareParameterContainer(FCDEAttribute, attributes, FC("Attributes"));

	DeclareParameterRef(FCDAnimatedCustom, animated, FC("Custom Animatable"));

public:
	/** Constructor: do not use directly.
		Instead, call the FCDENode::AddChild function of the parent within the hierarchy.
		@param document The COLLADA document that owns the extra tree node.
		@param parent The extra tree node that contains this extra tree node. */
	FCDENode(FCDocument* document, FCDENode* parent);

	/** Destructor. */
	virtual ~FCDENode();

	/** Retrieves the name of the extra tree node.
		The name of the extra tree node is the name of the equivalent XML tree node.
		@return The name of the extra tree node. */
	inline const char* GetName() const { return name->c_str(); }

	/** Sets the name of the extra tree node.
		The name of the extra tree node is the name of the equivalent XML tree node.
		@param _name The name of the extra tree node. */
	inline void SetName(const char* _name) { fm::string n = _name; SetName(n); }
	inline void SetName(const fm::string& _name) { fm::string n = _name; SetName(n); } /**< See above. */
	void SetName(fm::string& _name);

	/** Cleans up extra tree node names and extra tree attribute names in order to
		always start with an alphabetic character or an underscore, as well as contain
		only alphanumeric characters or underscore.
		@param n The string to clean. This reference will be updated with the cleaned name. */
	static void CleanName(fm::string& n);

	/** Retrieves the textual content of the extra tree node.
		This value is only valid for extra tree node that have no children,
		as COLLADA doesn't allow for mixed-content.
		@return The textual content of the extra tree node. */
	const fchar* GetContent() const;

	/** Sets the textual content of the extra tree node.
		This function will release all the child node of this extra tree node,
		as COLLADA doesn't allow for mixed-content.
		@param _content The textual content. */
	void SetContent(const fchar* _content);
	inline void SetContent(const fstring& _content) { return SetContent(_content.c_str()); } /**< See above. */

	/** [INTERNAL] Set the content directly.
		@param _content The new content to set.
	*/
	void SetContentDirect(const fstring& _content) { content = _content; }

	/** Retrieves the animated values associated with this extra tree node.
		Extra tree node leaves may be animated. If this extra tree node leaf
		is animated, this animated value will contain the animation curves.
		@return The animated value. */
	FCDAnimatedCustom* GetAnimated() { return animated; }
	const FCDAnimatedCustom* GetAnimated() const { return animated; } /**< See above. */

	/**[INTERNAL] Set the customized animated. The old pointer is released first.
		@animatedCustom The new animated.
	*/
	void SetAnimated(FCDAnimatedCustom* animatedCustom);

	/** Retrieves the parent of an extra tree node.
		The hierarchy cannot be changed dynamically. If you to move an extra tree node,
		you will need to clone it manually and release the old extra tree node.
		@return The parent extra tree node within the hierarchy. This pointer
			will be NULL if the extra tree node is a extra tree technique. */
	FCDENode* GetParent() { return parent; }
	const FCDENode* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the children of an extra tree node.
		@return The list of child extra tree nodes. */
	DEPRECATED(3.05A, GetChildNodeCount and GetChildNode(index)) void GetChildNodes() const {}

	/** Retrieves the number of children of an extra tree node.
		@return The number of children. */
	size_t GetChildNodeCount() const { return children.size(); }

	/** Retrieves a specific child extra tree node.
		@param index The index of the child extra tree node.
		@return The child extra tree node. This pointer will be NULL if the index
			is out-of-bounds. */
	FCDENode* GetChildNode(size_t index) { FUAssert(index < children.size(), return NULL); return children.at(index); }
	const FCDENode* GetChildNode(size_t index) const { FUAssert(index < children.size(), return NULL); return children.at(index); } /**< See above. */

	/** Adds a new child extra tree to this extra tree node.
		@see AddParameter
		@return The new child extra tree node. */
	FCDENode* AddChildNode();

	/** Adds a new, named, child extra tree to this extra tree node.
		@see AddParameter
		@param name The name of the child node.
		@return The new child extra tree node. */
	FCDENode* AddChildNode(const char* name);
	inline FCDENode* AddChildNode(const fm::string& name) { return AddChildNode(name.c_str()); } /**< See above. */

	/** Retrieves the child extra tree node with the given name.
		@param name A name.
		@return The child extra tree node that matches the given name.
			This pointer will be NULL if no child extra tree node matches
			the given name. */
	inline FCDENode* FindChildNode(const char* name) { return const_cast<FCDENode*>(const_cast<const FCDENode*>(this)->FindChildNode(name)); }
	const FCDENode* FindChildNode(const char* name) const; /**< See above. */
	inline FCDENode* FindChildNode(const fm::string& name) { return FindChildNode(name.c_str()); } /**< See above. */
	inline const FCDENode* FindChildNode(const fm::string& name) const { return FindChildNode(name.c_str()); } /**< See above. */

	/** Retrieves the child extra tree nodes with the given name.
		@param name A name.
		@param nodes A list of nodes to fill in with the nodes
			that match a given name. */
	void FindChildrenNodes(const char* name, FCDENodeList& nodes) const;
	inline void FindChildrenNodes(const fm::string& name, FCDENodeList& nodes) const { FindChildrenNodes(name.c_str(), nodes); } /**< See above. */

	/** Retrieves the child extra tree node with the given name.
		A parameter has no child nodes and is described as: \<X\>value\</X\>.
		The first child extra tree node where the name matches 'X' will be returned.
		@param name The parameter name.
		@return The first child extra tree node holding the wanted parameter within the hierarchy.
			This pointer will be NULL to indicate that no parameter matches the given name. */
	const FCDENode* FindParameter(const char* name) const;
	inline FCDENode* FindParameter(const char* name) { return const_cast<FCDENode*>(const_cast<const FCDENode*>(this)->FindParameter(name)); } /**< See above. */

	/** Retrieves a list of all the parameters contained within the hierarchy.
		A parameter has no child nodes and is described as: \<X\>value\</X\>.
		Using this function, The parameter would be returned with the name 'X'.
		@param nodes The list of parameters to fill in. This list is not emptied by the function.
		@param names The list of names of the parameters. This list is not emptied by the function. */
	void FindParameters(FCDENodeList& nodes, StringList& names);

	/** Retrieves the list of attributes for this extra tree node.
		@return The list of attributes. */
	DEPRECATED(3.05A, GetAttributeCount and GetAttribute(index)) void GetAttributes() const {}

	/** Retrieves the number of attributes for this extra tree node.
		@return The number of attributes. */
	size_t GetAttributeCount() const { return attributes.size(); }

	/** Retrieves a specific attribute of this extra tree node.
		@param index The index.
		@return The attribute at this index. This pointer will be NULL
			if the index is out-of-bounds. */
	FCDEAttribute* GetAttribute(size_t index) { FUAssert(index < attributes.size(), return NULL); return attributes.at(index); }
	const FCDEAttribute* GetAttribute(size_t index) const { FUAssert(index < attributes.size(), return NULL); return attributes.at(index); } /**< See above. */

	/** Adds a new attribute to this extra tree node.
		If an attribute with the same name already exists, this function simply
		assigns the new value to the existing attribute and returns the existing attribute.
		@param _name The name of the attribute. If this parameter is
			a non-constant fm::string reference, it will be updated with the cleaned name.
		@param _value The value of the attribute.
		@return The new attribute. */
	FCDEAttribute* AddAttribute(fm::string& _name, const fchar* _value);
	inline FCDEAttribute* AddAttribute(const char* _name, const fchar* _value) { fm::string n = _name; return AddAttribute(n, _value); } /**< See above. */
	inline FCDEAttribute* AddAttribute(const fm::string& _name, const fchar* _value) { fm::string n = _name; return AddAttribute(n, _value); } /**< See above. */
	inline FCDEAttribute* AddAttribute(const char* _name, const fstring& _value) { fm::string n = _name; return AddAttribute(n, _value.c_str()); } /**< See above. */
	inline FCDEAttribute* AddAttribute(fm::string& _name, const fstring& _value) { return AddAttribute(_name, _value.c_str()); } /**< See above. */
	inline FCDEAttribute* AddAttribute(const fm::string& _name, const fstring& _value) { fm::string n = _name; return AddAttribute(n, _value.c_str()); } /**< See above. */
	template <typename T> inline FCDEAttribute* AddAttribute(const char* _name, const T& _value) { fm::string n = _name; return AddAttribute(n, TO_FSTRING(_value)); } /**< See above. */
	template <typename T> inline FCDEAttribute* AddAttribute(fm::string& _name, const T& _value) { return AddAttribute(_name, TO_FSTRING(_value)); } /**< See above. */
	template <typename T> inline FCDEAttribute* AddAttribute(const fm::string& _name, const T& _value) { fm::string n = _name; return AddAttribute(n, TO_FSTRING(_value)); } /**< See above. */

	/** Retrieve the attribute of this extra tree node with the given name.
		Attribute names are unique within an extra tree node.
		@param name The attribute name.
		@return The attribute that matches the name. This pointer will be NULL if
			there is no attribute with the given name. */
	inline FCDEAttribute* FindAttribute(const char* name) { return const_cast<FCDEAttribute*>(const_cast<const FCDENode*>(this)->FindAttribute(name)); }
	const FCDEAttribute* FindAttribute(const char* name) const; /**< See above. */

	/** Retrieves the value of an attribute on this extra tree node.
		Attributes names are unique within an extra tree node.
		@param name The attribute name.
		@return The value attached to the attribute with the given name. This value
			will be the empty string when no attribute exists with the given name. */
	const fstring& ReadAttribute(const char* name) const;

	/** Adds a parameter as the child node.
		A parameter is the simplest child node possible:
		with a name and a value, represented as the node's content.
		@see AddChildNode
		@param name The parameter name.
		@param value The parameter value. */
	FCDENode* AddParameter(const char* name, const fchar* value);
	inline FCDENode* AddParameter(const fm::string& name, const fchar* value) { return AddParameter(name.c_str(), value); } /**< See above. */
	inline FCDENode* AddParameter(const char* name, const fstring& value) { return AddParameter(name, value.c_str()); } /**< See above. */
	inline FCDENode* AddParameter(const fm::string& name, const fstring& value) { return AddParameter(name.c_str(), value.c_str()); } /**< See above. */
	template <class T>
	inline FCDENode* AddParameter(const char* name, const T& value) { return AddParameter(name, TO_FSTRING(value)); } /**< See above. */
	template <class T>
	inline FCDENode* AddParameter(const fm::string& name, const T& value) { return AddParameter(name.c_str(), TO_FSTRING(value)); } /**< See above. */

	/** Clones the extra tree node.
		@param clone The extra tree node that will receive the clone information.
			This pointer cannot be NULL.
		@return The clone. You will need to release the returned pointer manually. */
	virtual FCDENode* Clone(FCDENode* clone) const;
};

/**
	A COLLADA extra tree technique.

	For convenience, this extra tree technique is based on top of the FCDENode class.
	An extra tree technique is the root of the extra tree specific to 
	the profile of an application.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDETechnique : public FCDENode
{
private:
	DeclareObjectType(FCDENode);

	FCDEType* parent;
	DeclareParameterPtr(FUTrackable, pluginOverride, FC("Plug-in Override Object"));
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, profile, FC("Profile Name"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDEType::AddTechnique function.
		@param document The COLLADA document that owns the technique.
		@param parent The extra type that contains this technique.
		@param profile The application-specific profile name. */
	FCDETechnique(FCDocument* document, FCDEType* parent, const char* profile);

	/** Destructor. */
	virtual ~FCDETechnique();

	/** Retrieves the name of the application-specific profile of the technique.
		@return The name of the application-specific profile. */
	const char* GetProfile() const { return profile->c_str(); }

	/** Sets the name of the application-specific profile of the technique.
		Be careful when modifying the application-specific profile name.
		There is an assumption that within a typed-extra, all application-specific
		profile names are unique.
		@param _profile The new name of the application-specific profile. */
	void SetProfile(const fm::string& _profile) { profile = _profile; }

	/** Retrieves the plug-in object that overrides the extra tree for this profile.
		The plug-in object should contain all the necessary information and this extra tree
		is expected to be empty.
		@return The profile-specific plug-in object. */
	FUTrackable* GetPluginObject() { return pluginOverride; }
	const FUTrackable* GetPluginObject() const { return pluginOverride; } /**< See above. */

	/** Sets the plug-in object that overrides the extra tree for this profile.*/
	void SetPluginObject(FUTrackable* plugin) { pluginOverride = plugin; }

	/** Clones the extra tree node.
		@param clone The extra tree node that will receive the clone information.
			If this pointer is NULL, a new extra tree technique will be created and you will
			need to release the returned pointer manually.
		@return The clone. */
	virtual FCDENode* Clone(FCDENode* clone) const;
};

/**
	An extra tree attribute.
	Contains a name and a value string.
*/
class FCDEAttribute : public FUParameterizable
{
private:
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, name, FC("Attribute Name")); /**< The attribute name. Must be provided. */
	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, value, FC("Attribute Value")); /**< The attribute value. Is optional. */

public:
	/** Default constructor.
		The name and the value string will be blank. */
	FCDEAttribute();

	/** Constructor.
		Sets the attribute name and the attribute value appropriately.
		@param name The attribute name.
		@param value The attribute value. */
	FCDEAttribute(const char* name, const fchar* value);

	/** Retrieves the name of the attribute.
		@return The name of the attribute. */
	inline const fm::string& GetName() const { return name; }

	/** Sets the name of the attribute.
		@param _name The new name of the attribute. */
	inline void SetName(const fm::string& _name) { name = _name; }

	/** Retrieves the value of the attribute.
		@return The value of the attribute. */
	inline const fstring& GetValue() const { return value; }

	/** Sets the value of the attribute.
		@param _value The new value of the attribute. */
	inline void SetValue(const fstring& _value) { value = _value; }
};

#endif // _FCD_EXTRA_H_
