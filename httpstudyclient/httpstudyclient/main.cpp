#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8080

using json = nlohmann::json;


SOCKET create_socket() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[ERR] WSAStartup 실패" << std::endl;
        return INVALID_SOCKET;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "[ERR] 소켓 생성 실패" << std::endl;
        WSACleanup();
        return INVALID_SOCKET;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDRESS, &server_address.sin_addr);

    if (connect(client_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) == SOCKET_ERROR) {
        std::cerr << "[ERR] 서버 연결 실패" << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    std::cout << "[INFO] 서버에 연결 성공" << std::endl;
    return client_socket;
}

void send_get_request(SOCKET client_socket) {
    std::string get_request = "GET / HTTP/1.1\r\nHost: " + std::string(SERVER_ADDRESS) + "\r\n\r\n";
    send(client_socket, get_request.c_str(), get_request.length(), 0);

    char buffer[1024] = { 0 };
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        std::cout << "[INFO] GET 응답:\n" << buffer << std::endl;
    }
}

void send_post_request(SOCKET client_socket) {
    json json_obj = {
        {"name", "han seungho"},
        {"age", 27},
        {"message", "HTTPS Test From Client."}
    };
    std::string json_string = json_obj.dump();

    std::string post_request = "POST / HTTP/1.1\r\nHost: " + std::string(SERVER_ADDRESS) + "\r\n";
    post_request += "Content-Type: application/json\r\n";
    post_request += "Content-Length: " + std::to_string(json_string.length()) + "\r\n\r\n";
    post_request += json_string;

    send(client_socket, post_request.c_str(), post_request.length(), 0);

    char buffer[1024] = { 0 };
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        std::cout << "[INFO] POST 응답:\n" << buffer << std::endl;
    }
}

int main() {
    SOCKET client_socket = create_socket();
    if (client_socket == INVALID_SOCKET) {
        return 1;
    }

    std::string choice;
    std::cout << "GET 요청을 보내려면 'get'을, POST 요청을 보내려면 'post'를 입력하세요: ";
    std::cin >> choice;

    if (choice == "get") {
        send_get_request(client_socket);
    }
    else if (choice == "post") {
        send_post_request(client_socket);
    }
    else {
        std::cout << "[INFO] 잘못된 요청입니다." << std::endl;
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
