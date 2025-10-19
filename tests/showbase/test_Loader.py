from panda3d.core import Filename, NodePath, LoaderFileTypeRegistry
from direct.showbase.Loader import Loader
import pytest
import sys


@pytest.fixture
def loader():
    loader = Loader(base=None)
    yield loader
    loader.destroy()


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


def test_loader_entry_points(tmp_path):
    # A dummy loader for .fnrgl files.
    (tmp_path / "fnargle.py").write_text("""
from panda3d.core import ModelRoot
import sys

sys._fnargle_loaded = True

class FnargleLoader:
    name = "Fnargle"
    extensions = ['fnrgl']
    supports_compressed = False

    @staticmethod
    def load_file(path, options, record=None):
        return ModelRoot("fnargle")
""")
    (tmp_path / "fnargle.dist-info").mkdir()
    (tmp_path / "fnargle.dist-info" / "METADATA").write_text("""
Metadata-Version: 2.1
Name: fnargle
Version: 1.0.0
""")
    (tmp_path / "fnargle.dist-info" / "entry_points.txt").write_text("""
[panda3d.loaders]
fnrgl = fnargle:FnargleLoader
""")

    model_path = tmp_path / "test.fnrgl"
    model_path.write_text("")

    if sys.version_info >= (3, 11):
        import sysconfig
        stdlib = sysconfig.get_path("stdlib")
        platstdlib = sysconfig.get_path("platstdlib")
    else:
        from distutils import sysconfig
        stdlib = sysconfig.get_python_lib(False, True)
        platstdlib = sysconfig.get_python_lib(True, True)

    registry = LoaderFileTypeRegistry.get_global_ptr()
    prev_loaded = Loader._loadedPythonFileTypes
    prev_path = sys.path
    file_type = None
    try:
        # We do this so we don't re-register thirdparty loaders
        sys.path = [str(tmp_path), platstdlib, stdlib]

        Loader._loadedPythonFileTypes = False
        loader = Loader()

        if not Loader._loadedPythonFileTypes:
            Loader._loadPythonFileTypes()
        assert Loader._loadedPythonFileTypes

        # Should be registered, not yet loaded
        file_type = registry.get_type_from_extension('fnrgl')
        assert file_type is not None
        assert not hasattr(sys, '_fnargle_loaded')

        assert file_type.supports_load()
        assert not file_type.supports_save()
        assert not file_type.supports_compressed()
        assert file_type.get_extension() == 'fnrgl'

        # The above should have caused it to load
        assert sys._fnargle_loaded
        assert 'fnargle' in sys.modules

        # Now try loading a fnargle file
        model_fn = Filename(model_path)
        model_fn.make_true_case()
        model = loader.load_model(model_fn, noCache=True)
        assert model is not None
        assert model.name == "fnargle"

    finally:
        # Set everything back to what it was
        Loader._loadedPythonFileTypes = prev_loaded
        sys.path = prev_path

        if hasattr(sys, '_fnargle_loaded'):
            del sys._fnargle_loaded

        if 'fnargle' in sys.modules:
            del sys.modules['fnargle']

        if file_type is not None:
            registry.unregister_type(file_type)
