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

#include "plexe/apps/OvertakeApp.h"

#include "plexe/protocols/BaseProtocol.h"
#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"
#include "veins/base/utils/FindModule.h"
#include "plexe/scenarios/OvertakeScenario.h"

using namespace veins;

namespace plexe {

Define_Module(OvertakeApp);

void OvertakeApp::initialize(int stage)
{
    BaseApp::initialize(stage);
    
    if (stage == 1) {
        // connect maneuver application to protocol
        protocol->registerApplication(MANEUVER_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));
        // register to the signal indicating failed unicast transmissions
        findHost()->subscribe(Mac1609_4::sigRetriesExceeded, this);

        std::string overtakeManeuverName = par("overtakeManeuver").stdstringValue();
        if (overtakeManeuverName == "OvertakeManeuver")
            overtakeManeuver = new OvertakeManeuver(this);
        else
            throw new cRuntimeError("Invalid overtake maneuver implementation chosen");

        if (positionHelper->isLeader()) setPlatoonRole(PlatoonRole::LEADER);
        else setPlatoonRole(PlatoonRole::FOLLOWER);

        scenario = FindModule<OvertakeScenario*>::findSubModule(getParentModule());

        virtualDistanceOut.setName("virtualDistance");
    }
}

void OvertakeApp::logVehicleData(bool crashed)
{
    BaseApp::logVehicleData(crashed);
}

void OvertakeApp::handleSelfMsg(cMessage* msg)
{
    if (std::strcmp(msg->getName(), "manovra") == 0) {
        overtakeManeuver->handleSelfMsg(msg);
    }
    else if (std::strcmp(msg->getName(), "speed") == 0) {
        plexeTraciVehicle->setFixedAcceleration(0, 0);
    }
    else if (std::strcmp(msg->getName(), "consumo") == 0) {
        EV_INFO << "consumo - " << positionHelper->getId() << " | " << simTime().dbl() << "| : " << traciVehicle->getFuelConsumption() << std::endl;
    }
    else {
        BaseApp::handleSelfMsg(msg);
    }
}

double OvertakeApp::getStandstillDistance(enum ACTIVE_CONTROLLER controller)
{
    return scenario->getStandstillDistance(controller);
}

double OvertakeApp::getHeadway(enum ACTIVE_CONTROLLER controller)
{
    return scenario->getHeadway(controller);
}

double OvertakeApp::getTargetDistance(enum ACTIVE_CONTROLLER controller, double speed)
{
    return scenario->getTargetDistance(controller, speed);
}

double OvertakeApp::getTargetDistance(double speed)
{
    return scenario->getTargetDistance(speed);
}

enum ACTIVE_CONTROLLER OvertakeApp::getTargetController()
{
    OvertakeScenario* overtakeScenario = dynamic_cast<OvertakeScenario*>(scenario);
    if (!overtakeScenario) throw cRuntimeError("getTargetController() invoked from a simulation not inheriting from ManeuverScenario");
    return overtakeScenario->getTargetController();
}

void OvertakeApp::sendUnicast(cPacket* msg, int destination)
{
    Enter_Method_Silent();
    take(msg);
    BaseFrame1609_4* frame = new BaseFrame1609_4("BaseFrame1609_4", msg->getKind());
    frame->setRecipientAddress(destination);
    frame->setChannelNumber(static_cast<int>(Channel::cch));
    frame->encapsulate(msg);
    // send unicast frames using 11p only
    PlexeInterfaceControlInfo* ctrl = new PlexeInterfaceControlInfo();
    ctrl->setInterfaces(PlexeRadioInterfaces::VEINS_11P);
    frame->setControlInfo(ctrl);
    sendDown(frame);
}

