"""EntityTypes module: contains classes that describe Entity types"""

from EntityTypeDesc import EntityTypeDesc
from SpecImports import *

class Entity(EntityTypeDesc):
    abstract = 1
    type = 'entity'
    attribs = (
        ('type', None),
        ('name', '<unnamed>', 'string'),
        ('comment', '', 'string'),
        )

class LevelMgr(Entity):
    type = 'levelMgr'
    permanent = 1
    attribs = (
        ('cogLevel', 0, 'int', {'min':0, 'max':11}),
        ('cogTrack', 'c', 'choice', {'choiceSet':['c','s','l','m']}),
        ('modelFilename', '', 'bamfilename'),
        )

class EditMgr(Entity):
    type = 'editMgr'
    permanent = 1
    attribs = (
        ('requestSave', None),
        ('requestNewEntity', None),
        ('insertEntity', None),
        ('removeEntity', None),
        )

class Nodepath(Entity):
    type = 'nodepath'
    attribs = (
        ('parentEntId', 0, 'entId', {'type':'nodepath'}),
        ('pos', Point3(0,0,0), 'pos'),
        ('hpr', Vec3(0,0,0), 'hpr'),
        )

class Zone(Nodepath):
    type = 'zone'
    permanent = 1
    delAttribs = (
        'parentEntId',
        'pos',
        'hpr',
        )
    attribs = (
        ('description', '', 'string'),
        ('modelZoneNum', -1, 'int'),
        ('visibility', [], 'visZoneList'),
        )

class LogicGate(Entity):
    type = 'logicGate'
    output = 'bool'
    attribs = (
        ('input1Event', 0, 'entId', {'output':'bool'}),
        ('input2Event', 0, 'entId', {'output':'bool'}),
        ('isInput1', 0, 'bool'),
        ('isInput2', 0, 'bool'),
        ('logicType', 'or', 'choice',
         {'choiceSet':['or','and','xor','nand','nor','xnor']}),
        )

class CutScene(Entity):
    type = 'cutScene'
    output = 'bool'
    attribs = (
        ('pos', Point3(0,0,0), 'pos'),
        ('hpr', Vec3(0,0,0), 'hpr'),
        ('startStopEvent', 0, 'entId', {'output':'bool'}),
        ('effect', 'irisInOut', 'choice', {'choiceSet':['nothing','irisInOut','letterBox']}),
        ('motion', 'foo1', 'choice', {'choiceSet':['foo1']}),
        ('duration', 5.0, 'float'),
        )

class Model(Nodepath):
    type = 'model'
    attribs = (
        ('scale', 1, 'pos'),
        ('modelPath', None, 'bamfilename'),
        )

class Path(Nodepath):
    type = 'path'
    attribs = (
        ('scale', 1, 'pos'),
        ('pathIndex', 0, 'int'),
        )
    
