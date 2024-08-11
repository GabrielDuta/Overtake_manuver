
package org.car2x.plexe.apps;
import org.car2x.plexe.apps.BaseApp;

simple DetachingApp like BaseApp {
  parameters:
    int headerLength @unit("bit") = default(0 bit);
    @class(plexe::DetachingApp);
  
  gates:
    input lowerLayerIn;
    output lowerLayerOut;
    input lowerControlIn;
    output lowerControlOut;
}
