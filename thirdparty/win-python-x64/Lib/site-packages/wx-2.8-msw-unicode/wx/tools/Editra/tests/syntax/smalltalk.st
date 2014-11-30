" Syntax Highlighting Test File for Smalltalk "
" Comments are like this "
" Hello World in smalltalk "

" Heres helloworld doesnt get much easier that this "
'Hello World!'

" Heres a version in a window "
| window |
window := Window new.
window label: 'Hello World'.
window open.
 
" Character Definition"
| mychar |
mychar := $A

" Symbol Definition "
| mySym |
mySym := #foo

" Symbol Array "
mySym := #(1 2 3 4)

" Class Def "
Object subclass: #MessagePublisher
    instanceVariableNames: ''
    classVariableNames: ''
    poolDictionaries: ''
    category: 'Smalltalk Examples'

| rectangles aPoint|
rectangles := OrderedCollection 
  with: (Rectangle left: 0 right: 10 top: 100 bottom: 200)
  with: (Rectangle left: 10 right: 10 top: 110 bottom: 210).
aPoint := Point x: 20 y: 20.
collisions := rectangles select: [:aRect | aRect containsPoint: aPoint].

result := a > b
    ifTrue:[ 'greater' ]
    ifFalse:[ 'less' ]
