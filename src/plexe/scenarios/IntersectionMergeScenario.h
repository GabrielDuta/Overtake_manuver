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

#pragma once

#include "plexe/scenarios/ManeuverScenario.h"
#include "plexe/apps/GeneralPlatooningApp.h"
#include "plexe/messages/ManeuverMessage_m.h"
#include "plexe/utilities/DynamicPositionManager.h"
#include "plexe/maneuver/IntersectionMergeManeuver.h"

namespace plexe {

class IntersectionMergeScenario : public ManeuverScenario {

public:
    virtual void initialize(int stage) override;
    virtual void handleSelfMsg(cMessage* msg) override;

public:
    IntersectionMergeScenario()
        :positions(DynamicPositionManager::getInstance())
    {
    }

protected:

    void prepareManeuverCars();
    void setupFormation();

    // used to retrieve the initial formation setup
    DynamicPositionManager& positions;
};

} // namespace plexe
