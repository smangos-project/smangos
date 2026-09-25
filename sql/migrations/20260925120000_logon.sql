DROP PROCEDURE IF EXISTS add_migration;
DELIMITER ??
CREATE PROCEDURE `add_migration`()
BEGIN
DECLARE v INT DEFAULT 1;
SET v = (SELECT COUNT(*) FROM `migrations` WHERE `id`='20260925120000');
IF v = 0 THEN
INSERT INTO `migrations` VALUES ('20260925120000');
-- Add your query below.

-- Battle.net login for the modern client. One game account per Battle.net account: `email` holds
-- the upper-cased game account username, and the SRP identity is derived from it.
-- `salt` and `verifier` are hex text: Field::GetCppString() stops at the first zero byte, which
-- corrupts random binary data.
CREATE TABLE IF NOT EXISTS `battlenet_accounts` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Identifier',
  `email` varchar(320) NOT NULL COMMENT 'Upper-cased game account username',
  `srp_version` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `salt` char(64) DEFAULT NULL COMMENT 'Hex-encoded 32-byte SRP salt',
  `verifier` text COMMENT 'Hex-encoded SRP verifier',
  `joindate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_ip` varchar(30) NOT NULL DEFAULT '0.0.0.0',
  `last_login` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_email` (`email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Battle.net Account System';

CREATE TABLE IF NOT EXISTS `battlenet_account_bans` (
  `id` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Battle.net account id',
  `bandate` bigint(40) NOT NULL DEFAULT '0',
  `unbandate` bigint(40) NOT NULL DEFAULT '0',
  `bannedby` varchar(50) NOT NULL,
  `banreason` varchar(255) NOT NULL,
  `active` tinyint(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`,`bandate`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Battle.net Ban List';

ALTER TABLE `account`
  ADD COLUMN `battlenet_account` int(11) unsigned DEFAULT NULL COMMENT 'Linked battlenet_accounts.id',
  ADD COLUMN `battlenet_index` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Game account slot within the Battle.net account',
  ADD COLUMN `session_key_bnet` varchar(128) DEFAULT NULL COMMENT 'Hex-encoded 64-byte world session key',
  ADD UNIQUE KEY `uk_bnet_acc` (`battlenet_account`,`battlenet_index`);

CREATE TABLE IF NOT EXISTS `account_last_played_character` (
  `accountId` int(11) unsigned NOT NULL,
  `region` tinyint(3) unsigned NOT NULL,
  `battlegroup` tinyint(3) unsigned NOT NULL,
  `realmId` int(11) unsigned NOT NULL,
  `characterName` varchar(12) NOT NULL,
  `characterGuid` bigint(20) unsigned NOT NULL,
  `lastPlayedTime` int(11) unsigned NOT NULL,
  PRIMARY KEY (`accountId`,`region`,`battlegroup`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Last character a game account played, per sub-region';

-- End of migration.
END IF;
END??
DELIMITER ;
CALL add_migration();
DROP PROCEDURE IF EXISTS add_migration;
