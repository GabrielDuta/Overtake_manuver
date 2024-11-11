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

#ifndef OVERTAKEAPP_H_
#define OVERTAKEAPP_H_

#include <algorithm>
#include <memory>

#include "plexe/apps/BaseApp.h"
#include "plexe/maneuver/OvertakeManeuver.h"

#include "plexe/messages/ManeuverMessage_m.h"
#include "plexe/messages/UpdatePlatoonData_m.h"
#include "plexe/messages/VehicleDataMessage_m.h"
#include "plexe/messages/NewSpeedMessage_m.h"
#include "plexe/messages/RequestDataMessage_m.h"

#include "plexe/scenarios/OvertakeScenario.h"

#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/utility/SignalManager.h"

#include "plexe/utilities/DynamicPositionManager.h"

using namespace veins;

namespace plexe {

/** possible roles of this vehicle */
enum class PlatoonRole : size_t {
    NONE, ///< The vehicle is not in a Platoon
    LEADER, ///< The vehicle is the leader of its Platoon
    FOLLOWER, ///< The vehicle is a normal follower in its Platoon
    JOINER ///< The vehicle is in the process of joining a Platoon
};

/** Struct to save the data of other vehicles */
struct VehicleData {
    double velocity;
    double positionX;
    double platoonSize;
    int leaderId;
};

/**
 * This class handles maintenance of an existing Platoon by storing relevant
 * information about it and feeding the CACC.
 * It also provides an easily extendable base for supporting arbitrary
 * maneuvers.
 *
 * @see OvertakeManeuver
 */
class OvertakeApp : public BaseApp {

public:
    /** c'tor for GeneralPlatooningApp */
    OvertakeApp()
        : inManeuver(false)
        , scenario(nullptr)
        , role(PlatoonRole::NONE)
        , overtakeManeuver(nullptr)
        , positions(DynamicPositionManager::getInstance())
    {
    }

    /** d'tor for GeneralPlatooningApp */
    virtual ~OvertakeApp();

    /**
     * Returns the role of this car in the platoon
     *
     * @return PlatoonRole the role in the platoon
     * @see PlatoonRole
     */
    const PlatoonRole& getPlatoonRole() const
    {
        return role;
    }

    /**
     * Sets the role of this car in the platoon
     *
     * @param PlatoonRole r the role in the platoon
     * @see PlatoonRole
     */
    void setPlatoonRole(PlatoonRole r);

    /** override from BaseApp */
    virtual void initialize(int stage) override;

    /** override from BaseApp */
    virtual void handleSelfMsg(cMessage* msg) override;


    /**
     * Request start of the OvertakeManeuver
     */
    //void startOvertakeManeuver(int platoonId, int leaderId);
    void startOvertakeManeuver();

    /**
     * Returns whether this car is in a maneuver
     * @return bool true, if this car is in a maneuver, else false
     */
    bool isInManeuver() const
    {
        return inManeuver;
    }

    /**
     * Set whether this car is in a maneuver
     * @param bool b whether this car is in a maneuver
     * @param maneuver the maneuver that is currently active
     */
    void setInManeuver(bool b)
    {
        inManeuver = b;
    }

    BasePositionHelper* getPositionHelper()
    {
        return positionHelper;
    }

    veins::TraCIMobility* getMobility()
    {
        return mobility;
    }

    veins::TraCICommandInterface* getTraci()
    {
        return traci;
    }

    veins::TraCICommandInterface::Vehicle* getTraciVehicle()
    {
        return traciVehicle;
    }

    traci::CommandInterface* getPlexeTraci()
    {
        return plexeTraci;
    }

    traci::CommandInterface::Vehicle* getPlexeTraciVehicle()
    {
        return plexeTraciVehicle.get();
    }

    /**
     * Sends a unicast message
     *
     * @param cPacket msg message to be encapsulated into the unicast
     * message
     * @param int destination of the message
     */
    virtual void sendUnicast(cPacket* msg, int destination);

    /**
     * Fills members of a ManeuverMessage
     *
     * @param msg ManeuverMessage the message to be filled
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     */
    void fillManeuverMessage(ManeuverMessage* msg, int vehicleId, std::string externalId, int platoonId, int destinationId);

    void fillVehicleDataMessage(VehicleDataMessage* msg);

    /**
     * Handles a OvertakeMessage in the context of this
     * application
     *
     * @param OvertakeMessage msg to handle
     */
    virtual void handleOvertakeMessage(const OvertakeMessage* msg);

    /**
     * Returns the stand still distance for the active controller
     */
    double getStandstillDistance(enum ACTIVE_CONTROLLER controller);

    /**
     * Returns the time headway for the active controller
     */
    double getHeadway(enum ACTIVE_CONTROLLER controller);

    /**
     * Returns the inter-vehicle distance for the chosen controller and the given speed
     */
    double getTargetDistance(double speed);

    /**
     * Returns the inter-vehicle distance for the given controller and the given speed
     */
    double getTargetDistance(enum ACTIVE_CONTROLLER controller, double speed);

    /**
     * Returns the target controller to be used after joining a platoon
     * This only works if the scenario inherits from ManeuverScenario
     */
    enum ACTIVE_CONTROLLER getTargetController();

    /** used by maneuvers to schedule self messages, as they are not omnet modules */
    virtual void scheduleSelfMsg(simtime_t t, cMessage* msg);

    /** Send position and velocity to the B platoon to make the calculations for the maneuver */
    virtual void sendVehicleInfoToBLeader();

    virtual void sendNewSpeedMessage(int destinationPlatoonId, double speed, double manTime);

    virtual void sendRequestDataMessage(int platoonId);

    virtual VehicleData getVehicleData(int platoonId);

    virtual bool getDataSet(int platoonId);
    virtual void setDataSet(bool value);

protected:
    /** override this method of BaseApp. we want to handle it ourself */
    virtual void handleLowerMsg(cMessage* msg) override;

    virtual void handleVehicleDataMessage(const VehicleDataMessage* msg);

    virtual void handleNewSpeedMessage(const NewSpeedMessage* msg);

    virtual void handleRequestDataMessage(const RequestDataMessage* msg);


    /**
     * Handles PlatoonBeacons
     *
     * @param PlatooningBeacon pb to handle
     */
    virtual void onPlatoonBeacon(const PlatooningBeacon* pb) override;

    /**
     * Handles ManeuverMessages
     *
     * @param ManeuverMessage mm to handle
     */
    virtual void onManeuverMessage(ManeuverMessage* mm);

    /** am i in a maneuver? */
    bool inManeuver;
    /** which maneuver is currently active? */
   // Maneuver* activeManeuver;

    /** used to receive the "retries exceeded" signal **/
    virtual void receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details) override;

    //OvertakeScenario* scenario;
    BaseScenario* scenario;

    cOutVector virtualDistanceOut;
    // override method to add additional output
    virtual void logVehicleData(bool crashed) override;

    DynamicPositionManager& positions;

private:
    /** the role of this vehicle */
    PlatoonRole role;
    /** overtake maneuver implementation */
    OvertakeManeuver* overtakeManeuver;

    VehicleData platoonAData {};
    VehicleData platoonCData {};

    bool aDataSet = false;
    bool cDataSet = false;
    double oldSpeed = 0;
    double time = 0;
};

} // namespace plexe

#endif
