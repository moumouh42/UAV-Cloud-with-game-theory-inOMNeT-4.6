//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_UDPBASICAPP_H
#define __INET_UDPBASICAPP_H










#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"




namespace inet {

/**
 * UDP application. See NED for more info.
 */
class INET_API UDPBasicApp : public ApplicationBase
{
public:
    // Autres membres publics de la classe

    static std::vector<int> storage;
    static std::vector<int> timaemaxstorge;
    static std::vector<int> coutstarge;
    static std::vector<double> energydrone;
    static std::vector<int> Nbresend;





    // Autres membres publics de la classe

public:
    // Méthodes publiques de la classe

    static std::vector<int> calcstorage(int x);
    static std::vector<int> calctimemaxstorge(int x);
    static std::vector<int> calccoutstorge(int x);
    static std::vector<double> calcenergy(int x);
    static std::vector<int> calcNbresend(int x);
    static std::vector<std::vector<std::string>> parseData(const std::vector<std::string>& lines);
    static std::string removeSuffix(const std::string& str, const std::string& suffix);
    static std::vector<std::vector<double>> convertMatrix(const std::vector<std::vector<std::string>>& stringMatrix);
    static std::vector<std::vector<double>> calculateQProbabilities(const std::vector<std::vector<double>>& doubleMatrixClient,
            const std::vector<std::vector<double>>& doubleMatrixProvider);
    static std::string simtimeToString(const simtime_t& time);

protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };



//************************** mohamed added a values for the services and type of drones *************************
          simtime_t lastsent;
          std::string typdrone;
          std::string ServiceOffre;
          std::string ServiceRequest;
          std::vector<std::string> strListClient;
          std::vector<std::string> strListProvider;
          int lastpacketm=0;
          std::vector<std::vector<std::string>> strlistclientfinale;
          std::vector<std::vector<std::string>> strlistproviderfinale;



    // parameters
    std::vector<L3Address> destAddresses;
    int localPort = -1, destPort = -1;
    simtime_t startTime;
    simtime_t stopTime;
    const char *packetName = nullptr;

    // state
    UDPSocket socket;
    cMessage *selfMsg = nullptr;

    // statistics
    int numSent = 0;
    int numReceived = 0;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    // chooses random destination address
    virtual L3Address chooseDestAddr();
    virtual void sendPacket();
    virtual void processPacket(cPacket *msg);
    virtual void setSocketOptions();

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();


    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    virtual void handleNodeCrash() override;

  public:


    UDPBasicApp() {}
    ~UDPBasicApp();
};

} // namespace inet

#endif // ifndef __INET_UDPBASICAPP_H

