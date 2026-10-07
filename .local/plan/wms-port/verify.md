# Pending verification

Things written but not checked. Tick off when verified.

## Collision / server
- [ ] Procedural M2 collision mesh: run the vmap extractor on a generated M2 (not `NoCollision`), then walk on it in the real client/server.
- [ ] Procedural WMO collision: same check (render polys count as collision).
- [ ] Extract batch op (`wow.extract`) run through the editor UI, incl. dry-run staleness summary.

## Client rendering
- [ ] Imported water up close in the viewport (use look.sh with a Create()d entity; Focus(entity from All()) fails).
- [ ] Magma/slime written as HeightUv: check in the real client.
- [ ] WMO group names (MOGN) in the real client.
- [ ] F5 NPC appearance (CreatureDisplayInfoExtra) in the viewport.

## Data / migration
- [ ] `WaterImported` column against an existing (old) editor DB.
- [ ] Scratch DB in /tmp/wmsdbg has leftover TestPanel entities (cleanup only).

## Spell editor (G5)
- [ ] Added below as it lands.
- [ ] Spell catalog (`Spell`, SpellFactory): open an existing spell, edit, save, reopen; check satellite rows (`dbc_spellname/misc/cooldowns/categories/levels/spellpower/spelleffect`) updated in place, new spell inserts rows with fresh IDs, delete removes them. Staging (Upsert/NextId) has no automated test.
- [ ] Spell links "Spell Cast Times"/"Spell Duration"/"Spell Range" resolve in the UI; effect editor draws and undoes.
