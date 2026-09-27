INSERT INTO `migrations` VALUES ('20260411152432');

DELETE FROM `pool_pool` WHERE `pool_id` IN (1235, 3242, 3241, 3243, 9804);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 1' WHERE `entry` IN (1235, 3242, 3241, 3243, 9804);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 1' WHERE `pool_entry` IN (1235, 3242, 3241, 3243, 9804);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4960+1, 1, 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1235, 4960+1, 0, 'Azshara - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (3242, 4960+1, 0, 'Azshara - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (3241, 4960+1, 0, 'Azshara - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (3243, 4960+1, 0, 'Azshara - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (9804, 4960+1, 0, 'Azshara - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (9808, 3238, 9809, 9810, 9823);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 2' WHERE `entry` IN (9808, 3238, 9809, 9810, 9823);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 2' WHERE `pool_entry` IN (9808, 3238, 9809, 9810, 9823);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4960+2, 1, 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (9808, 4960+2, 0, 'Azshara - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (3238, 4960+2, 0, 'Azshara - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (9809, 4960+2, 0, 'Azshara - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (9810, 4960+2, 0, 'Azshara - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (9823, 4960+2, 0, 'Azshara - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (9813, 3239, 9816, 9824, 9806);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 3' WHERE `entry` IN (9813, 3239, 9816, 9824, 9806);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 3' WHERE `pool_entry` IN (9813, 3239, 9816, 9824, 9806);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4960+3, 1, 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (9813, 4960+3, 0, 'Azshara - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (3239, 4960+3, 0, 'Azshara - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (9816, 4960+3, 0, 'Azshara - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (9824, 4960+3, 0, 'Azshara - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (9806, 4960+3, 0, 'Azshara - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (3244, 3240, 9807, 3984, 9820);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 4' WHERE `entry` IN (3244, 3240, 9807, 3984, 9820);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 4' WHERE `pool_entry` IN (3244, 3240, 9807, 3984, 9820);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4960+4, 1, 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3244, 4960+4, 0, 'Azshara - Group 4 - Truesilver Deposit / Rich Thorium Vein', 0), (3240, 4960+4, 0, 'Azshara - Group 4 - Truesilver Deposit / Rich Thorium Vein', 0), (9807, 4960+4, 0, 'Azshara - Group 4 - Truesilver Deposit / Rich Thorium Vein', 0), (3984, 4960+4, 0, 'Azshara - Group 4 - Truesilver Deposit / Rich Thorium Vein', 0), (9820, 4960+4, 0, 'Azshara - Group 4 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (3245, 3247, 3246, 9812, 3988);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 5' WHERE `entry` IN (3245, 3247, 3246, 9812, 3988);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 5' WHERE `pool_entry` IN (3245, 3247, 3246, 9812, 3988);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4960+5, 1, 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3245, 4960+5, 0, 'Azshara - Group 5 - Truesilver Deposit / Rich Thorium Vein', 0), (3247, 4960+5, 0, 'Azshara - Group 5 - Truesilver Deposit / Rich Thorium Vein', 0), (3246, 4960+5, 0, 'Azshara - Group 5 - Truesilver Deposit / Rich Thorium Vein', 0), (9812, 4960+5, 0, 'Azshara - Group 5 - Truesilver Deposit / Rich Thorium Vein', 0), (3988, 4960+5, 0, 'Azshara - Group 5 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (3249, 3248, 9814, 9821, 3982);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 6' WHERE `entry` IN (3249, 3248, 9814, 9821, 3982);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 6' WHERE `pool_entry` IN (3249, 3248, 9814, 9821, 3982);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4960+6, 1, 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 6 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3249, 4960+6, 0, 'Azshara - Group 6 - Truesilver Deposit / Rich Thorium Vein', 0), (3248, 4960+6, 0, 'Azshara - Group 6 - Truesilver Deposit / Rich Thorium Vein', 0), (9814, 4960+6, 0, 'Azshara - Group 6 - Truesilver Deposit / Rich Thorium Vein', 0), (9821, 4960+6, 0, 'Azshara - Group 6 - Truesilver Deposit / Rich Thorium Vein', 0), (3982, 4960+6, 0, 'Azshara - Group 6 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (3250, 9822, 9805, 3981);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 7' WHERE `entry` IN (3250, 9822, 9805, 3981);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 7' WHERE `pool_entry` IN (3250, 9822, 9805, 3981);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4960+7, 1, 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 7 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3250, 4960+7, 0, 'Azshara - Group 7 - Truesilver Deposit / Rich Thorium Vein', 0), (9822, 4960+7, 0, 'Azshara - Group 7 - Truesilver Deposit / Rich Thorium Vein', 0), (9805, 4960+7, 0, 'Azshara - Group 7 - Truesilver Deposit / Rich Thorium Vein', 0), (3981, 4960+7, 0, 'Azshara - Group 7 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (3251, 3253, 3252, 9815, 3983);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 8' WHERE `entry` IN (3251, 3253, 3252, 9815, 3983);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 8' WHERE `pool_entry` IN (3251, 3253, 3252, 9815, 3983);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4960+8, 1, 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 8 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3251, 4960+8, 0, 'Azshara - Group 8 - Truesilver Deposit / Rich Thorium Vein', 0), (3253, 4960+8, 0, 'Azshara - Group 8 - Truesilver Deposit / Rich Thorium Vein', 0), (3252, 4960+8, 0, 'Azshara - Group 8 - Truesilver Deposit / Rich Thorium Vein', 0), (9815, 4960+8, 0, 'Azshara - Group 8 - Truesilver Deposit / Rich Thorium Vein', 0), (3983, 4960+8, 0, 'Azshara - Group 8 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (9818, 9817, 9811, 9819, 9803);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 9' WHERE `entry` IN (9818, 9817, 9811, 9819, 9803);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 9' WHERE `pool_entry` IN (9818, 9817, 9811, 9819, 9803);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4960+9, 1, 'Truesilver Deposit / Rich Thorium Vein - Azshara - Group 9 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (9818, 4960+9, 0, 'Azshara - Group 9 - Truesilver Deposit / Rich Thorium Vein', 0), (9817, 4960+9, 0, 'Azshara - Group 9 - Truesilver Deposit / Rich Thorium Vein', 0), (9811, 4960+9, 0, 'Azshara - Group 9 - Truesilver Deposit / Rich Thorium Vein', 0), (9819, 4960+9, 0, 'Azshara - Group 9 - Truesilver Deposit / Rich Thorium Vein', 0), (9803, 4960+9, 0, 'Azshara - Group 9 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` IN (3603, 3604, 3605, 3606, 3607, 3608, 3609, 3610, 3611);

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 1 - Master' WHERE `entry` = 3942;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 1' WHERE `pool_entry` = 3942;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 2 - Master' WHERE `entry` = 3943;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 2' WHERE `pool_entry` = 3943;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 3 - Master' WHERE `entry` = 3944;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 3' WHERE `pool_entry` = 3944;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 4 - Master' WHERE `entry` = 3945;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 4' WHERE `pool_entry` = 3945;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 5 - Master' WHERE `entry` = 3946;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 5' WHERE `pool_entry` = 3946;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 6 - Master' WHERE `entry` = 3947;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 6' WHERE `pool_entry` = 3947;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 7 - Master' WHERE `entry` = 3948;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 7' WHERE `pool_entry` = 3948;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 8 - Master' WHERE `entry` = 3949;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 8' WHERE `pool_entry` = 3949;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 9 - Master' WHERE `entry` = 3950;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 9' WHERE `pool_entry` = 3950;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 10 - Master' WHERE `entry` = 3951;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 10' WHERE `pool_entry` = 3951;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 11 - Master' WHERE `entry` = 3952;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 11' WHERE `pool_entry` = 3952;

UPDATE `pool_template` SET `description` = 'Dreamfoil - Azshara - Group 12 - Master' WHERE `entry` = 3953;

UPDATE `pool_gameobject` SET `description` = 'Dreamfoil - Azshara - Group 12' WHERE `pool_entry` = 3953;

UPDATE `pool_template` SET `description` = 'Azshara - Azsharite Formation' WHERE `entry` IN (3600, 3762, 3763, 3764, 3765, 3766, 3767, 3768, 3769, 3770, 3771, 3772, 3773, 3774, 3775, 3776, 3777, 3778, 3779, 3780, 3781, 3782, 3783, 3784, 3785, 3786, 3787, 3788, 3789, 3790, 3791, 3792, 3793, 3794, 3795, 3796, 3797, 3798, 3799, 3800, 3801, 3802, 3803, 3804, 3805, 3806, 3807, 3808, 3809, 3810, 3811, 3812, 3813, 3814);

-- End of migration.
;
