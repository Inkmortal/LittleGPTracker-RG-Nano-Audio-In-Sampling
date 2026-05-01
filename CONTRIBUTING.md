# Contributing

This fork is public and open for experiments, forks, issues, and pull requests under the repository's license terms.

The main purpose of this repository is to publish source for the RG Nano audio-in/sampling experiment, keep license obligations clear, and make builds reproducible. Contributions that improve the RG Nano build, simulator, documentation, sampling workflow, or general tracker reliability are welcome.

By submitting a pull request, you confirm that you have the right to contribute the code and that your contribution can be distributed under this repository's license terms.

## Branch & Merge Strategy

- **master** is the integration branch. It may contain unstable/experimental features.
- **Pull Requests** should target `master`.
- **Releases** follow the upstream release candidate workflow:
  - release candidates: `1.2.0-bacon0`, `1.2.0-bacon2`, etc.
  - stable releases: `v1.2.0` after a testing period.
- Users should prefer tagged releases. `master` may contain unfinished experimental work.

## Pull Request Requirements

Before submitting a PR, please check:

- [ ] Version bumped in `sources/Application/Model/Project.h` when appropriate.
- [ ] Documentation updated for behavior or workflow changes.
- [ ] CHANGELOG updated when the change is user-facing.
- [ ] Commit history is understandable. Squash-friendly PRs are preferred.
- [ ] Testing is described in the PR.

For RG Nano simulator changes, run:

```powershell
.\run.ps1 -Task dev -NoSetup
.\run.ps1 -Task smoke -NoSetup
```

Do not include secrets, device-specific private data, generated binaries, logs, copied runtime DLLs, or local config files.

## Support

There is no support guarantee for this fork. Please keep backups of tracker projects and samples before testing experimental builds on hardware.
