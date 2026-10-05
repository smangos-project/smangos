# Bots

**Code:** `src/game/RealBots`, `contrib/realbotrunner`, `tests/conformance`, `tests/unit/realbots`

A bot framework that speaks the modern 1.15 protocol over the network exactly like a real client. Bots touch no server state, so they can run inside `mangosd` or in the standalone `realbotrunner`, against any server.

Bot procedures and test scenarios are written as coroutines that await packets, stages and timeouts. Any test can send or intercept any packet, including malformed ones, and drive login stage by stage.

## What you can do

- **Conformance tests:** `tests-conformance` runs scenarios (login, character screen, world entry, combat, auction house, groups and more) against a target server and compares results with expectation files.
- **Stability and load tests:** scenarios in `tests/conformance/load` run many bots at once.
- **Unit tests:** `tests-realbots` tests the framework itself against a fake world server, with no live server needed.

Build with `ENABLE_REALBOTS`. See `src/game/RealBots/README.md` for the design.
