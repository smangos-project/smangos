INSERT INTO `migrations` VALUES ('20250530110153');

-- Add your query below.
 CREATE TABLE `transports__new` (
  `entry` INTEGER NOT NULL DEFAULT 0,
  `build` INTEGER NOT NULL DEFAULT 0,
  `name` TEXT COLLATE NOCASE,
  `period` INTEGER NOT NULL DEFAULT 0,
  PRIMARY KEY (`entry`, `build`)
);

INSERT INTO `transports__new` (`entry`, `build`, `name`, `period`)
SELECT `entry`, 0, `name`, `period` FROM `transports`;

DROP TABLE `transports`;
ALTER TABLE `transports__new` RENAME TO `transports`;

DELETE FROM `transports`;

INSERT INTO `transports` (`entry`, `build`, `name`, `period`) VALUES (20808, 0, 'Ratchet and Booty Bay', 339575), (20808, 4695, 'Ratchet and Booty Bay', 350818), (164871, 0, 'Orgrimmar and Undercity', 360016), (164871, 4695, 'Orgrimmar and Undercity', 356284), (175080, 0, 'Grom''Gol Base Camp and Orgrimmar', 303463), (176231, 0, 'Menethil Harbor and Theramore Isle', 311153), (176231, 4695, 'Menethil Harbor and Theramore Isle', 329313), (176244, 0, 'Teldrassil and Auberdine', 316251), (176310, 0, 'Menethil Harbor and Auberdine', 283065), (176310, 4695, 'Menethil Harbor and Auberdine', 295579), (176495, 0, 'Grom''Gol Base Camp and Undercity', 333044), (177233, 0, 'Forgotton Coast and Feathermoon Stronghold', 299437), (177233, 5464, 'Forgotton Coast and Feathermoon Stronghold', 317040), (181056, 5464, 'Naxxramas', 1208014);

-- End of migration.
;
