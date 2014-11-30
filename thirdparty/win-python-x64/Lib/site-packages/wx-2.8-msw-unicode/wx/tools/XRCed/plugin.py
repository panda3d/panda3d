# Name:         plugin.py
# Purpose:      Pluggable component support
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      31.05.2007
# RCS-ID:       $Id: plugin.py 51262 2008-01-17 17:07:19Z ROL $

'''
Functions for loading plugins.
'''

import os, sys, glob
from xml.dom import minidom
from globals import *
from presenter import Manager
import component
import meta

def load_plugins_from_dirs():
    '''Load plugins from XRCEDPATH directories.'''
    dirs = os.getenv('XRCEDPATH')
    if dirs:
        for dir in dirs.split(':'):
            if os.path.isdir(dir):
                load_plugins(dir)

def load_plugins(dir):
    '''Load plugins from C{dir}.'''
    sys_path = sys.path
    cwd = os.getcwd()
    dir = os.path.abspath(os.path.normpath(dir))
    TRACE('* load_plugins from %s' % dir)
    os.chdir(dir)
    sys.path = sys_path + [dir]
    try:                                # try/finally shield
        ff_py = glob.glob('[!_]*.py')
        for f in ff_py:
            name = os.path.splitext(f)[0]
            TRACE('* __import__ %s', name)
            try:
                __import__(name, globals(), locals(), ['*'])
            except ImportError:
                logger.exception('importing %s failed', name)
        ff_crx = glob.glob('*.crx')
        for crx in ff_crx:
            TRACE('* load_crx %s', crx)
            try:
                load_crx(crx)
            except:
                logger.exception('parsing CRX file %s failed', crx)
        dirs = glob.glob('*/')
        for dir in dirs:
            if os.path.isfile(os.path.join(dir, '__init__.py')):
                TRACE('* import __init__.py in %s', dir)
                try:
                    __import__(dir, globals(), locals(), ['*'])
                except ImportError:
                    logger.exception('importing __init__.py failed')
    finally:
        sys.path = sys_path
        os.chdir(cwd)

def load_crx(filename):
    '''Load components defined in a manifest file.'''
    dom = minidom.parse(filename)
    for node in dom.documentElement.childNodes:
        if node.nodeType == node.ELEMENT_NODE and node.tagName == 'component':
            create_component(node)

def create_component(node):
    '''Create component from a manifest data.

    @param node: DOM C{Element} object containing component manifest data.
    '''
    klass = node.getAttribute('class')
    name = node.getAttribute('name')
    TRACE('create_component %s', name)
    comp = getattr(meta, klass)
    compClass = getattr(component, comp.klass) # get component class
    attributesIn = comp.getAttribute(node, 'attributes')
    # Process attr:klass pairs
    attributes = []
    specials = {}
    for a in attributesIn:
        i = a.find(':')
        if i != -1:
            a,kl = a[:i],a[i+1:]
            specials[a] = getattr(component, kl)
        attributes.append(a)
    attParamsIn = comp.getAttribute(node, 'params')
    # Process attr:param_class pairs
    params = {}
    for a in attParamsIn:
        i = a.find(':')
        if i != -1:
            a,kl = a[:i],a[i+1:]
            params[a] = getattr(component.params, kl)
    groups = comp.getAttribute(node, 'groups')
    styles = comp.getAttribute(node, 'styles')
    events = comp.getAttribute(node, 'events')
    c = compClass(name, groups, attributes, specials=specials, params=params, events=events)
    c.hasName = bool(comp.getAttribute(node, 'has-name'))
    c.addStyles(*styles)
    Manager.register(c)
    menu = comp.getAttribute(node, 'menu')
    label = comp.getAttribute(node, 'label')
    if menu and label:
        try:
            index = int(comp.getAttribute(node, 'index'))
        except:
            index = 1000
        help = comp.getAttribute(node, 'help')
        Manager.setMenu(c, menu, label, help, index)
    panel = comp.getAttribute(node, 'panel')
    if panel:
        try:
            pos = map(int, comp.getAttribute(node, 'pos').split(','))
        except:
            pos = component.DEFAULT_POS
        try:
            span = map(int, comp.getAttribute(node, 'span').split(','))
        except:
            span = (1, 1)
        Manager.setTool(c, panel, pos=pos, span=span)
    dlName = comp.getAttribute(node, 'DL')
    if dlName:
        TRACE('Loading dynamic library: %s', dlName)
        if not g._CFuncPtr:
            try:
                import ctypes
                g._CFuncPtr = ctypes._CFuncPtr
            except:
                print 'import ctypes module failed'
        if g._CFuncPtr:
            dl = ctypes.CDLL(dlName)
            try:
                Manager.addXmlHandler(dl.AddXmlHandlers)
            except:
                logger.exception('DL registration failed')
    module = comp.getAttribute(node, 'module')
    handler = comp.getAttribute(node, 'handler')
    if module and handler:
        TRACE('importing handler %s from %s', handler, module)
        try:
            mod = __import__(module, globals(), locals(), [handler])
            Manager.addXmlHandler(getattr(mod, handler))
        except ImportError:
            logger.exception("can't import handler module")
        except AttributeError:
            logger.exception("can't find handler class")

