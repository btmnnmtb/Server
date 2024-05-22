// Server.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#define WIN32_LEAN_AND_MEAN // макрос 
#include <iostream>
#include <Windows.h>
#include<WinSock2.h>
#include <WS2tcpip.h>
using namespace std; 

int main()
{
    // служебная структура для хранение информации (ВЕРСИИ, СТРУКТУРЫ(НАПРИМЕР СЕМЕЙНУЮ)
   // о реализации Windows Sockets
    WSADATA wsaData;
    ADDRINFO hints;
    ADDRINFO* addrResult;
    SOCKET ListenSocket = INVALID_SOCKET; //слушает соединение
    SOCKET ConnectSocket = INVALID_SOCKET;
    char recvBuffer[512];

    const char* sendBuffer = "Hello from server";
    // старт использования библиотеки сокетов процессом определния версии и структуры

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    // Если произошла ошибка подгрузки библиотеки
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));//нам необходимо 
    // изначально занулить память, 1-ый паметр, что зануляем,2-ой сколько
    hints.ai_family = AF_INET;//4-БАЙТНЫЙ Ethernet
    hints.ai_socktype = SOCK_STREAM;//задаем потоковый тип сокета
    hints.ai_protocol = IPPROTO_TCP;// Используем протокол TCP
    hints.ai_flags = AI_PASSIVE; // Пассивная сторона, потому что просто ждет соединения

    result = getaddrinfo(NULL, "666", &hints, &addrResult); // функциия хранит в себе адрес, порт,семейство структур, адрес сокета
    if (result != 0) { // если ошибка
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    // Если создание сокета завершилось с ошибкой, выводим сообщение,
   // освобождаем память, выделенную под структуру addr,
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();//очситка WSAStartup
        return 1;
    }
    // Привязываем сокет к IP-адресу (соединились с сервером)
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);

    if (result == SOCKET_ERROR) {// Если привязать адрес к сокету не удалось, то выводим сообщение
        // об ошибке, освобождаем память, выделенную под структуру addr.
          // и закрываем открытый сокет.
          // Выгружаем DLL-библиотеку из памяти и закрываем программу.
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    result = listen(ListenSocket, SOMAXCONN); // сервер начинает прослушвание текущих входящих сообщений
    // на сокете ListenSoсket. SomaXCONN указывает максимальное кол-во ожидающих соедененй в  очереди
    if (result == SOCKET_ERROR) { // если ошибка

        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    ConnectSocket = accept(ListenSocket, NULL, NULL); // сервер пинимет входящие соеденение и создает новый сокет
    // для взаимодействия с клиентом
    if (ConnectSocket == INVALID_SOCKET) { // есси ошибка 
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket); // после принятия соеденения сокет для прослушивания закрывается , так как он бодьше не нужен

    do {
        ZeroMemory(recvBuffer, 512); // обнуляет буфер recvBuffer размером 512 байт для приема данных
        result = recv(ConnectSocket, recvBuffer, 512, 0); // сервер читает данные сокета ConnectSocet в буфер recvBuffer
        if (result > 0) { // Если количество принятых байтов больше нуля, сервер выводит информацию о принятых данных и отправляет ответ клиенту 
            //с помощью `send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);`.
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) { //
                cout << "Send failed, error: " << result << endl;
                closesocket(ConnectSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0) { // Если количество принятых байтов равно нулю, это означает, что клиент закрыл соединение, и сервер выводит сообщение о закрытии соединения.
            cout << "Connection closing" << endl;
        }
        else {
           // сли возникла ошибка при чтении данных, сервер выводит сообщение об ошибке, закрывает сокет, освобождает ресурсы и завершает работу.

            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            return 1;
        }
    } while (result > 0);

    result = shutdown(ConnectSocket, SD_SEND); // сервер отключает возможность отправки данных по сокету `ConnectSocket`, 
    //чтобы уведомить клиента о том, что сервер больше не будет отправлять данные.
    if (result == SOCKET_ERROR) {
        //Проверка результата вызова `shutdown()`. Если возвращается `SOCKET_ERROR`, сервер выводит сообщение об ошибке.

        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket); //закрывает сокет `ConnectSocket`, через который был установлен контакт с клиентом.
        freeaddrinfo(addrResult);
        //  освобождает память, выделенную для структуры `addrResult`, которая использовалась для хранения информации о сетевом интерфейсе.

        WSACleanup(); // завершает работу библиотеки Windows Sockets, освобождая связанные с ней ресурсы.
        return 1;
    }

    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
    return 0; // Возвращается 0, что указывает на успешное завершение программы.

}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
