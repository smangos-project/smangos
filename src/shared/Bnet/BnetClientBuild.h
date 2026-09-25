#ifndef SHARED_BNETCLIENTBUILD_H
#define SHARED_BNETCLIENTBUILD_H

#include "Common.h"

// The one modern client this server speaks. Independent of SUPPORTED_CLIENT_BUILD, which still
// selects the legacy data layouts.
constexpr uint32 BNET_CLIENT_BUILD = 58797;
constexpr uint32 BNET_CLIENT_VERSION_MAJOR = 1;
constexpr uint32 BNET_CLIENT_VERSION_MINOR = 15;
constexpr uint32 BNET_CLIENT_VERSION_REVISION = 6;

#endif
