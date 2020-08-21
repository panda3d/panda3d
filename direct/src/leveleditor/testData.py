from panda3d.core import *

if hasattr(base, 'le'):
    objectMgr = base.le.objectMgr
    ui = base.le.ui
    ui.sceneGraphUI.reset()

else:
    objectMgr = base.objectMgr
# temporary place holder for nodepath
objects = {}

objects['1252538687.73gjeon'] = objectMgr.addNewObject('Smiley', '1252538687.73gjeon', 'models/smiley.egg', None)
if objects['1252538687.73gjeon']:
    objects['1252538687.73gjeon'].setPos(Point3(8.66381, 0, 7.13246))
    objects['1252538687.73gjeon'].setHpr(VBase3(0, 0, 0))
    objects['1252538687.73gjeon'].setScale(VBase3(1, 1, 1))
    objectMgr.updateObjectColor(1.000000, 1.000000, 1.000000, 1.000000, objects['1252538687.73gjeon'])
    objectMgr.updateObjectProperties(objects['1252538687.73gjeon'], {'123': 1, 'Abc': 'a', 'Number': 1, 'Happy': True})

objects['1252538690.2gjeon'] = objectMgr.addNewObject('H Double Smiley', '1252538690.2gjeon', None, objects['1252538687.73gjeon'])
if objects['1252538690.2gjeon']:
    objects['1252538690.2gjeon'].setPos(Point3(0, 0, 2.12046))
    objects['1252538690.2gjeon'].setHpr(VBase3(0, 0, 0))
    objects['1252538690.2gjeon'].setScale(VBase3(1, 1, 1))
    objectMgr.updateObjectColor(1.000000, 1.000000, 1.000000, 1.000000, objects['1252538690.2gjeon'])
    objectMgr.updateObjectProperties(objects['1252538690.2gjeon'], {'Distance': 2.0, 'Abc': 'a'})

objects['1252539696.69gjeon'] = objectMgr.addNewObject('H Double Smiley', '1252539696.69gjeon', None, objects['1252538687.73gjeon'])
if objects['1252539696.69gjeon']:
    objects['1252539696.69gjeon'].setPos(Point3(-9.53674e-006, 0, 0))
    objects['1252539696.69gjeon'].setHpr(VBase3(0, 0, 0))
    objects['1252539696.69gjeon'].setScale(VBase3(1, 1, 1))
    objectMgr.updateObjectColor(1.000000, 1.000000, 1.000000, 1.000000, objects['1252539696.69gjeon'])
    objectMgr.updateObjectProperties(objects['1252539696.69gjeon'], {'Distance': 2.0, 'Abc': 'a'})

objects['1252539897.22gjeon'] = objectMgr.addNewObject('H Double Smiley', '1252539897.22gjeon', None, objects['1252538687.73gjeon'])
if objects['1252539897.22gjeon']:
    objects['1252539897.22gjeon'].setPos(Point3(0.06987, 0, -2.12046))
    objects['1252539897.22gjeon'].setHpr(VBase3(0, 0, 0))
    objects['1252539897.22gjeon'].setScale(VBase3(1, 1, 1))
    objectMgr.updateObjectColor(1.000000, 1.000000, 1.000000, 1.000000, objects['1252539897.22gjeon'])
    objectMgr.updateObjectProperties(objects['1252539897.22gjeon'], {'Distance': 2.0, 'Abc': 'a'})

objects['1252538689.13gjeon'] = objectMgr.addNewObject('V Double Smiley', '1252538689.13gjeon', None, objects['1252538687.73gjeon'])
if objects['1252538689.13gjeon']:
    objects['1252538689.13gjeon'].setPos(Point3(6.07152, 0, -0.863791))
    objects['1252538689.13gjeon'].setHpr(VBase3(0, 0, 0))
    objects['1252538689.13gjeon'].setScale(VBase3(1, 1, 1))
    objectMgr.updateObjectColor(1.000000, 1.000000, 1.000000, 1.000000, objects['1252538689.13gjeon'])
    objectMgr.updateObjectProperties(objects['1252538689.13gjeon'], {'Distance': 1.0, 'Abc': 'a'})

objects['1252539974.19gjeon'] = objectMgr.addNewObject('Smiley', '1252539974.19gjeon', 'models/frowney.egg', objects['1252538689.13gjeon'])
if objects['1252539974.19gjeon']:
    objects['1252539974.19gjeon'].setPos(Point3(0.06987, 0, 3.09209))
    objects['1252539974.19gjeon'].setHpr(VBase3(0, 0, 0))
    objects['1252539974.19gjeon'].setScale(VBase3(1, 1, 1))
    objectMgr.updateObjectColor(1.000000, 1.000000, 1.000000, 1.000000, objects['1252539974.19gjeon'])
    objectMgr.updateObjectProperties(objects['1252539974.19gjeon'], {'123': 1, 'Abc': 'a', 'Number': 1, 'Happy': True})

objects['1252623762.9gjeon'] = objectMgr.addNewObject('Panda', '1252623762.9gjeon', None, None)
if objects['1252623762.9gjeon']:
    objects['1252623762.9gjeon'].setPos(Point3(0, 0, 0))
    objects['1252623762.9gjeon'].setHpr(VBase3(0, 0, 0))
    objects['1252623762.9gjeon'].setScale(VBase3(0.005, 0.005, 0.005))
    objectMgr.updateObjectColor(1.000000, 0.000000, 0.000000, 0.517647, objects['1252623762.9gjeon'])
    objectMgr.updateObjectProperties(objects['1252623762.9gjeon'], {})

if hasattr(base, 'le'):
    ui.layerEditorUI.reset()
    ui.layerEditorUI.addLayerEntry('Layer1', 1 )
    ui.layerEditorUI.addLayerData(1, '1252538687.73gjeon')
