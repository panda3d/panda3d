
    def putArg(self, arg, subatomicType):
        # Import the type numbers
        from DCSubatomicType import *
        if subatomicType == STInt8:
            self.addInt8(arg)
        elif subatomicType == STInt16:
            self.addInt16(arg)
        elif subatomicType == STInt32:
            self.addInt32(arg)
        elif subatomicType == STInt64:
            self.addInt64(arg)
        elif subatomicType == STUint8:
            self.addUint8(arg)
        elif subatomicType == STUint16:
            self.addUint16(arg)
        elif subatomicType == STUint32:
            self.addUint32(arg)
        elif subatomicType == STUint64:
            self.addUint64(arg)
        elif subatomicType == STFloat64:
            self.addFloat64(arg)
        elif subatomicType == STString:
            self.addString(arg)
        else:
            raise Exception("Error: No such type as: " + subatomicType)
        return None
