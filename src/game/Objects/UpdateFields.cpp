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

#include "UpdateFields.h"
#include "Log.h"
#include "ObjectGuid.h"
#include <array>

#include "UpdateFields_1_12_1.cpp"

UpdateFieldData const* UpdateFields::GetUpdateFieldDataByName(char const* name)
{
    for (const auto& itr : g_updateFieldsData)
    {
        if (strcmp(itr.name, name) == 0)
            return &itr;
    }
    return nullptr;
}

UpdateFieldData const* UpdateFields::GetUpdateFieldDataByTypeMaskAndOffset(uint8 objectTypeMask, uint16 offset)
{
    for (auto const& itr : g_updateFieldsData)
    {
        if ((itr.objectTypeMask & objectTypeMask) == 0)
            continue;

        if (offset == itr.offset || (offset > itr.offset && offset < (itr.offset + itr.size)))
            return &itr;
    }
    return nullptr;
}
