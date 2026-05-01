# Contributing

This fork is public and open for experiments, forks, issues, and pull requests under the repository's license terms.

The main purpose of this repository is to publish source for the RG Nano audio-in/sampling experiment, keep license obligations clear, and make builds reproducible. Contributions that improve the RG Nano build, simulator, documentation, sampling workflow, or general tracker reliability are welcome.

By submitting a pull request, you confirm that you have the right to contribute the code and that your contribution can be distributed under this repository's license terms.

## Before Opening a Pull Request

- Prefer focused changes, especially around RG Nano behavior, simulator support, docs, or build reliability.
- Do not include secrets, device-specific private data, generated binaries, logs, or local config files.
- Run the relevant build or smoke test when possible:

```powershell
.\run.ps1 -Task dev -NoSetup
.\run.ps1 -Task smoke -NoSetup
```

## Support

There is no support guarantee for this fork. Please keep backups of tracker projects and samples before testing experimental builds on hardware.
