from panda3d.core import LoaderFileTypeRegistry, ModelRoot, Loader, LoaderOptions, Filename
import pytest
import tempfile
import os
from contextlib import contextmanager


@pytest.fixture
def test_filename():
    """Fixture returning a filename to an existent .test file."""
    fp = tempfile.NamedTemporaryFile(suffix='.test', delete=False)
    fp.write(b"test")
    fp.close()
    filename = Filename.from_os_specific(fp.name)
    filename.make_true_case()
    yield filename
    os.unlink(fp.name)


@pytest.fixture
def test_pz_filename():
    """Fixture returning a filename to an existent .test.pz file."""
    fp = tempfile.NamedTemporaryFile(suffix='.test.pz', delete=False)
    fp.write(b"test")
    fp.close()
    filename = Filename.from_os_specific(fp.name)
    filename.make_true_case()
    yield filename
    os.unlink(fp.name)


@contextmanager
def registered_type(type):
    """Convenience method allowing use of register_type in a with block."""
    registry = LoaderFileTypeRegistry.get_global_ptr()
    registry.register_type(type)
    yield
    registry.unregister_type(type)


class DummyLoader:
    """The simplest possible successful LoaderFileType."""

    extensions = ["test"]

    @staticmethod
    def load_file(path, options, record=None):
        return ModelRoot("loaded")


def test_loader_invalid():
    """Tests that registering a malformed loader fails."""

    class MissingExtensionsLoader:
        pass

    class InvalidTypeExtensionsLoader:
        extensions = "abc"

    class EmptyExtensionsLoader:
        extensions = []

    class InvalidExtensionsLoader:
        extensions = [123, None]

    registry = LoaderFileTypeRegistry.get_global_ptr()

    with pytest.raises(Exception):
        registry.register_type("invalid")

    with pytest.raises(Exception):
        registry.register_type(MissingExtensionsLoader)

    with pytest.raises(TypeError):
        registry.register_type(InvalidTypeExtensionsLoader)

    with pytest.raises(ValueError):
        registry.register_type(EmptyExtensionsLoader)

    with pytest.raises(TypeError):
        registry.register_type(InvalidExtensionsLoader)


def test_loader_success(test_filename):
    """Tests that a normal dummy loader successfully loads."""

    with registered_type(DummyLoader):
        model = Loader.get_global_ptr().load_sync(test_filename, LoaderOptions(LoaderOptions.LF_no_cache))
        assert model is not None
        assert model.name == "loaded"


def test_loader_extensions(test_filename):
    """Tests multi-extension loaders."""

    class MultiExtensionLoader:
        extensions = ["test1", "teSt2"]

        @staticmethod
        def load_file(path, options, record=None):
            return ModelRoot("loaded")

    fp1 = tempfile.NamedTemporaryFile(suffix='.test1', delete=False)
    fp1.write(b"test1")
    fp1.close()
    fn1 = Filename.from_os_specific(fp1.name)
    fn1.make_true_case()

    fp2 = tempfile.NamedTemporaryFile(suffix='.TEST2', delete=False)
    fp2.write(b"test2")
    fp2.close()
    fn2 = Filename.from_os_specific(fp2.name)
    fn2.make_true_case()

    try:
        with registered_type(MultiExtensionLoader):
            model1 = Loader.get_global_ptr().load_sync(fn1, LoaderOptions(LoaderOptions.LF_no_cache))
            assert model1 is not None
            assert model1.name == "loaded"

            model2 = Loader.get_global_ptr().load_sync(fn2, LoaderOptions(LoaderOptions.LF_no_cache))
            assert model2 is not None
            assert model2.name == "loaded"
    finally:
        os.unlink(fp1.name)
        os.unlink(fp2.name)

    # Ensure that both were unregistered.
    registry = LoaderFileTypeRegistry.get_global_ptr()
    assert not registry.get_type_from_extension("test1")
    assert not registry.get_type_from_extension("test2")


def test_loader_nonexistent():
    """Verifies that non-existent files fail before calling load_file."""
    flag = [False]

    class AssertiveLoader:
        extensions = ["test"]

        @staticmethod
        def load_file(path, options, record=None):
            flag[0] = True
            assert False, "should never get here"

    with registered_type(AssertiveLoader):
        model = Loader.get_global_ptr().load_sync("/non-existent", LoaderOptions(LoaderOptions.LF_no_cache))
        assert model is None
        assert not flag[0]


def test_loader_exception(test_filename):
    """Tests for a loader that raises an exception."""

    class FailingLoader:
        extensions = ["test"]

        @staticmethod
        def load_file(path, options, record=None):
            raise Exception("test error")

    with registered_type(FailingLoader):
        model = Loader.get_global_ptr().load_sync(test_filename, LoaderOptions(LoaderOptions.LF_no_cache))
        assert model is None


def test_loader_compressed(test_pz_filename):
    """Tests for loading .pz files and the supports_compressed flag."""

    class TestLoader:
        extensions = ["test"]

        @staticmethod
        def load_file(path, options, record=None):
            return ModelRoot("loaded")

    # Test with property absent
    with registered_type(TestLoader):
        model = Loader.get_global_ptr().load_sync(test_pz_filename, LoaderOptions(LoaderOptions.LF_no_cache))
        assert model is None

    # Test with property False, should give same result
    TestLoader.supports_compressed = False
    with registered_type(TestLoader):
        model = Loader.get_global_ptr().load_sync(test_pz_filename, LoaderOptions(LoaderOptions.LF_no_cache))
        assert model is None

    # Test with property True, should work
    TestLoader.supports_compressed = True
    with registered_type(TestLoader):
        model = Loader.get_global_ptr().load_sync(test_pz_filename, LoaderOptions(LoaderOptions.LF_no_cache))
        assert model is not None
        assert model.name == "loaded"

    # Test with property invalid type, should not register
    TestLoader.supports_compressed = None
    with pytest.raises(TypeError):
        LoaderFileTypeRegistry.get_global_ptr().register_type(TestLoader)


def test_loader_ram_cache(test_filename):
    """Tests that the Python loader plug-ins write to the RAM cache."""

    # Ensure a clean slate.
    from panda3d.core import ModelPool
    ModelPool.release_all_models()

    with registered_type(DummyLoader):
        model1 = Loader.get_global_ptr().load_sync(test_filename, LoaderOptions(LoaderOptions.LF_no_disk_cache | LoaderOptions.LF_allow_instance))
        assert model1 is not None
        assert model1.name == "loaded"

        assert ModelPool.has_model(test_filename)
        assert ModelPool.get_model(test_filename, True) == model1

        model2 = Loader.get_global_ptr().load_sync(test_filename, LoaderOptions(LoaderOptions.LF_cache_only | LoaderOptions.LF_allow_instance))
        assert model2 is not None
        assert model1 == model2

        ModelPool.release_model(model2)
