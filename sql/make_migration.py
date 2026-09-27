import sys
import time

DATABASES = ('characters', 'logon', 'logs', 'world')

WORLD_TEMPLATE = """INSERT INTO `migrations` VALUES ('{date}');

-- Add your query below.



-- End of migration.
"""

PROCEDURE_TEMPLATE = """DROP PROCEDURE IF EXISTS add_migration;
DELIMITER ??
CREATE PROCEDURE `add_migration`()
BEGIN
DECLARE v INT DEFAULT 1;
SET v = (SELECT COUNT(*) FROM `migrations` WHERE `id`='{date}');
IF v = 0 THEN
INSERT INTO `migrations` VALUES ('{date}');
-- Add your query below.



-- End of migration.
END IF;
END??
DELIMITER ;
CALL add_migration();
DROP PROCEDURE IF EXISTS add_migration;
"""

if len(sys.argv) != 2 or sys.argv[1] not in DATABASES:
    print(f"Usage: make_migration.py <{'|'.join(DATABASES)}>")
    sys.exit(1)

db = sys.argv[1]
date = time.strftime('%Y%m%d%H%M%S', time.gmtime())  # UTC DATE
fpath = f"migrations/{date}_{db}.sql"

template = WORLD_TEMPLATE if db == 'world' else PROCEDURE_TEMPLATE
sql_content = template.format(date=date)

try:
    with open(fpath, 'w', newline='\n') as output_file:
        output_file.write(sql_content)
    print(f"File created: {fpath}")
except Exception as e:
    print(f"Failed to create file: {e}")
    sys.exit(1)
