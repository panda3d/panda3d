from PandaObject import *

def setCameraOffsets(camList,camGroup):
    # setup camera offsets based on chanconfig
    chanConfig = base.config.GetString('chan-config', 'single')
    if chanConfig == 'cave3':
        camList[0].setH(camGroup[0],-60)
        camList[2].setH(camGroup[0], 60)
    elif chanConfig == 'cave3+god':
        camList[0].setH(camGroup[0], -60)
        camList[2].setH(camGroup[0], 60)
    elif chanConfig == 'mono-cave':
        camList[4].setH(camGroup[0], 120)
        camList[5].setH(camGroup[0], 60)
    elif chanConfig == 'mono-modelcave-pipe0':
        camList[0].setH(camGroup[0], 120)
        camList[1].setH(camGroup[0], 60)
        camList[2].setH(camGroup[0], 0)
    elif chanConfig == 'mono-modelcave-pipe1':
        camList[0].setH(camGroup[0], -120)
        camList[1].setH(camGroup[0], -60)
        
    
    

