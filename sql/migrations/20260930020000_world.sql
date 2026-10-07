INSERT INTO `migrations` VALUES ('20260930020000');

-- Deeprun Tram paths: ElevatorTransport expects the vanilla axis order.
UPDATE `dbc_transportanimation` SET `Pos1` = -`Pos2`, `Pos2` = `Pos1` WHERE `TransportID` IN (176080, 176081, 176082, 176083, 176084);
UPDATE `dbc_transportanimation` SET `Pos1` = `Pos2`, `Pos2` = `Pos1` WHERE `TransportID` = 176085;
