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

#include "IntersectionMergingBeaconing.h"
#include "plexe/messages/IntersectionMergingBeacon_m.h"
#include "plexe/maneuver/IntersectionMergeManeuver.h"

#include "veins/modules/mac/ieee80211p/Mac1609_4.h"

using namespace veins;

namespace plexe {

Define_Module(IntersectionMergingBeaconing)

void IntersectionMergingBeaconing::initialize(int stage)
{
    SimplePlatooningBeaconing::initialize(stage);

    if (stage == 1) {
        platoonALeader = positions.getMemberId(PLATOON_ID_A, 0);
        auto platoonAFormation = positions.getPlatoonFormation(platoonALeader);
        platoonALast = platoonAFormation[platoonAFormation.size()-1];
        auto platoonCFormation = positions.getPlatoonFormation(positions.getMemberId(PLATOON_ID_C, 0));
        platoonCLast = platoonCFormation[platoonCFormation.size()-1];
    }
}

void IntersectionMergingBeaconing::messageReceived(PlatooningBeacon* pkt, BaseFrame1609_4* frame)
{
    if (positionHelper->getPlatoonId() == PLATOON_ID_B && positionHelper->isLeader()) {
        if (pkt->getVehicleId() == platoonALeader) {
            fwdLeaderControllerAcceleration = pkt->getControllerAcceleration();
            fwdLeaderAcceleration = pkt->getAcceleration();
            fwdLeaderSpeed = pkt->getSpeed();
        }
        else if (pkt->getVehicleId() == platoonALast) {
            fwdLastControllerAcceleration = pkt->getControllerAcceleration();
            fwdLastAcceleration = pkt->getAcceleration();
            fwdLastSpeed = pkt->getSpeed();
            // TODO: add something to the protocol to stop this when platoon C merges in (distance should change by a large amount)
            double distance, relSpeed, distanceToIntersection;

            double position = traciVehicle->getLanePosition();
            std::string edge = traciVehicle->getRoadId();
            distanceToIntersection = traci->getDistanceRoad(edge, position, "RIGHT_1", 0, true);

            plexeTraciVehicle->getRadarMeasurements(distance, relSpeed);
            fwdLastDistance = distance - distanceToIntersection;
        }
    }
    if (positionHelper->getPlatoonId() == PLATOON_ID_C && positionHelper->isLeader()) {
        if (pkt->getVehicleId() == platoonCLast) {
            fwdLastControllerAcceleration = pkt->getControllerAcceleration();
            fwdLastAcceleration = pkt->getAcceleration();
            fwdLastSpeed = pkt->getSpeed();

            veins::Coord lastPosition(pkt->getPositionX(), pkt->getPositionY());
            VEHICLE_DATA data;
            double distance;
            plexeTraciVehicle->getVehicleData(&data);
            veins::Coord position(data.positionX, data.positionY);
            distance = traci->getDistance(lastPosition, position, false) + 4;
            // use the distance field to encode the length of the platoon
            fwdLastDistance = distance;
        }
    }
}

std::unique_ptr<BaseFrame1609_4> IntersectionMergingBeaconing::createBeacon(int destinationAddress)
{
    if (!enableFwdForwarding && !enableLastForwarding)
        return BaseProtocol::createBeacon(destinationAddress);

    // vehicle's data to be included in the message
    VEHICLE_DATA data;
    // get information about the vehicle via traci
    plexeTraciVehicle->getVehicleData(&data);

    // create and send beacon
    auto wsm = veins::make_unique<BaseFrame1609_4>("", BEACON_TYPE);
    wsm->setRecipientAddress(LAddress::L2BROADCAST());
    wsm->setChannelNumber(static_cast<int>(Channel::cch));
    wsm->setUserPriority(priority);

    // create platooning beacon with data about the car
    IntersectionMergingBeacon* pkt = new IntersectionMergingBeacon();
    pkt->setControllerAcceleration(data.u);
    pkt->setAcceleration(data.acceleration);
    pkt->setSpeed(data.speed);
    pkt->setVehicleId(myId);
    pkt->setPositionX(data.positionX);
    pkt->setPositionY(data.positionY);
    // set the time to now
    pkt->setTime(data.time);
    pkt->setLength(length);
    pkt->setSpeedX(data.speedX);
    pkt->setSpeedY(data.speedY);
    pkt->setAngle(data.angle);
    pkt->setKind(BEACON_TYPE);
    pkt->setByteLength(packetSize);
    pkt->setSequenceNumber(seq_n++);

    if (enableFwdForwarding) {
        // send data about preceding platoon to the platoon merging in
        pkt->setFwdLeaderControllerAcceleration(fwdLeaderControllerAcceleration);
        pkt->setFwdLeaderAcceleration(fwdLeaderAcceleration);
        pkt->setFwdLeaderSpeed(fwdLeaderSpeed);
        pkt->setFwdLastControllerAcceleration(fwdLastControllerAcceleration);
        pkt->setFwdLastAcceleration(fwdLastAcceleration);
        pkt->setFwdLastSpeed(fwdLastSpeed);
        pkt->setFwdLastDistance(fwdLastDistance);
    }
    else if (enableLastForwarding) {
        // send data about the last vehicle in the platoon to the platoon letting us merge in
        pkt->setFwdLastControllerAcceleration(fwdLastControllerAcceleration);
        pkt->setFwdLastAcceleration(fwdLastAcceleration);
        pkt->setFwdLastSpeed(fwdLastSpeed);
        pkt->setFwdLastDistance(fwdLastDistance);
    }

    wsm->encapsulate(pkt);

    return wsm;

}

void IntersectionMergingBeaconing::setEnableFwdForwarding(bool forwarding)
{
    enableFwdForwarding = forwarding;
}

void IntersectionMergingBeaconing::setEnableLastForwarding(bool forwarding)
{
    enableLastForwarding = forwarding;
}

IntersectionMergingBeaconing::IntersectionMergingBeaconing()
    :positions(DynamicPositionManager::getInstance())
{
}

IntersectionMergingBeaconing::~IntersectionMergingBeaconing()
{
}

} // namespace plexe
