//
// Copyright (C) 2012-2022 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2018-2022 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#include "plexe/maneuver/IntersectionMergeManeuver.h"
#include "plexe/apps/GeneralPlatooningApp.h"
#include "plexe/messages/IntersectionMergingBeacon_m.h"

namespace plexe {

#undef LOG
#define LOG std::cout

IntersectionMergeManeuver::IntersectionMergeManeuver(GeneralPlatooningApp* app)
    : Maneuver(app)
    , state(IntersectionMergeManeuverState::IDLE)
    , positions(DynamicPositionManager::getInstance())
{
    prot = dynamic_cast<IntersectionMergingBeaconing*>(app->getProtocol());
    if (prot == nullptr)
        throw cRuntimeError("IntersectionMergeManeuver needs the IntersectionMergingBeaconing protocol to work!");

    platoonALeader = positions.getMemberId(PLATOON_ID_A, 0);
    auto platoonAFormation = positions.getPlatoonFormation(platoonALeader);
    platoonALast = platoonAFormation[platoonAFormation.size()-1];
    platoonBLeader = positions.getMemberId(PLATOON_ID_B, 0);
    platoonCLeader = positions.getMemberId(PLATOON_ID_C, 0);
    auto platoonCFormation = positions.getPlatoonFormation(platoonCLeader);
    platoonCLast = platoonCFormation[platoonCFormation.size()-1];

    if (positionHelper->getPlatoonId() == PLATOON_ID_C && positionHelper->isLast()) {
        // the last vehicle in platoon C has the duty to check when merging is completed
        checkOnIntersection = new cMessage("checkOnIntersection");
        app->scheduleSelfMsg(simTime() + 0.1, checkOnIntersection);
    }
}

bool IntersectionMergeManeuver::initializeIntersectionMergeManeuver(const void* parameters)
{
    IntersectionMergeManeuverParameters* pars = (IntersectionMergeManeuverParameters*) parameters;
    if (state == IntersectionMergeManeuverState::IDLE) {
        if (app->isInManeuver()) {
            LOG << positionHelper->getId() << " cannot begin the maneuver because already involved in another one\n";
            return false;
        }

        app->setInManeuver(true, this);

        // collect information about target Platoon
        follower.leaderId = pars->leaderId;
        follower.platoonId = pars->platoonId;
        return true;
    }
    else {
        return false;
    }
}

void IntersectionMergeManeuver::sendMergeRequest()
{
    LOG << positionHelper->getId() << " sending IntersectionMergeRequest to platoon with id " << follower.platoonId << " (leader id " << follower.leaderId << ")\n";
    int lastVehicleId = positionHelper->getPlatoonFormation()[positionHelper->getPlatoonSize()-1];
    IntersectionMergeRequest* msg = createIntersectionMergeRequest(positionHelper->getId(), positionHelper->getExternalId(), follower.platoonId, follower.leaderId, lastVehicleId);
    app->sendUnicast(msg, follower.leaderId);
}

void IntersectionMergeManeuver::startManeuver(const void* parameters)
{
    if (initializeIntersectionMergeManeuver(parameters)) {
        // send merge request to follower platoon
        std::cout << "!!Sending merge request\n";
        sendMergeRequest();
        mergeRequestTimeout = new cMessage("sendRequestTimeout");
        app->scheduleSelfMsg(simTime() + 1, mergeRequestTimeout);
    }
}

bool IntersectionMergeManeuver::handleSelfMsg(cMessage* msg)
{
    if (msg == mergeRequestTimeout) {
        sendMergeRequest();
        app->scheduleSelfMsg(simTime() + 1, mergeRequestTimeout);
        return true;
    }
    else if (msg == checkOnIntersection) {
        if (traciVehicle->getRoadId().compare("BOTTOM") == 0) {
            app->scheduleSelfMsg(simTime() + 0.1, checkOnIntersection);
        }
        else {
            app->cancelAndDelete(checkOnIntersection);
            checkOnIntersection = nullptr;
            if (positionHelper->isLeader()) {
                // platoon C has now merged into the intersection
                // leader changes state
                state = IntersectionMergeManeuverState::C_MERGED_IN;
                LOG << "Leader of platoon C merged in\n";
            }
            else {
                // the last vehicle of platoon C merged into the intersection
                // inform platoon B
                IntersectionMergeCompleted* mergeCompleted = new IntersectionMergeCompleted();
                app->fillManeuverMessage(mergeCompleted, positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), platoonBLeader);
                app->sendUnicast(mergeCompleted, platoonBLeader);
                LOG << "Last vehicle of platoon C merged in\n";
            }
        }
        return true;
    }
    else {
        return false;
    }
}