void OvertakeApp::handleLowerMsg(cMessage* msg)
{
    BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(msg);

    cPacket* enc = frame->getEncapsulatedPacket();
    ASSERT2(enc, "received a BaseFrame1609_4s with nothing inside");

    if (enc->getKind() == MANEUVER_TYPE) {
        ManeuverMessage* mm = check_and_cast<ManeuverMessage*>(frame->decapsulate());
        if (OvertakeMessage* msg = dynamic_cast<OvertakeMessage*>(mm)) {
            handleOvertakeMessage(msg);
            delete msg;
        }
        else if (VehicleDataMessage* msg = dynamic_cast<VehicleDataMessage*>(mm)) {
            handleVehicleDataMessage(msg);
            delete msg;
        }
        else if (NewSpeedMessage* msg = dynamic_cast<NewSpeedMessage*>(mm)) {
            handleNewSpeedMessage(msg);
            delete msg;
        }
        else if (RequestDataMessage* msg = dynamic_cast<RequestDataMessage*>(mm)) {
            handleRequestDataMessage(msg);
            delete msg;
        }
        else {
            onManeuverMessage(mm);
        }
        delete frame;
    }
    else {
        BaseApp::handleLowerMsg(msg);
    }
}

void OvertakeApp::handleOvertakeMessage(const OvertakeMessage* msg)
{
    if (getPlatoonRole() != PlatoonRole::FOLLOWER) return;
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return;
    if (msg->getDestinationId() != positionHelper->getId()) return;

    if (msg->getLane() == 0)
        plexeTraciVehicle->setFixedLane(msg->getLane(), true);
    else
        plexeTraciVehicle->setFixedLane(msg->getLane(), false);
}

void OvertakeApp::handleRequestDataMessage(const RequestDataMessage* msg)
{
    if (getPlatoonRole() != PlatoonRole::LEADER) return;
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return;
    if (msg->getDestinationId() != positionHelper->getId()) return;

    sendVehicleInfoToBLeader();
}

void OvertakeApp::handleVehicleDataMessage(const VehicleDataMessage* msg)
{
    if (getPlatoonRole() != PlatoonRole::LEADER) return;
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return;
    if (msg->getDestinationId() != positionHelper->getId()) return;

    if (msg->getSenderPlatoonId() == 0)
    {
        platoonAData.velocity = msg->getSenderVelocity();
        platoonAData.positionX = msg->getSenderX();
        platoonAData.platoonSize = msg->getPlatoonSize();
        platoonAData.leaderId = msg->getVehicleId();
        aDataSet = true;
    }
    else if (msg->getSenderPlatoonId() == 2)
    {
        platoonCData.velocity = msg->getSenderVelocity();
        platoonCData.positionX = msg->getSenderX();
        platoonCData.platoonSize = msg->getPlatoonSize();
        platoonCData.leaderId = msg->getVehicleId();
        cDataSet = true;
    }

}

bool OvertakeApp::getDataSet(int platoonId)
{
    if (platoonId == PLATOON_ID_A) return aDataSet;
    if (platoonId == PLATOON_ID_C) return cDataSet;
}

void OvertakeApp::setDataSet(bool value)
{
    aDataSet = value;
    cDataSet = value;
}

void OvertakeApp::handleNewSpeedMessage(const NewSpeedMessage* msg)
{
    if (getPlatoonRole() != PlatoonRole::LEADER) return;
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return;
    if (msg->getDestinationId() != positionHelper->getId()) return;

    VEHICLE_DATA data;
    plexeTraciVehicle->getVehicleData(&data);
    double newAcceleration = (msg->getNewSpeed() - data.speed) / msg->getManeuverTime();
    double timeToReachNewSpeed = (msg->getNewSpeed() - data.speed) / newAcceleration + 0.5;

    plexeTraciVehicle->setFixedAcceleration(1, newAcceleration);
    oldSpeed = data.speed;
    time = timeToReachNewSpeed;

    if (msg->getNewSpeed() < data.speed)
    {
        scheduleSelfMsg(simTime() + timeToReachNewSpeed, new cMessage("speed"));
    }
}

void OvertakeApp::setPlatoonRole(PlatoonRole r)
{
    role = r;
}

