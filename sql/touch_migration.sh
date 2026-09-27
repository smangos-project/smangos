#!/usr/bin/env bash

DB="$1"
case "$DB" in
    characters|logon|logs|world) ;;
    *)
        echo "Usage: touch_migration.sh <characters|logon|logs|world>"
        exit 1
        ;;
esac

DATE=$(date -u +%Y%m%d%H%M%S) # UTC DATE
FPATH="migrations/${DATE}_${DB}.sql"

if [ "$DB" = "world" ]; then
    cat <<EOF > "$FPATH"
INSERT INTO \`migrations\` VALUES ('$DATE');

-- Add your query below.



-- End of migration.
EOF
else
    cat <<EOF > "$FPATH"
DROP PROCEDURE IF EXISTS add_migration;
DELIMITER ??
CREATE PROCEDURE \`add_migration\`()
BEGIN
DECLARE v INT DEFAULT 1;
SET v = (SELECT COUNT(*) FROM \`migrations\` WHERE \`id\`='$DATE');
IF v = 0 THEN
INSERT INTO \`migrations\` VALUES ('$DATE');
-- Add your query below.



-- End of migration.
END IF;
END??
DELIMITER ;
CALL add_migration();
DROP PROCEDURE IF EXISTS add_migration;
EOF
fi

if [ $? -eq 0 ]; then
    echo "File created: $FPATH"
else
    echo "Error creating file: $FPATH"
fi
