-- Syntax Highlighting test file for Haskell
-- Some comments about this file

-- Hello World in Haskell
putStrLn "Hello, Haskell"

-- Simple do structure
do putStrLn "What is 2 + 2?"
    x <- readLn
    if x == 4
      then putStrLn "You're right!"
      else putStrLn "You're wrong!"

-- Class def
class Num a  where
    (+)    :: a -> a -> a
    negate :: a -> a

-- Data Declaration
data Set a = NilSet 
           | ConsSet a (Set a)

-- Import statement
import somthing

-- Instance
instance Num Int  where
    x + y       =  addInt x y
    negate x    =  negateInt x

-- Module
module Tree ( Tree(Leaf,Branch), fringe ) where
 
data Tree a                = Leaf a | Branch (Tree a) (Tree a) 
 
fringe :: Tree a -> [a]
fringe (Leaf x)            = [x]
fringe (Branch left right) = fringe left ++ fringe right

