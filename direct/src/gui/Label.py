from ShowBaseGlobal import *
from DirectObject import *
from GuiGlobals import *

## These are the styles of labels we might commonly see.  They control
## the way the label looks.

ButtonUp = 1
ButtonUpGui = 2
ButtonLit = 3
ButtonDown = 4
ButtonInactive = 5
Sign = 6
ScrollTitle = 7
ScrollItem = 8

def textLabel(string, style,
              scale = 0.1,
              width = None,
              drawOrder = getDefaultDrawOrder(),
              font = getDefaultFont()):
    """textLabel(string, int style, float scale, float width,
                 int drawOrder, Node font)

    Generates a text label suitable for adding to a GuiButton or
    GuiSign.

    """
    
    (label, text) = \
            textLabelAndText(string, style, scale, width, drawOrder, font)
    return label
    
def textLabelAndText(string, style,
                     scale = 0.1,
                     width = None,
                     drawOrder = getDefaultDrawOrder(),
                     font = getDefaultFont()):
    """textLabelAndText(string, int style, float scale, float width,
                        int drawOrder, Node font)

    Generates a text label suitable for adding to a GuiButton or
    GuiSign.

    This function returns both the label and the TextNode that is
    within the label, allowing the calling function to update the
    label's text later.  If there are a limited number of text
    strings, however, it would probably be better to create a separate
    GuiItem for each possible text string, and rotate them in and out.

    """
    text = TextNode()

    # Freeze the text so we can set up its properties.
    text.freeze()
    
    text.setFont(font)
    text.setAlign(TMALIGNCENTER)
    text.setDrawOrder(drawOrder)
    text.setTextColor(0.0, 0.0, 0.0, 1.0)
    text.setCardColor(1.0, 1.0, 1.0, 1.0)
    text.setCardAsMargin(0.1, 0.1, 0.0, 0.0)
    text.setTransform(Mat4.scaleMat(scale))

    if style == ButtonUp:
        # This is the default: black on white.
        pass

    elif style == ButtonUpGui:
        # special GUI button with our lovely gui color
        text.setCardColor(0.25, 0.7, 0.85, 1.0)
        
    elif style == ButtonLit:
        # When the mouse is over the button, the background turns
        # yellow.
        text.setCardColor(1.0, 1.0, 0.0, 1.0)
        
    elif style == ButtonDown:
        # When the button is being depressed, its turns to white
        # on black.
        text.setTextColor(1.0, 1.0, 1.0, 1.0)
        text.setCardColor(0.0, 0.0, 0.0, 1.0)
        
    elif style == ButtonInactive:
        # When the button is inactive, it's gray on gray.
        text.setTextColor(0.4, 0.4, 0.4, 1.0)
        text.setCardColor(0.6, 0.6, 0.6, 1.0)
        
    elif style == Sign:
        # For a sign, we want red text with no background card.
        text.setTextColor(1., 0., 0., 1.)
        text.clearCard()

    elif style == ScrollTitle:
        text.setTextColor(1., 0., 0., 1.)
        text.setCardColor(1., 1., 1., 0.)
    
    elif style == ScrollItem:
        pass

    else:
        raise ValueError


    # Don't set the text until the very last thing, so the TextNode
    # has minimal work to do (even though it's frozen).
    text.setText(string)

    v = text.getCardActual()

    if width != None:
        # If the user specified a specific width to use, keep it.
        v = VBase4(-width / 2.0, width / 2.0, v[2], v[3])
        if text.hasCard():
            text.setCardActual(v[0], v[1], v[2], v[3])

    # Now we're completely done setting up the text, and we can safely
    # thaw it.
    text.thaw()

    # Now create a GuiLabel containing this text.
    label = GuiLabel.makeModelLabel(text, v[0] * scale, v[1] * scale,
                                    v[2] * scale, v[3] * scale)

    label.setDrawOrder(drawOrder)
    return (label, text)


def modelLabel(model,
               geomRect = None,
               style = None,
               scale = 0.1,
               drawOrder = getDefaultDrawOrder()):

    # Preserve transitions on the arc by creating an intervening node.
    topnode = NamedNode('model')
    topnp = NodePath(topnode)
    mi = model.instanceTo(topnp)
    mi.setScale(scale)
    mi.setBin('fixed', drawOrder)

    if style != None:
        # For a modelLabel, the style determines the color that may be
        # applied.
        color = None
        
        # The style might *itself* be a four-component color.
        if (isinstance(style, types.TupleType) or
            isinstance(style, VBase4)):
            color = style

        # Otherwise, there might be a predefined default color for one
        # of the canned styles.  Most don't have a default color,
        # though.
        elif style == ButtonInactive:
            color = (0.5, 0.5, 0.5, 1.0)
            
        if color != None:
            mi.setColor(color[0], color[1], color[2], color[3])
        

    if geomRect == None:
        geomRect = (1, 1)

    if len(geomRect) == 2:
        # If we got only two parameters, it's height and width.
        label = GuiLabel.makeModelLabel(topnode,
                                        geomRect[0] * scale,
                                        geomRect[1] * scale)
    elif len(geomRect) == 4:
        # If we got four, they're left, right, bottom, top.
        label = GuiLabel.makeModelLabel(topnode,
                                        geomRect[0] * scale,
                                        geomRect[1] * scale,
                                        geomRect[2] * scale,
                                        geomRect[3] * scale)
    else:
        raise ValueError
    
    label.setDrawOrder(drawOrder)
    return label
                                    
