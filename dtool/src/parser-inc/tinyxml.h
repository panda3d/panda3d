#ifndef TINYXML_H
#define TINYXML_H

// A simple header to mirror the subset of the tinyxml interface we
// wish to expose to interrogate.  This is intended to protect us from
// having to run interrogate directly on the tinyxml.h header file.

class TiXmlBase;
class TiXmlNode;
class TiXmlElement;
class TiXmlDocument;

class TiXmlBase {
};


class TiXmlNode : public TiXmlBase {
public:
  const char *Value() const;
  void SetValue(const char *_value);

  TiXmlNode *InsertEndChild(const TiXmlNode &addThis);
  bool RemoveChild( TiXmlNode* removeThis );
  
  const TiXmlElement *NextSiblingElement() const;
  TiXmlElement *NextSiblingElement();

  const TiXmlElement* NextSiblingElement(const char *) const;
  TiXmlElement* NextSiblingElement(const char *_next);

  const TiXmlElement* FirstChildElement() const;
  TiXmlElement* FirstChildElement();

  const TiXmlElement* FirstChildElement( const char * _value ) const;
  TiXmlElement* FirstChildElement( const char * _value );

  virtual TiXmlNode* Clone() const;
};


class TiXmlElement : public TiXmlNode {
public:
  TiXmlElement(const char * in_value);
  TiXmlElement( const TiXmlElement& );

  const char* Attribute( const char* name ) const;
  void SetAttribute( const char* name, const char * _value );
  void RemoveAttribute( const char * name );
};

class TiXmlDeclaration : public TiXmlNode {
public:
  TiXmlDeclaration(const char* _version,
                   const char* _encoding,
                   const char* _standalone);
};


class TiXmlDocument : public TiXmlNode {
public:
  TiXmlDocument();
  TiXmlDocument(const char * documentName);

  bool LoadFile();
  bool SaveFile() const;
  bool LoadFile(const char * filename);
  bool SaveFile(const char * filename) const;
};


#endif
