#include "plexe/apps/BaseApp.h"
#include "plexe/messages/AbandonPlatoon_m.h"
#include "plexe/messages/NewFormation_m.h"

namespace plexe {
class DetachingApp : public BaseApp {
  public:
    void sendAbandonMessage();
    virtual void sendUnicast(cPacket *msg, int destination);

  protected:
    virtual void initialize(int stage) override;
    virtual void handleLowerMsg(cMessage *msg) override;

  private:
    AbandonPlatoon* createAbandonMessage();
    NewFormation* createNewFormationMessage(const std::vector<int>& newPlatoonFormation);
    void handleAbandonPlatoon(const AbandonPlatoon* msg);
    void handleNewFormation(const NewFormation* msg);
    void sendNewFormationToFollowers(const std::vector<int>& newPlatoonFormation);
};

}
