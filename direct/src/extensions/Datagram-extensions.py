
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
        else:
            raise Exception("Error: No such type as: " + subatomicType)
        return None
