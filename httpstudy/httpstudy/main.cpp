#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#pragma comment(lib, "ws2_32.lib")

#define BUF_SIZE 1000
#define PORT 8080

using json = nlohmann::json;

// 소켓 바인드
bool bind_socket(SOCKET server_socket, int port) {
    sockaddr_in address{};
    // ip 주소 체계 작성
    address.sin_family = AF_INET;
    // 랜카드 고유 식별 값 넣기
    address.sin_addr.s_addr = INADDR_ANY;
    // 포트 번호 작성
    address.sin_port = htons(port);

    // 값을 넣어서 소켓 바인드
    return bind(server_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != SOCKET_ERROR;
}

// 클라이언트에게 리턴
void send_json_response(SOCKET client_socket, int code, const std::string& message) {
    json json_obj = {
        {"status", "success"},
        {"code", code},
        {"message", message}
    };
    std::string json_string = json_obj.dump();

    // 헤더와 제이슨 데이터 반환
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Length: " + std::to_string(json_string.length()) + "\r\n";
    response += "Content-Type: application/json\r\n\r\n";
    response += json_string;

    send(client_socket, response.c_str(), response.length(), 0);
}

// 제이슨 파싱
void parse_and_print_json(const std::string& json_data) {
    try {
        json json_obj = json::parse(json_data);
        if (json_obj.contains("name")) {
            std::cout << "Name: " << json_obj["name"].get<std::string>() << std::endl;
        }
        if (json_obj.contains("age")) {
            std::cout << "Age: " << json_obj["age"].get<int>() << std::endl;
        }
        if (json_obj.contains("message")) {
            std::cout << "Message: " << json_obj["message"].get<std::string>() << std::endl;
        }
    }
    catch (json::parse_error& e) {
        std::cerr << "[ERR] JSON 파싱 실패: " << e.what() << std::endl;
    }
}

// request 처리
void handle_request(SOCKET client_socket) {
    char buffer[BUF_SIZE] = { 0 };
    int bytes_read = recv(client_socket, buffer, BUF_SIZE, 0);
    if (bytes_read == SOCKET_ERROR) {
        std::cerr << "[ERR] 요청 읽기 실패" << std::endl;
        return;
    }

    std::string request(buffer);

    std::cout << "[INFO] 요청 출력\n" << request << std::endl;

    std::string method = request.substr(0, request.find(" "));

    if (method == "GET") {
        std::cout << "[INFO] GET 요청 처리" << std::endl;
        send_json_response(client_socket, 200, "GET request received successfully.");
    }
    else if (method == "POST") {
        std::cout << "[INFO] POST 요청 처리" << std::endl;
        size_t json_start = request.find("\r\n\r\n");
        if (json_start != std::string::npos) {
            std::string json_data = request.substr(json_start + 4);
            parse_and_print_json(json_data);
            send_json_response(client_socket, 201, "POST request processed successfully.");
        }
    }
    else {
        std::cerr << "[ERR] 지원되지 않는 요청: " << method << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[ERR] WSAStartup 실패" << std::endl;
        return 1;
    }

    // TCP로 소켓 생성
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "[ERR] 소켓 생성 실패" << std::endl;
        WSACleanup();
        return 1;
    }

    // 바인드를 하여 실제 통로를 열어줌
    if (!bind_socket(server_socket, PORT)) {
        std::cerr << "[ERR] 소켓 바인드 실패" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // 클라이언트가 접속 대기
    if (listen(server_socket, 10) == SOCKET_ERROR) {
        std::cerr << "[ERR] 소켓 리슨 실패" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "[INFO] 서버가 포트 " << PORT << "에서 대기 중입니다." << std::endl;

    while (true) {
        sockaddr_in client_address;
        int client_len = sizeof(client_address);
        // 대기 후 접속을 받는다.
        SOCKET client_socket = accept(server_socket, reinterpret_cast<sockaddr*>(&client_address), &client_len);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "[ERR] 연결 수락 실패" << std::endl;
            continue;
        }

        handle_request(client_socket);
        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
