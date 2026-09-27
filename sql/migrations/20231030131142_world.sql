INSERT INTO `migrations` VALUES ('20231030131142');

-- Add your query below.
 -- Talisman of Ascendance should most likely only proc once per cast.
 INSERT INTO `spell_proc_event` (`entry`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `procFlags`, `procEx`, `ppmRate`, `CustomChance`, `Cooldown`, `build_min`, `build_max`) VALUES (28200, 0, 0, 0, 0, 0, 0, 524288, 0, 0, 0, 5464, 5875);

-- End of migration.
;
