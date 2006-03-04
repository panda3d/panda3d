import FFIConstants
from string import *


pythonKeywords = ['and','del','for','is','raise','assert','elif','from','lambda','return','break','else','global','not','try','class','except','if','or','while','continue','exec','import','pass','def','finally','in','print']


methodRenameDictionary = {
    'operator==':  'eq',
    'operator!=':  'ne',
    'operator<<':  '__lshift__',
    'operator>>':  '__rshift__',
    'operator<':   'lessThan',
    'operator>':   'greaterThan',
    'operator<=':  'lessThanOrEqual',
    'operator>=':  'greaterThanOrEqual',
    'operator=':   'assign',
    'operator()':  '__call__',
    'operator[]':  '__getitem__',
    'operator++':  'increment',
    'operator--':  'decrement',
    'operator^':   '__xor__',
    'operator%':   '__mod__',
    'operator!':   'logicalNot',
    'operator~':   'bitwiseNot',
    'operator&':   '__and__',
    'operator&&':  'logicalAnd',
    'operator|':   '__or__',
    'operator||':  'logicalOr',
    'operator+':   '__add__',
    'operator-':   '__sub__',
    'operator*':   '__mul__',
    'operator/':   '__div__',
    'operator+=':  '__iadd__',
    'operator-=':  '__isub__',
    'operator*=':  '__imul__',
    'operator/=':  '__idiv__',
    'operator,':   'concatenate',
    'operator|=':  '__ior__',
    'operator&=':  '__iand__',
    'operator^=':  '__ixor__',
    'operator~=':  'bitwiseNotEqual',
    'operator->':  'dereference',
    'operator<<=': '__ilshift__',
    'operator>>=': '__irshift__',
    'print':       'Cprint',
    'CInterval.setT': '_priv__cSetT',
    }
    
classRenameDictionary = {
    'Loader':                    'PandaLoader',
    'String':                    'CString',
    'LMatrix4f':                 'Mat4',
    'LMatrix3f':                 'Mat3',
    'LVecBase4f':                'VBase4',
    'LVector4f':                 'Vec4',
    'LPoint4f':                  'Point4',
    'LVecBase3f':                'VBase3',
    'LVector3f':                 'Vec3',
    'LPoint3f':                  'Point3',
    'LVecBase2f':                'VBase2',
    'LVector2f':                 'Vec2',
    'LPoint2f':                  'Point2',
    'LQuaternionf':              'Quat',
    'LMatrix4d':                 'Mat4D',
    'LMatrix3d':                 'Mat3D',
    'LVecBase4d':                'VBase4D',
    'LVector4d':                 'Vec4D',
    'LPoint4d':                  'Point4D',
    'LVecBase3d':                'VBase3D',
    'LVector3d':                 'Vec3D',
    'LPoint3d':                  'Point3D',
    'LVecBase2d':                'VBase2D',
    'LVector2d':                 'Vec2D',
    'LPoint2d':                  'Point2D',
    'LQuaterniond':              'QuatD',
    'Plane':                     'PlaneBase',
    'Planef':                    'Plane',
    'Planed':                    'PlaneD',
    'Frustum':                   'FrustumBase',
    'Frustumf':                  'Frustum',
    'Frustumd':                  'FrustumD'
    }


def checkKeyword(cppName):
    if cppName in pythonKeywords:
        cppName = '_' + cppName
    return cppName

# TODO: Make faster - this thing is horribly slow    
def classNameFromCppName(cppName):
    # initialize to empty string
    className = ''
    # These are the characters we want to strip out of the name
    badChars = '!@#$%^&*()<>,.-=+~{}? '
    nextCap = 0
    firstChar = 1
    for char in cppName:
        if (char in badChars):
            continue
        elif (char == '_'):
            nextCap = 1
            continue
        elif (nextCap or firstChar):
            className = className + capitalize(char)
            nextCap = 0
            firstChar = 0
        else:
            className = className + char
    if classRenameDictionary.has_key(className):
        className = classRenameDictionary[className]

    if (className == ''):
        FFIConstants.notify.warning('Renaming class: ' + cppName + ' to empty string')
    # FFIConstants.notify.debug('Renaming class: ' + cppName + ' to: ' + className)
    # Note we do not have to check for keywords because class name are capitalized
    return className
    
def nonClassNameFromCppName(cppName):
    className = classNameFromCppName(cppName)
    # Make the first character lowercase
    newName = lower(className[0])+className[1:]
    # Mangle names that happen to be python keywords so they are not anymore
    newName = checkKeyword(newName)
    return newName

def methodNameFromCppName(cppName, className = None):
    methodName = ''
    badChars = ' '
    nextCap = 0
    for char in cppName:
        if (char in badChars):
            continue
        elif (char == '_'):
            nextCap = 1
            continue
        elif nextCap:
            methodName = methodName + capitalize(char)
            nextCap = 0
        else:
            methodName = methodName + char

    if className != None:
        methodName = methodRenameDictionary.get(className + '.' + methodName, methodName)
    methodName = methodRenameDictionary.get(methodName, methodName)
    
    # Mangle names that happen to be python keywords so they are not anymore
    methodName = checkKeyword(methodName)
    return methodName

