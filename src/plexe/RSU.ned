package org.car2x.plexe;

import org.car2x.veins.base.modules.IBaseApplLayer;
import org.car2x.veins.base.modules.BaseMobility;
import org.car2x.veins.modules.nic.Nic80211p;

import org.car2x.plexe.utilities.BasePositionHelper;
import org.car2x.plexe.scenarios.BaseScenario;
import org.car2x.plexe.protocols.BaseProtocol;
import org.car2x.plexe.apps.BaseApp;
import org.car2x.plexe.driver.Veins11pRadioDriver;

module RSU
{
    parameters:
        string appl_type; //type of the application layer

    submodules:
        appl: <appl_type> like org.car2x.veins.base.modules.IBaseApplLayer {
            parameters:
                @display("p=60,50");
        }

        nic: Nic80211p {
            parameters:
                @display("p=60,166");
        }

        mobility: BaseMobility {
            parameters:
                @display("p=300,205;i=block/cogwheel");
        }
        
    connections allowunconnected:
        nic.upperLayerOut --> appl.lowerLayerIn;
        nic.upperLayerIn <-- appl.lowerLayerOut;
        nic.upperControlOut --> appl.lowerControlIn;
        nic.upperControlIn <-- appl.lowerControlOut;
}