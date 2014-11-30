"""

  dexml:  a dead-simple Object-XML mapper for Python

Let's face it: xml is a fact of modern life.  I'd even go so far as to say
that it's *good* at what is does.  But that doesn't mean it's easy to work
with and it doesn't mean that we have to like it.  Most of the time, XML
just needs to get the hell out of the way and let you do some actual work
instead of writing code to traverse and manipulate yet another DOM.

The dexml module takes the obvious mapping between XML tags and Python objects
and lets you capture that as cleanly as possible.  Loosely inspired by Django's
ORM, you write simple class definitions to define the expected structure of
your XML document.  Like so:

  >>> import dexml
  >>> from dexml import fields
  >>> class Person(dexml.Model):
  ...   name = fields.String()
  ...   age = fields.Integer(tagname='age')

Then you can parse an XML document into an object like this:

  >>> p = Person.parse("<Person name='Foo McBar'><age>42</age></Person>")
  >>> p.name
  u'Foo McBar'
  >>> p.age
  42

And you can render an object into an XML document like this:

  >>> p = Person(name="Handsome B. Wonderful",age=36)
  >>> p.render()
  '<?xml version="1.0" ?><Person name="Handsome B. Wonderful"><age>36</age></Person>'

Malformed documents will raise a ParseError:

  >>> p = Person.parse("<Person><age>92</age></Person>")
  Traceback (most recent call last):
      ...
  ParseError: required field not found: 'name'

Of course, it gets more interesting when you nest Model definitions, like this:

  >>> class Group(dexml.Model):
  ...   name = fields.String(attrname="name")
  ...   members = fields.List(Person)
  ...
  >>> g = Group(name="Monty Python")
  >>> g.members.append(Person(name="John Cleese",age=69))
  >>> g.members.append(Person(name="Terry Jones",age=67))
  >>> g.render(fragment=True)
  '<Group name="Monty Python"><Person name="John Cleese"><age>69</age></Person><Person name="Terry Jones"><age>67</age></Person></Group>'

There's support for XML namespaces, default field values, case-insensitive
parsing, and more fun stuff.  Check out the documentation on the following
classes for more details:

  :Model:  the base class for objects that map into XML
  :Field:  the base class for individual model fields
  :Meta:   meta-information about how to parse/render a model

"""

__ver_major__ = 0
__ver_minor__ = 3
__ver_patch__ = 7
__ver_sub__ = ""
__version__ = "%d.%d.%d%s" % (__ver_major__,__ver_minor__,__ver_patch__,__ver_sub__)
                              

import copy
from xml.dom import minidom

## Local Imports
import fields
from _util import *

