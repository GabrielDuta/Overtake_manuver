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

#include "plexe/apps/OvertakeApp.h"
#include "plexe/maneuver/OvertakeManeuver.h"


namespace plexe {

OvertakeManeuver::OvertakeManeuver(OvertakeApp* app)
    : app(app)
    , positionHelper(app->getPositionHelper())
    , mobility(app->getMobility())
    , traci(app->getTraci())
    , traciVehicle(app->getTraciVehicle())
    , plexeTraci(app->getPlexeTraci())
    , plexeTraciVehicle(app->getPlexeTraciVehicle())
{
    start_speed = 0;
}


void OvertakeManeuver::prepareManeuver()
{
    VEHICLE_DATA data; 
    plexeTraciVehicle->getVehicleData(&data);

    if (start_speed == 0)
    {
        start_speed = data.speedX;
    }
    
    if (data.speedX >= start_speed - 1) {
        app->scheduleSelfMsg(simTime() + 0.1, new cMessage("manovra"));
        return;
    }

    startManeuver();
}

/**
 * Overtake methods:
 *  1 - Platoon B has enough space to overtake Platoon A
 *  2 - If Platoon C decellerates by 10% platoon B can overtake
 *  3 - Platoon B waits for Platoon C to pass
 */
void OvertakeManeuver::startManeuver()
{
    if (app->getDataSet(PLATOON_ID_A) == false || app->getDataSet(PLATOON_ID_C) == false)
    {
        app->sendRequestDataMessage(PLATOON_ID_A);
        app->sendRequestDataMessage(PLATOON_ID_C);
        app->scheduleSelfMsg(simTime() + 0.05, new cMessage("manovra"));
        return;
    }

    VEHICLE_DATA data; 
    plexeTraciVehicle->getVehicleData(&data);

    VehicleData platoonAData = app->getVehicleData(0);
    VehicleData platoonCData = app->getVehicleData(2);

    double epsilon_error = 10; // m
    double head_safety_distance = 10;
    // Data for the overtaking car
    //  Space to leave in front of platoon A after the overtake
    double frontSpace = (platoonAData.velocity * 3.6) * 3 / 10; // distanza di sicurezza per il platoon A dal platoon B quando B si immette nella corsia
    //  The total space needed for the overtake
    double totalSpaceForOvertake = (platoonAData.positionX - data.positionX) + frontSpace; // m

    /// Calculations to consider the acceleration of car B when overtaking to have more precise measurements
    //  Average speed of car B, necessary considering the acceleration during the overtake
    double averageSpeedOfB = (start_speed + data.speed) / 2; // m/s
    //  Speed difference between the platoon B and the platoon A
    //double deltaVsa = averageSpeedOfB - platoonAData.velocity; // m/s
    double deltaVsa = ((start_speed - platoonAData.velocity) + (data.speed - platoonAData.velocity)) / 2; // m/s

    //  Time to change acceleration to desired one (1.5 m/s^2); (2.5 m/s^2 is the maximum acceleration)
    double timeChange = (1.5 - data.acceleration) / 2.5; // s
    // Time taken to accelerate to start_speed for B
    //double timeAcc = ((start_speed - platoonAData.velocity) - (data.speed - platoonAData.velocity)) / start_speed + epsilon_error; // s
    double timeAcc = ((start_speed - data.speed) / 1.5); // s

    //  Distance covered in the acceleration phase
    double distanceAcc = deltaVsa * timeAcc; // m

    //  Total time and space needed for the overtake
    double timeOvertake = timeAcc; // s

    //  Total distance covered during the overtake
    double totalOvertakDistance = averageSpeedOfB * timeAcc; // m

    if (distanceAcc < totalSpaceForOvertake)
    {
        //  Remaining space for the overtake
        double remainingSpace = totalSpaceForOvertake - distanceAcc; // m
        //  Necessary time during the acceleration = 0 phase
        double timeConst = remainingSpace / (start_speed - platoonAData.velocity) + 1; // s

        timeOvertake += timeConst; // s
        totalOvertakDistance += start_speed * timeConst; // m
    }
    else if (distanceAcc > totalOvertakDistance)
    {
        timeOvertake = totalOvertakDistance / averageSpeedOfB; // s
    }

    //  Final position of the platoon B after the overtake
    double finalXPosition = data.positionX + totalOvertakDistance + epsilon_error;
    //double finalXPosition = data.positionX + (timeAcc * averageSpeedOfB) + (timeOvertake - timeAcc) * data.speed; // m

    /*
    // A print of all the precedent variables
    std::cout << "B x: " << data.positionX << std::endl;
    std::cout << "A x: " << platoonAData.positionX << std::endl;
    std::cout << "Total space for overtake: " << totalSpaceForOvertake << std::endl;
    std::cout << "Start speed of B: " << start_speed << std::endl;
    std::cout << "Average speed of B: " << averageSpeedOfB << std::endl;
    std::cout << "deltaVsa: " << deltaVsa << std::endl;
    std::cout << "B acc: " << data.acceleration << std::endl;
    std::cout << "Time change: " << timeChange << std::endl;
    std::cout << "Time acc: " << timeAcc << std::endl;
    std::cout << "Time overtake: " << timeOvertake << std::endl;
    std::cout << "Distance overtake: " << totalOvertakDistance << std::endl;
    std::cout << "Final x position: " << finalXPosition << std::endl;
    */

    // Data from the opposite platoon
    double platoonCFinalXPosition = platoonCData.positionX - (platoonCData.velocity * timeOvertake) - epsilon_error;


    startedManeuver = true;
    // If the platoon C is behind the platoon B, start the overtaking maneuver
    if (platoonCData.positionX + epsilon_error < data.positionX)
    {
        timeOvertake += 3;
        recordScalar("Overtake_method", 3);
        plexeTraciVehicle->changeLaneRelative(1, 0);
        app->setInManeuver(true);
        app->scheduleSelfMsg(simTime() + timeOvertake, new cMessage("manovra"));
        return;
    }

    // If the platoon C is in the way, reduce the speed of the platoon C
    if (platoonCFinalXPosition - head_safety_distance <= finalXPosition) {
        double newPlatoonCXPosition = platoonCData.positionX - ((platoonCData.velocity * 0.9) * timeOvertake) - epsilon_error;

        // If platoon C is still in the way, way for it to pass
        if (newPlatoonCXPosition - head_safety_distance - 2 <= finalXPosition)
        {
            if (!startedManeuver)
                recordScalar("Overtake_start", simTime().dbl());
            app->setDataSet(false);
            app->scheduleSelfMsg(simTime() + 1, new cMessage("manovra"));
            return;
        }

        recordScalar("Overtake_method", 2);
        app->sendNewSpeedMessage(2, platoonCData.velocity * 0.9, timeOvertake);
        /*
            if (!startedManeuver)
                recordScalar("Overtake_start", simTime().dbl());
            app->setDataSet(false);
            app->scheduleSelfMsg(simTime() + 1, new cMessage("manovra"));
            return;
            */
    } else
    {
        recordScalar("Overtake_method", 1);
        recordScalar("Overtake_start", simTime().dbl());
    }

    plexeTraciVehicle->changeLaneRelative(1, 0);
    app->setInManeuver(true);
    app->scheduleSelfMsg(simTime() + timeOvertake, new cMessage("manovra"));
}

void OvertakeManeuver::maneuver()
{
    plexeTraciVehicle->changeLaneRelative(-1, 0);
    recordScalar("Overtake_stop", simTime().dbl());
    //app->scheduleSelfMsg(simTime() + 0.5, new cMessage("manovra"));
}

void OvertakeManeuver::sendOvertakeMessage(int lane)
{
    int lastVehicleId = positionHelper->getPlatoonFormation()[positionHelper->getPlatoonSize()-1];

    for (int i = positionHelper->getId(); i <= lastVehicleId; i++)
    {
        OvertakeMessage* msg = new OvertakeMessage();
        app->fillManeuverMessage(msg, positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), 1);
        msg->setLane(lane);
        app->sendUnicast(msg, i);
    }
}

void OvertakeManeuver::onManeuverMessage(const ManeuverMessage* mm)
{
    std::cout << "OvertakeManeuver::onManeuverMessage" << std::endl;
}

void OvertakeManeuver::onFailedTransmissionAttempt(const ManeuverMessage* mm)
{
    std::cout << "OvertakeManeuver::onFailedTransmissionAttempt" << std::endl;
}

bool OvertakeManeuver::handleSelfMsg(cMessage* msg)
{
    if (app->isInManeuver())
    {
        maneuver();
    } else
    {
        prepareManeuver();
    }
    return true;
}

void OvertakeManeuver::finish()
{

}

} // namespace plexe
