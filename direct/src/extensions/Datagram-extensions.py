
    def putArg(self, arg, subatomicType, divisor=1):
        # Import the type numbers
        from DCSubatomicType import *
        if subatomicType == STInt8:
            self.addInt8(int(arg*divisor))
        elif subatomicType == STInt16:
            self.addInt16(int(arg*divisor))
        elif subatomicType == STInt32:
            self.addInt32(int(arg*divisor))
        elif subatomicType == STInt64:
            self.addInt64(int(arg*divisor))
        elif subatomicType == STUint8:
            self.addUint8(int(arg*divisor))
        elif subatomicType == STUint16:
            self.addUint16(int(arg*divisor))
        elif subatomicType == STUint32:
            self.addUint32(int(arg*divisor))
        elif subatomicType == STUint64:
            self.addUint64(int(arg*divisor))
        elif subatomicType == STFloat64:
            self.addFloat64(arg)
        elif subatomicType == STString:
            self.addString(arg)
        elif subatomicType == STBlob:
            self.addString(arg)
        elif subatomicType == STInt8array:
            self.addUint8(len(arg) << 1)
            for i in arg:
                self.addInt8(int(i*divisor))
        elif subatomicType == STInt16array:
            self.addUint16(len(arg) << 1)
            for i in arg:
                self.addInt16(int(i*divisor))
        elif subatomicType == STInt32array:
            self.addUint16(len(arg) << 2)
            for i in arg:
                self.addInt32(int(i*divisor))
        elif subatomicType == STUint8array:
            self.addUint8(len(arg) << 1)
            for i in arg:
                self.addUint8(int(i*divisor))
        elif subatomicType == STUint16array:
            self.addUint16(len(arg) << 1)
            for i in arg:
                self.addUint16(int(i*divisor))
        elif subatomicType == STUint32array:
            self.addUint16(len(arg) << 2)
            for i in arg:
                self.addUint32(int(i*divisor))
        else:
            raise Exception("Error: No such type as: " + subatomicType)
        return None
