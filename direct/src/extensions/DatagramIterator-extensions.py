
    def getArg(self, subatomicType, divisor=1):
        # Import the type numbers
        from DCSubatomicType import *
        if divisor == 1:
            # No division necessary
            if subatomicType == STInt8:
                retVal = self.getInt8()
            elif subatomicType == STInt16:
                retVal = self.getInt16()
            elif subatomicType == STInt32:
                retVal = self.getInt32()
            elif subatomicType == STInt64:
                retVal = self.getInt64()
            elif subatomicType == STUint8:
                retVal = self.getUint8()
            elif subatomicType == STUint16:
                retVal = self.getUint16()
            elif subatomicType == STUint32:
                retVal = self.getUint32()
            elif subatomicType == STUint64:
                retVal = self.getUint64()
            elif subatomicType == STFloat64:
                retVal = self.getFloat64()
            elif subatomicType == STString:
                retVal = self.getString()
            else:
                raise Exception("Error: No such type as: " + str(subAtomicType))
        else:
            # This needs to be divided
            if subatomicType == STInt8:
                retVal = (self.getInt8()/float(divisor))
            elif subatomicType == STInt16:
                retVal = (self.getInt16()/float(divisor))
            elif subatomicType == STInt32:
                retVal = (self.getInt32()/float(divisor))
            elif subatomicType == STInt64:
                retVal = (self.getInt64()/float(divisor))
            elif subatomicType == STUint8:
                retVal = (self.getUint8()/float(divisor))
            elif subatomicType == STUint16:
                retVal = (self.getUint16()/float(divisor))
            elif subatomicType == STUint32:
                retVal = (self.getUint32()/float(divisor))
            elif subatomicType == STUint64:
                retVal = (self.getUint64()/float(divisor))
            elif subatomicType == STFloat64:
                retVal = self.getFloat64()
            elif subatomicType == STString:
                retVal = self.getString()
            else:
                raise Exception("Error: No such type as: " + str(subAtomicType))


            
        return retVal

                
