"""EntityTypes module: contains classes that describe Entity types"""

class Entity:
    attribs = (
        'type',
        'name',
        'comment',
        )

class LevelMgr(Entity):
    name = 'levelMgr'
    attribs = (
        'cogLevel',
        'cogTrack',
        'modelFilename',
        )

class LogicGate(Entity):
    name = 'logicGate'
    attribs = (
        'input_input1_bool',
        'input_input2_bool',
        'isInput1',
        'isInput2',
        'logicType',
        'output',
        )

class NodepathImpl:
    attribs = (
        'parent',
        'pos',
        'hpr',
        )

# Note: this covers Nodepath and DistributedNodepath
class Nodepath(Entity, NodepathImpl):
    name = 'nodepath'

class NodepathAttribs:
    attribs = (
        'parent',
        'pos',
        'hpr',
        )

class Zone(Entity, NodepathAttribs):
    name = 'zone'
    delAttribs = (
        'parent',
        'pos',
        'hpr',
        )
    attribs = (
        'description',
        'modelZoneNum',
        )

class BarrelBase(Nodepath):
    delAttribs = (
        'hpr',
        )
    attribs = (
        'h',
        )

class BeanBarrel(BarrelBase):
    name = 'beanBarrel'

class GagBarrel(BarrelBase):
    name = 'gagBarrel'
    attribs = (
        'gagLevel',
        'gagTrack',
        )

class Switch(Entity, NodepathImpl):
    attribs = (
        'scale',
        'color',
        'model',
        'input_isOn_bool',
        'isOn',
        'output',
        'secondsOn',
        )

class Button(Switch):
    name = 'button'

class Trigger(Switch):
    name = 'trigger'

class Crate(Nodepath):
    name = 'crate'
    delAttribs = (
        'hpr',
        )
    attribs = (
        'scale',
        'gridId',
        'pushable',
        )

class Door(Entity):
    name = 'door'
    attribs = (
        'parent',
        'pos',
        'hpr',
        'scale',
        'color',
        'model',
        'doorwayNum',
        'input_Lock0_bool',
        'input_Lock1_bool',
        'input_Lock2_bool',
        'input_Lock3_bool',
        'input_isOpen_bool',
        'isLock0Unlocked',
        'isLock1Unlocked',
        'isLock2Unlocked',
        'isLock3Unlocked',
        'isOpen',
        'output',
        'secondsOpen',
        )

class Grid(Nodepath):
    name = 'grid'
    delAttribs = (
        'hpr',
        )
    attribs = (
        'cellSize',
        'numCol',
        'numRow',
        )

class Lift(Nodepath):
    name = 'lift'
    attribs = (
        'duration',
        'startPos',
        'endPos',
        'modelPath',
        'floorName',
        'modelScale',
        )

class Platform(Nodepath):
    name = 'platform'
    delAttribs = (
        'pos',
        )
    attribs = (
        'startPos',
        'endPos',
        'speed',
        'waitDur',
        'phaseShift',
        )

class SinkingPlatform(Nodepath):
    name = 'sinkingPlatform'
    delAttribs = (
        'pos',
        )
    attribs = (
        'endPos',
        'phaseShift',
        'startPos',
        'verticalRange',
        'sinkRate',
        'riseRate',
        'speed',
        'waitDur',
        )

class Stomper(Nodepath):
    name = 'stomper'
    attribs = (
        'headScale',
        'motion',
        'period',
        'phaseShift',
        'range',
        'shaftScale',
        'soundLen',
        'soundOn',
        'style',
        'zOffset',
        )

class StomperPair(Nodepath):
    name = 'stomperPair'
    attribs = (
        'headScale',
        'motion',
        'period',
        'phaseShift',
        'range',
        'shaftScale',
        'soundLen',
        'soundOn',
        'stomperIds',
        'style',
        )
