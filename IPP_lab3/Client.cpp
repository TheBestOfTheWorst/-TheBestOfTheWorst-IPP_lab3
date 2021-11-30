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

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

//Завдання:
//Програма-сервер містить два вікна edit, у які користувач може уводити свій текст.
//Програма-клієнт має 2 кнопки, по натисканню однієї з яких 
//клієнт одержує текст відповідного вікна програми-сервера.

//all global declarations
HINSTANCE hInst;
HANDLE hSyncEvent;

WSADATA wsaData;
SOCKET ConnectSocket = INVALID_SOCKET;
struct addrinfo *result = NULL, *ptr = NULL, hints;

ATOM MyRegisterClass(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL ReceiveData(HWND, int);

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
	wcex.lpszClassName = L"IPP3_CLIENT_LYSENKO"; //window class name
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}
//creating window program
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	wchar_t txBuffer[DEFAULT_BUFLEN]{};

	ZeroMemory(&hints, sizeof(hints));
	hInst = hInstance;

	HWND hWnd = CreateWindow(L"IPP3_CLIENT_LYSENKO",//window class name
		L"Third lab (Client)",						//window name
		WS_OVERLAPPED | WS_SYSMENU,					//window style
		725,						//x position on the screen
		255, 						//y position on the screen
		500,						//x-coordinate size
		300,						//y-coordinate size
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hWnd)
		return FALSE;

	hSyncEvent = CreateEvent(NULL, TRUE, FALSE, L"syncEvent");

	if (hSyncEvent == NULL)
	{
		wsprintf(txBuffer, L"CreateEvent failed (%d)", GetLastError());
		MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	ShowWindow(hWnd, SW_SHOW);	//show a window

	return TRUE;
}
//function which receives data
BOOL ReceiveData(HWND hWnd, int receiveNumber)
{
	char recvbuf[BUFFER_SIZE]{};
	wchar_t txBuffer[DEFAULT_BUFLEN]{};
	int iResult;

	// Send edit box number
	switch (receiveNumber)
	{
		case 0: iResult = send(ConnectSocket, "0", 2, 0); break;
		case 1: iResult = send(ConnectSocket, "1", 2, 0); break;
		default: MessageBox(hWnd, L"receiveNumber value is not supported", L"Error", MB_OK | MB_ICONERROR); return FALSE; break;
	}
	if (iResult == SOCKET_ERROR) 
	{
		wsprintf(txBuffer, L"send failed with error: %d", WSAGetLastError());
		MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
		closesocket(ConnectSocket);
		WSACleanup();
		return FALSE;
	}

	// Receive and send data
	iResult = recv(ConnectSocket, recvbuf, BUFFER_SIZE, 0);
	if (iResult > 0)
	{
		switch (receiveNumber)
		{
			case 0: MessageBoxA(hWnd, recvbuf, "Text from Edit 1", MB_OK | MB_ICONINFORMATION); break;
			case 1: MessageBoxA(hWnd, recvbuf, "Text from Edit 2", MB_OK | MB_ICONINFORMATION); break;
			default: MessageBox(hWnd, L"receiveNumber value is not supported", L"Error", MB_OK | MB_ICONERROR); return FALSE; break;
		}
	}
	else if (iResult == 0)
	{
		MessageBox(hWnd, L"Connection with server closed", L"Information", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		wsprintf(txBuffer, L"recv failed with error: %d", WSAGetLastError());
		MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
		closesocket(ConnectSocket);
		WSACleanup();
		return FALSE;
	}

	return TRUE;
}
//message processing function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//buttons descriptors
	static HWND hBtnEdit1, hBtnEdit2;
	//buffers
	wchar_t txBuffer[DEFAULT_BUFLEN];
	int iResult;

	//process messages of the window
	switch (message)
	{
	case WM_COMMAND:
		if (!SetEvent(hSyncEvent))
		{
			wsprintf(txBuffer, L"SetEvent failed (%d)", GetLastError());
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			return FALSE;
		}

		if (lParam == (int)hBtnEdit1)
			ReceiveData(hWnd, 0);
		else if (lParam == (int)hBtnEdit2)
			ReceiveData(hWnd, 1);
		break;
	case WM_CREATE:
		hBtnEdit1 = CreateWindow(L"BUTTON", L"Text from Edit 1", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			165, 90, 150, 30, hWnd, NULL, hInst, NULL);
		hBtnEdit2 = CreateWindow(L"BUTTON", L"Text from Edit 2", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			165, 140, 150, 30, hWnd, NULL, hInst, NULL);

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			wsprintf(txBuffer, L"WSAStartup failed with error: %d", iResult);
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			return FALSE;
		}

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		iResult = getaddrinfo(DEFAULT_SERVER_NAME, DEFAULT_PORT, &hints, &result);
		if (iResult != 0)
		{
			wsprintf(txBuffer, L"getaddrinfo failed with error: %d", iResult);
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			WSACleanup();
			return FALSE;
		}

		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET)
			{
				wsprintf(txBuffer, L"Socket failed with error: %d", WSAGetLastError());
				MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
				WSACleanup();
				return FALSE;
			}

			// Connect to server.
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				int test = WSAGetLastError();
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}

		freeaddrinfo(result);

		if (ConnectSocket == INVALID_SOCKET)
		{
			MessageBox(hWnd, L"Unable to connect to server!", L"Error", MB_OK | MB_ICONERROR);
			WSACleanup();
			return FALSE;
		}

		break;
	case WM_DESTROY:
		// shutdown the connection since no more data will be sent
		if (shutdown(ConnectSocket, SD_SEND) == SOCKET_ERROR)
		{
			wsprintf(txBuffer, L"shutdown failed with error: %d", WSAGetLastError());
			MessageBox(hWnd, txBuffer, L"Error", MB_OK | MB_ICONERROR);
			closesocket(ConnectSocket);
			WSACleanup();
			return FALSE;
		}
		// Cleanup
		closesocket(ConnectSocket);
		WSACleanup();

		//ending my application
		CloseHandle(hSyncEvent);
		PostQuitMessage(0);
		break;
	default:
		//let Windows handle the rest
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}