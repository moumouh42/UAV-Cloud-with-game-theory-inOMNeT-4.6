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

#include "inet/applications/udpapp/UDPBasicApp.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"

#include <inet/power/contract/IEnergyStorage.h>
#include <inet/common/INETDefs.h>

#include <string>      // for std::string
#include <vector>      // for std::vector
#include <sstream>     // for std::istringstream
#include <algorithm>   // for std::remove




namespace inet {

Define_Module(UDPBasicApp);

simsignal_t UDPBasicApp::sentPkSignal = registerSignal("sentPk");
simsignal_t UDPBasicApp::rcvdPkSignal = registerSignal("rcvdPk");

//******************************** Mohamed added a vectors (seize of 50 drones) that represent all drones' storages which are generated randomly ******************

std::vector<int> UDPBasicApp::storage=UDPBasicApp::calcstorage(50);
std::vector<int> UDPBasicApp::timaemaxstorge=UDPBasicApp::calctimemaxstorge(50);
std::vector<int> UDPBasicApp::coutstarge=UDPBasicApp::calccoutstorge(50);
std::vector<double> UDPBasicApp::energydrone=UDPBasicApp::calcenergy(50);
std::vector<int> UDPBasicApp::Nbresend=UDPBasicApp::calcNbresend(50);







UDPBasicApp::~UDPBasicApp()
{
    cancelAndDelete(selfMsg);
}

void UDPBasicApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);


    if (stage == INITSTAGE_LOCAL) {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime").doubleValue();
        stopTime = par("stopTime").doubleValue();
        packetName = par("packetName");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("sendTimer");


//**************** asma takes the half of drones as providers *******************************************************
        int r = getParentModule()->getId() % 2;

   //*************************************** Resources affectation ********************************************************

        int sto =storage[getParentModule()->getId()];
        int timesto = timaemaxstorge[getParentModule()->getId()];
        int coutsto = coutstarge[getParentModule()->getId()];
        double energ = energydrone[getParentModule()->getId()];
        int sendn = Nbresend[getParentModule()->getId()];

        if (r==0) {
            typdrone = "Supplier";


            ServiceOffre = std::to_string(getParentModule()->getId())+" "+ typdrone+ " "+std::to_string(sto) + "Mo " + std::to_string(timesto) + "h " + std::to_string(coutsto) + "$/h"+" "+std::to_string(energ)+"j "+std::to_string(sendn);

            std::cout << "I am Drone: " <<getParentModule()->getId() <<" in initialization"<<" i am a "<<typdrone<< endl;

                              std::cout<<"I offer storage like Saas: "<<ServiceOffre<<endl;
                              std::cout<<endl;
        } else
        {

            typdrone = "Client";


            ServiceRequest = std::to_string(getParentModule()->getId())+" "+ typdrone+ " "+std::to_string(sto) + "Mo " + std::to_string(timesto) + "h " + std::to_string(coutsto) + "$/h"+" "+std::to_string(energ)+"j "+std::to_string(sendn);
            std::cout << "I am Drone: " <<getParentModule()->getId() <<" in initialization"<<" i am a "<<typdrone<< endl;

                                          std::cout<<"I request storage like Saas: "<<ServiceRequest<<endl;
                                          std::cout<<endl;

        }
//****************************************************************************




        }



    }


void UDPBasicApp::finish()
{

    //*******************************************************************************************************

    if (simTime() == 100) {
                    std::cout  << " **************************************************************** "<<endl;
                for (const std::string& str : strListProvider) {
                                std::cout << str << " ";
                                std::cout<<endl;
                            }


                for (const std::string& str : strListClient) {
                    std::cout << str << " ";
                    std::cout<<endl;
                }
                }







    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
    //std::cout<<"mouuuuuuuuuuuuuuuuuuh finish"<<endl;
}

void UDPBasicApp::setSocketOptions()
{

    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int typeOfService = par("typeOfService");
    if (typeOfService != -1)
        socket.setTypeOfService(typeOfService);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0]) {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        InterfaceEntry *ie = ift->getInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups) {
        MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
        socket.joinLocalMulticastGroups(mgl);
    }
}

