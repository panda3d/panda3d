# Name:         meta.py
# Purpose:      
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      05.07.2007
# RCS-ID:       $Id: core.py 47129 2007-07-04 22:38:37Z ROL $

from globals import *
import component
from attribute import *
import params

# Meta-components for loading CRX files

Component = component.SimpleComponent(
    'Component', ['component'],
    ['provider', 'version', 'url', 
     'groups', 'attributes', 'params', 'has-name', 'styles', 'events',
     'DL', 'module', 'handler',
     'menu', 'label', 'help', 'index', 'panel', 'pos', 'span'],
    specials={'provider': AttributeAttribute,
              'version': AttributeAttribute,
              'url': AttributeAttribute,
              'groups': ContentAttribute,
              'attributes': ContentAttribute,
              'params': ContentAttribute,
              'styles': ContentAttribute,
              'events': ContentAttribute},
    params={'provider': params.ParamLongText,
            'url': params.ParamLongText,
            'groups': params.ParamContent,
            'attributes': params.ParamContent,
            'params': params.ParamContent,
            'styles': params.ParamContent,
            'events': params.ParamContent,
            'has-name': params.ParamBool,
            'handler': params.ParamLongText,
            'index': params.ParamInt,
            'pos': params.ParamPosSize, 
            'span': params.ParamPosSize})

