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

#include "SimplePlatooningBeaconing.h"
#include "plexe/utilities/DynamicPositionManager.h"

namespace plexe {

class IntersectionMergingBeaconing : public SimplePlatooningBeaconing {
protected:

    bool enableFwdForwarding = false;
    bool enableLastForwarding = false;
    int platoonALeader = -1;
    int platoonALast = -1;
    int platoonCLast = -1;
    double fwdLeaderControllerAcceleration = 0;
    double fwdLeaderAcceleration = 0;
    double fwdLeaderSpeed = 0;
    double fwdLastControllerAcceleration = 0;
    double fwdLastAcceleration = 0;
    double fwdLastSpeed = 0;
    double fwdLastDistance = 0;

    DynamicPositionManager& positions;

    virtual void initialize(int stage) override;
    virtual std::unique_ptr<BaseFrame1609_4> createBeacon(int destinationAddress) override;
    virtual void messageReceived(PlatooningBeacon* pkt, BaseFrame1609_4* frame) override;

public:
    IntersectionMergingBeaconing();
    virtual ~IntersectionMergingBeaconing();

    /**
     * Enables/disables the forwarding of data about the preceding platoon
     */
    void setEnableFwdForwarding(bool forwarding);

    /**
     * Enables/disables the forwarding of data about last vehicle in own platoon
     */
    void setEnableLastForwarding(bool forwarding);
};

} // namespace plexe
