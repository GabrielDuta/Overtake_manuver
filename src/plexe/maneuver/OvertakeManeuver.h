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

#ifndef OVERTAKE_MANEUVER_H_
#define OVERTAKE_MANEUVER_H_


#include "plexe/utilities/BasePositionHelper.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "plexe/mobility/CommandInterface.h"
#include "plexe/messages/OvertakeMessage_m.h"

#include <omnetpp.h>

using namespace veins;

namespace plexe {

#define PLATOON_ID_A 0
#define PLATOON_ID_B 1
#define PLATOON_ID_C 2

class OvertakeApp;

class OvertakeManeuver : cSimpleModule /*: public Maneuver*/ {

public:
    /**
     * Constructor
     *
     * @param app pointer to the generic application used to fetch parameters and inform it about a concluded maneuver
     */
    OvertakeManeuver(OvertakeApp* app);
    ~OvertakeManeuver(){};

    /**
     * This method is invoked by the generic application to start the maneuver
     */
    virtual void prepareManeuver();


    /**
     * This method is invoked by the generic application to abort the maneuver
     */
    //virtual void abortManeuver() = 0;

    /**
     * This method is invoked by the generic application when a maneuver message is received.
     * The maneuver must not free the memory of the message, as this might be needed by other maneuvers as well.
     */
    void onManeuverMessage(const ManeuverMessage* mm);

    /**
     * This method is invoked by the generic application when a beacon message is received
     * The maneuver must not free the memory of the message, as this might be needed by other maneuvers as well.
     */
    //virtual void onPlatoonBeacon(const PlatooningBeacon* pb) = 0;

    /**
     * This method is invoked by the generic application when a failed transmission occurred, indicating the packet for which transmission has failed
     * The manuever must not free the memory of the message, as this might be needed by other maneuvers as well.
     */
    void onFailedTransmissionAttempt(const ManeuverMessage* mm);

    /**
     * If the self message is handled by the maneuver, it should return true. If not it should return false and the message is passed over to the next maneuver.
     * If no maneuver handles the self message, it is passed to BaseApp::handleSelfMsg().
     */
    virtual bool handleSelfMsg(cMessage* msg);

    virtual void finish();

private:
    void maneuver();
    void sendOvertakeMessage(int lane);
    virtual void startManeuver();

protected:
    OvertakeApp* app;
    BasePositionHelper* positionHelper;
    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    traci::CommandInterface* plexeTraci;
    traci::CommandInterface::Vehicle* plexeTraciVehicle;

    double start_speed;
    bool dataRequested = false;
    bool startedManeuver = false;
};

} // namespace plexe

#endif
