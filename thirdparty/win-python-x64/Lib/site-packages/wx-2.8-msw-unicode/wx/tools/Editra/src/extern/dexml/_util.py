
class Error(Exception):
    """Base exception class for the dexml module."""
    pass

class ParseError(Error):
    """Exception raised when XML could not be parsed into objects."""
    pass

class RenderError(Error):
    """Exception raised when object could not be rendered into XML."""
    pass

class XmlError(Error):
    """Exception raised to encapsulate errors from underlying XML parser."""
    pass

class PARSE_DONE:
    """Constant returned by a Field when it has finished parsing."""
    pass
class PARSE_MORE:
    """Constant returned by a Field when it wants additional nodes to parse."""
    pass
class PARSE_SKIP:
    """Constant returned by a Field when it cannot parse the given node."""
    pass
class PARSE_CHILDREN:
    """Constant returned by a Field to parse children from its container tag."""
    pass

class Meta:
    """Class holding meta-information about a dexml.Model subclass.

    Each dexml.Model subclass has an attribute 'meta' which is an instance
    of this class.  That instance holds information about how to model 
    corresponds to XML, such as its tagname, namespace, and error handling
    semantics.  You would not ordinarily create an instance of this class;
    instead let the ModelMetaclass create one automatically.

    These attributes control how the model corresponds to the XML:

        * tagname:  the name of the tag representing this model
        * namespace:  the XML namespace in which this model lives

    These attributes control parsing/rendering behaviour:

        * namespace_prefix:  the prefix to use for rendering namespaced tags
        * ignore_unknown_elements:  ignore unknown elements when parsing
        * case_sensitive:    match tag/attr names case-sensitively
        * order_sensitive:   match child tags in order of field definition

    """

    _defaults = {"tagname":None,
                 "namespace":None,
                 "namespace_prefix":None,
                 "ignore_unknown_elements":True,
                 "case_sensitive":True,
                 "order_sensitive":True}

    def __init__(self,name,meta_attrs):
        for (attr,default) in self._defaults.items():
            setattr(self,attr,meta_attrs.get(attr,default))
        if self.tagname is None:
            self.tagname = name

def _meta_attributes(meta):
    """Extract attributes from a "meta" object."""
    meta_attrs = {}
    if meta:
        for attr in dir(meta):
            if not attr.startswith("_"):
                meta_attrs[attr] = getattr(meta,attr)
    return meta_attrs

class ModelMetaclass(type):
    """Metaclass for dexml.Model and subclasses.

    This metaclass is responsible for introspecting Model class definitions
    and setting up appropriate default behaviours.  For example, this metaclass
    sets a Model's default tagname to be equal to the declared class name.
    """

    instances = {}

    def __new__(mcls,name,bases,attrs):
        cls = super(ModelMetaclass,mcls).__new__(mcls,name,bases,attrs)
        #  Don't do anything if it's not a subclass of Model
        parents = [b for b in bases if isinstance(b, ModelMetaclass)]
        # HACK
        import fields

        if not parents:
            return cls
        #  Set up the cls.meta object, inheriting from base classes
        meta_attrs = {}
        for base in bases:
            if isinstance(base,ModelMetaclass) and hasattr(base,"meta"):
                meta_attrs.update(_meta_attributes(base.meta))
        meta_attrs.pop("tagname",None)
        meta_attrs.update(_meta_attributes(attrs.get("meta",None)))
        cls.meta = Meta(name,meta_attrs)
        #  Create ordered list of field objects, telling each about their
        #  name and containing class.  Inherit fields from base classes
        #  only if not overridden on the class itself.
        base_fields = {}
        for base in bases:
            if not isinstance(base,ModelMetaclass):
                continue
            for field in base._fields:
                if field.field_name not in base_fields:
                    field = copy.copy(field)
                    field.model_class = cls
                    base_fields[field.field_name] = field
        cls_fields = []
        for (name,value) in attrs.iteritems():
            if isinstance(value,fields.Field):
                base_fields.pop(name,None)
                value.field_name = name
                value.model_class = cls
                cls_fields.append(value)
        cls._fields = base_fields.values() + cls_fields
        cls._fields.sort(key=lambda f: f._order_counter)
        #  Register the new class so we can find it by name later on
        mcls.instances[(cls.meta.namespace,cls.meta.tagname)] = cls
        return cls

    @classmethod
    def find_class(mcls,tagname,namespace=None):
        """Find dexml.Model subclass for the given tagname and namespace."""
        return mcls.instances.get((namespace,tagname))
