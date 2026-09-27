INSERT INTO `migrations` VALUES ('20250509222846');

-- Add your query below.
 /*
World of Warcraft Client Patch 1.3.0 (2005-03-22)
- A new type of turtle is now found on the beaches of northern Tanaris 
  in the level 42-43 range. They can be skinned for turtle scales.
*/
 -- Steeljaw Snappers were added in 1.3.
 -- https://web.archive.org/web/20050528235051/http://wow.allakhazam.com/news/sdetail5329.html
 UPDATE `creature_template` SET `patch` =1 WHERE `entry` =14123;

UPDATE `creature` SET `patch_min` =1 WHERE `id` =14123;

-- End of migration.
;
