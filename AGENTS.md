# Agent Environment

Use `.\run.ps1` as the canonical entrypoint for this repo.

- Setup only: `.\run.ps1 -Task setup`
- Default app dev run: `.\run.ps1`
- Specific app: `.\run.ps1 -App <app-id> -Task dev`
- Skip dependency install: `.\run.ps1 -NoSetup`

Authoritative runtime config is in `.agent-env.json`.
Do not guess package manager or Python setup outside this file.

## User Collaboration Contract

- The user's sole responsibility is to test the application like an end user and provide feedback. Never ask the user to copy/paste text, manually open developer tools, inspect logs, run commands, or fix anything by hand.
- All investigation, reproduction, debugging, instrumentation, log retrieval, data inspection, browser/devtools work, and environment actions must be performed by the agent using available tools. If needed information is not currently retrievable, add or adjust tooling, logging, scripts, or documentation so the agent can retrieve it directly.
- Do not solve defects with fallback behavior, silent downgrades, stubbed paths, or alternate flows that hide the intended behavior. Fix the root cause so the intended behavior works as designed.
- Before writing new code, look for existing relevant implementations, utilities, patterns, and abstractions to reuse. Prefer shared, reusable code over one-off logic.
- Do not take shortcuts because a dependency, asset, API, fixture, or tool is missing. Download, install, generate, or research what is needed when appropriate; web search and downloads are permitted for this purpose.
- Any simplification, scope reduction, temporary workaround, or change to intended behavior requires explicit user approval before implementation.
