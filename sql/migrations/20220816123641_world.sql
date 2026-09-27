INSERT INTO `migrations` VALUES ('20220816123641');

-- Add your query below.
 -- Increase Lay on Hands threat so it does 95% of its heal amount with Improved Righteous Fury.
 INSERT INTO `spell_threat` (`entry`, `Threat`, `multiplier`, `ap_bonus`) VALUES (633, 0, 2, 0);

-- End of migration.
;
