from panda3d.core import Filename, NodePath
from direct.showbase.Loader import Loader
import pytest


@pytest.fixture
def loader():
    return Loader(base=None)


@pytest.fixture
def temp_model():
    from panda3d.core import ModelPool, ModelRoot

    root = ModelRoot('model')
    root.fullpath = '/test-model.bam'

    ModelPool.add_model(root.fullpath, root)
    yield root.fullpath
    ModelPool.release_model(root.fullpath)


def test_load_model_filename(loader, temp_model):
    model = loader.load_model(Filename(temp_model))
    assert model
    assert isinstance(model, NodePath)
    assert model.name == 'model'


def test_load_model_str(loader, temp_model):
    model = loader.load_model(str(temp_model))
    assert model
    assert isinstance(model, NodePath)
    assert model.name == 'model'


def test_load_model_list(loader, temp_model):
    models = loader.load_model([temp_model, temp_model])
    assert models
    assert isinstance(models, list)
    assert len(models) == 2
    assert isinstance(models[0], NodePath)
    assert isinstance(models[1], NodePath)


def test_load_model_tuple(loader, temp_model):
    models = loader.load_model((temp_model, temp_model))
    assert models
    assert isinstance(models, list)
    assert len(models) == 2
    assert isinstance(models[0], NodePath)
    assert isinstance(models[1], NodePath)


def test_load_model_set(loader, temp_model):
    models = loader.load_model({temp_model})
    assert models
    assert isinstance(models, list)
    assert len(models) == 1
    assert isinstance(models[0], NodePath)


def test_load_model_missing(loader):
    with pytest.raises(IOError):
        loader.load_model('/nonexistent.bam')


def test_load_model_okmissing(loader):
    model = loader.load_model('/nonexistent.bam', okMissing=True)
    assert model is None
