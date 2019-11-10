

some self given rules to make project management easier:

- stick to pep8 (see autopep8)

PRs:

- PRs should be easy to review
- PRs that are subject to discussion should be created as drafts
- PRs should have low number of commits
- PRs should be smaller than 10-20 files
- PRs should be smaller than 200 lines
- PRs that introduce new functions need to introduce tests as well

to remove functions or classes:

- ensure they're not used internally
- then add a deprecation warning in release n
- wait for feedback from users
- if there is no reason to keep them, remove them in release n+1
