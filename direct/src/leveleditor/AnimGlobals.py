"""
This contains data structure and constants related with animation handling.
"""

# index for keyFramesInfo list structure
# data strucrure: {[nodeUID, propertyName] : [frameNum,
#                                             value,
#                                            [inSlopeX, inSlopeY],
#                                            [outSlopeX, outSlopeY]]}
UID = 0
PROP_NAME = 1

FRAME = 0
VALUE  = 1
INSLOPE = 2
OUTSLOPE = 3

# index for curveAnimation list structure
# data strucrure: {[nodeUID, curveUID] : [nodeUID,
#                                         curveUID,
#                                         time]}
NODE = 0
CURVE = 1

NODE = 0
CURVE  = 1
TIME = 2

# index for animation curve generation information list structure(self.X, self.Y, self.Z in GraphEditorUI)
# data structur: [key,
#                 i ,
#                [[keyFrameX, keyFrameY], keyFrame_select],
#                [[inTangentX, inTangentY], inTangent_select],
#                [[outTangentX, outTangentY], outTangent_select],
#                [inSlopeX, inSlopeY],
#                [outSlopeX, outSlopeY]]

KEY = 0
I = 1
KEYFRAME = 2
IN_TANGENT = 3
OUT_TANGENT = 4
IN_SLOPE = 5
OUT_SLOPE = 6

LOCAL_VALUE = 0
SELECT = 1

#index for coordinate
X = 0
Y = 1
Z = 2

