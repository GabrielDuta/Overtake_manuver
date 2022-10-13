//
// Copyright (C) 2012-2022 Michele Segata <segata@ccs-labs.org>
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

#include "plexe/scenarios/IntersectionMergeScenario.h"

namespace plexe {

Define_Module(IntersectionMergeScenario);

void IntersectionMergeScenario::initialize(int stage)
{

    ManeuverScenario::initialize(stage);

    if (stage == 2) {
        prepareManeuverCars();
    }
}

void IntersectionMergeScenario::prepareManeuverCars()
{
    if (positionHelper->isLeader() && positionHelper->getPlatoonId() == PLATOON_ID_A) {
        plexeTraciVehicle->setCruiseControlDesiredSpeed(positionHelper->getPlatoonSpeed());
        plexeTraciVehicle->setActiveController(positionHelper->getController());
        plexeTraciVehicle->setFixedLane(0);
        app->setPlatoonRole(PlatoonRole::LEADER);
    }
    else if (positionHelper->isLeader() && positionHelper->getPlatoonId() == PLATOON_ID_B) {
        plexeTraciVehicle->setCruiseControlDesiredSpeed(positionHelper->getPlatoonSpeed());
        plexeTraciVehicle->setActiveController(plexe::FAKED_CACC);
        plexeTraciVehicle->setFixedLane(0);
        app->setPlatoonRole(PlatoonRole::LEADER);
    }
    else if (positionHelper->isLeader() && positionHelper->getPlatoonId() == PLATOON_ID_C) {
        plexeTraciVehicle->setCruiseControlDesiredSpeed(positionHelper->getPlatoonSpeed());
        plexeTraciVehicle->setActiveController(positionHelper->getController());
        plexeTraciVehicle->setFixedLane(0);
        app->setPlatoonRole(PlatoonRole::LEADER);
        startManeuver = new cMessage();
        scheduleAt(simTime() + SimTime(1), startManeuver);
    }
    else {
        // these are the followers which are already in the platoon
        plexeTraciVehicle->setCruiseControlDesiredSpeed(130.0 / 3.6);
        plexeTraciVehicle->setActiveController(positionHelper->getController());
        plexeTraciVehicle->setFixedLane(0);
        app->setPlatoonRole(PlatoonRole::FOLLOWER);
    }

}

void IntersectionMergeScenario::handleSelfMsg(cMessage* msg)
{

    // this takes car of feeding data into CACC and reschedule the self message
    ManeuverScenario::handleSelfMsg(msg);

    LOG << "Starting the intersection merge maneuver\n";
    if (msg == startManeuver) app->startIntersectionMergeManeuver(PLATOON_ID_B, positions.getMemberId(PLATOON_ID_B,  0));
}

} // namespace plexe
