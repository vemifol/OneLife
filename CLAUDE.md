# OneLife

Core game source repository. Contains the client, server, and shared utilities.

## Directory Structure

| Folder | Purpose |
|--------|---------|
| `gameSource/` | All client-side game logic (C++): game loop, UI pages, rendering, pathfinding, networking |
| `server/` | Headless game server (C++): world management, player physics, game state, client connections |
| `commonSource/` | Shared between client and server: `sayLimit.cpp/h`, `fractalNoise.cpp/h` |
| `build/` | Build scripts |
| `documentation/` | Documentation files |
| `scripts/` | Helper shell scripts |

## UI Page System

- `PageComponent` (`gameSource/PageComponent.h`) — base class for all UI elements. Tree structure: components contain sub-components via `addComponent()`. Events and draw calls cascade down via `base_*` methods; subclasses override `step()`, `draw()`, `pointerDown()`, `keyDown()`, etc.
- `GamePage` (`gameSource/GamePage.h`) — extends `PageComponent` for full screens. Adds signals (`setSignal("nextPage")`), status messages, waiting/spinner icon, and lifecycle hooks (`makeActive()`/`makeNotActive()`).
- `game.cpp` — drives the active `GamePage` each frame (`base_step()`, `base_draw()`), routes input, and switches pages when a signal is detected via `checkSignal()`.

Pages: `LivingLifePage`, `LoadingPage`, `SettingsPage`, `ExistingAccountPage`, `TwinPage`, `RebirthChoicePage`, `ReviewPage`, `PollPage`, `FinalMessagePage`, `GeneticHistoryPage`, `AutoUpdatePage`, `ServerActionPage`, `ExtendedMessagePage`, `Editor*Page` variants.

## Key Files

| File | Role |
|------|------|
| `server/protocol.txt` | Authoritative protocol specification — read this first |
| `server/server.cpp` (~25k lines) | Server main loop, `parseMessage()`, all game logic, `sendMessageToPlayer()` |
| `gameSource/LivingLifePage.cpp` (~30k lines) | Client main file: socket I/O, message dispatch, all player actions |
| `gameSource/LivingLifePage.h` | Client data structures: `LiveObject`, `GraveInfo`, `OwnerInfo`, `PointerHitRecord` |

---

## Server ↔ Client Protocol

### Overview
- Plain ASCII text over raw TCP
- Every message ends with `#`
- After login, messages are batched into frames ending with `FM\n#`
- Large messages are zlib-compressed and wrapped in `CM` envelopes
- Map chunks (`MC`) are sent as raw binary (zlib-compressed) after the `#`
- Default port: **5077**

---

### Connection & Login Handshake

1. **Server → Client: `SN`**
   ```
   SN
   current_players/max_players
   challenge_string
   required_version_number
   #
   ```
   Or `SHUTDOWN\n...\n#` / `SERVER_FULL\n...\n#` if applicable.

2. **Client → Server: `LOGIN`**
   ```
   LOGIN email pw_hash key_hash tutorial_num [twin_hash twin_count]#
   ```
   `pw_hash = HMAC_SHA1(password, challenge_string)`
   `key_hash = HMAC_SHA1(account_key, challenge_string)`

3. **Server → Client:** `ACCEPTED`, `REJECTED`, or `NO_LIFE_TOKENS`

---

### Message Framing

After login, the server batches all updates for one time-step and terminates with:
```
FM
#
```
The client collects all messages until `FM`, then releases them atomically. Map chunks, PONGs, and flight destinations bypass the frame queue.

---

### Server → Client Messages

| Tag | Meaning |
|-----|---------|
| `CM` | Compressed envelope wrapping any other message |
| `MC` | Full binary map chunk (zlib-compressed grid of object IDs) |
| `PU` | Full player state (position, held object, clothing, age, heat) |
| `PM` | Movement start: path deltas, ETA, truncation flag |
| `PO` | Player IDs that moved out of render range |
| `PS` | Speech text from a player |
| `LS` | Speech text at a world coordinate (signs read aloud) |
| `PE` | Emotion display for a player |
| `MX` | Map cell changes (new floor/object ID, optional movement animation) |
| `FX` | Food store, capacity, last eaten object, move speed, yum data |
| `HX` | Heat value, food drain time, indoor bonus |
| `LN` | Player lineage chain back to Eve |
| `NM` | Player first/last name |
| `AP` | Apocalypse pending |
| `AD` | Apocalypse over |
| `DY` | Player mortally wounded |
| `HE` | Player healed |
| `MN` | Monument bell rung at a location |
| `GV` | Grave at x,y belongs to player id |
| `GM` | Grave moved from one position to another |
| `GO` | Pre-existing grave info (response to client `GRAVE` request) |
| `OW` | Owner list for a map position |
| `VS` | Valley y-spacing and offset for this player |
| `CU` | Cursed players and their curse levels |
| `CX` | Player curse token count changed |
| `CS` | Player excess curse score |
| `FD` | Player flying to a destination (teleport) |
| `VU` | VOG mode update at a coordinate |
| `PH` | Photo signature response |
| `SD` | Forced shutdown — client should close |
| `MS` | Server-wide broadcast message |
| `FL` | Player facing direction flip |
| `CR` | Client's craving food type and yum bonus |
| `PONG` | Response to client `PING` |
| `FM` | End of one server time-step batch |

