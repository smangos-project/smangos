-- key: id
SELECT "ID" AS "id",
       "Faction" AS "faction_id",
       "Flags" AS "faction_flags",
       "FactionGroup" AS "our_mask",
       "FriendGroup" AS "friendly_mask",
       "EnemyGroup" AS "hostile_mask",
       "Enemies1" AS "enemy_faction1",
       "Enemies2" AS "enemy_faction2",
       "Enemies3" AS "enemy_faction3",
       "Enemies4" AS "enemy_faction4",
       "Friend1" AS "friend_faction1",
       "Friend2" AS "friend_faction2",
       "Friend3" AS "friend_faction3",
       "Friend4" AS "friend_faction4"
FROM "dbc_factiontemplate"
