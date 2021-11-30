// Windows Header Files
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

// Define useful macros
#define DEFAULT_BUFLEN 512
#define DEFAULT_SERVER_NAME "localhost"
#define DEFAULT_PORT "27015"
#define BUFFER_SIZE 25

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

//Завдання:
//Програма-сервер містить два вікна edit, у які користувач може уводити свій текст.
//Програма-клієнт має 2 кнопки, по натисканню однієї з яких
//клієнт одержує текст відповідного вікна програми-сервера.

//all global declarations
HINSTANCE hInst;
HANDLE hSyncEvent;

WSADATA wsaData;
SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;
struct addrinfo* result = NULL;
struct addrinfo hints;

ATOM MyRegisterClass(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL InitConnection(HWND);
BOOL SendData(HWND, char*, int, char*, int);
BOOL TerminateConnection(HWND);

//main function
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
//registering window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_INFORMATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(255, 255, 204));
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"IPP3_SERVER_LYSENKO"; //window class name
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}
//creating a window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	HWND hWnd = CreateWindow(L"IPP3_SERVER_LYSENKO",//window class name
		L"Third lab (Server)",						//window name
		WS_OVERLAPPED | WS_SYSMENU,					//window style
		215,						//x position on the screen
		255, 						//y position on the screen
		500,						//x-coordinate size
		300,						//y-coordinate size
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, SW_SHOW);	//show a window

	return TRUE;
}
//function which sends data
BOOL SendData(HWND hWnd, char* edit1Tx, int edit1TxLen, char* edit2Tx, int edit2TxLen)
{
	int iResult, iSendResult;
	char recvbuf[BUFFER_SIZE]{};
	wchar_t txBuffer[DEFAULT_BUFLEN]{};

	// Receive nd send data
	iResult = recv(ClientSocket, recvbuf, BUFFER_SIZE, 0);
	if (iResult > 0)
	{
		if (!strcmp(recvbuf, "0"))
		{
			iSendResult = send(ClientSocket, edit1Tx, edit1TxLen, 0);

			if (iSendResult == SOCKET_ERROR)
			{
				wsprintf(txBuffer, L"send failed with error: %d", WSAGetLastError());
				MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
				closesocket(ClientSocket);
				WSACleanup();
				return FALSE;
			}
		}
		else if (!strcmp(recvbuf, "1"))
		{
			iSendResult = send(ClientSocket, edit2Tx, edit2TxLen, 1);

			if (iSendResult == SOCKET_ERROR)
			{
				wsprintf(txBuffer, L"send failed with error: %d", WSAGetLastError());
				MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
				closesocket(ClientSocket);
				WSACleanup();
				return FALSE;
			}
		}
		else
		{
			MessageBoxA(hWnd, "Client data was corrupted", "Error", MB_OK | MB_ICONERROR);
		}
	}
	else if (iResult == 0)
	{
		MessageBox(hWnd, L"Connection with client closed", L"Information", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		wsprintf(txBuffer, L"recv failed with error: %d", WSAGetLastError());
		MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
		closesocket(ClientSocket);
		WSACleanup();
		return FALSE;
	}

	return TRUE;
}
//message processing function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc = GetDC(hWnd); //context descriptor
	//logical coordinates of the window
	RECT rt; GetClientRect(hWnd, &rt);
	//edits descriptors
	static HWND hEdit1, hEdit2;
	//buffers
	int iResult;
	wchar_t txBuffer[DEFAULT_BUFLEN];

	hSyncEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"syncEvent");
	if (hSyncEvent != NULL)
	{
		if (WaitForSingleObject(hSyncEvent, 0) == WAIT_OBJECT_0)
		{
			char editTxt1[BUFFER_SIZE];
			GetWindowTextA(hEdit1, editTxt1, BUFFER_SIZE);

			char editTxt2[BUFFER_SIZE];
			GetWindowTextA(hEdit2, editTxt2, BUFFER_SIZE);

			SendData(hWnd, editTxt1, BUFFER_SIZE, editTxt2, BUFFER_SIZE);
		}
		ResetEvent(hSyncEvent);
	}
	//process messages of the window
	switch (message)
	{
	case WM_PAINT:
	{
		//painting text
		PAINTSTRUCT ps;
		hdc = BeginPaint(hWnd, &ps);
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 100, 95, L"Edit 1:", 8);
		TextOut(hdc, 100, 145, L"Edit 2:", 8);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CREATE:
		hEdit1 = CreateWindow(L"EDIT", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE,
			150, 95, 200, 20, hWnd, NULL, hInst, NULL);
		hEdit2 = CreateWindow(L"EDIT", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE,
			150, 145, 200, 20, hWnd, NULL, hInst, NULL);

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			wsprintf(txBuffer, L"WSAStartup failed with error: %d", iResult);
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			return FALSE;
		}

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(DEFAULT_SERVER_NAME, DEFAULT_PORT, &hints, &result);
		if (iResult != 0)
		{
			wsprintf(txBuffer, L"getaddrinfo failed with error: %d", iResult);
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			WSACleanup();
			return FALSE;
		}

		// Create a SOCKET for connecting to server
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET)
		{
			wsprintf(txBuffer, L"Socket failed with error: %d", WSAGetLastError());
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			freeaddrinfo(result);
			WSACleanup();
			return FALSE;
		}

		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			wsprintf(txBuffer, L"bind failed with error: %d", WSAGetLastError());
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return FALSE;
		}

		freeaddrinfo(result);

		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			wsprintf(txBuffer, L"listen failed with error: %d", WSAGetLastError());
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			closesocket(ListenSocket);
			WSACleanup();
			return FALSE;
		}

		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, 0);
		if (ClientSocket == INVALID_SOCKET)
		{
			wsprintf(txBuffer, L"accept failed with error: %d", WSAGetLastError());
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			closesocket(ListenSocket);
			WSACleanup();
			return FALSE;
		}

		// No longer need server socket
		closesocket(ListenSocket);
		break;
	case WM_DESTROY:
		// shutdown the connection since we're done
		if (shutdown(ClientSocket, SD_SEND) == SOCKET_ERROR)
		{
			wsprintf(txBuffer, L"shutdown failed with error: %d", WSAGetLastError());
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			closesocket(ClientSocket);
			WSACleanup();
			return FALSE;
		}

		// Cleanup
		closesocket(ClientSocket);
		WSACleanup();

		//ending my application and releasing semaphore one resource
		PostQuitMessage(0);
		break;
	default:
		//let Windows handle the rest
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}