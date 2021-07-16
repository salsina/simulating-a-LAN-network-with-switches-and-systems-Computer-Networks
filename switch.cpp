#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>

using namespace std;

vector<int> lookupTable;
vector<int> connections;
vector<bool> enables;
vector<string> switchOrSystem;
int switchNumber;

void initialLookUpTable(int numOfPort){
    for (int i=0;i < numOfPort;i++){
        lookupTable.push_back(-1);
        connections.push_back(-1);
        switchOrSystem.push_back("system");
        enables.push_back(false); 
    }
}

void setSrcPort(int portNumber,int src,string determine = "system"){ 
    connections[portNumber - 1] = src;
    switchOrSystem[portNumber - 1] = determine;
}

void writeInFileName(int fd,string msg){
    char msgChar[1024];
    strcpy(msgChar, msg.c_str());
    write(fd,msgChar,1025);
}


void findDestAndSendMsg(int dest,string msg,int srcSw = -1){
    for (int i=0;i<lookupTable.size();i++){
        if (lookupTable[i] == -1) continue;
        if (lookupTable[i] == dest && switchOrSystem[i] == "system"){
            int sysfd = open(("cache/sy" + to_string(dest)).c_str(),O_WRONLY);
            writeInFileName(sysfd,"swSentToSy");
            writeInFileName(sysfd,msg);
            writeInFileName(sysfd,to_string(dest));
            close(sysfd);
            return;
        }
    }

    //send msg to all
    cout << "Switch " << switchNumber << " is broadcasting..." << endl;
    for (int i=0;i<connections.size();i++){
        if (connections[i] == -1) continue;
        if (switchOrSystem[i] == "switch" && connections[i] != srcSw && enables[i]){
            int swFd = open(("cache/sw" + to_string(connections[i])).c_str(),O_WRONLY);
            writeInFileName(swFd,"swSentToSw");
            writeInFileName(swFd,msg);
            writeInFileName(swFd,to_string(dest));
            writeInFileName(swFd,to_string(switchNumber));
            close(swFd);
        }
        else if (switchOrSystem[i] == "system"){
            int sysfd = open(("cache/sy" + to_string(connections[i])).c_str(),O_WRONLY);
            writeInFileName(sysfd,"swSentToSy");
            writeInFileName(sysfd,msg);
            writeInFileName(sysfd,to_string(dest));
            close(sysfd);
        }
    }

    for (int i=0;i<enables.size();i++)
        enables[i] = false;
}

void connect(int switchfd,char* ans){
    bzero(ans,1025);
    read(switchfd,ans,1025);
    int sysNum = atoi(ans);
    bzero(ans,1025);
    read(switchfd,ans,1025);
    int port = atoi(ans);

    setSrcPort(port,sysNum);
    
    cout << "Connection of system " << sysNum << " to switch " << switchNumber << " by port " << port
            << " was successful!" << endl;
}

void connectSwitch(int switchfd,char* ans){
    bzero(ans,1025);
    read(switchfd,ans,1025);
    int switchNum2 = atoi(ans);
    bzero(ans,1025);
    read(switchfd,ans,1025);
    int port = atoi(ans);

    setSrcPort(port,switchNum2,"switch");

    cout << "Connection of switch " << switchNumber << " to switch " << switchNum2 << " by port " << port
            << " was successful!" << endl;
}

void systemSent(int switchfd,char* ans){
    bzero(ans,1025);
    read(switchfd,ans,1025);
    string msg = ans;

    bzero(ans,1025);
    read(switchfd,ans,1025);
    int destSys = atoi(ans);

    bzero(ans,1025);
    read(switchfd,ans,1025);
    int srcSys = atoi(ans);

    for (int i=0;i<connections.size();i++)
        if (connections[i] == srcSys && switchOrSystem[i] == "system"){
            int port = i;
            lookupTable[port] = srcSys;
            break;
        }

    findDestAndSendMsg(destSys,msg);
}

void switchSentToSwitch(int switchfd,char* ans){
    bzero(ans,1025);
    read(switchfd,ans,1025);
    string msg = ans;

    bzero(ans,1025);
    read(switchfd,ans,1025);
    int destSys = atoi(ans);

    bzero(ans,1025);
    read(switchfd,ans,1025);
    int srcSw = atoi(ans);

    for (int i=0;i<connections.size();i++){
        if (connections[i] == srcSw && switchOrSystem[i] == "switch"){
            int port = i;
            lookupTable[port] = srcSw;
            break;
        }
    }

    findDestAndSendMsg(destSys,msg,srcSw);
}

void span(int switchfd,char* ans){
    bzero(ans,1025);
    read(switchfd,ans,1025);
    int enableSwitchNum = atoi(ans);

    for (int i=0;i<connections.size();i++)
        if (switchOrSystem[i] == "switch" && connections[i] == enableSwitchNum)
            enables[i] = true;
        
}

int main(int argc,char** argv) {
    int numOfPorts;
    numOfPorts = atoi(argv[0]);
    switchNumber = atoi(argv[1]);
    cout << "Creation of switch " << switchNumber << " was successful!" << endl;
    initialLookUpTable(numOfPorts);

    while(1) {
        int switchfd = open(("cache/sw" + to_string(switchNumber)).c_str(),O_RDONLY);
        char ans[1025];
        bzero(ans,1025);
        read(switchfd,ans,1025);
        if (strcmp(ans,"Connect") == 0) connect(switchfd,ans);
        else if (strcmp(ans,"Connect_Switch") == 0) connectSwitch(switchfd,ans);
        else if (strcmp(ans,"sysSent") == 0) systemSent(switchfd,ans);
        else if (strcmp(ans,"swSentToSw") == 0) switchSentToSwitch(switchfd,ans);
        else if (strcmp(ans,"spanning") == 0) span(switchfd,ans);
        close(switchfd);
    }
}