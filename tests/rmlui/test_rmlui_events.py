import gc
import pytest

# Skip if rmlui is not built.
rmlui = pytest.importorskip("panda3d.rmlui")

from .conftest import load

BUTTON = "<div id='btn' style='width:100px;height:40px;'>x</div>"


def test_click_fires_callback(rml_context):
    doc = load(rml_context, BUTTON)
    btn = doc.get_element_by_id("btn")

    fired = []
    btn.add_event_listener("click", lambda ev: fired.append(1))

    btn.click()
    rml_context.update()

    assert fired == [1]


def test_callback_receives_rml_event(rml_context):
    doc = load(rml_context, BUTTON)
    btn = doc.get_element_by_id("btn")

    # The RmlEvent is only valid during the callback, so inspect it there.
    seen = {}

    def on_click(ev):
        seen["is_event"] = isinstance(ev, rmlui.RmlEvent)
        seen["type"] = ev.get_event_type()
        seen["type_prop"] = ev.event_type  # also exposed as a property

    btn.add_event_listener("click", on_click)
    btn.click()
    rml_context.update()

    assert seen["is_event"] is True
    assert seen["type"] == "click"
    assert seen["type_prop"] == "click"


def test_event_parameters(rml_context):
    doc = load(rml_context, BUTTON)
    btn = doc.get_element_by_id("btn")

    seen = {}

    def on_click(ev):
        seen["has_button"] = ev.has_parameter("button")
        seen["button"] = ev.get_parameter_int("button", -1)
        seen["missing"] = ev.get_parameter("nope", "default")

    btn.add_event_listener("click", on_click)
    btn.click()
    rml_context.update()

    assert seen["has_button"] is True
    assert seen["button"] == 0  # left mouse button
    assert seen["missing"] == "default"


def test_multiple_listeners_on_same_element(rml_context):
    doc = load(rml_context, BUTTON)
    btn = doc.get_element_by_id("btn")

    a, b = [], []
    btn.add_event_listener("click", lambda ev: a.append(1))
    btn.add_event_listener("click", lambda ev: b.append(1))

    btn.click()
    rml_context.update()

    assert a == [1]
    assert b == [1]


def test_listener_survives_wrapper_gc(rml_context):
    """Regression: the listener is owned by the RmlUi element, not by the
    Python RmlElement wrapper.  Dropping and garbage-collecting the wrapper must
    NOT remove the listener."""
    doc = load(rml_context, BUTTON)

    fired = []
    btn = doc.get_element_by_id("btn")
    btn.add_event_listener("click", lambda ev: fired.append(1))

    # Drop every reference to the wrapper the listener was attached through.
    del btn
    gc.collect()
    gc.collect()

    # Re-fetch a fresh wrapper for the same element and dispatch a click.
    btn2 = doc.get_element_by_id("btn")
    btn2.click()
    rml_context.update()

    assert fired == [1]


def test_listener_cleaned_up_on_document_close(rml_context):
    """Closing a document detaches and frees its listeners without crashing,
    and the context remains usable afterwards."""
    doc = load(rml_context, BUTTON)
    btn = doc.get_element_by_id("btn")
    btn.add_event_listener("click", lambda ev: None)

    doc.close()
    # close() finalizes on the next update.
    rml_context.update()
    rml_context.update()

    # The context is still healthy: load again and dispatch.
    fired = []
    doc2 = load(rml_context, BUTTON)
    btn2 = doc2.get_element_by_id("btn")
    btn2.add_event_listener("click", lambda ev: fired.append(1))
    btn2.click()
    rml_context.update()

    assert fired == [1]


def test_plain_callable_is_accepted(rml_context):
    """A plain Python callable is coerced to a CallbackObject; an explicit
    PythonCallbackObject works too."""
    from panda3d.core import PythonCallbackObject

    doc = load(rml_context, BUTTON)
    btn = doc.get_element_by_id("btn")

    fired = []
    btn.add_event_listener("click", PythonCallbackObject(lambda ev: fired.append(1)))

    btn.click()
    rml_context.update()

    assert fired == [1]
