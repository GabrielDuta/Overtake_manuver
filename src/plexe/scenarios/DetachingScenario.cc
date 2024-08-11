//
// Copyright (C) 2018-2023 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#include "plexe/scenarios/DetachingScenario.h"

using namespace veins;

namespace plexe {

Define_Module(DetachingScenario);

void DetachingScenario::initialize(int stage)
{

    BaseScenario::initialize(stage);

    if (stage == 0)
        // get pointer to application
        appl = FindModule<DetachingApp*>::findSubModule(getParentModule());

    if (stage == 2) {
        // average speed
        leaderSpeed = par("leaderSpeed").doubleValue() / 3.6;

        if (positionHelper->isLeader()) {
            // set base cruising speed
            plexeTraciVehicle->setCruiseControlDesiredSpeed(leaderSpeed);
        }
        else {
            // let the follower set a higher desired speed to stay connected
            // to the leader when it is accelerating
            plexeTraciVehicle->setCruiseControlDesiredSpeed(leaderSpeed + 10);

            // Check if we are last vehicle in the platoon
            if(positionHelper->isLast()) {
              // Prepare self messages
              startBraking = new cMessage("Start braking");
              checkDistance = new cMessage("Check distance");
              
              scheduleAt(10, startBraking);
            }
        }
    }
}

void DetachingScenario::handleSelfMsg(cMessage *msg) {
  if(msg == startBraking) {
    // Increment CACC spacing (set to 15m)
    plexeTraciVehicle->setCACCConstantSpacing(15.0);
    traciVehicle->setColor(TraCIColor(100, 100, 100, 255));

    // Start cheking distance
    scheduleAt(simTime() + 0.1, checkDistance);
  }
  else if(msg == checkDistance) {
    double distance = 0;
    double speed = 0;
    plexeTraciVehicle->getRadarMeasurements(distance, speed);
    LOG << "Leaving vehicle now at: " << distance << " meters" << endl;

    if(distance > 14.9) {
      plexeTraciVehicle->setActiveController(ACC);
      plexeTraciVehicle->setACCHeadwayTime(1.2);
      traciVehicle->setColor(TraCIColor(200, 200, 200, 255));

      appl->sendAbandonMessage();
    }
    else {
      scheduleAt(simTime() + 0.1, checkDistance);
    }
  }
}

} // namespace plexe
