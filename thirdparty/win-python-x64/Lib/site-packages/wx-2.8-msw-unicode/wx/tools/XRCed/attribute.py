# Name:         attribute.py
# Purpose:      Attribute classes
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      25.06.2007
# RCS-ID:       $Id$

'''
Attribute processing classes.

This module contains some predefined classes which can be used to store and
retrieve XML data into Python objects.
'''

import cPickle
from model import Model

class Attribute:
    '''Base class, used for simple attributes, i.e. single attributes
    storing data as text strings.'''
    @staticmethod
    def add(parentNode, attribute, value):
        '''Store attribute value in DOM as a text node.

        @param attribute: Attribute name.
        @param value: Attribute value (Python string).
        '''
        if attribute == '':
            elem = parentNode
        else:
            elem = Model.dom.createElement(attribute)
            parentNode.appendChild(elem)
        text = Model.dom.createTextNode(value)
        elem.appendChild(text)
    @staticmethod
    def get(node):
        '''Get value (or return a default value) from a DOM C{Element} object.'''
        if node is None: return ''
        try:
            n = node.childNodes[0]
            return n.wholeText
        except IndexError:
            return ''

class ContentAttribute:
    '''Content attribute class. Value is a list of strings.'''
    @staticmethod
    def add(parentNode, attribute, value):
        contentElem = Model.dom.createElement(attribute)
        parentNode.appendChild(contentElem)
        for item in value:
            elem = Model.dom.createElement('item')
            text = Model.dom.createTextNode(item)
            elem.appendChild(text)
            contentElem.appendChild(elem)
    @staticmethod
    def get(node):
        if node is None: return []
        value = []
        for n in node.childNodes:
            if n.nodeType == node.ELEMENT_NODE and n.tagName == 'item':
                value.append(Attribute.get(n))
        return value

class CheckContentAttribute:
    '''CheckList content. Value is a list of tuples (checked, string).'''
    @staticmethod
    def add(parentNode, attribute, value):
        contentElem = Model.dom.createElement(attribute)
        parentNode.appendChild(contentElem)
        for checked,item in value:
            elem = Model.dom.createElement('item')
            if checked:
                elem.setAttribute('checked', '1')
            text = Model.dom.createTextNode(item)
            elem.appendChild(text)
            contentElem.appendChild(elem)
    @staticmethod
    def get(node):
        if node is None: return []
        value = []
        for n in node.childNodes:
            if n.nodeType == node.ELEMENT_NODE and n.tagName == 'item':
                checked = bool(n.getAttribute('checked'))
                value.append((checked, Attribute.get(n)))
        return value

class DictAttribute:
    '''DictAttribute uses dictionary object for passing data.'''
    attributes = []
    @classmethod
    def add(cls, parentNode, attribute, value):
        fontElem = Model.dom.createElement(attribute)
        parentNode.appendChild(fontElem)
        for a in filter(value.has_key, cls.attributes):
            elem = Model.dom.createElement(a)
            text = Model.dom.createTextNode(value[a])
            elem.appendChild(text)
            fontElem.appendChild(elem)
    @staticmethod
    def get(node):
        if node is None: return {}
        value = {}
        for n in node.childNodes:
            if n.nodeType == node.ELEMENT_NODE:
                value[n.tagName] = Attribute.get(n)
        return value

class FontAttribute(DictAttribute):
    attributes = ['size', 'style', 'weight', 'underlined', 'family', 'face', 'encoding', 
                  'sysfont', 'relativesize']

class CodeAttribute(DictAttribute):
    attributes = ['events', 'assign_var']

class MultiAttribute:
    '''Repeated attribute like growablecols.'''
    @staticmethod
    def add(parentNode, attribute, value):
        for v in value:
            elem = Model.dom.createElement(attribute)
            parentNode.appendChild(elem)
            text = Model.dom.createTextNode(v)
            elem.appendChild(text)
    @staticmethod
    def get(node):
        if node is None: return []
        tag = node.tagName  # remember tag name
        value = []
        # Look for multiple tags
        while node:
            if node.nodeType == node.ELEMENT_NODE and node.tagName == tag:
                value.append(Attribute.get(node))
            node = node.nextSibling
        return value

class BitmapAttribute:
    '''Bitmap attribute.'''
    @staticmethod
    def add(parentNode, attribute, value):
        if not value[0] and not value[1]: return
        if attribute == 'object':
            elem = parentNode
        else:
            elem = Model.dom.createElement(attribute)
            parentNode.appendChild(elem)
        if value[0]:
            elem.setAttribute('stock_id', value[0])
        else:
            if elem.hasAttribute('stock_id'): elem.removeAttribute('stock_id')
            text = Model.dom.createTextNode(value[1])
            elem.appendChild(text)
    @staticmethod
    def get(node):
        if node is None: return []
        return [node.getAttribute('stock_id'), Attribute.get(node)]
            
class AttributeAttribute:
    '''Attribute as an XML attribute of the element node.'''
    @staticmethod
    def add(elem, attribute, value):
        if value:
            elem.setAttribute(attribute, value)
        else:
            if elem.hasAttribute(attribute): elem.removeAttribute(attribute)
    @staticmethod
    def getAA(elem, attribute):
        return elem.getAttribute(attribute)

class EncodingAttribute(AttributeAttribute):
    '''Encoding is a special attribute stored in dom object.'''
    @staticmethod
    def add(elem, attribute, value):
        Model.dom.encoding = value
    @staticmethod
    def getAA(elem, attribute):
        return Model.dom.encoding
            
class CDATAAttribute(Attribute):
    def add(parentNode, attribute, value):
        '''value is a dictionary.'''
        if value:
            elem = Model.dom.createElement(attribute)
            parentNode.appendChild(elem)
            data = Model.dom.createCDATASection(cPickle.dumps(value))
            elem.appendChild(data)
    @staticmethod
    def get(node):
        '''Get XRCED data from a CDATA text node.'''
        try:
            n = node.childNodes[0]
            if n.nodeType == n.CDATA_SECTION_NODE:
                return cPickle.loads(n.wholeText.encode())
        except IndexError:
            pass
    
class CommentAttribute(AttributeAttribute):
    '''Comment is the value of comment object.'''
    @staticmethod
    def add(node, attribute, value):
        node.data = value
    @staticmethod
    def getAA(node, attribute):
        return node.data
            
