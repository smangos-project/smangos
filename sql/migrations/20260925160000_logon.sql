DROP PROCEDURE IF EXISTS add_migration;
DELIMITER ??
CREATE PROCEDURE `add_migration`()
BEGIN
DECLARE v INT DEFAULT 1;
SET v = (SELECT COUNT(*) FROM `migrations` WHERE `id`='20260925160000');
IF v = 0 THEN
INSERT INTO `migrations` VALUES ('20260925160000');
-- Add your query below.

-- realmd (and the vanilla SRP6 world-login path it served) is gone; bnetserver's SRP6 login
-- owns `session_key_bnet`/`battlenet_accounts` instead (see the 20260925120000 migration).
ALTER TABLE `account`
    DROP COLUMN `sessionkey`,
    DROP COLUMN `v`,
    DROP COLUMN `s`;

-- End of migration.
END IF;
END??
DELIMITER ;
CALL add_migration();
DROP PROCEDURE IF EXISTS add_migration;
