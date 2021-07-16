#include <iostream>
#include <bits/stdc++.h>
#include <vector>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fstream>
#include <ftw.h>
#include <algorithm>
#include <signal.h>
#include <stdlib.h>

using namespace std;

vector<vector<int>> matrix;
vector<vector<int>> eachNodeConnections;
vector<int> switches;

vector<string> splitCommand(string str) {
    vector<string> result;
    string word = "";
    for (auto x : str) {
        if (x == ' ') {
            result.push_back(word);
            word = "";
        }
        else word = word + x;
    }
    result.push_back(word);
    return result;
}

void showCmds(){
    cout << "Commands: " << endl;
    cout << "MySwitch <number_of_ports> <switch_number>" << endl;
    cout << "MySystem <system_number>" << endl;
    cout << "Connect <system_number> <switch_number> <port_number>" << endl;
    cout << "Send <source_system_number> <destination_system_number> <message>" << endl;
    cout << "Connect_Switch <switch_number_1> <switch_number_2> <port_number_switch_1> <port_number_switch_2>" << endl;
    cout << "Exit" << endl;
}

void writeInSwitchFileName(int fd,string msg){
    char msgChar[1024];
    strcpy(msgChar, msg.c_str());
    write(fd,msgChar,1025);
}

void createSwitch(vector<string> lineVec){
    for (int i=0;i<switches.size();i++)
        if (switches[i] == stoi(lineVec[2])){
            cout << "Switch " << lineVec[2] << " has been already created!" << endl;
            return;
        }
    switches.push_back(stoi(lineVec[2]));
    sort(switches.begin(),switches.end());
    int tmpIndx;
    for (int i=0;i<switches.size();i++)
        if (switches[i] == stoi(lineVec[2])){
            tmpIndx = i;
            break;
        }

    eachNodeConnections.insert(eachNodeConnections.begin() + tmpIndx, vector<int>());
    int pid = fork();
    if (pid == 0) {
        char *numOfPortsCstr = new char[lineVec[1].length() + 1];
        strcpy(numOfPortsCstr, lineVec[1].c_str());
        char *switchNumCstr = new char[lineVec[2].length() + 1];
        strcpy(switchNumCstr, lineVec[2].c_str());
        char* msg[] = {numOfPortsCstr,switchNumCstr,NULL};
        mkfifo(("cache/sw" + lineVec[2]).c_str(),0666);
        execvp("./switch.out",msg);
    }
}

void createSystem(vector<string> lineVec){
    int pid = fork();
    if (pid == 0){
        char *sysNumCstr = new char[lineVec[1].length() + 1];
        strcpy(sysNumCstr, lineVec[1].c_str());
        char* msg[] = {sysNumCstr,NULL};
        mkfifo(("cache/sy" + lineVec[1]).c_str(),0666);
        execvp("./system.out",msg);
    }
}

void connect(vector<string> lineVec){
    string switchFileName = "cache/sw" + lineVec[2];
    int fd = open(switchFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong switch number. Try again!" << endl;
        return;
    }
    writeInSwitchFileName(fd,"Connect");
    writeInSwitchFileName(fd,lineVec[1]);
    writeInSwitchFileName(fd,lineVec[3]);
    close(fd);

    string sysFileName = "cache/sy" + lineVec[1];
    fd = open(sysFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong system number. Try again!" << endl;
        return;
    }
    writeInSwitchFileName(fd,lineVec[0]);
    writeInSwitchFileName(fd,lineVec[2]);

    close(fd);
}

void makeEachNodeConnections(vector<string> lineVec){
    int switchOneIndx,switchTwoIndx;
    for (int i=0;i<switches.size();i++){
        if (switches[i] == stoi(lineVec[1])) switchOneIndx = i;
        if (switches[i] == stoi(lineVec[2])) switchTwoIndx = i;
    }

    eachNodeConnections[switchOneIndx].push_back(switches[switchTwoIndx]);
    eachNodeConnections[switchTwoIndx].push_back(switches[switchOneIndx]);
}

int findIndx(int switchNum){
    for (int indx=0;indx < switches.size();indx++)
        if (switchNum == switches[indx])
            return indx;
    return -1;
}

void createMatrix(){
    vector<vector<int>> tmp;
    for (int i=0;i<eachNodeConnections.size();i++){
        tmp.push_back(vector<int>());
        for (int j=0;j<eachNodeConnections.size();j++) tmp[i].push_back(0);
    }

    for (int i=0;i<eachNodeConnections.size();i++) {
        for (int j=0;j<eachNodeConnections[i].size();j++){
            int indx = findIndx(eachNodeConnections[i][j]);
            tmp[i][indx] = 1;
        }
    }

    matrix = tmp;
}

