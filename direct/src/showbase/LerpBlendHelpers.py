"""LerpBlendHelpers module: contains LerpBlendHelpers class"""

from PandaModules import *

"""global lerp blend types for lerp function"""
    
easeIn = EaseInBlendType()

easeOut = EaseOutBlendType()

easeInOut = EaseInOutBlendType()

noBlend = NoBlendType()

            
def getBlend(blendType):
    """getBlend(string)
    Return the C++ blend class corresponding to blendType string
    """
    # Note, this is temporary until blend functions get exposed
    if (blendType == "easeIn"):
        return easeIn
    elif (blendType == "easeOut"):
        return easeOut
    elif (blendType == "easeInOut"):
        return easeInOut
    elif (blendType == "noBlend"):
        return noBlend
    else:
        raise Exception(
            'Error: LerpInterval.__getBlend: Unknown blend type')
