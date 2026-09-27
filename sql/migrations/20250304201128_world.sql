INSERT INTO `migrations` VALUES ('20250304201128');

-- Add your query below.
 -- Add texts to Silithus PvP Event.
 UPDATE `broadcast_text` SET `chat_type` =1 WHERE `entry` IN (13469, 13470, 13476, 13477, 13478, 13479, 13480, 13481);

-- End of migration.
;
