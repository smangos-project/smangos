/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "Anticheat.h"
#include "Log.h"

AnticheatManager* AnticheatManager::instance()
{
    static AnticheatManager i;
    return &i;
}

AnticheatManager* GetAnticheatLib()
{
    return AnticheatManager::instance();
}

#include "World.h"
#include "WorldSession.h"

#include "MovementAnticheat/MovementAnticheat.h"

void AnticheatManager::LoadAnticheatData()
{
}

MovementAnticheat* AnticheatManager::CreateAnticheatFor(Player* player)
{
    MovementAnticheat* cd = new MovementAnticheat(player);
    cd->Init();
    return cd;
}

