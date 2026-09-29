INSERT INTO `migrations` VALUES ('20260929260000');

-- Taxi nodes a new character of the race knows; a race without a row starts with none.
CREATE TABLE `race_taxi_mask` (
  `race` INTEGER NOT NULL PRIMARY KEY,
  `taxi_mask` INTEGER NOT NULL DEFAULT 0
);

INSERT INTO `race_taxi_mask` (`race`, `taxi_mask`) VALUES
(1, 2),
(2, 4194304),
(3, 32),
(4, 100663296),
(5, 1024),
(6, 2097152),
(7, 32),
(8, 4194304),
(9, 0);
