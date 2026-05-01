# Contributing

This fork is public, but it is maintainer-directed.

The main purpose of this repository is to publish source for the RG Nano audio-in/sampling experiment, keep license obligations clear, and make builds reproducible. Issues are disabled, and unsolicited pull requests may be closed without review.

If you want to make larger changes, please fork the project under the terms of the license and experiment there. Small fixes that directly improve the RG Nano build, documentation, or simulator may still be considered when they are easy to review and clearly tested.

By submitting a pull request, you confirm that you have the right to contribute the code and that your contribution can be distributed under this repository's license terms.

## Before Opening a Pull Request

- Keep changes focused on RG Nano behavior, simulator support, docs, or build reliability.
- Do not include secrets, device-specific private data, generated binaries, logs, or local config files.
- Run the relevant build or smoke test when possible:

```powershell
.\run.ps1 -Task dev -NoSetup
.\run.ps1 -Task smoke -NoSetup
```

## Support

There is no support guarantee for this fork. Please keep backups of tracker projects and samples before testing experimental builds on hardware.
