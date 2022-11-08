//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

import org.car2x.plexe.PlexeScenario;
import org.car2x.veins.modules.obstacle.ObstacleControl;
import org.car2x.veinsrms.Rms;
import org.car2x.veinsrms.RmsConnectionManager;
import org.car2x.veins.visualizer.roads.RoadsCanvasVisualizer;

network Intersection extends PlexeScenario
{
    parameters:
        bool useBuildingShadowing;
    submodules:
        obstacles: ObstacleControl if useBuildingShadowing {
            @display("p=240,50");
        }
		obstaclesRms: ObstacleControl {
           @display("p=240,50");
        }
        rms: Rms {
        	@display("i=block/broadcast");
        }
        rmsConnectionManager: RmsConnectionManager {
        	parameters:
                @display("p=300,0;i=abstract/multicast");
        }
        shockwave[0]: ShockwaveCar {
        }
        roadsCanvasVisualizer: RoadsCanvasVisualizer {
            @display("p=300,0");
        }
}
