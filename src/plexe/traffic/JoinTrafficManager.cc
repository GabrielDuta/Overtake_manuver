//
// Copyright (C) 2014-2022 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "JoinTrafficManager.h"

namespace plexe {

Define_Module(JoinTrafficManager);

void JoinTrafficManager::initialize(int stage)
{

    PlatoonsTrafficManager::initialize(stage);

    if (stage == 0) {

        insertJoinerMessage = new cMessage("");
        scheduleAt(platoonInsertTime + SimTime(5), insertJoinerMessage);
    }
}

void JoinTrafficManager::handleSelfMsg(cMessage* msg)
{

    PlatoonsTrafficManager::handleSelfMsg(msg);

    if (msg == insertJoinerMessage) {
        insertJoiner();
    }
}

void JoinTrafficManager::insertJoiner()
{
    automated.position = 0;
    automated.lane = 2;
    addVehicleToQueue(0, automated);
    VehicleInfo vehicleInfo;
    vehicleInfo.id = 4;
    vehicleInfo.platoonId = -1;
    vehicleInfo.position = -1;
    vehicleInfo.controller = ACC;
    vehicleInfo.distance = 2;
    vehicleInfo.headway = 1.2;
    positions.vehicleInfo[4] = vehicleInfo;
}

JoinTrafficManager::~JoinTrafficManager()
{
    cancelAndDelete(insertJoinerMessage);
    insertJoinerMessage = nullptr;
}

} // namespace plexe
