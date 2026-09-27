INSERT INTO `migrations` VALUES ('20260406074746');

DELETE FROM `pool_pool` WHERE `pool_id` IN (1783, 1784, 1799, 1805, 1782, 1778);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 1' WHERE `entry` IN (1783, 1784, 1799, 1805, 1782, 1778);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 1' WHERE `pool_entry` IN (1783, 1784, 1799, 1805, 1782, 1778);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4917+1, 1, 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1783, 4917+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (1784, 4917+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (1799, 4917+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (1805, 4917+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (1782, 4917+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (1778, 4917+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1797, 1776, 1773, 1775, 14413, 1774);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 2' WHERE `entry` IN (1797, 1776, 1773, 1775, 14413, 1774);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 2' WHERE `pool_entry` IN (1797, 1776, 1773, 1775, 14413, 1774);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4917+2, 1, 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1797, 4917+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (1776, 4917+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (1773, 4917+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (1775, 4917+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (14413, 4917+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (1774, 4917+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1777, 1798, 1779, 1781, 14414, 1780);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 3' WHERE `entry` IN (1777, 1798, 1779, 1781, 14414, 1780);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 3' WHERE `pool_entry` IN (1777, 1798, 1779, 1781, 14414, 1780);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4917+3, 1, 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1777, 4917+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (1798, 4917+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (1779, 4917+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (1781, 4917+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (14414, 4917+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (1780, 4917+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1802, 1794, 1803, 1801, 14416, 1791);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 4' WHERE `entry` IN (1802, 1794, 1803, 1801, 14416, 1791);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 4' WHERE `pool_entry` IN (1802, 1794, 1803, 1801, 14416, 1791);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4917+4, 1, 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1802, 4917+4, 0, 'Un''Goro Crater - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (1794, 4917+4, 0, 'Un''Goro Crater - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (1803, 4917+4, 0, 'Un''Goro Crater - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (1801, 4917+4, 0, 'Un''Goro Crater - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (14416, 4917+4, 0, 'Un''Goro Crater - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (1791, 4917+4, 0, 'Un''Goro Crater - Group 4 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14417, 1795, 1796, 1789, 14415);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 5' WHERE `entry` IN (14417, 1795, 1796, 1789, 14415);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 5' WHERE `pool_entry` IN (14417, 1795, 1796, 1789, 14415);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4917+5, 1, 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14417, 4917+5, 0, 'Un''Goro Crater - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (1795, 4917+5, 0, 'Un''Goro Crater - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (1796, 4917+5, 0, 'Un''Goro Crater - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (1789, 4917+5, 0, 'Un''Goro Crater - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (14415, 4917+5, 0, 'Un''Goro Crater - Group 5 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1785, 1800, 1788, 1786, 1787);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 6' WHERE `entry` IN (1785, 1800, 1788, 1786, 1787);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 6' WHERE `pool_entry` IN (1785, 1800, 1788, 1786, 1787);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4917+6, 1, 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 6 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1785, 4917+6, 0, 'Un''Goro Crater - Group 6 - Truesilver Deposit / Small Thorium Vein', 0), (1800, 4917+6, 0, 'Un''Goro Crater - Group 6 - Truesilver Deposit / Small Thorium Vein', 0), (1788, 4917+6, 0, 'Un''Goro Crater - Group 6 - Truesilver Deposit / Small Thorium Vein', 0), (1786, 4917+6, 0, 'Un''Goro Crater - Group 6 - Truesilver Deposit / Small Thorium Vein', 0), (1787, 4917+6, 0, 'Un''Goro Crater - Group 6 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1804, 1793, 1792, 1400, 1790);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 7' WHERE `entry` IN (1804, 1793, 1792, 1400, 1790);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 7' WHERE `pool_entry` IN (1804, 1793, 1792, 1400, 1790);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4917+7, 1, 'Truesilver Deposit / Small Thorium Vein - Un''Goro Crater - Group 7 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1804, 4917+7, 0, 'Un''Goro Crater - Group 7 - Truesilver Deposit / Small Thorium Vein', 0), (1793, 4917+7, 0, 'Un''Goro Crater - Group 7 - Truesilver Deposit / Small Thorium Vein', 0), (1792, 4917+7, 0, 'Un''Goro Crater - Group 7 - Truesilver Deposit / Small Thorium Vein', 0), (1400, 4917+7, 0, 'Un''Goro Crater - Group 7 - Truesilver Deposit / Small Thorium Vein', 0), (1790, 4917+7, 0, 'Un''Goro Crater - Group 7 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` IN (1211, 8118);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1488, 1491, 14441, 1487, 1489);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Un''Goro Crater - Group 1' WHERE `entry` IN (1488, 1491, 14441, 1487, 1489);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Un''Goro Crater - Group 1' WHERE `pool_entry` IN (1488, 1491, 14441, 1487, 1489);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3553+1, 1, 'Truesilver Deposit / Rich Thorium Vein - Un''Goro Crater - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1488, 3553+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (1491, 3553+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (14441, 3553+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (1487, 3553+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (1489, 3553+1, 0, 'Un''Goro Crater - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1493, 1486, 1483, 14439, 1492);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Un''Goro Crater - Group 2' WHERE `entry` IN (1493, 1486, 1483, 14439, 1492);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Un''Goro Crater - Group 2' WHERE `pool_entry` IN (1493, 1486, 1483, 14439, 1492);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3553+2, 1, 'Truesilver Deposit / Rich Thorium Vein - Un''Goro Crater - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1493, 3553+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (1486, 3553+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (1483, 3553+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (14439, 3553+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (1492, 3553+2, 0, 'Un''Goro Crater - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14440, 17199, 1485, 1484, 1490);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Un''Goro Crater - Group 3' WHERE `entry` IN (14440, 17199, 1485, 1484, 1490);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Un''Goro Crater - Group 3' WHERE `pool_entry` IN (14440, 17199, 1485, 1484, 1490);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3553+3, 1, 'Truesilver Deposit / Rich Thorium Vein - Un''Goro Crater - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14440, 3553+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (17199, 3553+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (1485, 3553+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (1484, 3553+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (1490, 3553+3, 0, 'Un''Goro Crater - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` IN (1147, 1146, 1141);

-- End of migration.
;
