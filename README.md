This is a [Gitflow](https://www.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow) repo and you should install the [gitflow extensions](https://github.com/petervanderdoes/gitflow-avh/wiki/Installation) to get the best milage. Gitflow affords a stricter release policy that other workflows and, as this package is used to build official versions of ASKAP software, it is important that this process is reliable and repeatable and so this package is a controlled dependency.

In accordance with Gitflow workflow:
* If you are after the latest release then take it from `tags/<ver>` or the head of `master`
* Development and fixes should only proceed in features/branches and then via pull requests into `develop`
* Releases are prepared in `release` branches and then canonised to `master`
* Official releases are tagged on `master`
