# Contributing to Panda3D

Panda3D is an open-source, community-driven project, completely dependent on the
contribution of volunteers.  As such we highly welcome you to contribute code to
the project.  This document aims to outline some guidelines for doing so.

## Before implementing a change

We highly recommend that you file a GitHub Issue before making a change.  Issues 
are used to track bugs and feature requests but also to get feedback from the 
other developers about design decisions, or a specific implementation strategy.
Note, the [good first issue](https://github.com/panda3d/panda3d/issues?q=state%3Aopen%20label%3A%22good%20first%20issue%22) 
tag is maintained with issues that could be easily resolved, but are minor enough 
to be valuable to keep as learning/teaching tools for new contributors. If you 
want to contribute, try starting with one of those! Please note; the value of 
submitting a *quality* PR for one of these issues is in the experience and skills 
you will gain working with Panda3D. It would be pointless and use much more time 
to review these were they done with LLM agents, as the experience would be lost 
and the community would not continue to grow. Please don't contribute generated 
code for the 'good first issue's, and take care when submitting any form of 
generated code that you take the time to ensure it meets all standards.

## Standards

In order for a change to be accepted, it must be implemented in a way that fits
the general design principles of the Panda3D API, and with the general
priorities of the team.  You *must* discuss any changes with other developers. 
You may use GitHub Issues for this, but we invite you to visit the #development 
channel on our Discord, where we can offer more help and guidance on 
contributing to Panda3D.

If you haven't built Panda3D before, ensure you have an understanding of how to 
use either [CMake](cmake/README.md) or the [makepanda](doc/INSTALL) system to 
build the engine. Over time, we intend to transition to using CMake as our 
preferred build system, so use that if you're not sure which to use. The 
project [README](README.md) may offer more information, and you can always ask 
questions in the Discord server or on the [forums](https://discourse.Panda3D.org/). 

We also recommend that you familiarize yourself with the established [coding
style and design patterns](doc/CODING_STYLE.md) of Panda3D, as any 
inconsistencies will need to be fixed before the changes can be accepted. To 
minimise how many changes will need to be made, you must ensure that your 
contribution meets these standards. 

Please note, if you are using any sort of code-generation, particularly 
if through an arms-reach Large Language Model, you are expected to go through 
all of the changes you suggest in detail, and have a good understanding of 
what sort of changes you are trying to make to the engine. We require all 
changes to be of good quality and meet the standards above, regardless of 
the method by which it was produced. 

## Submitting a change

All changes from non-core contributors are made via pull requests ('PRs').  
Submitting a PR requires you to:
1) fork the Panda3D repository,
2) create a branch for your change,
3) push your changes to this branch,
4) review that your changes meet the style guide, design patterns, systems,
and the copyright standards are met (you own the rights to all code), and
that your code does not break existing Panda3D API uses. You should now
establish it is covered by the test suite where required, and run tests,
5) then you can request that this branch is merged into the upstream branch.
Each pull request is reviewed by a maintainer and automatically tested for
regressions and unit test coverage.
6) The maintainer will suggest any changes, which you can add by committing
more code to the same branch.  You may need to do a force push to make these
changes.  Once the change is in an acceptable state, the maintainer will merge
your change into the appropriate branch of the Panda3D repository.

#### Preparing to Submit a change
1) To make it easier for the maintainer to review your changes, prepare a clear
and concise description of your intentions, being sure to link any issues that
are resolved by the change).
2) Prepare unit tests which contribute to a high level of confidence that this
change does not break any existing behaviours.
3) Break up your PR into as many sub-changes as is reasonable. We recommend that
each PR cover only one change; we rarely accept larger PRs.

If your change is still a work in progress, please mark the PR as "draft".  This
will prevent github sending an email to all other contributors every time you 
push a new change to your branch.  Setting PRs as 'draft PRs' in this way is 
a good way to show others your work in progess and get feedback. If you're 
uncertain that you're going in the right direction, you should definitely mark 
the PR as a draft.

For more information on making a contribution, please visit the
[Get Involved](https://www.Panda3D.org/get-involved/) page on our website, or
the excellent [Open Source Guide](https://opensource.guide/how-to-contribute/).

## Copyright Notice

The code in the Panda3D repository is copyrighted to Carnegie Mellon University
and licensed under the Modified BSD License.  By submitting your changes, you
accept that your code becomes placed under the same license.  Except in specific
agreed-upon cases, we do not accept code contributions under alternate licenses.
