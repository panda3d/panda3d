"""EntityTypes module: contains classes that describe Entity types"""

from SpecImports import *

class Entity:
    attribs = (
        ('type', None),
        ('name', 'unnamed'),
        ('comment', ''),
        )

class ActiveCell(Entity):
    type = 'activeCell'
    attribs = (
        ('row', 0),
        ('col', 0),
        ('gridId', None)
        )

class DirectionalCell(ActiveCell):
    type = 'directionalCell'
    attribs = (
        ('dir', [0,0]),
        )
    
class LevelMgr(Entity):
    type = 'levelMgr'
    attribs = (
        ('cogLevel', 0),
        ('cogTrack', 'c'),
        ('modelFilename', None),
        )

class EditMgr(Entity):
    type = 'editMgr'
    attribs = (
        ('insertEntity', None),
        ('removeEntity', None),
        )

class LogicGate(Entity):
    type = 'logicGate'
    attribs = (
        ('input_input1_bool', 0, 'boolean'),
        ('input_input2_bool', 0, 'boolean'),
        ('isInput1', 0),
        ('isInput2', 0),
        ('logicType', 'or'),
        ('output', 'bool'),
        )

class NodepathImpl:
    attribs = (
        ('parent', 0),
        ('pos', Point3(0,0,0), 'pos'),
        ('hpr', Vec3(0,0,0), 'hpr'),
        )

# Note: this covers Nodepath and DistributedNodepath
class Nodepath(Entity, NodepathImpl):
    type = 'nodepath'

class NodepathAttribs:
    attribs = (
        ('parent', 0),
        ('pos', Point3(0,0,0), 'pos'),
        ('hpr', Vec3(0,0,0), 'hpr'),
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
        ('visibility', []),
        )

class CutScene(Entity):
    type = 'cutScene'
    output = 'bool'
    attribs = (
        ('pos', Point3(0,0,0), 'pos'),
        ('hpr', Vec3(0,0,0), 'hpr'),
        ('startStop', 0),
        ('effect', 'irisInOut'),
        ('motion', 'foo1'),
        ('duration', 5.0),
        )

class BarrelBase(Nodepath):
    delAttribs = (
        'hpr',
        )
    attribs = (
        ('h', 0, 'float'),
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
        ('color', Vec4(1,1,1,1), 'color'),
        ('model', None),
        ('input_isOn_bool', 0, 'boolean'),
        ('isOn', 0),
        ('output', 'bool'),
        ('secondsOn', 1),
        )

class Button(Switch):
    type = 'button'

class Trigger(Switch):
    type = 'trigger'

class ConveyorBelt(Nodepath):
    type = 'conveyorBelt'
    attribs = (
        ('speed', 1.0),
        ('length', 1.0),
        ('widthScale', 1.0),
        ('treadLength', 1.0),
        ('treadModelPath', 'phase_7/models/cogHQ/platform1'),
        ('floorName', 'platformcollision'),
        )        
        
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
        ('pos', Point3(0,0,0), 'pos'),
        ('hpr', Vec3(0,0,0), 'hpr'),
        ('scale', 1),
        ('color', Vec4(1,1,1,1), 'color'),
        ('model', None),
        ('doorwayNum', 0),
        ('input_Lock0_bool', 0, 'boolean'),
        ('input_Lock1_bool', 0, 'boolean'),
        ('input_Lock2_bool', 0, 'boolean'),
        ('input_Lock3_bool', 0, 'boolean'),
        ('input_isOpen_bool', 0, 'boolean'),
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
        ('startPos', Point3(0,0,0), 'pos'),
        ('endPos', Point3(0,0,0), 'pos'),
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
        ('startPos', Point3(0,0,0), 'pos'),
        ('endPos', Point3(0,0,0), 'pos'),
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
        ('endPos', Point3(0,0,0), 'pos'),
        ('phaseShift', 0.),
        ('startPos', Point3(0,0,0), 'pos'),
        ('verticalRange', 1),
        ('sinkRate', 1),
        ('riseRate', 1),
        ('speed', 1),
        ('waitDur', 1),
        )

class Stomper(Nodepath):
    type = 'stomper'
    attribs = (
        ('headScale', Vec3(1,1,1), 'scale'),
        ('motion', 3),
        ('period', 2.),
        ('phaseShift', 0.),
        ('range', 6),
        ('shaftScale', Vec3(1,1,1), 'scale'),
        ('soundLen', 0),
        ('soundOn', 0),
        ('style', 'horizontal'),
        ('zOffset', 0),
        )

class StomperPair(Nodepath):
    type = 'stomperPair'
    attribs = (
        ('headScale', Vec3(1,1,1), 'scale'),
        ('motion', 3),
        ('period', 2.),
        ('phaseShift', 0.),
        ('range', 6),
        ('shaftScale', Vec3(1,1,1), 'scale'),
        ('soundLen', 0),
        ('soundOn', 0),
        ('stomperIds', []),
        ('style', 'horizontal'),
        )
