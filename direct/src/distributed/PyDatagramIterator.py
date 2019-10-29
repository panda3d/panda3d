
# This is built as a subclass instead of an extension so we can define the
# class variable FuncDict and so we can import DCSubatomicType at the top
# of the file rather than every time we call the putArg function.

from panda3d.core import *
from panda3d.direct import *
# Import the type numbers


class PyDatagramIterator(DatagramIterator):

    # This is a little helper Dict to replace the huge <if> statement
    # for trying to match up datagram subatomic types with add funtions
    # If Python had an O(1) "case" statement we would use that instead
    FuncDict = {
        STInt8:  DatagramIterator.getInt8,
        STInt16: DatagramIterator.getInt16,
        STInt32: DatagramIterator.getInt32,
        STInt64: DatagramIterator.getInt64,
        STUint8:  DatagramIterator.getUint8,
        STUint16: DatagramIterator.getUint16,
        STUint32: DatagramIterator.getUint32,
        STUint64: DatagramIterator.getUint64,
        STFloat64: DatagramIterator.getFloat64,
        STString: DatagramIterator.getString,
        STBlob: DatagramIterator.getBlob,
        STBlob32: DatagramIterator.getBlob32,
        }

    getChannel = DatagramIterator.getUint64

    def getArg(self, subatomicType, divisor=1):
        # Import the type numbers
        if divisor == 1:
            # See if it is in the handy dict
            getFunc = self.FuncDict.get(subatomicType)
            if getFunc:
                retVal = getFunc(self)
            # No division necessary
            elif subatomicType == STInt8array:
                len = self.getUint16()
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt8())
            elif subatomicType == STInt16array:
                len = self.getUint16() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt16())
            elif subatomicType == STInt32array:
                len = self.getUint16() >> 2
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt32())
            elif subatomicType == STUint8array:
                len = self.getUint16()
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint8())
            elif subatomicType == STUint16array:
                len = self.getUint16() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint16())
            elif subatomicType == STUint32array:
                len = self.getUint16() >> 2
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint32())
            elif subatomicType == STUint32uint8array:
                len = self.getUint16() / 5
                retVal = []
                for i in range(len):
                    a = self.getUint32()
                    b = self.getUint8()
                    retVal.append((a, b))
            else:
                raise Exception("Error: No such type as: " + str(subatomicType))
        else:
            # See if it is in the handy dict
            getFunc = self.FuncDict.get(subatomicType)
            if getFunc:
                retVal = (getFunc(self)/float(divisor))
            elif subatomicType == STInt8array:
                len = self.getUint8() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt8()/float(divisor))
            elif subatomicType == STInt16array:
                len = self.getUint16() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt16()/float(divisor))
            elif subatomicType == STInt32array:
                len = self.getUint16() >> 2
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt32()/float(divisor))
            elif subatomicType == STUint8array:
                len = self.getUint8() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint8()/float(divisor))
            elif subatomicType == STUint16array:
                len = self.getUint16() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint16()/float(divisor))
            elif subatomicType == STUint32array:
                len = self.getUint16() >> 2
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint32()/float(divisor))
            elif subatomicType == STUint32uint8array:
                len = self.getUint16() / 5
                retVal = []
                for i in range(len):
                    a = self.getUint32()
                    b = self.getUint8()
                    retVal.append((a / float(divisor), b / float(divisor)))
            else:
                raise Exception("Error: No such type as: " + str(subatomicType))

        return retVal
