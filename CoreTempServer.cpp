#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <ws2tcpip.h>
#include <process.h>
#include <sstream>
#include "lib/CoreTempPlugin.h"
#include "CoreTempServerClass.h"

#define DEFAULT_PORT "3480"
#pragma comment(lib, "Ws2_32.lib")

CoreTempServerClass* pluginClass = NULL;

int leer(SOCKET s, char* recvbuf)
{
	SOCKET clisock = s;
	int iResult = 0;
	int recvbuflen = 255;
	iResult = recv(clisock, recvbuf, recvbuflen, 0);
	if (iResult == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAETIMEDOUT)
		{
			std::cout << "Timeout. More than 10 seconds passed since last receive block was received. Disconnecting." << std::endl;
			closesocket(clisock);
			return 1;
		}
		else
		{
			if (WSAGetLastError() == 10053L)
			{
				std::cout << "The client closed the connection unexpectedly (without sending 'BYE'). Disconnecting." << std::endl;
				closesocket(clisock);
				return 1;
			}
			std::cout << "An error ocurred while receiving data from client:" << WSAGetLastError() << ". Disconnecting." << std::endl;
			closesocket(clisock);
			return 1;
		}
	}
	return 0;
}

unsigned WINAPI handleClient(void* data) {
	SOCKET sock = *(SOCKET*)data;
	char* recvbuf = (char*)malloc(500 * sizeof(char));
	if (recvbuf == NULL) return 1;
	int res;
	bool salir = false;
	while (!salir) {
		res = leer(sock, recvbuf);
		if (res != 0) salir = true;
		else {
			if (strcmp(recvbuf, "GET_INFO") == 0) {
				std::stringstream response;

				response << pluginClass->latest->sCPUName << "\n";
				response << pluginClass->latest->fCPUSpeed << "\n";
				response << pluginClass->latest->fFSBSpeed << "\n";
				response << pluginClass->latest->fMultiplier << "\n";
				response << pluginClass->latest->fVID << "\n";
				for (size_t i = 0; i < pluginClass->latest->uiCPUCnt; i++) {
					response << i << "\n";
					response << pluginClass->latest->uiTjMax[i];
					for (size_t j = 0; j < pluginClass->latest->uiCoreCnt; j++) {
						size_t index = j + (i * pluginClass->latest->uiCoreCnt);
						response << pluginClass->latest->fTemp[index];
					}
				}

				std::string finalMsg = response.str();
				send(sock, finalMsg.c_str(), finalMsg.length(), 0);
			}
			else if (strcmp(recvbuf, "EXIT") == 0) {
				char response[] = "BYE";
				send(sock, response, strlen(response), 0);
				salir = true;
			}
			else {
				char response[] = "?";
				send(sock, response, strlen(response), 0);
			}
		}
	}
	free(recvbuf);
	return 0;
}

unsigned WINAPI initializeServer(void *data) {
	SOCKET sock = INVALID_SOCKET;
	SOCKET clisock = INVALID_SOCKET;
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		return 1;
	}
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 2;
	}
	sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sock == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 3;
	}
	iResult = bind(sock, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(sock);
		WSACleanup();
		return 4;
	}
	freeaddrinfo(result);
	if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 5;
	}
	int maxconn = 6;
	int conn = 0;
	std::cout << "Servidor iniciado con exito.";
	while (!pluginClass->stop) {
		clisock = accept(sock, NULL, NULL);
		if (clisock == INVALID_SOCKET) {
			printf("Accept failed with error: %ld\n", WSAGetLastError());
		}
		unsigned threadID;
		conn++;
		std::cout << "Client connected. #" << conn << std::endl;
		HANDLE hThread = NULL;
		hThread = (HANDLE)_beginthreadex(NULL, 0, handleClient, (void*)&clisock, 0, &threadID);
		if (hThread != NULL) CloseHandle(hThread);
		std::cout << "Current clients connected:" << conn << std::endl;
	}
	return 0;
}

int Start()
{
	int result = pluginClass->Start();
	HANDLE hThread = NULL;
	unsigned threadID;
	hThread = (HANDLE)_beginthreadex(NULL, 0, initializeServer, NULL, 0, &threadID);
	if (hThread != NULL) CloseHandle(hThread);

	return result;
}

void Update(const LPCoreTempSharedData data)
{
	pluginClass->Update(data);
}

void Stop()
{
	pluginClass->Stop();
}

int Configure()
{
	return pluginClass->Configure();
}

void Remove(const wchar_t* path)
{
	pluginClass->Remove(path);
}

void createConsole();

LPCoreTempPlugin WINAPI GetPlugin(HMODULE hModule)
{
	// Attach a console window for this process.
	//createConsole();
	std::cout << "TESTING";
	pluginClass = new CoreTempServerClass();
	LPCoreTempPlugin plugin = pluginClass->GetPluginInstance(hModule);
	plugin->Start = Start;
	plugin->Update = Update;
	plugin->Stop = Stop;
	plugin->Configure = Configure;
	plugin->Remove = Remove;
	return plugin;
}

void WINAPI ReleasePlugin()
{
	if (pluginClass)
	{
		delete pluginClass;
		pluginClass = NULL;
	}

	// Detach the console window.
	//FreeConsole();
}

/*
	Source for the following function:
	http://www.halcyon.com/~ast/dload/guicon.htm
*/
void createConsole()
{
	int hConHandle;
	long lStdHandle;
	const WORD MAX_CONSOLE_LINES = 500;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE* fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen(hConHandle, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "r");
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);

	// point to console as well
	std::ios::sync_with_stdio();
}