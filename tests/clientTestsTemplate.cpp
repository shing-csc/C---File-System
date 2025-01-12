#include <iostream>
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
#include <vector>
#include <cassert>

using namespace std;
namespace fs = std::filesystem;
const int SERVER_PORT = 8080;

// Helper functions to start and stop the server
pid_t startServer(const std::string& serverExecutable) {
    pid_t pid = fork(); // Create a child process

    if (pid < 0) {
        throw std::runtime_error("Failed to fork process to start server");
    } else if (pid == 0) {
        // Child process runs the server
        execl(serverExecutable.c_str(), serverExecutable.c_str(), nullptr);
        perror("Failed to start server executable");
        _exit(EXIT_FAILURE);
    }

    // Parent process returns the PID of the child
    return pid;
}

void stopServer(pid_t serverPid) {
    kill(serverPid, SIGTERM);
}

// Function to simulate client upload
void mockClient(const std::string& command, const std::string& testOrgF, const std::string& testOutF, const std::string& validity) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    std::string fullFilePath = fs::path(CLIENT_DB_PATH) / testOrgF;

    // Open the file to send
    std::ifstream inFile(fullFilePath, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::runtime_error("Failed to open client file: " + fullFilePath);
    }

    // Create a socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        throw std::runtime_error("Failed to create client socket");
    }

    // Set up the server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        close(clientSocket);
        throw std::runtime_error("Failed to connect to server");
    }

    // Send the command to the server
    if (send(clientSocket, command.c_str(), command.size(), 0) == -1) {
        close(clientSocket);
        throw std::runtime_error("Failed to send command to server");
    }

    uint32_t fileOrgNameSize = htonl(testOrgF.size());
    uint32_t fileOutNameSize = htonl(testOutF.size());

    // Send the original file name size
    if (send(clientSocket, &fileOrgNameSize, sizeof(fileOrgNameSize), 0) == -1) {
        close(clientSocket);
        throw std::runtime_error("Failed to send original filename size to server");
    }

    // Send the original file name
    if (send(clientSocket, testOrgF.c_str(), testOrgF.size(), 0) == -1) {
        close(clientSocket);
        throw std::runtime_error("Failed to send original filename to server");
    }

    // Send file validity
    if (send(clientSocket, validity.c_str(), validity.size(), 0) == -1) {
        close(clientSocket);
        throw std::runtime_error("Failed to send file validity to server");
    }

    // Send the output file name size
    if (send(clientSocket, &fileOutNameSize, sizeof(fileOutNameSize), 0) == -1) {
        close(clientSocket);
        throw std::runtime_error("Failed to send output filename size to server");
    }

    // Send the output file name
    if (send(clientSocket, testOutF.c_str(), testOutF.size(), 0) == -1) {
        close(clientSocket);
        throw std::runtime_error("Failed to send output filename to server");
    }

    // Send file data
    while (inFile.read(buffer, sizeof(buffer))) {
        if (send(clientSocket, buffer, sizeof(buffer), 0) == -1) {
            close(clientSocket);
            throw std::runtime_error("Failed to send file data");
        }
    }

    if (inFile.gcount() > 0) {
        if (send(clientSocket, buffer, inFile.gcount(), 0) == -1) {
            close(clientSocket);
            throw std::runtime_error("Failed to send last chunk of file data");
        }
    }

    // Send EOF marker
    //send(clientSocket, "EOF", 3, 0);

    close(clientSocket);
}

// Function to test upload functionality
void uploadFunctionTest(pid_t serverPid) {
    fs::path clientFullPath = fs::path(CLIENT_DB_PATH) / "city.txt";
    fs::path serverFullPath = fs::path(SERVER_DB_PATH) / "mockClient.txt";

    // Ensure the client file exists
    if (!fs::exists(clientFullPath)) {
        throw std::runtime_error("Client file does not exist: " + clientFullPath.string());
    }

    // Start the mock client in a thread
    std::thread clientThread(mockClient, "U", "city.txt", "mockClient.txt", "V");

    // Wait for the server file to exist
    const int maxRetries = 20; // Retry for up to 10 seconds
    int retries = 0;
    while (!fs::exists(serverFullPath) && retries < maxRetries) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        retries++;
    }

    if (!fs::exists(serverFullPath)) {
        throw std::runtime_error("Server file does not exist after upload: " + serverFullPath.string());
    }

    // Compare the content of the client and server files
    std::ifstream clientFile(clientFullPath, std::ios::binary);
    std::ifstream serverFile(serverFullPath, std::ios::binary);

    if (!clientFile.is_open()) {
        throw std::runtime_error("Failed to open client file: " + clientFullPath.string());
    }
    if (!serverFile.is_open()) {
        throw std::runtime_error("Failed to open server file: " + serverFullPath.string());
    }

    const size_t BUFFER_SIZE = 1024 * 1024;
    std::vector<char> clientBuffer(BUFFER_SIZE);
    std::vector<char> serverBuffer(BUFFER_SIZE);

    while (!clientFile.eof() && !serverFile.eof()) {
        clientFile.read(clientBuffer.data(), BUFFER_SIZE);
        serverFile.read(serverBuffer.data(), BUFFER_SIZE);

        std::streamsize clientBytesRead = clientFile.gcount();
        std::streamsize serverBytesRead = serverFile.gcount();

        if (clientBytesRead != serverBytesRead) {
            throw std::runtime_error("File sizes differ or files are truncated");
        }

        if (!std::equal(clientBuffer.begin(), clientBuffer.begin() + clientBytesRead, serverBuffer.begin())) {
            throw std::runtime_error("File content mismatch in chunk comparison");
        }
    }

    if (!clientFile.eof() || !serverFile.eof()) {
        throw std::runtime_error("Files have different sizes or EOF mismatch");
    }

    clientThread.join();
    stopServer(serverPid);
}

// Main function
int main() {
    try {
        // Start the server
        pid_t serverPid = startServer(SERVER_EXECUTABLE);
        std::this_thread::sleep_for(std::chrono::seconds(5)); // Wait for the server to start

        // Run the upload function test
        uploadFunctionTest(serverPid);

        std::cout << "Upload function test passed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}