void connectSwitch(vector<string> lineVec){
    string switchNum1 = "cache/sw" + lineVec[1];
    string switchNum2 = "cache/sw" + lineVec[2];
    string port1 = lineVec[3];
    string port2 = lineVec[4];

    int fd = open(switchNum1.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong switch number. Try again!" << endl;
        return;
    }

    makeEachNodeConnections(lineVec);

    writeInSwitchFileName(fd,lineVec[0]);
    writeInSwitchFileName(fd,lineVec[2]);
    writeInSwitchFileName(fd,lineVec[3]);

    close(fd);

    int fd2 = open(switchNum2.c_str(),O_WRONLY);
    if (fd2 == -1){
        cerr << "Wrong switch number. Try again!" << endl;
        return;
    }

    writeInSwitchFileName(fd2,lineVec[0]);
    writeInSwitchFileName(fd2,lineVec[1]);
    writeInSwitchFileName(fd2,lineVec[4]);

    close(fd2);

}

int minKey(int key[], bool mstSet[]) {
    int min = INT_MAX, min_index; 
    for (int v = 0; v < matrix.size(); v++) 
        if (mstSet[v] == false && key[v] < min) 
            min = key[v], min_index = v; 
  
    return min_index; 
} 
  
void printMST(int parent[]) {
    for (int i=0;i<matrix.size();i++)
        for (int j=0;j<matrix[i].size();j++)
            matrix[i][j] = 0;

    cout << "Available connections between switches are" << endl;
    for (int i = 1; i < matrix.size(); i++){
        matrix[parent[i]][i] = 1;
        matrix[i][parent[i]] = 1;
        cout << "Switch " << switches[parent[i]] <<" is connected to switch "<<switches[i] << endl;
    }
} 
  
void spanningMST() {
    int parent[matrix.size()];
    int key[matrix.size()]; 
    bool mstSet[matrix.size()]; 
    for (int i = 0; i < matrix.size(); i++) 
        key[i] = INT_MAX, mstSet[i] = false; 
  
    key[0] = 0;
    parent[0] = -1; 
  
    for (int count = 0; count < matrix.size() - 1; count++) {
        int u = minKey(key, mstSet); 
        mstSet[u] = true; 
        for (int v = 0; v < matrix.size(); v++) 
            if (matrix[u][v] && mstSet[v] == false && matrix[u][v] < key[v]) 
                parent[v] = u, key[v] = matrix[u][v]; 
    }
    printMST(parent); 
}

void informSwitchesEnablePorts(){
    for (int i=0;i<matrix.size();i++){
        for (int j=0;j<matrix[i].size();j++) {
            if (matrix[i][j] == 1) {
                int fd = open(("cache/sw" + to_string(switches[i])).c_str(),O_WRONLY);
                writeInSwitchFileName(fd,"spanning");
                writeInSwitchFileName(fd,to_string(switches[j]));
                close(fd);
            }
        }
    }
}

void send(vector<string> lineVec){
    string srcSysFileName = "cache/sy" + lineVec[1];
    int fd = open(srcSysFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong system number. Try again!" << endl;
        return;
    }

    cout << "Spanning tree algorithm is running..." << endl;
    createMatrix();
    spanningMST();
    informSwitchesEnablePorts();

    writeInSwitchFileName(fd,lineVec[0]);
    writeInSwitchFileName(fd,lineVec[2]);
    writeInSwitchFileName(fd,lineVec[3]);
    close(fd);
}

static int rmFiles(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb) {
    if(remove(pathname) < 0) {
        cout<< "ERROR: remove" << endl;
        return -1;
    }
    return 0;
}

void Exit(int s){
    if (nftw("cache", rmFiles,10, FTW_DEPTH|FTW_MOUNT|FTW_PHYS) < 0){
        cout << "Couldnt exit!" << endl;
        return;
    }
    exit(-1);
}

bool checkEmptyLine(string line){
    for (int i=0;i<line.size();i++)
        if (line[i] != ' ') return false;
    return true;
}


int main(int argc,char** argv){
    string line;
    vector<string> lineVec;
    mkdir("cache",0777);
    showCmds();
    while (getline(cin,line)) {
        lineVec = splitCommand(line);
        if (checkEmptyLine(line)) continue;
        if (lineVec[0] == "MySwitch" && lineVec.size() == 3) createSwitch(lineVec);
        else if (lineVec[0] == "MySystem" && lineVec.size() == 2) createSystem(lineVec);
        else if (lineVec[0] == "Connect" && lineVec.size() == 4) connect(lineVec);
        else if (lineVec[0] == "Send" && lineVec.size() == 4) send(lineVec);            
        else if (lineVec[0] == "Connect_Switch" && lineVec.size() == 5) connectSwitch(lineVec);
        else if (lineVec[0] == "Exit" && lineVec.size() == 1) Exit(1);  
        else cout << "Invalid command!" << endl;

        struct sigaction sigIntHandler;
        sigIntHandler.sa_handler = Exit;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, NULL);
    }
}