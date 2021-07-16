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

vector<int> switchesConnected;

void writeInSwitchFileName(int fd,string msg){
    char msgChar[1024];
    strcpy(msgChar, msg.c_str());
    write(fd,msgChar,1025);
}

void connect(int sysfd,char* ans){
    bzero(ans,1025);
    read(sysfd,ans,1025);
    switchesConnected.push_back(atoi(ans));
}

void send(int sysfd,char* ans,int sysNum){
    bzero(ans,1025);
    read(sysfd,ans,1025);

    int destSys = atoi(ans);

    bzero(ans,1025);
    read(sysfd,ans,1025);
    string fileName = ans;

    cout << "File " << fileName << " should be delivered from system " << sysNum 
    << " to system " << destSys << endl;

    ifstream file(fileName);
    string fileLines = "";
    string everyFileLine;
    while (getline(file,everyFileLine))
        fileLines += everyFileLine + "\n";

    int position = 0;
    while (position < fileLines.size()) {
        string cutFile = fileLines.substr(position,1025);
        for (int i=0;i<switchesConnected.size();i++){
            int switchfd = open(("cache/sw" + to_string(switchesConnected[i])).c_str(),O_WRONLY);
            writeInSwitchFileName(switchfd,"sysSent");
            writeInSwitchFileName(switchfd,cutFile);
            writeInSwitchFileName(switchfd,to_string(destSys));
            writeInSwitchFileName(switchfd,to_string(sysNum));
            close(switchfd);
        }
        position += 1025;
    }
}

void switchSentToSystem(int sysfd,char* ans,int sysNum){
    bzero(ans,1025);
    read(sysfd,ans,1025);
    string msg = ans;

    bzero(ans,1025);
    read(sysfd,ans,1025);
    string dest = ans;

    if (to_string(sysNum) == dest)
        cout << "Message\n{" << msg << "}" << endl << "is delivered to system " << sysNum << endl;
}

int main(int argc,char** argv) {
    int sysNum = atoi(argv[0]);
    cout << "Creation of system " << sysNum << " was successful!" << endl;

    while (1) {
        int sysfd = open(("cache/sy" + to_string(sysNum)).c_str(),O_RDONLY);
        char ans[1025];
        bzero(ans,1025);
        read(sysfd,ans,1025);
        if (strcmp(ans,"Connect") == 0) connect(sysfd,ans);
        else if (strcmp(ans,"Send") == 0) send(sysfd,ans,sysNum);
        else if (strcmp(ans,"swSentToSy") == 0) switchSentToSystem(sysfd,ans,sysNum);
        close(sysfd);
    }
}