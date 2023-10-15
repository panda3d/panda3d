from panda3d.core import Filename, NodePath, LoaderFileTypeRegistry
from direct.showbase.Loader import Loader
import pytest
import sys


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


def test_loader_entry_points(tmpdir):
    # A dummy loader for .fnrgl files.
    tmpdir.join("fnargle.py").write("""
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
    tmpdir.join("fnargle.dist-info").mkdir()
    tmpdir.join("fnargle.dist-info", "METADATA").write("""
Metadata-Version: 2.0
Name: fnargle
Version: 1.0.0
""")
    tmpdir.join("fnargle.dist-info", "entry_points.txt").write("""
[panda3d.loaders]
fnrgl = fnargle:FnargleLoader
""")

    model_path = tmpdir.join("test.fnrgl")
    model_path.write("")

    if sys.version_info >= (3, 11):
        import sysconfig
        stdlib = sysconfig.get_path("stdlib")
        platstdlib = sysconfig.get_path("platstdlib")
    else:
        from distutils import sysconfig
        stdlib = sysconfig.get_python_lib(False, True)
        platstdlib = sysconfig.get_python_lib(True, True)

    if sys.version_info < (3, 8):
        # Older Python versions don't have importlib.metadata, so we rely on
        # pkg_resources - but this caches the results once.  Fortunately, it
        # provides this function for reinitializing the cached entry points.
        # See pypa/setuptools#373
        pkg_resources = pytest.importorskip("pkg_resources")
        if not hasattr(pkg_resources, "_initialize_master_working_set"):
            pytest.skip("pkg_resources too old")

    registry = LoaderFileTypeRegistry.get_global_ptr()
    prev_loaded = Loader._loadedPythonFileTypes
    prev_path = sys.path
    file_type = None
    try:
        # We do this so we don't re-register thirdparty loaders
        sys.path = [str(tmpdir), platstdlib, stdlib]
        if sys.version_info < (3, 8):
            pkg_resources._initialize_master_working_set()

        Loader._loadedPythonFileTypes = False

        # base parameter is only used for audio
        loader = Loader(None)
        assert Loader._loadedPythonFileTypes

        if sys.version_info < (3, 8):
            # pkg_resources relies on things in site-packages (_markerlib)
            sys.path += [sysconfig.get_python_lib(True, False), sysconfig.get_python_lib(False, False)]

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
        model_fn = Filename.from_os_specific(str(model_path))
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

        if sys.version_info < (3, 8):
            pkg_resources._initialize_master_working_set()
