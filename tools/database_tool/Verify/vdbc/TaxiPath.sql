-- key: ID
SELECT d."ID" AS "ID",
       d."FromTaxiNode" AS "FromTaxiNode",
       d."ToTaxiNode" AS "ToTaxiNode",
       d."Cost" AS "Cost"
FROM "dbc_taxipath" d