void OvertakeApp::onPlatoonBeacon(const PlatooningBeacon* pb)
{
    //overtakeManeuver->onPlatoonBeacon(pb);
    // maintain platoon
    BaseApp::onPlatoonBeacon(pb);
}

void OvertakeApp::onManeuverMessage(ManeuverMessage* mm)
{
    overtakeManeuver->onManeuverMessage(mm);
    delete mm;
}

void OvertakeApp::fillManeuverMessage(ManeuverMessage* msg, int vehicleId, std::string externalId, int platoonId, int destinationId)
{
    msg->setKind(MANEUVER_TYPE);
    msg->setVehicleId(vehicleId);
    msg->setExternalId(externalId.c_str());
    msg->setPlatoonId(platoonId);
    msg->setDestinationId(destinationId);
}

void OvertakeApp::startOvertakeManeuver()
{
    ASSERT(getPlatoonRole() == PlatoonRole::LEADER);
    ASSERT(!inManeuver);

    overtakeManeuver->prepareManeuver();
}

void OvertakeApp::sendVehicleInfoToBLeader()
{
    VehicleDataMessage* msg = new VehicleDataMessage();
    fillVehicleDataMessage(msg);
    sendUnicast(msg, msg->getDestinationId());
}

void OvertakeApp::fillVehicleDataMessage(VehicleDataMessage* msg)
{
    int destinationPlatoonId = 1;
    int destinationId = positions.getMemberId(destinationPlatoonId, 0); // get id of the leader of platoon B
    fillManeuverMessage(msg, positionHelper->getId(), positionHelper->getExternalId(), destinationPlatoonId, destinationId);
    msg->setSenderPlatoonId(positionHelper->getPlatoonId());
    double platoonSize = positionHelper->getPlatoonFormation().size();
    msg->setPlatoonSize(platoonSize);
    VEHICLE_DATA data;
    plexeTraciVehicle->getVehicleData(&data);
    msg->setSenderX(data.positionX);
    msg->setSenderVelocity(data.speed);
}

VehicleData OvertakeApp::getVehicleData(int platoonId)
{
    if (platoonId == PLATOON_ID_A)
    {
        aDataSet = false;
        return platoonAData;
    }
    else if (platoonId == PLATOON_ID_C)
    { 
        cDataSet = false;
        return platoonCData;
    } 
}

void OvertakeApp::sendRequestDataMessage(int platoonId)
{
    RequestDataMessage* msg = new RequestDataMessage();
    int destinationId = positions.getMemberId(platoonId, 0); // get id of the leader of the intended platoon
    fillManeuverMessage(msg, positionHelper->getId(), positionHelper->getExternalId(), platoonId, destinationId);
    sendUnicast(msg, destinationId);
}

void OvertakeApp::sendNewSpeedMessage(int destinationPlatoonId, double speed, double manTime)
{
    NewSpeedMessage* msg = new NewSpeedMessage();
    int destinationId = positions.getMemberId(destinationPlatoonId, 0); // get id of the leader of the intended platoon
    fillManeuverMessage(msg, positionHelper->getId(), positionHelper->getExternalId(), destinationPlatoonId, destinationId);
    msg->setNewSpeed(speed);
    msg->setManeuverTime(manTime);
    sendUnicast(msg, destinationId);
}

void OvertakeApp::receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details)
{
    if (id == Mac1609_4::sigRetriesExceeded) {
        BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(value);
        ManeuverMessage* mm = check_and_cast<ManeuverMessage*>(frame->getEncapsulatedPacket());
        if (frame) {
            overtakeManeuver->onFailedTransmissionAttempt(mm);
        }
    }
}

void OvertakeApp::scheduleSelfMsg(simtime_t t, cMessage* msg)
{
    Enter_Method_Silent();
    take(msg);
    scheduleAt(t, msg);
}

OvertakeApp::~OvertakeApp()
{
    delete overtakeManeuver;
}

} // namespace plexe
