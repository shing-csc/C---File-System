#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include <string>

#include "../include/client_serverDir.h"



using namespace std;

void handleServerDirectory(int clientSocket, int bufferSize){

    string cmd;
    string dir;
    dir = "~";
    int BUFFER_SIZE = bufferSize;

    // Recurrsively looping by sending data packets to server indicating the action
    cout << endl;
    cout << "==================== TERMINAL ====================" << endl;
    
    while(true){
        
        cout << "commands: ls | cd <dir> | select <dir>" << endl;
        cout << "(base) user "<< dir << " $" << endl;
        cin >> cmd;
        
        // TODO: LS
        if (cmd == "ls"){

            char lsBuffer[BUFFER_SIZE];
            string fullResponse = "";

            send(clientSocket, cmd.c_str(), cmd.size(), 0);

            // Receive response from server
            while(true){

                ssize_t bytesReceived = recv(
                    clientSocket,
                    lsBuffer,
                    BUFFER_SIZE,
                    0
                );
                if (bytesReceived == 0) {
                    cout << "LOG: Server closed the connection." << endl;
                    break;
                }   
                fullResponse.append(lsBuffer, bytesReceived);             
            }
            cout << fullResponse << endl;
        }
        // TODO: CD
        else if (cmd == "cd"){
            cin >> dir;
            send(clientSocket, cmd.c_str(), cmd.size(), 0);

            // TODO: is input dir valid

        }

        // TODO: SELECT
        else if (cmd == "select"){
            cin >> dir;
            send(clientSocket, cmd.c_str(), cmd.size(), 0);

            // TODO: is input dir valid

        }
        else{
            cout << "Invalid Command, please enter again" << endl;
            continue;
        }

    }

}