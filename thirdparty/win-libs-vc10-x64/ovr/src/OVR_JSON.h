/************************************************************************************

PublicHeader:   None
Filename    :   OVR_JSON.h
Content     :   JSON format reader and writer
Created     :   April 9, 2013
Author      :   Brant Lewis
Notes       :

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_JSON_H
#define OVR_JSON_H

#include "Kernel/OVR_RefCount.h"
#include "Kernel/OVR_String.h"
#include "Kernel/OVR_List.h"

namespace OVR {  

// JSONItemType describes the type of JSON item, specifying the type of
// data that can be obtained from it.
enum JSONItemType
{
    JSON_None      = 0,
    JSON_Null      = 1,
    JSON_Bool      = 2,
    JSON_Number    = 3,
    JSON_String    = 4,
    JSON_Array     = 5,
    JSON_Object    = 6
};


//-----------------------------------------------------------------------------
// ***** JSON

// JSON object represents a JSON node that can be either a root of the JSON tree
// or a child item. Every node has a type that describes what is is.
// New JSON trees are typically loaded JSON::Load or created with JSON::Parse.

class JSON : public RefCountBase<JSON>, public ListNode<JSON>
{
protected:
    List<JSON>      Children;

public:
    JSONItemType    Type;       // Type of this JSON node.
    String          Name;       // Name part of the {Name, Value} pair in a parent object.
    String          Value;
    double          dValue;

public:
    ~JSON();

    // *** Creation of NEW JSON objects

    static JSON*    CreateObject()               { return new JSON(JSON_Object);}
    static JSON*    CreateNull()                 { return new JSON(JSON_Null); }
    static JSON*    CreateArray()                { return new JSON(JSON_Array); }
    static JSON*    CreateBool(bool b)           { return createHelper(JSON_Bool, b ? 1.0 : 0.0); }
    static JSON*    CreateNumber(double num)     { return createHelper(JSON_Number, num); }
    static JSON*    CreateString(const char *s)  { return createHelper(JSON_String, 0.0, s); }

    // Creates a new JSON object from parsing string.
    // Returns null pointer and fills in *perror in case of parse error.
    static JSON*    Parse(const char* buff, const char** perror = 0);

    // Loads and parses a JSON object from a file.
    // Returns 0 and assigns perror with error message on fail.
    static JSON*    Load(const char* path, const char** perror = 0);

    // Saves a JSON object to a file.
    bool            Save(const char* path);


    // *** Object Member Access

    // These provide access to child items of the list.
    bool            HasItems() const         { return Children.IsEmpty(); }
    // Returns first/last child item, or null if child list is empty
    JSON*           GetFirstItem()           { return (!Children.IsEmpty()) ? Children.GetFirst() : 0; }
    JSON*           GetLastItem()            { return (!Children.IsEmpty()) ? Children.GetLast() : 0; }

    // Counts the number of items in the object; these methods are inefficient.
    unsigned        GetItemCount() const;
    JSON*           GetItemByIndex(unsigned i);
    JSON*           GetItemByName(const char* name);

    // Returns next item in a list of children; 0 if no more items exist.
    JSON*           GetNextItem(JSON* item)  { return Children.IsNull(item->pNext) ? 0 : item->pNext; }
    JSON*           GetPrevItem(JSON* item)  { return Children.IsNull(item->pPrev) ? 0 : item->pPrev; }


    // Child item access functions
    void            AddItem(const char *string, JSON* item);
    void            AddNullItem(const char* name)                    { AddItem(name, CreateNull()); }
    void            AddBoolItem(const char* name, bool b)            { AddItem(name, CreateBool(b)); }
    void            AddNumberItem(const char* name, double n)        { AddItem(name, CreateNumber(n)); }
    void            AddStringItem(const char* name, const char* s)   { AddItem(name, CreateString(s)); }
//    void            ReplaceItem(unsigned index, JSON* new_item);
//    void            DeleteItem(unsigned index);

    // *** Array Element Access

    // Add new elements to the end of array.
    void            AddArrayElement(JSON *item);
    void            AddArrayNumber(double n)        { AddArrayElement(CreateNumber(n)); }
    void            AddArrayString(const char* s)   { AddArrayElement(CreateString(s)); }

    // Accessed array elements; currently inefficient.
    int             GetArraySize();
    double          GetArrayNumber(int index);
    const char*     GetArrayString(int index);


protected:
    JSON(JSONItemType itemType = JSON_Object);

    static JSON*    createHelper(JSONItemType itemType, double dval, const char* strVal = 0);

    // JSON Parsing helper functions.
    const char*     parseValue(const char *buff, const char** perror);
    const char*     parseNumber(const char *num);
    const char*     parseArray(const char* value, const char** perror);
    const char*     parseObject(const char* value, const char** perror);
    const char*     parseString(const char* str, const char** perror);

    char*           PrintValue(int depth, bool fmt);
    char*           PrintObject(int depth, bool fmt);
    char*           PrintArray(int depth, bool fmt);
};


}

#endif
