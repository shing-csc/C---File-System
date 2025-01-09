#include <iostream>
#include <fstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include <string>

#include "../include/client_upload.h"
//#include "client_serverDir.h"

using namespace std;

void handleUpload(int clientSocket, int bufferSize){

    //handleServerDirectory(clientSocket, bufferSize);
    
    string filename; 
    string targetFilename;

    int BUFFER_SIZE = bufferSize;
    char buffer[BUFFER_SIZE];

    cout << "==== Ensure that the file to be upload is in '/client_directory' ====" << endl;

    
    cout << "Enter the filename of the file to upload: " << endl;
    cin >> filename;

    if (send(clientSocket, filename.c_str(), filename.size(), 0) == -1) {
        cerr << "Failed to send filename to server." << endl;
        close(clientSocket);
        return;
    }

    string directory = "../../client_directory/";
    string fullFilePath = directory + filename;
    char buffer_validFile[BUFFER_SIZE];

    ifstream inFile(fullFilePath, ios::binary);
    if (!inFile.is_open()){
        send(clientSocket, "INVALID", strlen("INVALID"), 0);
        //cerr << "Failed to open file " << filename << endl;
        close(clientSocket);
        return;
    }
    else{
        send(clientSocket, "VALID", strlen("VALID"), 0);
    }

    cout << "Enter the target filename in the system: " << endl;
    cin >> targetFilename;
    send(clientSocket, targetFilename.c_str(), targetFilename.size(), 0);

    // CLIENT ACTION: 
    // Obtain data from ifstream, load it to buffer and send to the client socket 
    while(inFile.read(buffer, sizeof(buffer))){
        cout << buffer << endl;
        if (send(clientSocket, buffer, sizeof(buffer), 0) == -1){
            cerr << "Failed to send file data " << endl;
            close(clientSocket);
            return;
        }
    }

    if (inFile.gcount() > 0){ // gcount = number of characters atually read by the ifstream

        if (send(clientSocket, buffer, inFile.gcount(), 0) == -1){
            cerr << "Failed to send last chuck of file data " << endl;
            close(clientSocket);
            return;
        }
        send(clientSocket, "EOF", 3, 0);
    }

    cout << "File upload complete!" << endl;
    inFile.close();





}