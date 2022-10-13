//
// Copyright (c) 2012-2018 Michele Segata <segata@ccs-labs.org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "plexe/scenarios/ShockwaveScenario.h"

using namespace veins;

namespace plexe {

Define_Module(ShockwaveScenario);

int ShockwaveScenario::shockwaveCarsCount = 0;

void ShockwaveScenario::initialize(int stage)
{

    BaseScenario::initialize(stage);

    if (stage == 1) {
        // get the speed
        jamSpeed = par("jamSpeed").doubleValue() / 3.6;
        freeSpeed = par("freeSpeed").doubleValue() / 3.6;
        jamDeceleration = par("jamDeceleration").doubleValue();
        jamDuration = &par("jamDuration");

        startJam = new cMessage("startJam");
        stopJam = new cMessage("stopJam");
        checkSpeed = new cMessage("checkSpeed");

        shockwaveCarIndex = shockwaveCarsCount;
        shockwaveCarsCount++;

        plexeTraciVehicle->setActiveController(plexe::ACC);
        plexeTraciVehicle->setFixedLane(shockwaveCarIndex);

        traciVehicle->setColor(TraCIColor(100, 100, 100, 255));

        scheduleAt(simTime() + SimTime(uniform(0.1, 3.1)), startJam);
    }
}

ShockwaveScenario::~ShockwaveScenario()
{
    if (startJam) {
        cancelAndDelete(startJam);
        startJam = nullptr;
    }
    if (stopJam) {
        cancelAndDelete(stopJam);
        stopJam = nullptr;
    }
    if (checkSpeed) {
        cancelAndDelete(checkSpeed);
        checkSpeed = nullptr;
    }
}

void ShockwaveScenario::handleSelfMsg(cMessage* msg)
{
    BaseScenario::handleSelfMsg(msg);
    if (msg == startJam) {
        // start decelerating due to the traffic jam
        plexeTraciVehicle->setFixedAcceleration(1, -jamDeceleration);
        scheduleAt(simTime() + SimTime(0.1), checkSpeed);
    }
    if (msg == checkSpeed) {
        if (mobility->getSpeed() <= jamSpeed) {
            // disable deceleration
            plexeTraciVehicle->setFixedAcceleration(0, 0);
            // set jam speed as cruise speed
            plexeTraciVehicle->setCruiseControlDesiredSpeed(jamSpeed);
            // schedule stop jam
            scheduleAt(simTime() + jamDuration->doubleValue(), stopJam);
        }
        else {
            scheduleAt(simTime() + SimTime(0.1), checkSpeed);
        }
    }
    if (msg == stopJam) {
        // accelerate to reach free speed. end of traffic jam
        plexeTraciVehicle->setCruiseControlDesiredSpeed(freeSpeed);
        // schedule start of jam
        scheduleAt(simTime() + jamDuration->doubleValue(), startJam);
    }
}

}
