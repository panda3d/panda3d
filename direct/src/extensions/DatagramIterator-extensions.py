
    def getArg(self, subatomicType, divisor=1):
        # Import the type numbers
        import DCSubatomicType
        if divisor == 1:
            # No division necessary
            if subatomicType == DCSubatomicType.STInt8:
                retVal = self.getInt8()
            elif subatomicType == DCSubatomicType.STInt16:
                retVal = self.getInt16()
            elif subatomicType == DCSubatomicType.STInt32:
                retVal = self.getInt32()
            elif subatomicType == DCSubatomicType.STInt64:
                retVal = self.getInt64()
            elif subatomicType == DCSubatomicType.STUint8:
                retVal = self.getUint8()
            elif subatomicType == DCSubatomicType.STUint16:
                retVal = self.getUint16()
            elif subatomicType == DCSubatomicType.STUint32:
                retVal = self.getUint32()
            elif subatomicType == DCSubatomicType.STUint64:
                retVal = self.getUint64()
            elif subatomicType == DCSubatomicType.STFloat64:
                retVal = self.getFloat64()
            elif subatomicType == DCSubatomicType.STString:
                retVal = self.getString()
            elif subatomicType == DCSubatomicType.STBlob:
                retVal = self.getString()
            elif hasattr(DCSubatomicType, "STBlob32") and subatomicType == DCSubatomicType.STBlob32:
                retVal = self.getString32()
            elif subatomicType == DCSubatomicType.STInt8array:
                len = self.getUint16()
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt8())
            elif subatomicType == DCSubatomicType.STInt16array:
                len = self.getUint16() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt16())
            elif subatomicType == DCSubatomicType.STInt32array:
                len = self.getUint16() >> 2
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt32())
            elif subatomicType == DCSubatomicType.STUint8array:
                len = self.getUint16()
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint8())
            elif subatomicType == DCSubatomicType.STUint16array:
                len = self.getUint16() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint16())
            elif subatomicType == DCSubatomicType.STUint32array:
                len = self.getUint16() >> 2
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint32())
            elif subatomicType == DCSubatomicType.STUint32uint8array:
                len = self.getUint16() / 5
                retVal = []
                for i in range(len):
                    a = self.getUint32()
                    b = self.getUint8()
                    retVal.append((a, b))
            else:
                raise Exception("Error: No such type as: " + str(subAtomicType))
        else:
            # This needs to be divided
            if subatomicType == DCSubatomicType.STInt8:
                retVal = (self.getInt8()/float(divisor))
            elif subatomicType == DCSubatomicType.STInt16:
                retVal = (self.getInt16()/float(divisor))
            elif subatomicType == DCSubatomicType.STInt32:
                retVal = (self.getInt32()/float(divisor))
            elif subatomicType == DCSubatomicType.STInt64:
                retVal = (self.getInt64()/float(divisor))
            elif subatomicType == DCSubatomicType.STUint8:
                retVal = (self.getUint8()/float(divisor))
            elif subatomicType == DCSubatomicType.STUint16:
                retVal = (self.getUint16()/float(divisor))
            elif subatomicType == DCSubatomicType.STUint32:
                retVal = (self.getUint32()/float(divisor))
            elif subatomicType == DCSubatomicType.STUint64:
                retVal = (self.getUint64()/float(divisor))
            elif subatomicType == DCSubatomicType.STFloat64:
                retVal = self.getFloat64()
            elif subatomicType == DCSubatomicType.STString:
                retVal = self.getString()
            elif subatomicType == DCSubatomicType.STBlob:
                retVal = self.getString()
            elif subatomicType == DCSubatomicType.STInt8array:
                len = self.getUint8() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt8()/float(divisor))
            elif subatomicType == DCSubatomicType.STInt16array:
                len = self.getUint16() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt16()/float(divisor))
            elif subatomicType == DCSubatomicType.STInt32array:
                len = self.getUint16() >> 2
                retVal = []
                for i in range(len):
                    retVal.append(self.getInt32()/float(divisor))
            elif subatomicType == DCSubatomicType.STUint8array:
                len = self.getUint8() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint8()/float(divisor))
            elif subatomicType == DCSubatomicType.STUint16array:
                len = self.getUint16() >> 1
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint16()/float(divisor))
            elif subatomicType == DCSubatomicType.STUint32array:
                len = self.getUint16() >> 2
                retVal = []
                for i in range(len):
                    retVal.append(self.getUint32()/float(divisor))
            elif subatomicType == DCSubatomicType.STUint32uint8array:
                len = self.getUint16() / 5
                retVal = []
                for i in range(len):
                    a = self.getUint32()
                    b = self.getUint8()
                    retVal.append((a / float(divisor), b / float(divisor)))
            else:
                raise Exception("Error: No such type as: " + str(subatomicType))



        return retVal


