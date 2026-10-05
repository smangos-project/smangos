# Server

**Code:** `src/game` (gameplay), `src/mangosd` (world server), `src/bnetserver` (login), `src/shared` (networking, crypto, database)

A vmangos-based server speaking the 1.15 Classic Era protocol. It builds and runs on Windows and Linux.

| Feature | Where |
|---|---|
| Spells and auras | `src/game/Spells` |
| Transports | `src/game/Transports` |
| Flight paths | `src/game/Handlers/TaxiHandler.cpp`, `src/game/Objects/PlayerTaxi.cpp` |
| Vendors and trainers | `src/game/Handlers/NPCHandler.cpp` |
| Auction house | `src/game/AuctionHouse`, `src/game/Handlers/AuctionHouseHandler.cpp` |
| Modern protocol packets | `src/game/Server/Packets`, `src/game/Server/Protocol` |
| Login (Battle.net REST and RPC) | `src/bnetserver` |

## What you can do

- Log into the server with the real 1.15 client, pick a character and play: combat, spells, vendors, trainers, flight paths, transports, auctions.
- Use the Battle.net launcher login flow, which works against `bnetserver`.
- Run it on Linux or Windows; see [Setup](../setup.md).
