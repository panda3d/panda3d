
def ifAbsentPut(dict, key, newValue):
    """
    If dict has key, return the value, otherwise insert the newValue and return it
    """
    if dict.has_key(key):
        return dict[key]
    else:
        dict[key] = newValue
        return newValue


def indent(stream, numIndents, str):
    """
    Write str to stream with numIndents in front it it
    """
    #indentString = '\t'
    # To match emacs, instead of a tab character we will use 4 spaces
    indentString = '    '
    stream.write(indentString * numIndents + str)