class Model(object):
    """Base class for dexml Model objects.

    Subclasses of Model represent a concrete type of object that can parsed 
    from or rendered to an XML document.  The mapping to/from XML is controlled
    by two things:

        * attributes declared on an inner class named 'meta'
        * fields declared using instances of fields.Field

    Here's a quick example:

        class Person(dexml.Model):
            # This overrides the default tagname of 'Person'
            class meta
                tagname = "person"
            # This maps to a 'name' attributr on the <person> tag
            name = fields.String()
            # This maps to an <age> tag within the <person> tag
            age = fields.Integer(tagname='age')

    See the 'Meta' class in this module for available meta options, and the
    'fields' submodule for available field types.
    """

    __metaclass__ = ModelMetaclass
    _fields = []

    def __init__(self,**kwds):
        """Default Model constructor.

        Keyword arguments that correspond to declared fields are processed
        and assigned to that field.
        """
        for f in self._fields:
            val = kwds.get(f.field_name)
            setattr(self,f.field_name,val)

    @classmethod
    def parse(cls,xml):
        """Produce an instance of this model from some xml.

        The given xml can be a string, a readable file-like object, or
        a DOM node; we might add support for more types in the future.
        """
        self = cls()
        node = self._make_xml_node(xml)
        self.validate_xml_node(node)
        #  Keep track of fields that have successfully parsed something
        fields_found = []
        #  Try to consume all the node's attributes
        attrs = node.attributes.values()
        for field in self._fields:
            unused_attrs = field.parse_attributes(self,attrs)
            if len(unused_attrs) < len(attrs):
                fields_found.append(field)
            attrs = unused_attrs
        for attr in attrs:
            self._handle_unparsed_node(attr)
        #  Try to consume all child nodes
        if self.meta.order_sensitive:
            self._parse_children_ordered(node,self._fields,fields_found)
        else:
            self._parse_children_unordered(node,self._fields,fields_found)
        #  Check that all required fields have been found
        for field in self._fields:
            if field.required and field not in fields_found:
                err = "required field not found: '%s'" % (field.field_name,)
                raise ParseError(err)
            field.parse_done(self)
        #  All done, return the instance so created
        return self

    def _parse_children_ordered(self,node,fields,fields_found):
        """Parse the children of the given node using strict field ordering."""
        cur_field_idx = 0 
        for child in node.childNodes:
            idx = cur_field_idx
            #  If we successfully break out of this loop, one of our
            #  fields has consumed the node.
            while idx < len(fields):
                field = fields[idx]
                res = field.parse_child_node(self,child)
                if res is PARSE_DONE:
                    if field not in fields_found:
                        fields_found.append(field)
                    cur_field_idx = idx + 1
                    break
                if res is PARSE_MORE:
                    if field not in fields_found:
                        fields_found.append(field)
                    cur_field_idx = idx
                    break
                if res is PARSE_CHILDREN:
                    self._parse_children_ordered(child,[field],fields_found)
                    cur_field_idx = idx
                    break
                idx += 1
            else:
                self._handle_unparsed_node(child)

    def _parse_children_unordered(self,node,fields,fields_found):
        """Parse the children of the given node using loose field ordering."""
        done_fields = {}
        for child in node.childNodes:
            idx = 0
            #  If we successfully break out of this loop, one of our
            #  fields has consumed the node.
            while idx < len(fields):
                if idx in done_fields:
                    idx += 1
                    continue
                field = fields[idx]
                res = field.parse_child_node(self,child)
                if res is PARSE_DONE:
                    done_fields[idx] = True
                    if field not in fields_found:
                        fields_found.append(field)
                    break
                if res is PARSE_MORE:
                    if field not in fields_found:
                        fields_found.append(field)
                    break
                if res is PARSE_CHILDREN:
                    self._parse_children_unordered(child,[field],fields_found)
                    break
                idx += 1
            else:
                self._handle_unparsed_node(child)

    def _handle_unparsed_node(self,node):
        if not self.meta.ignore_unknown_elements:
            if node.nodeType == node.ELEMENT_NODE:
                err = "unknown element: %s" % (node.nodeName,)
                raise ParseError(err)
            elif node.nodeType in (node.TEXT_NODE,node.CDATA_SECTION_NODE):
                if node.nodeValue.strip():
                    err = "unparsed text node: %s" % (node.nodeValue,)
                    raise ParseError(err)
            elif node.nodeType == node.ATTRIBUTE_NODE:
                if not node.nodeName.startswith("xml"):
                    err = "unknown attribute: %s" % (node.name,)
                    raise ParseError(err)

    def render(self,encoding=None,fragment=False,nsmap=None):
        """Produce XML from this model's instance data.

        A unicode string will be returned if any of the objects contain
        unicode values; specifying the 'encoding' argument forces generation
        of an ASCII string.

        By default a complete XML document is produced, including the
        leading "<?xml>" declaration.  To generate an XML fragment set
        the 'fragment' argument to True.

        The 'nsmap' argument maintains the current stack of namespace
        prefixes used during rendering; it maps each prefix to a list of
        namespaces, with the first item in the list being the current
        namespace for that prefix.  This argument should never be given
        directly; it is for internal use by the rendering routines.
         
        """
        if nsmap is None:
            nsmap = {}
        data = []
        if not fragment:
            if encoding:
                s = '<?xml version="1.0" encoding="%s" ?>' % (encoding,)
                data.append(s)
            else:
                data.append('<?xml version="1.0" ?>')
        data.extend(self._render(nsmap))
        xml = "".join(data)
        if encoding:
            xml = xml.encode(encoding)
        return xml

    def _render(self,nsmap):
        """Render this model as an XML fragment."""
        #  Determine opening and closing tags
        pushed_ns = False
        if self.meta.namespace:
            namespace = self.meta.namespace
            prefix = self.meta.namespace_prefix
            try:
                cur_ns = nsmap[prefix]
            except KeyError:
                cur_ns = []
                nsmap[prefix] = cur_ns
            if prefix:
                tagname = "%s:%s" % (prefix,self.meta.tagname)
                open_tag_contents = [tagname]
                if not cur_ns or cur_ns[0] != namespace:
                    cur_ns.insert(0,namespace)
                    pushed_ns = True
                    open_tag_contents.append('xmlns:%s="%s"'%(prefix,namespace))
                close_tag_contents = tagname
            else:
                open_tag_contents = [self.meta.tagname]
                if not cur_ns or cur_ns[0] != namespace:
                    cur_ns.insert(0,namespace)
                    pushed_ns = True
                    open_tag_contents.append('xmlns="%s"'%(namespace,))
                close_tag_contents = self.meta.tagname
        else:
            open_tag_contents = [self.meta.tagname] 
            close_tag_contents = self.meta.tagname
        # Find the attributes and child nodes
        attrs = []
        children = []
        num = 0
        for f in self._fields:
            val = getattr(self,f.field_name)
            attrs.extend(f.render_attributes(self,val,nsmap))
            children.extend(f.render_children(self,val,nsmap))
            if len(attrs) + len(children) == num and f.required:
                raise RenderError("Field '%s' is missing" % (f.field_name,))
        #  Actually construct the XML
        if pushed_ns:
            nsmap[prefix].pop(0)
        open_tag_contents.extend(attrs)
        if children:
            yield "<%s>" % (" ".join(open_tag_contents),)
            for chld in children:
                yield chld
            yield "</%s>" % (close_tag_contents,)
        else:
            yield "<%s />" % (" ".join(open_tag_contents),)

    @staticmethod
    def _make_xml_node(xml):
        """Transform a variety of input formats to an XML DOM node."""
        try:
            ntype = xml.nodeType
        except AttributeError:
            if isinstance(xml,basestring):
                try:
                    xml = minidom.parseString(xml)
                except Exception, e:
                    raise XmlError(e)
            elif hasattr(xml,"read"):
                try:
                    xml = minidom.parse(xml)
                except Exception, e:
                    raise XmlError(e)
            else:
                raise ValueError("Can't convert that to an XML DOM node")
            node = xml.documentElement
        else:
            if ntype == xml.DOCUMENT_NODE:
                node = xml.documentElement
            else:
                node = xml
        return node

    @classmethod
    def validate_xml_node(cls,node):
        """Check that the given xml node is valid for this object.

        Here 'valid' means that it is the right tag, in the right
        namespace.  We might add more eventually...
        """
        if node.nodeType != node.ELEMENT_NODE:
            err = "Class '%s' got a non-element node"
            err = err % (cls.__name__,)
            raise ParseError(err)
        equals = (lambda a, b: a == b) if cls.meta.case_sensitive else (lambda a, b: a.lower() == b.lower())
        if not equals(node.localName, cls.meta.tagname):
            err = "Class '%s' got tag '%s' (expected '%s')"
            err = err % (cls.__name__,node.localName,
                         cls.meta.tagname)
            raise ParseError(err)
        if cls.meta.namespace:
            if node.namespaceURI != cls.meta.namespace:
                err = "Class '%s' got namespace '%s' (expected '%s')"
                err = err % (cls.__name__,node.namespaceURI,
                             cls.meta.namespace)
                raise ParseError(err)
        else:
            if node.namespaceURI:
                err = "Class '%s' got namespace '%s' (expected no namespace)"
                err = err % (cls.__name__,node.namespaceURI,)
                raise ParseError(err)


