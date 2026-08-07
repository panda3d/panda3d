import pytest
from direct.actor.Actor import Actor

def test_actor_bare_init():
    Actor()
    
def test_actor_pprint():
    A=Actor()
    A.pprint()

def test_actor_with_tutorial_model():
    
    #tutorial init
    #path to model and animations as positional
    test_actor = Actor("../"+"models/panda-model",
                        {"walk": "../"+"models/panda-walk4"})

def test_actor_scale():
    test_actor = Actor("models/panda-model",
                                {"walk": "models/panda-walk4"})
    input_data = (0.005, 0.005, 0.005)
    test_actor.setScale(*input_data)
    output_data = test_actor.getScale()
    assert input_data == output_data
