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

#pragma once

#include "plexe/maneuver/Maneuver.h"
#include "plexe/messages/IntersectionMergeRequest_m.h"
#include "plexe/messages/IntersectionMergeCompleted_m.h"
#include "plexe/protocols/IntersectionMergingBeaconing.h"

#define PLATOON_ID_A 0
#define PLATOON_ID_B 1
#define PLATOON_ID_C 2

namespace plexe {

struct IntersectionMergeManeuverParameters {
    int platoonId;
    int leaderId;
};

class IntersectionMergeManeuver : public Maneuver {

public:
    /**
     * Constructor
     *
     * @param app pointer to the generic application used to fetch parameters and inform it about a concluded maneuver
     */
    IntersectionMergeManeuver(GeneralPlatooningApp* app);
    virtual ~IntersectionMergeManeuver(){};

    virtual void onManeuverMessage(const ManeuverMessage* mm) override;
    virtual void onPlatoonBeacon(const PlatooningBeacon* pb) override;
    virtual bool handleSelfMsg(cMessage* msg) override;

    virtual void startManeuver(const void* parameters) override;

    virtual void abortManeuver() override {};

    virtual void onFailedTransmissionAttempt(const ManeuverMessage* mm) override {};

    double getVirtualDistance();

protected:

    IntersectionMergingBeaconing* prot = nullptr;

    /** Possible states a vehicle can be in during an intersection merge maneuver */
    enum class IntersectionMergeManeuverState {
        IDLE, ///< The maneuver did not start
        // Platoon C (wants to merge in)
        C_WAIT_REPLY, // platoon C sent the request, waiting for the reply of platoon B
        C_MERGING_IN, // platoon C got the first message from platoon B, now regulating speed to merge in
        B_MERGING_IN, // got a request from platoon C, now regulating speed to let C merge in and forwarding data of A to C
        B_MERGED_IN, // platoon C has now merged in. maneuver done for platoon B
        C_MERGED_IN, // platoon C has now merged in. maneuver done for platoon C
    };

    /** the current state in the maneuver */
    IntersectionMergeManeuverState state;
    /** used to repeat the intersection merge request until getting a response */
    cMessage* mergeRequestTimeout = nullptr;
    /** used to check when merged into the intersection */
    cMessage* checkOnIntersection = nullptr;

    DynamicPositionManager& positions;
    int platoonALeader = -1;
    int platoonALast = -1;
    int platoonBLeader = -1;
    int platoonCLeader = -1;
    int platoonCLast = -1;

    // distance to the front vehicle fed to the FAKE_CACC controller
    double virtualDistance = -1e6;

    struct FollowerPlatoonData {
        int platoonId;
        int leaderId;
    };
    // data about the platoon letting us in (platoon B)
    FollowerPlatoonData follower;

    struct MergingPlatoonData {
        int platoonId;
        int leaderId;
    };
    // data about the platoon we're letting in (platoon C)
    MergingPlatoonData merging;

    bool initializeIntersectionMergeManeuver(const void* parameters);

    /**
     * Creates an IntersectionMergeRequest
     *
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     * @param int lastVehicleId the id of the last vehicle in the platoon
     */
    IntersectionMergeRequest* createIntersectionMergeRequest(int vehicleId, std::string externalId, int platoonId, int destinationId, int lastVehicleId);

    /**
     * Handles a IntersectionMergeRequest in the context of this application
     *
     * @param IntersectionMergeRequest msg to handle
     */
    virtual void handleIntersectionMergeRequest(const IntersectionMergeRequest* msg);

    /**
     * Handles a IntersectionMergeCompleted in the context of this application
     *
     * @param IntersectionMergeCompleted msg to handle
     */
    virtual void handleIntersectionMergeCompleted(const IntersectionMergeCompleted* msg);

    /**
     * Sends a merge request to the leader of the platoon that should be following us
     */
    void sendMergeRequest();

};

} // namespace plexe
