INSERT INTO `migrations` VALUES ('20260930010000');

-- Channel.cpp treats the client's flag 0x40000 as an LFG channel.
UPDATE `dbc_chatchannels` SET `Flags` = 0 WHERE `ID` = 24;
