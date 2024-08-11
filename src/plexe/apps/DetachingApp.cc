#include "plexe/apps/DetachingApp.h"

#include "plexe/protocols/BaseProtocol.h"
#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"
#include "veins/base/utils/FindModule.h"
#include "plexe/scenarios/ManeuverScenario.h"

using namespace veins;

namespace plexe {
Define_Module(DetachingApp);

void DetachingApp::initialize(int stage){
  BaseApp::initialize(stage);
  int num = static_cast<std::underlying_type<ACTIVE_CONTROLLER>::type>(plexeTraciVehicle->getActiveController());
  getSimulation()->getActiveEnvir()->alert(std::to_string(num).c_str());
  if(stage == 1) {
    // connect application to lower layer
    protocol->registerApplication(MANEUVER_TYPE, gate("lowerLayerIn"),
      gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));
    // register to the signal indicating failed unicast transmissions
    findHost()->subscribe(Mac1609_4::sigRetriesExceeded, this);
  }
}

AbandonPlatoon* DetachingApp::createAbandonMessage(){
  AbandonPlatoon *msg = new AbandonPlatoon();

  int leaderId = positionHelper->getLeaderId();
  msg->setVehicleId(positionHelper->getId());
  msg->setPlatoonId(positionHelper->getPlatoonId());
  msg->setDestinationId(leaderId);
  msg->setExternalId(positionHelper->getExternalId().c_str());
  msg->setKind(MANEUVER_TYPE);

  return msg;
}

NewFormation* DetachingApp::createNewFormationMessage(const std::vector<int>& newPlatoonFormation) {
  NewFormation* nfmsg = new NewFormation();
  nfmsg->setKind(MANEUVER_TYPE);
  nfmsg->setPlatoonFormationArraySize(newPlatoonFormation.size());
  for(unsigned int i = 0; i < newPlatoonFormation.size(); i++) {
    nfmsg->setPlatoonFormation(i, newPlatoonFormation[i]);
  }
  return nfmsg;
}

void DetachingApp::sendAbandonMessage() {
  getSimulation()->getActiveEnvir()->alert("Sending an abandon message");
  AbandonPlatoon* abmsg = createAbandonMessage();
  sendUnicast(abmsg, abmsg->getDestinationId());
}

void DetachingApp::sendNewFormationToFollowers(const std::vector<int>& newPlatoonFormation) {
  NewFormation* nfmsg = createNewFormationMessage(newPlatoonFormation);
  int dest;

  for(int i = 1; i < newPlatoonFormation.size(); i++) {
    dest = newPlatoonFormation[i];
    NewFormation* dup = nfmsg->dup();
    dup->setDestinationId(dest);
    sendUnicast(dup, dest);
  }
  delete nfmsg;
} 

void DetachingApp::sendUnicast(cPacket* msg, int destination){
  Enter_Method_Silent();
  take(msg);
  BaseFrame1609_4 *frame = new BaseFrame1609_4("BaseFrame1609_4",
                                               msg->getKind());
  frame->setRecipientAddress(destination);
  frame->setChannelNumber(static_cast<int>(Channel::cch));
  frame->encapsulate(msg);
  // send unicast frames using 11p only
  PlexeInterfaceControlInfo *ctrl = new PlexeInterfaceControlInfo();
  ctrl->setInterfaces(PlexeRadioInterfaces::VEINS_11P);
  frame->setControlInfo(ctrl);
  sendDown(frame);
}

void DetachingApp::handleAbandonPlatoon(const AbandonPlatoon* msg) {
  if(msg->getPlatoonId() != positionHelper->getPlatoonId())
    return;
  if(msg->getDestinationId() != positionHelper->getLeaderId())
    return;
  if(msg->getDestinationId() != positionHelper->getId())
    return;

  // releveant information from Abandon Messge
  int leaderId, platoonId, leaverId;
  leaderId = positionHelper->getId();
  leaverId = msg->getVehicleId();
  platoonId = msg->getPlatoonId();

  // Inforing SUMO via plexe interface to remove vehicle from platoon
  plexeTraciVehicle->removePlatoonMember(msg->getExternalId());

  // Change platoon formation
  std::vector<int> pFormation = positionHelper->getPlatoonFormation();

  // Remove vehicle
  pFormation.pop_back();
  positionHelper->setPlatoonFormation(pFormation);

  char text[250];
  sprintf(text, "LEADER[%d]: I'm removing v<%d> from platoon<%d>\n", leaderId, leaverId, platoonId);
  LOG << text << endl;
  getSimulation()->getActiveEnvir()->alert(text);

  sendNewFormationToFollowers(pFormation);
}

void DetachingApp::handleNewFormation(const NewFormation* msg) {
  std::vector<int> newFormation;
    for (int i = 0; i < msg->getPlatoonFormationArraySize(); i++)
        newFormation.push_back(msg->getPlatoonFormation(i));

    std::string formationString = "[ ";
    for (int i = 0; i < newFormation.size(); i++) {
        formationString += std::to_string(newFormation[i]) + " ";
    }
    formationString += "]";

    char text[250];
    sprintf(text, "v<%d> got newFormation = %s\n", positionHelper->getId(), formationString.c_str());
    getSimulation()->getActiveEnvir()->alert(text);

    positionHelper->setPlatoonFormation(newFormation);
}

void DetachingApp::handleLowerMsg(cMessage* msg){
  BaseFrame1609_4 *frame = check_and_cast<BaseFrame1609_4*>(msg);

  cPacket* enc = frame->getEncapsulatedPacket();
  ASSERT2(enc, "received a BaseFrame1609_4s with nothing inside");

  if(enc->getKind() == MANEUVER_TYPE) {
    ManeuverMessage *mm = check_and_cast<ManeuverMessage*>(frame->decapsulate());
    if(AbandonPlatoon* msg = dynamic_cast<AbandonPlatoon*>(mm)) {
      handleAbandonPlatoon(msg);
      delete msg;
    }
    else if(NewFormation* msg = dynamic_cast<NewFormation*>(mm)) {
      handleNewFormation(msg);
      delete msg;
    }
    delete frame;
  }
  else BaseApp::handleLowerMsg(msg);
}
}
