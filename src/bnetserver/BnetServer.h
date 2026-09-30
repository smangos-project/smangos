#ifndef BNETSERVER_BNETSERVER_H
#define BNETSERVER_BNETSERVER_H

// Starts the login portal and the RPC port; returns once both listen. Needs sConfig, sLog,
// LoginDatabase and sBnetRealmList ready.
bool BnetServerStart();

// Closes both ports and joins bnetserver's threads. Safe to call when Start failed or was not called.
void BnetServerStop();

#endif
