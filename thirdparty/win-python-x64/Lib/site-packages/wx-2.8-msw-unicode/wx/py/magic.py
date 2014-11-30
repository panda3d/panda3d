"""magic.py is a utility that allows a simple line from the interpreter
be translated from a more bash-like form to a python form.
For instance, 'plot a' is transformed to 'plot(a)'
Special exceptions are made for predefined ls,cd, and pwd functions"""

__author__ = "David N. Mashburn <david.n.mashburn@gmail.com>"
# created 07/01/2009

import keyword

from parse import testForContinuations

aliasDict = {}

#DNM
# TODO : Still Refining this... seems to be ok for now... still finding gotchas, though!
# TODO : Multi-line strings seem to be correctly broken into commands by PyCrust(PySlices)
# TODO : Is there a better version of ls, cd, pwd, etc that could be used?
def magicSingle(command):
    if command=='': # Pass if command is blank
        return command
    
    first_space=command.find(' ')
    
    if command[0]==' ': # Pass if command begins with a space
        pass
    elif command[0]=='?': # Do help if starts with ?
        command='help('+command[1:]+')'
    elif command[0]=='!': # Use os.system if starts with !
        command='sx("'+command[1:]+'")'
    elif command in ('ls','pwd'): # automatically use ls and pwd with no arguments
        command=command+'()'
    elif command[:3] in ('ls ','cd '): # when using the 'ls ' or 'cd ' constructs, fill in both parentheses and quotes
        command=command[:2]+'("'+command[3:]+'")'
    elif command[:6] == 'alias ':
        c = command[6:].lstrip().split(' ')
        if len(c)<2:
            #print 'Not enough arguments for alias!'
            command = ''
        else:
            n,v = c[0],' '.join(c[1:])
            aliasDict[n]=v
            command = ''
    elif command.split(' ')[0] in aliasDict.keys():
        c = command.split(' ')
        if len(c)<2:
            command = 'sx("'+aliasDict[c[0]]+'")'
        else:
            command = 'sx("'+aliasDict[c[0]]+' '+' '.join(c[1:])+'")'
    elif first_space!=-1:       # if there is at least one space, add parentheses at beginning and end
        cmds=command.split(' ')
        if len(cmds)>1:
            wd1=cmds[0]
            wd2=cmds[1]
            i=1
            while wd2=='':
                i+=1
                if len(cmds)==i:
                    break
                wd2=cmds[i]
            if wd2=='':
                return command
            if (wd1[0].isalpha() or wd1[0]=='_') and (wd2[0].isalnum() or (wd2[0] in """."'_""")) and not keyword.iskeyword(wd1) and not keyword.iskeyword(wd2):
                if wd1.replace('.','').replace('_','').isalnum():
                    command=wd1+'('+command[(first_space+1):]+')' # add parentheses where the first space was and at the end... hooray!
    return command

def magic(command):
    continuations = testForContinuations(command)
    
    if len(continuations)==2: # Error case...
        return command
    elif len(continuations)==4:
        stringContinuationList,indentationBlockList, \
        lineContinuationList,parentheticalContinuationList = continuations
    
    commandList=[]
    firstLine = True
    for i in command.split('\n'):
        if firstLine:
            commandList.append(magicSingle(i))
        elif stringContinuationList.pop(0)==False and \
              indentationBlockList.pop(0)==False and \
              lineContinuationList.pop(0)==False and \
              parentheticalContinuationList.pop(0)==False:
            commandList.append(magicSingle(i)) # unless this is in a larger expression, use magic
        else:
            commandList.append(i)
        
        firstLine=False
    
    return '\n'.join(commandList)
