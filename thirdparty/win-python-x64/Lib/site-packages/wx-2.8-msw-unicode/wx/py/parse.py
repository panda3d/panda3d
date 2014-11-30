"""parse.py is a utility that allows simple checking for line continuations
to give the shell information about where text is commented and where it is
not and also how to appropriately break up a sequence of commands into
separate multi-line commands...
"""

__author__ = "David N. Mashburn <david.n.mashburn@gmail.com>"
# created 12/20/2009

import re

# change this to testForContinuations

def testForContinuations(codeBlock,ignoreErrors=False):
    """ Test 4 different types of continuations:""" + \
    """ String Continuations (ie with ''')""" + \
    """ Indentation Block Continuations (ie "if 1:" )""" + \
    """ Line Continuations (ie with \\ character )""" + \
    """ Parenthetical continuations (, [, or {"""
    
    stringMark = None
    paraList = []
    indentNumber=[0]
    
    stringMarks = ['"""',"'''",'"',"'"]
    openMarks = ['(','[','{']
    closeMarks = [')',']','}']
    paraMarkDict = { '(':')', '[':']', '{':'}' }
    
    stringContinuationList=[]
    lineContinuationList=[] # For \ continuations ... False because cannot start as line Continuation...
    indentationBlockList=[]
    parentheticalContinuationList=[]
    newIndent=False
    lspContinuation=False
    for i,l in enumerate(codeBlock.split('\n')):
        currentIndentation = len(l)-len(l.lstrip())
        
        if i>0:
            lspContinuation = lineContinuationList[-1] or \
                              stringContinuationList[-1] or \
                              parentheticalContinuationList[-1]
        # first, check for non-executing lines (whitespace and/or comments only)
        if l.lstrip()=='':
            emptyLine=True
        elif l.strip()[0]=='#':
            emptyLine=True
        else: # otherwise, check the indentation...
            emptyLine=False
            if newIndent and currentIndentation>indentNumber[-1]:
                newIndent=False
                indentNumber.append(currentIndentation)
            elif lspContinuation:
                pass
            elif not newIndent and currentIndentation in indentNumber:
                while currentIndentation<indentNumber[-1]:
                    indentNumber.pop() # This is the end of an indentation block
            elif not ignoreErrors:
                #print 'Invalid Indentation!!'
                return ['Invalid Indentation Error',i]
        
        firstWord = re.match(' *\w*',l).group().lstrip()
        if firstWord in ['if','else','elif','for','while',
                         'def','class','try','except','finally']:
            hasContinuationWord = True
        else:
            hasContinuationWord = False
        
        
        commented=False
        nonCommentLength=len(l)
                
        result = re.finditer('"""'+'|'+"'''" + r'''|"|'|\"|\'|\(|\)|\[|\]|\{|\}|#''',l)
        for r in result:
            j = r.group()
            
            if stringMark == None:
                if j=='#': # If it is a legitimate comment, ignore everything after
                    commented=True
                    # get length up to last non-comment character
                    nonCommentLength = r.start()
                    break
                elif j in stringMarks:
                    stringMark=j
                else:
                    if paraList != [] and j in closeMarks:
                        if paraMarkDict[paraList[-1]]==j:
                            paraList.pop()
                        elif not ignoreErrors:
                            #print 'Invalid Syntax!!'
                            return ['Invalid Syntax Error',i]
                    if j in openMarks:
                        paraList.append(j)
            elif stringMark==j:
                stringMark=None
        
        stringContinuationList.append(stringMark!=None)
        
        indentationBlockList.append(False)
        nonCommentString = l[:nonCommentLength].rstrip()
        if nonCommentString!='' and stringContinuationList[-1]==False:
            if nonCommentString[-1]==':':
                indentationBlockList[-1]=True
                newIndent=True
        
        lineContinuationList.append(False)
        if len(l)>0 and not commented:
            if l[-1]=='\\':
                lineContinuationList[-1]=True
        
        parentheticalContinuationList.append( paraList != [] )
    
    # Now stringContinuationList (et al) is line by line key for magic
    # telling it whether or not each next line is part of a string continuation
    
    if (stringContinuationList[-1] or indentationBlockList[-1] or \
       lineContinuationList[-1] or parentheticalContinuationList[-1]) \
       and not ignoreErrors:
        #print 'Incomplete Syntax!!'
        return ['Incomplete Syntax Error',i]
    
    if newIndent and not ignoreErrors:
        #print 'Incomplete Indentation!'
        return ['Incomplete Indentation Error',i]
    
    # Note that if one of these errors above gets thrown, the best solution is to pass the resulting block
    # to the interpreter as exec instead of interp
    return stringContinuationList,indentationBlockList,lineContinuationList,parentheticalContinuationList
