def attach(self, body1, body2):
    """
    Attach two bodies together.
    If either body is None, the other will be attached to the environment.
    """
    if body1 and body2:
        self.attachBodies(body1, body2)
    elif body1 and not body2:
        self.attachBody(body1, 0)
    elif not body1 and body2:
        self.attachBody(body2, 1)
