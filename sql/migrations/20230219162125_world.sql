INSERT INTO `migrations` VALUES ('20230219162125');

-- Add your query below.
 -- Damage Variance
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4050;

-- Cenarion Caretaker (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4052;

-- Cenarion Druid (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4056;

-- Mirkfallon Keeper (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4061;

-- Mirkfallon Dryad (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4814;

-- Twilight Elementalist (was 0.17)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =5915;

-- Brother Ravenoak (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =8518;

-- Rynthariel the Keymaster (was 0.14)
 -- Damage Multipliers
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2151 AND `patch` IN (0);

-- Moon Priestess Amara (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3571 AND `patch` IN (0);

-- Teldrassil Sentinel (was 5.55)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4050 AND `patch` IN (0);

-- Cenarion Caretaker (was 3.7)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4052 AND `patch` IN (0);

-- Cenarion Druid (was 3.55)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4056 AND `patch` IN (0);

-- Mirkfallon Keeper (was 3.35)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4061 AND `patch` IN (0);

-- Mirkfallon Dryad (was 3.45)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4262 AND `patch` IN (0, 2);

-- Darnassus Sentinel (was 2.35)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4409 AND `patch` IN (0);

-- Gatekeeper Kordurus (was 3.25)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =5915 AND `patch` IN (0);

-- Brother Ravenoak (was 3.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6086 AND `patch` IN (0);

-- Auberdine Sentinel (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6287 AND `patch` IN (0);

-- Radnaal Maneweaver (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =8518 AND `patch` IN (0);

-- Rynthariel the Keymaster (was 3.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10604 AND `patch` IN (0);

-- Huntress Nhemai (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10606 AND `patch` IN (0);

-- Huntress Yaeliura (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12160 AND `patch` IN (0);

-- Shadowglen Sentinel (was 1.9)
 -- Armor Multipliers
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3571 AND `patch` IN (0);

-- Teldrassil Sentinel (was 1.4)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4262 AND `patch` IN (0, 2);

-- Darnassus Sentinel (was 1.2)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =12160 AND `patch` IN (0);

-- Shadowglen Sentinel (was 1.4)
 -- Nature Resistance
 UPDATE `creature_template` SET `nature_res` =52 WHERE `entry` =4050 AND `nature_res` =0;

-- Cenarion Caretaker
 UPDATE `creature_template` SET `nature_res` =54 WHERE `entry` =4052 AND `nature_res` =0;

-- Cenarion Druid
 UPDATE `creature_template` SET `nature_res` =80 WHERE `entry` =4056 AND `nature_res` =0;

-- Mirkfallon Keeper
 -- End of migration.
;
