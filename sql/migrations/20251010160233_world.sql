INSERT INTO `migrations` VALUES ('20251010160233');

-- Add your query below.

-- Add missing icon name field to gameobjects. It was added in 1.12.
CREATE TABLE `gameobject_template__new` (
  `entry` INTEGER NOT NULL DEFAULT 0,
  `patch` INTEGER NOT NULL DEFAULT 0,
  `type` INTEGER NOT NULL DEFAULT 0,
  `displayId` INTEGER NOT NULL DEFAULT 0,
  `name` TEXT NOT NULL DEFAULT '' COLLATE NOCASE,
  `icon` TEXT NOT NULL DEFAULT '' COLLATE NOCASE,
  `faction` INTEGER NOT NULL DEFAULT 0,
  `flags` INTEGER NOT NULL DEFAULT 0,
  `size` REAL NOT NULL DEFAULT 1,
  `data0` INTEGER NOT NULL DEFAULT 0,
  `data1` INTEGER NOT NULL DEFAULT 0,
  `data2` INTEGER NOT NULL DEFAULT 0,
  `data3` INTEGER NOT NULL DEFAULT 0,
  `data4` INTEGER NOT NULL DEFAULT 0,
  `data5` INTEGER NOT NULL DEFAULT 0,
  `data6` INTEGER NOT NULL DEFAULT 0,
  `data7` INTEGER NOT NULL DEFAULT 0,
  `data8` INTEGER NOT NULL DEFAULT 0,
  `data9` INTEGER NOT NULL DEFAULT 0,
  `data10` INTEGER NOT NULL DEFAULT 0,
  `data11` INTEGER NOT NULL DEFAULT 0,
  `data12` INTEGER NOT NULL DEFAULT 0,
  `data13` INTEGER NOT NULL DEFAULT 0,
  `data14` INTEGER NOT NULL DEFAULT 0,
  `data15` INTEGER NOT NULL DEFAULT 0,
  `data16` INTEGER NOT NULL DEFAULT 0,
  `data17` INTEGER NOT NULL DEFAULT 0,
  `data18` INTEGER NOT NULL DEFAULT 0,
  `data19` INTEGER NOT NULL DEFAULT 0,
  `data20` INTEGER NOT NULL DEFAULT 0,
  `data21` INTEGER NOT NULL DEFAULT 0,
  `data22` INTEGER NOT NULL DEFAULT 0,
  `data23` INTEGER NOT NULL DEFAULT 0,
  `mingold` INTEGER NOT NULL DEFAULT 0,
  `maxgold` INTEGER NOT NULL DEFAULT 0,
  `script_name` TEXT NOT NULL DEFAULT '' COLLATE NOCASE,
  PRIMARY KEY (`entry`, `patch`)
);

INSERT INTO `gameobject_template__new` (`entry`, `patch`, `type`, `displayId`, `name`, `icon`, `faction`, `flags`, `size`, `data0`, `data1`, `data2`, `data3`, `data4`, `data5`, `data6`, `data7`, `data8`, `data9`, `data10`, `data11`, `data12`, `data13`, `data14`, `data15`, `data16`, `data17`, `data18`, `data19`, `data20`, `data21`, `data22`, `data23`, `mingold`, `maxgold`, `script_name`)
SELECT `entry`, `patch`, `type`, `displayId`, `name`, '', `faction`, `flags`, `size`, `data0`, `data1`, `data2`, `data3`, `data4`, `data5`, `data6`, `data7`, `data8`, `data9`, `data10`, `data11`, `data12`, `data13`, `data14`, `data15`, `data16`, `data17`, `data18`, `data19`, `data20`, `data21`, `data22`, `data23`, `mingold`, `maxgold`, `script_name` FROM `gameobject_template`;

DROP TABLE `gameobject_template`;
ALTER TABLE `gameobject_template__new` RENAME TO `gameobject_template`;

-- Add missing icons to Silithyst Mound and Silithyst Geyser.
UPDATE `gameobject_template` SET `icon`='PVP' WHERE `entry` IN (181597, 181598);


-- End of migration.