---

### Client → Server Messages

| Message | Meaning |
|---------|---------|
| `KA x y#` | Keep-alive |
| `MOVE xs ys @seq xd0 yd0 ... xdN ydN#` | Path movement request |
| `USE x y#` | Use/pick-up at grid cell x,y |
| `DROP x y c#` | Drop held item (c=-1 ground, 0-5 clothing slot) |
| `SELF x y i#` | Use held item on self (eat, remove clothing slot i) |
| `BABY x y [id]#` | Pick up baby near x,y |
| `UBABY x y i [id]#` | Use held item on another player (feed, clothe, heal) |
| `REMV x y i#` | Remove item from container (index i, -1 for top) |
| `SREMV x y c i#` | Remove item from worn clothing slot c, container index i |
| `KILL x y [id]#` | Use deadly object on player at x,y |
| `JUMP x y#` | Baby jumps out of adult's arms |
| `EMOT x y e#` | Display emotion e |
| `DIE x y#` | Sudden infant death |
| `GRAVE x y#` | Request info about grave at x,y |
| `OWNER x y#` | Request owner list for x,y |
| `FORCE x y#` | Acknowledge forced-position sync from server |
| `PING x y unique_id#` | Request PONG |
| `SAY 0 0 text#` | Send speech |
| `MAP x y#` | Request map chunk centered on x,y |
| `TRIGGER n#` | Fire server-side trigger n |
| `BUG v text#` | Bug report |
| `FLIP x y#` | Request facing flip toward x,y |
| `VOGS/VOGN/VOGP/VOGM/VOGI/VOGT/VOGX ...#` | VOG (view-only ghost) mode commands |
| `PHOTO x y seq#` | Request photo approval/signature |
| `PHOID x y photo_id#` | Post a photo ID |
| `LOGIN ...#` | Login after receiving SN |

---

### Container Object Format

Object IDs in `PU` and `MX` may encode containers:
```
containerID,containedID1,containedID2,...
```
Sub-containers use colon delimiters:
```
containerID,subContainerID1:subContainedID1:subContainedID2,containedID2,...
```
Max two layers deep.

---

## Client Networking — Key Functions

All in `gameSource/LivingLifePage.cpp`:

| Function | Location | Purpose |
|----------|----------|---------|
| `readServerSocketFull()` | ~line 2043 | Reads all available bytes from socket into `serverSocketBuffer` |
| `getNextServerMessageRaw()` | ~line 2557 | Extracts one `#`-terminated message; handles `MC`/`CM` |
| `getNextServerMessage()` | ~line 2695 | Wraps raw; queues messages until `FM`, bypasses for `MC`/`PONG`/`FD` |
| `sendToServerSocket()` | ~line 2084 | Single send wrapper; tracks byte counts, handles disconnect |
| `getMessageType()` | ~line 2384 | Parses first line → enum |
| `step()` | ~line 15206 | Main frame loop: reads socket, dispatches all server messages |
| `applyReceiveOffset()` | — | Converts server world coords to local coords |
| `sendX()` / `sendY()` | — | Reverse-applies offset when sending coords to server |

Socket primitives declared in `minorGems/game/game.h`, implemented in `minorGems/game/platforms/SDL/gameSDL.cpp`:
- `openSocketConnection(addr, port)`
- `sendToSocket(handle, data, len)`
- `readFromSocket(handle, buffer, len)`
- `closeSocket(handle)`

---

## Server Networking — Key Functions

All in `server/server.cpp`:

| Function | Location | Purpose |
|----------|----------|---------|
| `main()` | ~line 13335 | Startup: creates `SocketServer` on port 5077, enters event loop |
| `parseMessage()` | ~line 2313 | Tokenizes raw client message → `ClientMessage` struct with enum |
| `sendMessageToPlayer()` | ~line 10867 | Single send wrapper; auto-wraps large messages in `CM` |

Message dispatch is a large `else if` chain after `parseMessage()`. Key handlers:
- `MOVE` ~line 16686 — path validation, sends `PM` to nearby players
- `SAY` ~line 17283 — text filtering, broadcasts `PS`
- `USE` ~line 17762 — largest handler; resolves transitions, containment, pickups
- `DROP` ~line 20553 — drop to ground or container
- `KILL` ~line 17688 — validates deadly object, applies damage
- `MAP` ~line 15780 — sends map chunk
- `PING` ~line 16108 — immediately sends `PONG`
- `FORCE` ~line 16091 — unblocks position-sensitive action handling
