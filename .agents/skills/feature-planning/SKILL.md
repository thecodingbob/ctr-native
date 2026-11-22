---
name: feature-planning
description: Plan a Crash Team Racing Expanded feature using repository evidence, subsystem ownership, and concrete validation steps. Write the plan in a temporary Markdown file with step checkboxes.
---

# Feature planning

Use this skill when asked to plan a feature or scope a cross-cutting change.
Planning does not authorize implementation: if the user only requested a plan,
do not change application code, configuration, or saves.

## Ground the plan in this checkout

1. Read `AGENTS.md` at the repository root and check the worktree for existing
   changes. Preserve user work and local files. If implementation follows, use
   a topic branch rather than editing on protected `master`; do not commit.
2. Define the requested player-facing behavior, acceptance criteria, and explicit
   non-goals. Separate confirmed requirements from assumptions and open questions.
   Ask about ambiguities that materially affect scope or compatibility; do not
   turn optional ideas into requirements.
3. Find the current implementation and similar features in `game/`, `platform/`,
   `include/`, `tests/`, `docs/`, and `README.md`. Read the relevant functions,
   callers, and consumers before proposing changes. Record concrete repository-
   root-relative paths and symbols; label unverified leads as investigation work.
4. Load `codebase-navigation` and the relevant subsystem skills listed in
   `AGENTS.md`. Use `game-domain-knowledge` for retail behavior and
   `reference-sources` when prior art is useful. External implementations are
   evidence to compare, not specifications to copy; record provenance if used.

## Choose the smallest coherent design

- Keep game rules in `game/`, host integrations in `platform/`, and shared
  declarations/layouts in `include/`. Reuse existing owner operations and helpers
  before adding abstractions. Group related files thematically without needless
  nesting; do not modify vendored `externals/SDL/` for application features.
- Account for the 32-bit C17 target and subsystem unity builds. Identify the owning
  include chain or independent CMake source, preserve include order, and consider
  file-static name collisions. Keep module constants beside their implementation
  with explicit consumer includes rather than expanding global layout headers.
- Check affected modes, player counts, overlays, transitions, initialization, and
  reset paths. Consult `docs/OVERLAYS.md` when ownership crosses logical overlays.
  Include only relevant cases, explaining why they matter.
- For rendering or resident-data changes, trace all affected consumers, pointer
  lifetimes, coordinate spaces, and layout assumptions with the relevant skills.
  Existing PS1 constraints are not immutable requirements, but changing one means
  updating its dependent paths coherently.
- For adjustable features, use `runtime-configuration` to plan defaults, the
  table-driven INI entry, in-game editing, persistence, preset behavior, and README
  documentation. New options default to vanilla unless requested otherwise;
  preserve existing defaults and never edit the user's local INI files as a
  substitute for shipping defaults.
- Note meaningful tradeoffs, dependencies, risks, and unresolved decisions. Prefer
  a focused implementation over unrelated cleanup or importing broad fork changes.

## Write and maintain a temporary checklist

Every feature plan must be written to a temporary `.md` file in the current
worktree, not only returned in chat. Use a unique descriptive name such as
`.plans/ctr-feature-<slug>-<unique-id>.md` relative to the worktree root.
Keep plan files untracked: never add, stage, or commit them. Do not overwrite
existing plans or change shared ignore rules merely to hide them. If the file
cannot be created, report the limitation rather than claiming it was saved.

Use Markdown checkboxes (`- [ ]`) for each actionable step. Order steps by
dependency, keep each independently verifiable, and name the target files or
symbols plus the intended result. Split investigation, implementation, tests,
and runtime validation where their completion differs. Size the plan to the
feature rather than padding it with irrelevant tasks.

Use this structure, adapting the checklist to the actual feature:

```markdown
# Feature: <name>

## Goal and acceptance criteria
<Requested behavior, defaults, observable success, and non-goals.>

## Repository findings and design
<Verified paths/symbols, ownership, reuse, decisions, and relevant references.>

## Risks and open questions
<Assumptions, affected modes/lifetimes, tradeoffs, and blockers.>

## Steps
- [ ] <Resolve a specific unknown in path/symbol; record the finding.>
- [ ] <Implement the owner behavior in path/symbol; state the expected result.>
- [ ] <Update the affected consumers/configuration/docs, if applicable.>
- [ ] <Add or adapt focused tests with named behavior and edge cases.>
- [ ] <Build and run the relevant suites; record commands and results.>
- [ ] <Validate the concrete runtime scenario and nearby regression case.>

## Validation evidence
<Commands, scenarios, prerequisites, results, and checks not yet run.>
```

Use `testing` and `debugging-and-validation` to select concrete checks. Build
scripts compile the game and suites but do not run tests: plan CTest separately
with the actual build directory/configuration. Focused suites must not require
game assets or a window; renderer, HUD, and mode integration also need runtime
checks, replays, or visual inspection. State unavailable assets/tools as blockers.

Return the plan file's path with a short summary and any blocking questions.
If implementation is subsequently requested, update the same file as work
progresses: mark `- [x]` only for completed steps, record validation evidence,
and leave blocked or unrun checks unchecked with a reason. Revise the plan when
new evidence changes the design; never present planned checks as passing results.