void IntersectionMergeManeuver::onManeuverMessage(const ManeuverMessage* mm)
{
    if (const IntersectionMergeRequest* msg = dynamic_cast<const IntersectionMergeRequest*>(mm)) {
        handleIntersectionMergeRequest(msg);
        return;
    }
    if (const IntersectionMergeCompleted* msg = dynamic_cast<const IntersectionMergeCompleted*>(mm)) {
        handleIntersectionMergeCompleted(msg);
        return;
    }
}

IntersectionMergeRequest* IntersectionMergeManeuver::createIntersectionMergeRequest(int vehicleId, std::string externalId, int platoonId, int destinationId, int lastVehicleId)
{
    IntersectionMergeRequest* msg = new IntersectionMergeRequest();
    app->fillManeuverMessage(msg, vehicleId, externalId, platoonId, destinationId);
    msg->setLastVehicleId(lastVehicleId);
    return msg;
}

void IntersectionMergeManeuver::handleIntersectionMergeRequest(const IntersectionMergeRequest* msg)
{

  std::cout << "!!!!!handleIntersectionMergeRequest\n";
    if (msg->getDestinationId() != positionHelper->getId() || app->isInManeuver())
        return;

    app->setInManeuver(true, this);
    merging.leaderId = msg->getVehicleId();
    merging.platoonId = msg->getPlatoonId();
    state = IntersectionMergeManeuverState::B_MERGING_IN;
    prot->setEnableFwdForwarding(true);
    LOG << positionHelper->getId() << " got IntersectionMergeRequest from " << merging.platoonId << " (leader id " << merging.leaderId << "). Enabling forwarding\n";

}

void IntersectionMergeManeuver::handleIntersectionMergeCompleted(const IntersectionMergeCompleted* msg)
{
    if (state == IntersectionMergeManeuverState::B_MERGING_IN) {
        state = IntersectionMergeManeuverState::B_MERGED_IN;
        LOG << "B Platoon Leader: Received IntersectionMergeCompleted from last vehicle of platoon C\n";
    }
}

