INSERT INTO `migrations` VALUES ('20260929210000');

-- Spell sets server logic checks for; `tag` is a SpellTag name (SpellDefines.h).
CREATE TABLE `spell_tag` (
  `spell_id` INTEGER NOT NULL,
  `tag` TEXT NOT NULL,
  PRIMARY KEY (`spell_id`, `tag`)
);

-- Stacking keys: spells with equal non-zero keys are compared as the same buff (stack_key),
-- ranks of one spell (rank_key) or one single-target aura (single_target_key).
CREATE TABLE `spell_stack_key` (
  `spell_id` INTEGER NOT NULL PRIMARY KEY,
  `stack_key` INTEGER NOT NULL DEFAULT 0,
  `rank_key` INTEGER NOT NULL DEFAULT 0,
  `single_target_key` INTEGER NOT NULL DEFAULT 0
);
