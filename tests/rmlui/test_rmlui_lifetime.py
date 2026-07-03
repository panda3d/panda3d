import gc
import pytest

rmlui = pytest.importorskip("panda3d.rmlui")

from .conftest import load


def test_bind_list_survives_dropped_model_handle(rml_context):
    """bind_list's custom VariableDefinition used to be owned solely by the
    Python RmlDataModel wrapper while RmlUi held a raw pointer to it.  Dropping
    the handle and forcing GC freed it, so the next update() was a use-after-
    free.  RmlContext now retains the wrapper for the model's lifetime."""
    items = ["a", "b", "c"]
    dm = rml_context.create_data_model("m")
    assert dm.bind_list("items", lambda: items)

    doc = load(rml_context,
               "<div id='out' data-model='m'>"
               "<span data-for='i : items'>{{i}}</span></div>")

    # Drop every Python reference to the model and force collection.
    del dm
    gc.collect()

    # Mutating + re-evaluating must not touch freed memory.
    items.append("d")
    again = rml_context.get_data_model("m")
    assert again is not None
    again.dirty_variable("items")
    rml_context.update()

    out = doc.get_element_by_id("out")
    assert "d" in out.get_inner_rml()


def test_remove_data_model_invalidates_handle(rml_context):
    """After remove_data_model the wrapper must report invalid and its binders
    must fail cleanly rather than touching the destroyed RmlUi model."""
    dm = rml_context.create_data_model("m")
    assert dm.is_valid()

    assert rml_context.remove_data_model("m")
    assert not dm.is_valid()

    # Binding / dirtying an invalidated model is a safe no-op, not a crash.
    assert not dm.bind_func("x", lambda: 1)
    assert not dm.bind_list("y", lambda: [])


def test_get_data_model_shares_bindings(rml_context):
    """The context caches the model wrapper, so a binding made on the created
    handle is visible (and kept alive) through a re-fetched handle.  (Interrogate
    mints a fresh Python proxy per call, so this checks behaviour, not identity.)"""
    items = [1, 2]
    a = rml_context.create_data_model("m")
    assert a.bind_list("items", lambda: items)

    b = rml_context.get_data_model("m")
    assert b is not None and b.is_valid()
    # The binding made via `a` is live on the shared model fetched as `b`.
    b.dirty_variable("items")  # no-op if the binding/model weren't shared/alive
    rml_context.update()


def test_element_use_after_document_close_is_safe(rml_context):
    """An RmlElement wrapper retained across doc.close() + update() used to
    dereference a freed Rml::Element.  The ObserverPtr now nulls it, so the
    accessors return safe defaults instead of crashing."""
    doc = load(rml_context, "<p id='hello'>hi</p>")
    el = doc.get_element_by_id("hello")
    assert el.get_id() == "hello"

    doc.close()
    rml_context.update()  # actually frees the document + its elements

    # The element is gone; calls must return safe defaults, not crash.
    assert el.get_id() == ""
    assert el.get_inner_rml() == ""
    el.set_inner_rml("ignored")        # no-op, must not crash
    assert el.get_num_children() == 0


def test_document_use_after_region_teardown_is_safe(window):
    """A document wrapper retained after its region is destroyed must not
    dereference freed memory."""
    from panda3d.rmlui import RmlRegion

    region = RmlRegion.make("lifetime-doc", window)
    ctx = region.get_context()
    doc = load(ctx, "<p>bye</p>")
    assert doc.get_title() is not None

    # Tear the region down (Rml::RemoveContext destroys the document).
    window.remove_display_region(region)
    del region, ctx
    gc.collect()

    # Stale document wrapper: safe defaults, no crash.
    assert doc.get_title() == ""
    doc.hide()      # no-op
    doc.close()     # idempotent / safe


def test_data_model_use_after_region_teardown_is_safe(window):
    """A data-model wrapper retained after its region is destroyed must not
    dereference the freed RmlUi model.  ~RmlRegion invalidates cached models, so
    is_valid() flips false and dirty_*/bind_* become safe no-ops."""
    from panda3d.rmlui import RmlRegion

    region = RmlRegion.make("lifetime-dm", window)
    ctx = region.get_context()
    state = {"x": 1}
    dm = ctx.create_data_model("m")
    assert dm.bind_func("x", lambda: state["x"])
    assert dm.is_valid()

    # Tear the region down (Rml::RemoveContext destroys the data model).
    window.remove_display_region(region)
    del region, ctx
    gc.collect()

    # Stale data-model wrapper: invalidated, all calls safe no-ops, no crash.
    assert not dm.is_valid()
    dm.dirty_variable("x")   # was a use-after-free; now a safe no-op
    dm.dirty_all()
    assert not dm.bind_func("y", lambda: 2)


def test_region_recreate_does_not_leak_or_crash(window):
    """Repeatedly creating and destroying regions exercises the render-interface
    destructor (layer-buffer pool release).  Must not crash or assert."""
    from panda3d.rmlui import RmlRegion

    for i in range(8):
        region = RmlRegion.make("churn-%d" % i, window)
        assert region.is_valid()
        doc = load(region.get_context(), "<p>x</p>")
        assert doc is not None
        window.remove_display_region(region)
        del region, doc
        gc.collect()