void IntersectionMergeManeuver::onPlatoonBeacon(const PlatooningBeacon* pb)
{

    if (positionHelper->getPlatoonId() == PLATOON_ID_B && positionHelper->isLeader()) {
        // leader of platoon B needs to feed the FAKE CACC
        /**
         * Platoon B using data of platoon A before the merge maneuver starts
         */
        if (state == IntersectionMergeManeuverState::IDLE) {
            // if merging maneuver has not yet started, the front platoon is platoon A
            if (pb->getVehicleId() == platoonALeader)
                plexeTraciVehicle->setLeaderVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed());
            else if (pb->getVehicleId() == platoonALast) {
                double distance, relSpeed;
                plexeTraciVehicle->getRadarMeasurements(distance, relSpeed);
                plexeTraciVehicle->setFrontVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed(), distance);
                virtualDistance = distance;
            }
        }
        /**
         * Platoon B using data of platoon C after the merge maneuver started
         */
        if (state == IntersectionMergeManeuverState::B_MERGING_IN) {
            // if merging maneuver has started, the front platoon is platoon C
            const IntersectionMergingBeacon* msg = dynamic_cast<const IntersectionMergingBeacon*>(pb);
            if (msg == nullptr)
                return;
            else {
                if (pb->getVehicleId() == platoonCLeader) {
                    double distance, relSpeed;
                    plexeTraciVehicle->setLeaderVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed());
                    plexeTraciVehicle->getRadarMeasurements(distance, relSpeed);
                    virtualDistance = distance - msg->getFwdLastDistance() - 5;
                    plexeTraciVehicle->setFrontVehicleFakeData(msg->getFwdLastControllerAcceleration(), msg->getFwdLastAcceleration(), msg->getFwdLastSpeed(), virtualDistance);
                    // VEHICLE_DATA data;
                    // plexeTraciVehicle->getVehicleData(&data);
                    // double position = traciVehicle->getLanePosition();
                    // std::string edge = traciVehicle->getRoadId();
                    // double distanceToIntersection = traci->getDistanceRoad(edge, position, "RIGHT_1", 0, true);
                    // std::printf("PLATOON B: To intersection: %.2f, B->C: %.2f, accel: %.2f\n", distanceToIntersection, distance - msg->getFwdLastDistance() - 5, data.acceleration);

                    // std::printf("PLATOON B: On edge %s, distance to front: %.2f\n", traciVehicle->getRoadId().c_str(), distance);
                }
            }
        }

        /**
         * Platoon C has completed the maneuver
         */
        if (state == IntersectionMergeManeuverState::B_MERGED_IN) {
            if (pb->getVehicleId() == platoonCLeader) {
                plexeTraciVehicle->setLeaderVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed());
                return;
            }
            if (pb->getVehicleId() == platoonCLast) {
                double distance, relSpeed;
                plexeTraciVehicle->getRadarMeasurements(distance, relSpeed);
                virtualDistance = distance;
                plexeTraciVehicle->setFrontVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed(), distance);
            }
        }

    }

    /**
     * Platoon C in IDLE state, looking if platoon B got his request by checking the type of packet
     * If so, it understands maneuver has started and changes its state
     */
    if (pb->getVehicleId() == follower.leaderId && state == IntersectionMergeManeuverState::IDLE) {
        // see if we should start the merging by looking at what is included in such beacon
        const IntersectionMergingBeacon* msg = dynamic_cast<const IntersectionMergingBeacon*>(pb);
        if (msg == nullptr)
            return;
        else {
            LOG << positionHelper->getId() << " got first forwarded packet from " << follower.platoonId << " (leader id " << follower.leaderId << "). Starting to merge in\n";
            state = IntersectionMergeManeuverState::C_MERGING_IN;
            app->cancelAndDelete(mergeRequestTimeout);
            mergeRequestTimeout = nullptr;
            prot->setEnableLastForwarding(true);

            checkOnIntersection = new cMessage("checkOnIntersection");
            app->scheduleSelfMsg(simTime() + 0.1, checkOnIntersection);
        }
    }

    /**
     * Platoon C is in merging and has got a packet from platoon B
     */
    if (pb->getVehicleId() == follower.leaderId && state == IntersectionMergeManeuverState::C_MERGING_IN) {
        const IntersectionMergingBeacon* msg = dynamic_cast<const IntersectionMergingBeacon*>(pb);
        if (msg == nullptr)
            throw cRuntimeError("Platoon C is merging in but Platoon B is not sending the proper type of packet!");

        plexeTraciVehicle->setActiveController(plexe::FAKED_CACC);
        double position = traciVehicle->getLanePosition();
        std::string edge = traciVehicle->getRoadId();
        double distanceToIntersection = traci->getDistanceRoad(edge, position, "RIGHT_1", 0, true);
        plexeTraciVehicle->setLeaderVehicleFakeData(msg->getFwdLeaderControllerAcceleration(), msg->getFwdLeaderAcceleration(), msg->getFwdLeaderSpeed());
        virtualDistance = distanceToIntersection + msg->getFwdLastDistance();
        plexeTraciVehicle->setFrontVehicleFakeData(msg->getFwdLastControllerAcceleration(), msg->getFwdLastAcceleration(), msg->getFwdLastSpeed(), virtualDistance);

        // VEHICLE_DATA data;
        // plexeTraciVehicle->getVehicleData(&data);
//        std::printf("PLATOON C: To intersection: %.2f, B->A: %.2f, C->A: %.2f, accel: %.2f\n", distanceToIntersection, msg->getFwdLastDistance(), distanceToIntersection + msg->getFwdLastDistance(), data.acceleration);

        // double distance, relSpeed;
        // plexeTraciVehicle->getRadarMeasurements(distance, relSpeed);
//        std::printf("PLATOON C: On edge %s, distance to front: %.2f\n", traciVehicle->getRoadId().c_str(), distance);
    }


    /**
     * Platoon C has completed the maneuver
     */
    if (state == IntersectionMergeManeuverState::C_MERGED_IN) {
        if (pb->getVehicleId() == platoonALeader) {
            plexeTraciVehicle->setLeaderVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed());
            return;
        }
        if (pb->getVehicleId() == platoonALast) {
            double distance, relSpeed;
            plexeTraciVehicle->getRadarMeasurements(distance, relSpeed);
            virtualDistance = distance;
            plexeTraciVehicle->setFrontVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed(), distance);
        }
    }

}

double IntersectionMergeManeuver::getVirtualDistance()
{
    double vd = virtualDistance;
    virtualDistance = -1e6;
    return vd;
}

} // namespace plexe
