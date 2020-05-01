import pytest
from direct.actor.Actor import Actor

def test_actor_bare_init():
    Actor()
    
def test_actor_pprint():
    A=Actor()
    A.pprint()
