#include <iostream>
#include <gtest/gtest.h>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <string>
#include <fstream>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>
#include <unistd.h>

#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

class ClientTest : public testing::Test{
    
    protected:

    pid_t serverPid;

    std::vector<std::thread> clientThreads;
    const std::string serverExecutable = fs::path(SERVER_EXECUTABLE);
    const std::string serverAddress = "127.0.0.1";
    const int serverPort = 8080;

    // SetUp of server communication
    void SetUp() override {
        serverPid = startServer(serverExecutable);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Teardown of threads: join 
    void TearDown() override {
        
        if (serverPid > 0){             // Use parent/main process to kill child process
            stopServer(serverPid);
        }
        
        // Join all threads in main process :: in clientThreads
        for (auto &t : clientThreads){
            if (t.joinable()){
                t.join();
            }
        }
    }


    pid_t startServer(const std::string serverExecutable){  // main process running 
        
        pid_t pid = fork();                                 // Creation of a child process

        /**
         * main and child process are running
         *      main process  :: value of pid (child process) as actual process ID
         *                    pid = 13123124523
         *      child process :: value of pid (itself) as 0
         *                    pid = 0
         */

        if (pid < 0){
            throw std::runtime_error("Failed to fork process to start server");
        }
        else if (pid == 0){                                                 // child process run here  
            execl(serverExecutable.c_str(), serverExecutable.c_str(), 0);   // Pass the whole server executable to be runned by the child process
            perror("Failed to start server executable");
            _exit(EXIT_FAILURE);
        }
        else{}                                                              // main process run here : do nothing  
        return pid;                                                         // main and child process run here
    }


    void stopServer(pid_t serverPid){
        kill(serverPid, SIGTERM);
    }

    // Mock Individual Client Activities - Run as a thread
    std::string mockClient(const std::string& command, const std::string& testOrgF, const std::string& testOutF, const std::string validity){
        
        int BUFFER_SIZE = 1024;
        char buffer[BUFFER_SIZE];

        std::string testOriginalFile = testOrgF;
        std::string testOutputFile = testOutF;
        std::string testValidity = validity;
        std::string fullFilePath = fs::path(CLIENT_DB_PATH) / testOriginalFile;
        ifstream inFile(fullFilePath, ios::binary);
 
        // Establish communication/connection with server
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        if (clientSocket == -1){
            throw std::runtime_error("Failed to create client socket");
        }
        
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(serverPort);
        serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");


        if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
            throw std::runtime_error("Failed to connect to server");
        }

        if (send(clientSocket, command.c_str(), command.size(), 0) == -1){                      // Command
            throw std::runtime_error("Failed to send command to server");
        }

        uint32_t fileOrgNameSize = htonl(testOrgF.size());
        uint32_t fileOutNameSize = htonl(testOutF.size());

        
        if (send(clientSocket, &fileOrgNameSize, sizeof(fileOrgNameSize), 0) == -1){            // Org Filename Size
            throw std::runtime_error("Failed to send org filename size to server");
        }
        
        if (send(clientSocket, testOriginalFile.c_str(), testOriginalFile.size(), 0) == -1){    // Org Filename
            throw std::runtime_error("Failed to send org filename to server");
        }
        
        if (send(clientSocket, testValidity.c_str(), testValidity.size(), 0) == -1){            // Org Filename Validity
            throw std::runtime_error("Failed to send org file validity to server");
        }
        
        if (send(clientSocket, &fileOutNameSize, sizeof(fileOutNameSize), 0) == -1){            // Out Filename Size
            throw std::runtime_error("Failed to send out filename size to server");
        }
        
        if (send(clientSocket, testOutputFile.c_str(), testOutputFile.size(), 0) == -1){        // Out Filename 
            throw std::runtime_error("Failed to send output filename to server");
        }

        while(inFile.read(buffer, sizeof(buffer))){
            cout << buffer << endl;
            if (send(clientSocket, buffer, sizeof(buffer), 0) == -1){
                close(clientSocket);
                return std::string("Failed to send file data");
            }
        }

        if (inFile.gcount() > 0){ 

            if (send(clientSocket, buffer, inFile.gcount(), 0) == -1){
                cerr << "Failed to send last chuck of file data " << endl;
                close(clientSocket);
                return std::string("Failed Communication");
            }
            //send(clientSocket, "EOF", 3, 0);
        }


        // TERMPORY STOP THE SERVER AFTER EACH REQUEST FOR TESTING PURPOSES, HANDLEDE IN THE FUTURE 
        if (serverPid > 0){             // Use parent/main process to kill child process
            stopServer(serverPid);
        }
        
        
        close(clientSocket);
        return std::string("Successful Communication");
    }

};

TEST_F(ClientTest, UploadFuncTesting){

    // Setup()

    // Creation of different threads from main ClientTest process as clients to interact with server
    fs::path clientFullPath = fs::path(CLIENT_DB_PATH) / "city.txt";
    fs::path serverFullpath = fs::path(SERVER_DB_PATH) / "mockClient.txt";

    clientThreads.emplace_back(std::thread([this]() {
        mockClient("U", "city.txt", "mockClient.txt", "V");
    }));
    std::this_thread::sleep_for(std::chrono::seconds(10)); 
    // Not sure what keep the test script not stopping? While loop? VS code broke?

    // Compare the content in clientFullPath and serverFullPath using EXPECT
    
    std::ifstream clientFile(clientFullPath, std::ios::binary);
    std::ifstream serverFile(serverFullpath, std::ios::binary);

    // CURRENT PROBLEM: server program cannot be opened: probably need time to edit

    EXPECT_TRUE(clientFile.is_open()) << "Failed to open client file: " << clientFullPath;
    EXPECT_TRUE(serverFile.is_open()) << "Failed to open server file: " << serverFullpath;

    const size_t BUFFER_SIZE = 1024 * 1024;
    std::vector<char> clientBuffer(BUFFER_SIZE);
    std::vector<char> serverBuffer(BUFFER_SIZE);

    while (!clientFile.eof() && !serverFile.eof()) {
        clientFile.read(clientBuffer.data(), BUFFER_SIZE);
        serverFile.read(serverBuffer.data(), BUFFER_SIZE);

        std::streamsize clientBytesRead = clientFile.gcount();
        std::streamsize serverBytesRead = serverFile.gcount();

        ASSERT_EQ(clientBytesRead, serverBytesRead) << "File sizes differ or files are truncated.";

        EXPECT_TRUE(std::equal(clientBuffer.begin(), clientBuffer.begin() + clientBytesRead, serverBuffer.begin()))
            << "File content mismatch in chunk comparison.";
    }

    // Ensure both files reach EOF at the same time
    EXPECT_TRUE(clientFile.eof() && serverFile.eof()) << "Files have different sizes.";
    // TearDown()
}

// TEST_F(ClientTest, DownloadFuncTesting){}
// TEST_F(ClientTest, RemovalFuncTesting){}

TEST_F(ClientTest, MultiClientPerformance){
    
    // Setup()

    // clientThreads.emplace_back(std::thread(mockClient, "UPLOAD"));
    
    // TearDown()
}



int main(){

    // Testing
    testing::InitGoogleTest();

    // Stop the server executable after testing

    return RUN_ALL_TESTS();

}