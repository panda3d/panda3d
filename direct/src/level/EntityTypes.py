"""EntityTypes module: contains classes that describe Entity types"""

from SpecImports import *

class Entity:
    attribs = (
        ('type', None),
        ('name', 'unnamed'),
        ('comment', ''),
        )

class LevelMgr(Entity):
    type = 'levelMgr'
    attribs = (
        ('cogLevel', 0),
        ('cogTrack', 'c'),
        ('modelFilename', None),
        )

class LogicGate(Entity):
    type = 'logicGate'
    attribs = (
        ('input_input1_bool', 0),
        ('input_input2_bool', 0),
        ('isInput1', 0),
        ('isInput2', 0),
        ('logicType', 'or'),
        ('output', 'bool'),
        )

class NodepathImpl:
    attribs = (
        ('parent', 0),
        ('pos', Point3(0,0,0)),
        ('hpr', Vec3(0,0,0)),
        )

# Note: this covers Nodepath and DistributedNodepath
class Nodepath(Entity, NodepathImpl):
    type = 'nodepath'

class NodepathAttribs:
    attribs = (
        ('parent', 0),
        ('pos', Point3(0,0,0)),
        ('hpr', Vec3(0,0,0)),
        )

class Zone(Entity, NodepathAttribs):
    type = 'zone'
    delAttribs = (
        'parent',
        'pos',
        'hpr',
        )
    attribs = (
        ('description', ''),
        ('modelZoneNum', None),
        )

class BarrelBase(Nodepath):
    delAttribs = (
        'hpr',
        )
    attribs = (
        ('h', 0),
        )

class BeanBarrel(BarrelBase):
    type = 'beanBarrel'

class GagBarrel(BarrelBase):
    type = 'gagBarrel'
    attribs = (
        ('gagLevel', 0),
        ('gagTrack', 0),
        )

class Switch(Entity, NodepathImpl):
    attribs = (
        ('scale', 1),
        ('color', Vec4(1,1,1,1)),
        ('model', None),
        ('input_isOn_bool', 0),
        ('isOn', 0),
        ('output', 'bool'),
        ('secondsOn', 1),
        )

class Button(Switch):
    type = 'button'

class Trigger(Switch):
    type = 'trigger'

class Crate(Nodepath):
    type = 'crate'
    delAttribs = (
        'hpr',
        )
    attribs = (
        ('scale', 1),
        ('gridId', None),
        ('pushable', 1),
        )

class Door(Entity):
    type = 'door'
    attribs = (
        ('parent', 0),
        ('pos', Point3(0,0,0)),
        ('hpr', Vec3(0,0,0)),
        ('scale', 1),
        ('color', Vec4(1,1,1,1)),
        ('model', None),
        ('doorwayNum', 0),
        ('input_Lock0_bool', 0),
        ('input_Lock1_bool', 0),
        ('input_Lock2_bool', 0),
        ('input_Lock3_bool', 0),
        ('input_isOpen_bool', 0),
        ('isLock0Unlocked', 0),
        ('isLock1Unlocked', 0),
        ('isLock2Unlocked', 0),
        ('isLock3Unlocked', 0),
        ('isOpen', 0),
        ('output', 'bool'),
        ('secondsOpen', 1),
        )

class Grid(Nodepath):
    type = 'grid'
    delAttribs = (
        'hpr',
        )
    attribs = (
        ('cellSize', 3),
        ('numCol', 3),
        ('numRow', 3),
        )

class Lift(Nodepath):
    type = 'lift'
    attribs = (
        ('duration', 1),
        ('startPos', Point3(0,0,0)),
        ('endPos', Point3(0,0,0)),
        ('modelPath', ''),
        ('floorName', ''),
        ('modelScale', 1),
        )

class Platform(Nodepath):
    type = 'platform'
    delAttribs = (
        'pos',
        )
    attribs = (
        ('startPos', Point3(0,0,0)),
        ('endPos', Point3(0,0,0)),
        ('speed', 1),
        ('waitDur', 1),
        ('phaseShift', 0.),
        )

class SinkingPlatform(Nodepath):
    type = 'sinkingPlatform'
    delAttribs = (
        'pos',
        )
    attribs = (
        ('endPos', Point3(0,0,0)),
        ('phaseShift', 0.),
        ('startPos', Point3(0,0,0)),
        ('verticalRange', 1),
        ('sinkRate', 1),
        ('riseRate', 1),
        ('speed', 1),
        ('waitDur', 1),
        )

class Stomper(Nodepath):
    type = 'stomper'
    attribs = (
        ('headScale', Vec3(1,1,1)),
        ('motion', 3),
        ('period', 2.),
        ('phaseShift', 0.),
        ('range', 6),
        ('shaftScale', Vec3(1,1,1)),
        ('soundLen', 0),
        ('soundOn', 0),
        ('style', 'horizontal'),
        ('zOffset', 0),
        )

class StomperPair(Nodepath):
    type = 'stomperPair'
    attribs = (
        ('headScale', Vec3(1,1,1)),
        ('motion', 3),
        ('period', 2.),
        ('phaseShift', 0.),
        ('range', 6),
        ('shaftScale', Vec3(1,1,1)),
        ('soundLen', 0),
        ('soundOn', 0),
        ('stomperIds', []),
        ('style', 'horizontal'),
        )
