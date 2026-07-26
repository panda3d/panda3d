import pytest

# Skip the whole rmlui test package if the module is not built.
rmlui = pytest.importorskip("panda3d.rmlui")


# RmlUi contexts are global and keyed by name, and the window fixture is
# session-scoped, so each region needs a unique context name and must be torn
# down (which removes its context) to avoid "context already exists" between
# tests.
_counter = [0]


@pytest.fixture
def rml_region(window):
    """An RmlRegion attached to the offscreen test window."""
    from panda3d.rmlui import RmlRegion

    _counter[0] += 1
    region = RmlRegion.make("test-%d" % _counter[0], window)
    assert region.is_valid()

    yield region

    # Removing the region from the window drops the last reference, running the
    # RmlRegion destructor which calls Rml::RemoveContext.
    window.remove_display_region(region)


@pytest.fixture
def rml_context(rml_region):
    """The RmlContext of the test region."""
    return rml_region.get_context()


def load(context, body, head=""):
    """Loads an in-memory RML document with the given <body>/<head> contents
    and returns it shown.  Updates the context once so layout settles."""
    rml = "<rml><head>%s</head><body>%s</body></rml>" % (head, body)
    doc = context.load_document_from_memory(rml)
    assert doc is not None
    doc.show()
    context.update()
    return doc
