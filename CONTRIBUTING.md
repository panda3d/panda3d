# Contributing to Panda3D

Panda3D is an open-source, community-driven project, completely dependent on the
contribution of volunteers.  As such we highly welcome you to contribute code to
the project.  This document aims to outline some guidelines for doing so.

If you would like to contribute but aren't sure where to start, please visit the
[Get Involved](https://www.panda3d.org/get-involved/) page on our website, or
the excellent [Open Source Guide](https://opensource.guide/how-to-contribute/).

## Before implementing a change

We highly recommend that you file issues before making a change.  Issues are
used to track bugs and feature requests but also to get feedback from the other
developers about design decisions or a specific implementation strategy.

It is important for acceptance that the change is implemented in a way that fits
the general design principles of the Panda3D API, and fits well with the general
priorities of the team.  Therefore, prior discussion with other developers is
critical.  Issues can be used to facilitate this, but we also invite you to
visit the #development channel on Discord (or #panda3d-devel on Libera Chat).

We also recommend that you familiarize yourself with the established coding
style and design patterns of Panda3D, to reduce the amount of changes that have
to be made during the review process.

## Submitting a change

All changes from non-core contributors are made via pull requests.  This
requires you to fork the Panda3D repository, create a branch for your change,
push your changes to this branch, and request that this branch is merged into
the upstream branch.  Each pull request is reviewed by a maintainer and
automatically tested for regressions and unit test coverage.  The maintainer
will suggest any changes, which you can add by committing more code to the same
branch (you can do a force push if necessary).  Once the change is deemed
acceptable, the maintainer will merge your change into the appropriate branch of
the repository.

To make it easier for the maintainer to review your changes, we highly recommend
that you give a clear and concise description of intent (linking to any issues
that are resolved by the change), as well as the inclusion of unit tests, which
contribute to a high level of confidence that this change does not break any
existing behaviours.  We also recommend breaking up separate changes into
separate PRs, rather than submitting one big PR with several unrelated changes.

If your change is still a work in progress, please mark the PR as "draft".  This
will prevent other contributors from receiving an email every time you push a
new change to your branch.  Draft PRs can also be used to invite early feedback
on your change, especially if you are uncertain about whether you are going in
the right direction.

The code in the Panda3D repository is copyrighted to Carnegie Mellon University
and licensed under the Modified BSD License.  By submitting your changes, you
accept that your code becomes placed under the same license.  Except in specific
agreed-upon cases, we do not accept code contributions under alternate licenses.
