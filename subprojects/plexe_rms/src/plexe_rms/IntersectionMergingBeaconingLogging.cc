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

#include "IntersectionMergingBeaconingLogging.h"
#include "veins/base/phyLayer/PhyToMacControlInfo.h"
#include "veins-rms/DeciderResultRms.h"
#include "veins-rms/PhyLayerRms.h"
#include "plexe/maneuver/IntersectionMergeManeuver.h"
using namespace veins;

namespace plexe {

Define_Module(IntersectionMergingBeaconingLogging)

void IntersectionMergingBeaconingLogging::initialize(int stage)
{
    IntersectionMergingBeaconing::initialize(stage);

    if (stage == 0) {

        antennaIdOut.setName("antennaId");
        antennaGainOut.setName("antennaGain");
        antennaDistanceOut.setName("antennaDistance");
        antennaPowerOut.setName("antennaPower");
        phiROut.setName("phiR");
        thetaROut.setName("thetaR");
        phiIOut.setName("phiI");
        thetaIOut.setName("thetaI");

        useRms = par("useRms");

    }
    if (stage == 1) {
        if (par("disableBeaconing").boolValue()) {
            cancelEvent(sendBeacon);
        }
    }
}

void IntersectionMergingBeaconingLogging::sendPlatooningMessage(int destinationAddress, enum PlexeRadioInterfaces interfaces)
{
    if (useRms) {
        // only leaders of platoons B and C should use the RMS
        if ((positionHelper->getPlatoonId() == PLATOON_ID_B && positionHelper->isLeader()) || (positionHelper->getPlatoonId() == PLATOON_ID_C && positionHelper->isLeader())) {
            // update my position at the RMS, so it can be reconfigured
            PhyLayerRms* rms = dynamic_cast<PhyLayerRms*>(findModuleByPath("Intersection.rms.nicRms.phyRms"));
            rms->updateVehiclePosition(myId, mobility->getPositionAt(simTime()));
            if (positionHelper->getPlatoonId() == PLATOON_ID_B)
                rms->requestReconfiguration(myId, positions.getMemberId(PLATOON_ID_C, 0));
            else
                rms->requestReconfiguration(myId, positions.getMemberId(PLATOON_ID_B, 0));
            BaseProtocol::sendPlatooningMessage(destinationAddress, interfaces);
            return;
        }
    }
    BaseProtocol::sendPlatooningMessage(destinationAddress, PlexeRadioInterfaces::VEINS_11P);
}

void IntersectionMergingBeaconingLogging::messageReceived(PlatooningBeacon* pkt, BaseFrame1609_4* frame, enum PlexeRadioInterfaces interface)
{
    if (interface == PlexeRadioInterfaces::VEINS_RMS && pkt->getVehicleId() != myId) {

        double position = traciVehicle->getLanePosition();
        std::string edge = traciVehicle->getRoadId();
        double distanceToIntersection = traci->getDistanceRoad(edge, position, "RIGHT_1", 0, true);

        PhyToMacControlInfo* ctrl = check_and_cast<PhyToMacControlInfo*>(frame->getControlInfo());
        DeciderResultRms* res = check_and_cast<DeciderResultRms*>(ctrl->getDeciderResult());
        if (!res->getReflected())
            return;
        antennaIdOut.record(myId);
        antennaGainOut.record(res->getGain());
        antennaDistanceOut.record(distanceToIntersection);
        antennaPowerOut.record(res->getRecvPower_dBm());
        phiROut.record(res->getPhiR());
        thetaROut.record(res->getThetaR());
        phiIOut.record(res->getPhiI());
        thetaIOut.record(res->getThetaI());

        printf("logging: id=%d phi=%f gain=%f\n", myId, res->getPhiR(), distanceToIntersection);
    }
}

IntersectionMergingBeaconingLogging::IntersectionMergingBeaconingLogging()
{
}

IntersectionMergingBeaconingLogging::~IntersectionMergingBeaconingLogging()
{
}

} // namespace plexe
