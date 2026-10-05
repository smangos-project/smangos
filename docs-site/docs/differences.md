# Differences

If you are familiar with emulator conventions, there are some design differences to be aware of before you start.

## World Database

The world database runs in a [doltlite](https://github.com/dolthub/doltlite.git) database rather than MySQL. `doltlite` is a version-controlled sqlite database that allows us to version control world data directly rather than rely excusively on migrations.

The world database contains both traditional server tables and dbc tables that the server reads directly rather than `.db2` files.

The database will be built in the below steps, and will have a slightly different schema from traditional vmangos to reduce redundancy and treat server and client tables in the same system.

We do not have a separate hotfix database and do not plan to add one. Instead, the plan is to use `doltlite` branches to compute differences between a published `.db2` patch and the live server data. This way, the user doesn't need to handle the flow of moving patches between the hotfix database and the client tables when new `.db2`s are distributed.

Login and character databases still use standard MySQL.

## Client

The client uses our [client extensions](https://github.com/smangos-project/smangos/tree/development/client) to load, leveraging a hooking library rather than standard arctium-style patching outside of initialization.

These extensions disable CASC by default, and instead builds a filesystem `Content` directory that the client reads its data from. This is intended for development, and for production you may re-enable CASC or use any other archive format of your choice.

## Server

The `server` is a single executable that can run both `bnetserver` and `mangosd` as services via command-line parameters. These servies can still run separately just like normal.

## Certificates

Certificate checking is disabled in the client to simplify development.

## Paths

Most of smangos paths are hardcoded to simplify development. The server and tools will target these paths automatically.

- `.client` contains the client used in development
- `.world` contains the world doltlite database
- `.server` contains server configuration and data