L3Address UDPBasicApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if (destAddresses[k].isLinkLocal()) {    // KLUDGE for IPv6
        const char *destAddrs = par("destAddresses");
        cStringTokenizer tokenizer(destAddrs);
        const char *token = nullptr;

        for (int i = 0; i <= k; ++i)
            token = tokenizer.nextToken();
        destAddresses[k] = L3AddressResolver().resolve(token);
    }
    return destAddresses[k];
}

void UDPBasicApp::sendPacket()
{


    std::ostringstream str;
    str << packetName << "-" << numSent;
    cPacket *payload = new cPacket(str.str().c_str());
    payload->setByteLength(par("messageLength").longValue());


       // send request or registration packet to the Gsc including energy value **************************************
    if (typdrone=="Supplier") {



    payload->addPar("info").setStringValue(ServiceOffre.c_str());
    } else {




        payload->addPar("info").setStringValue(ServiceRequest.c_str());
    }
    L3Address destAddr = chooseDestAddr();

    emit(sentPkSignal, payload);
    socket.sendTo(payload, destAddr, destPort);
    numSent++;

   // std::cout << "send packet UDP with additional information: " << additionalInfo <<" "<<getParentModule()->getId()<<endl;
}

void UDPBasicApp::processStart()
{

    socket.setOutputGate(gate("udpOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    setSocketOptions();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != nullptr) {
        L3Address result;
        L3AddressResolver().tryResolve(token, result);
        if (result.isUnspecified())
            EV_ERROR << "cannot resolve destination address: " << token << endl;
        else
            destAddresses.push_back(result);
    }

    if (!destAddresses.empty()) {
        selfMsg->setKind(SEND);
        processSend();
    }
    else {
        if (stopTime >= SIMTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }

}

void UDPBasicApp::processSend()
{

    sendPacket();
    simtime_t d = simTime() + par("sendInterval").doubleValue();
    if (stopTime < SIMTIME_ZERO || d < stopTime) {
        selfMsg->setKind(SEND);
        scheduleAt(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}

void UDPBasicApp::processStop()
{
    socket.close();
}

void UDPBasicApp::handleMessageWhenUp(cMessage *msg)
{


    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
                processSend();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else if (msg->getKind() == UDP_I_DATA) {
       // int sourcemodule =  getParentModule()->getId();



 //       std::cout <<" i am "<<sourcemodule<< " Received packet: " << msg->getName() <<" from "<< msg->getSenderModuleId()<< endl;


        // process incoming packet
        processPacket(PK(msg));
    }
    else if (msg->getKind() == UDP_I_ERROR) {
        EV_WARN << "Ignoring UDP error report\n";
        delete msg;
    }
    else {
        throw cRuntimeError("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (hasGUI()) {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void UDPBasicApp::processPacket(cPacket *pk)
{
//std::cout<<"receiving packet udpkkkk"<<endl;
    emit(rcvdPkSignal, pk);
  //  std::cout << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) <<" "<< pk->getId()<< endl;
  //********* mohamed try to display all the regestration and request packet that are sent to the Gsc

    if (pk->hasPar("info")) {

                   // receiving data in both cases request or registration and store them in two lists to apply game theory
            std::string Datadrone = pk->par("info").stringValue();


            if (Datadrone.find("Client") != std::string::npos) {

                  bool existsclient = false;
                    for (const std::string& str : strListClient) {
                       if (str == Datadrone) {
                           existsclient = true;
                            break;
                                             }
                                                                  }

                      // Add Datadrone to strListClient if it doesn't exist
                      if (!existsclient) {
                          std::cout << "Gsc has Received UDP packet that contain " << Datadrone << std::endl;
                         strListClient.push_back(Datadrone);
                                   }
                }
              else {

                    bool existsprovider = false;
                        for (const std::string& str : strListProvider) {
                         if (str == Datadrone) {
                             existsprovider = true;
                              break;
                                               }
                                 }
                                 // Add Datadrone to strListProvider if it doesn't exist
                             if (!existsprovider) {
                                 std::cout << "Gsc has Received UDP packet that contain " << Datadrone << std::endl;
                                 strListProvider.push_back(Datadrone);
                                                  }
              }

// ********************** the last packet triggers the Game theory process *****************************************
            //******************** to display the UAV cloud ***********************************************************
            std::string lastdrone = Datadrone.substr(0, 2);
            if ((lastdrone == std::to_string(10)) && lastpacketm == 3) {



                 std::cout  << " **************************************************************** UAV Cloud Contain these information"<<endl;
                                for (const std::string& str : strListProvider) {
                                                std::cout << str << " ";
                                                std::cout<<endl;
                                            }


                                for (const std::string& str : strListClient) {
                                    std::cout << str << " ";
                                    std::cout<<endl;
                                }

                                std::cout  << " **************************************************************** UAV Cloud with its information values "<<endl;
                                std::vector<std::vector<std::string>> strlistclientfinale = UDPBasicApp::parseData(strListClient);
                                std::vector<std::vector<std::string>> strlistproviderfinale = UDPBasicApp::parseData(strListProvider);

                                //*************************** convert the list to double in order to calculate q, p probabilities

                                std::vector<std::vector<double>> doubleMatrixClient =  UDPBasicApp::convertMatrix(strlistclientfinale);
                                std::vector<std::vector<double>> doubleMatrixProvider =  UDPBasicApp::convertMatrix(strlistproviderfinale);

                                std::cout << "Parsed Data from doubleMatrixClient:" << std::endl;
                                for (const auto& row : doubleMatrixClient) {
                                    for (const auto& token : row) {
                                        std::cout << token << " ";
                                    }
                                    std::cout << std::endl;
                                }

                                std::cout << "Parsed Data from doubleMatrixProvider:" << std::endl;
                                for (const auto& row : doubleMatrixProvider) {
                                    for (const auto& token : row) {
                                        std::cout << token << " ";
                                    }
                                    std::cout << std::endl;
                                }

                                std::cout  << " ****************************************************************  Probabilities of all drone Clients with providers Q  "<<endl;

                                std::vector<std::vector<double>> Qlistclient = calculateQProbabilities(doubleMatrixClient, doubleMatrixProvider);
                                std::vector<std::vector<double>> Qlistprovider = calculateQProbabilities(doubleMatrixProvider, doubleMatrixClient);

                                    // Print the Qlistclient
                                    for (const auto& qElement : Qlistclient) {
                                        std::cout << qElement[0] << " " << qElement[1] << " Q " << qElement[2] << std::endl;
                                    }
                                    std::cout  << " ****************************************************************  Probabilities of all drone Providers with clients P  "<<endl;

                                    // Print the Qlistclient
                                                                        for (const auto& qElement : Qlistprovider) {
                                                                            std::cout << qElement[0] << " " << qElement[1] << " P " << qElement[2] << std::endl;
                                                                        }







                     } else { if (lastdrone == std::to_string(10)){
                lastpacketm++;}
                        }




        } else {
            std::cout << "Received packet without data" <<endl;
        }

    //EV_INFO << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
    numReceived++;
}





bool UDPBasicApp::handleNodeStart(IDoneCallback *doneCallback)
{
    // std::cout<<"mouuuuuuuuuuuuuuuuuuh handl start nooooo"<<endl;
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
    return true;
}

bool UDPBasicApp::handleNodeShutdown(IDoneCallback *doneCallback)
{
    //std::cout<<"mouuuuuuuuuuuuuuuuuuh handle sht"<<endl;
    if (selfMsg)
        cancelEvent(selfMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPBasicApp::handleNodeCrash()
{
   // std::cout<<"mouuuuuuuuuuuuuuuuuuh handelcrash"<<endl;
    if (selfMsg)
        cancelEvent(selfMsg);
}



//*********************************Functions for random storage values *********************

std::vector<int> UDPBasicApp::calcstorage(int x ){
    std::vector<int>storage;
for(int i=1;i<= x ;i++){
int y = rand()%9;
if (y==0){
        y=70;
        storage.push_back(y);
} else {

    storage.push_back(y*10); }


}
return storage;
}

std::vector<int> UDPBasicApp::calctimemaxstorge(int x){
    std::vector<int>time;
    for(int i=1;i<= x ;i++){
    int y = rand()%9;

    if (y==0){
            y=7;
            time.push_back(y);
    } else {

        time.push_back(y); }

}
    return time;
}


std::vector<int> UDPBasicApp::calccoutstorge(int x){

    std::vector<int>coutstor;
        for(int i=1;i<= x ;i++){
        int y = rand()%3;
        if (y==0){ y=y+2;
        coutstor.push_back(y);
        } else {
            coutstor.push_back(y);
        }
        }
        return coutstor;

}

std::vector<double> UDPBasicApp::calcenergy(int x) {
    std::vector<double> energyValues;
    for (int i = 0; i < x; i++) {
        double energy = std::round((rand() % 9 + 1)) / 10.0;
        energyValues.push_back(energy);
    }
    return energyValues;
}


std::vector<int> UDPBasicApp::calcNbresend(int x) {
    std::vector<int> variable;
    for (int i = 0; i < x; i++) {
        double energy = (rand() % 41 + 10) / 10.0;
        variable.push_back(energy);
    }
    return variable;
}

std::string UDPBasicApp::removeSuffix(const std::string& str, const std::string& suffix) {
    if (str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix) {
        return str.substr(0, str.size() - suffix.size());
    }
    return str;
}

std::vector<std::vector<std::string>> UDPBasicApp::parseData(const std::vector<std::string>& lines) {
    std::vector<std::vector<std::string>> matrix;
    for (const std::string& line : lines) {
        std::vector<std::string> row;
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) {
            if (token != "Client" && token != "Supplier") {
                token = UDPBasicApp::removeSuffix(token, "Mo");
                token = UDPBasicApp::removeSuffix(token, "h");
                token = UDPBasicApp::removeSuffix(token, "/h");
                token = UDPBasicApp::removeSuffix(token, "j");
                token.erase(std::remove(token.begin(), token.end(), '$'), token.end());
                token.erase(std::remove(token.begin(), token.end(), 'C'), token.end()); // Remove 'C' character
                token.erase(std::remove(token.begin(), token.end(), 'S'), token.end()); // Remove 'S' character
                token.erase(std::remove(token.begin(), token.end(), '/'), token.end()); // Remove '/' character

                row.push_back(token);
            }
        }
        matrix.push_back(row);
    }
    return matrix;
}

std::vector<std::vector<double>> UDPBasicApp::convertMatrix(const std::vector<std::vector<std::string>>& stringMatrix)
{
    std::vector<std::vector<double>> doubleMatrix;

    for (const auto& row : stringMatrix)
    {
        std::vector<double> doubleRow;

        for (const auto& str : row)
        {
            try {
                double value = std::stod(str);
                doubleRow.push_back(value);
            } catch (const std::exception& e) {
                // Handle conversion errors
                std::cerr << "Error converting string to double: " << e.what() << std::endl;
                // You can choose to handle the error in a different way, such as skipping the value
                // or setting a default value in the double matrix.
            }
        }

        doubleMatrix.push_back(doubleRow);
    }

    return doubleMatrix;
}

//****************************************** function that calculates probabilities

std::vector<std::vector<double>> UDPBasicApp::calculateQProbabilities(const std::vector<std::vector<double>>& doubleMatrixClient,
                                                         const std::vector<std::vector<double>>& doubleMatrixProvider) {
    std::vector<std::vector<double>> Qlist;

    for (const auto& clientLine : doubleMatrixClient) {
        for (const auto& providerLine : doubleMatrixProvider) {
            double q = (providerLine[5]*providerLine[1]-(providerLine[4]+providerLine[3]))/(providerLine[5]*providerLine[1] +providerLine[2]);

            Qlist.push_back({clientLine[0], providerLine[0], q});
        }
    }

    return Qlist;
}






} // namespace inet



