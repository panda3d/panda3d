from PandaObject import *

def setCameraOffsets(camList):
    # setup camera offsets based on chanconfig
    chanConfig = base.config.GetString('chan-config', 'single')
    if chanConfig == 'cave3':
        camList[0].setHpr(camList[0], -90, 0, 0)
        camList[2].setHpr(camList[2], 90, 0, 0)
    elif chanConfig == 'cave3+god':
        camList[0].setHpr(camList[0], -90, 0, 0)
        camList[2].setHpr(camList[2], 90, 0, 0)
    

