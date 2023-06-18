#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <Tlhelp32.h>
#include "Sapo.h"

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

TCHAR szProgName[] = TEXT("Base");

HBITMAP hBmpCar;
HBITMAP hBmpFrog;
HBITMAP hBmpLane;
HBITMAP hBmpSpecialLane;
HBITMAP hBmpObstacle;

BITMAP bmpCar;
BITMAP bmpFrog;
BITMAP bmpLane;
BITMAP bmpSpecialLane;
BITMAP bmpObstacle;

int tamanhoBmp = 75;
int uiElements = 1;

HDC bmpDC;
HWND hWndGlobal;

HDC memDC = NULL;

//HANDLES
HANDLE hMutexUIElements;
HANDLE hNamedPipeMovement, hNamedPipeMap;

CLIENTMAP map;

int checkIfIsAlreadyRunning(TCHAR* processName) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    int counter = 0;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return(FALSE);
    }

    do {
        if (!wcscmp(pe32.szExeFile, processName)) {
            counter++;
        }

    } while (Process32Next(hProcessSnap, &pe32));

    return counter;
}

int checkIfCanRun(){
    HANDLE hNamedPipe;
    int pid;
    DWORD nBytes;

    if (checkIfIsAlreadyRunning(_T("Servidor.exe")) == 0) {
        return 1;
    }

    if (!WaitNamedPipe(FIFOBACKEND, 2000)) {
		return 2;
	}

    hNamedPipe = CreateFile(FIFOBACKEND, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(hNamedPipe == INVALID_HANDLE_VALUE) {
        return 3;
    }
    
    pid = (int) GetProcessId(GetCurrentProcess());
    
    if (!WriteFile(hNamedPipe, &pid, sizeof(int), &nBytes, NULL)){
        return 4;
    }

    if(!ReadFile(hNamedPipe, &pid, sizeof(int), &nBytes, NULL)){
        return 5;
    }
    
    return pid;
}

void drawBitmap(int x, int y, HBITMAP hBmp, BITMAP bmp, HDC hdc){
	SelectObject(bmpDC, hBmp);

	BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
}

void paintMap(HDC hdc) {
	WaitForSingleObject(hMutexUIElements, INFINITE);
	HMENU hMenu = GetMenu(hWndGlobal);
	if(hMenu != NULL){
		TCHAR buffer[32];
		_swprintf_p(buffer, 32, _T("Nivel: %d"), map.level);
		ModifyMenu(hMenu, ID_NIVEL, MF_BYCOMMAND | MF_STRING, ID_NIVEL, buffer);
		
		_swprintf_p(buffer, 32, _T("Pontos: %d"), map.points);
		ModifyMenu(hMenu, ID_PONTOS, MF_BYCOMMAND | MF_STRING, ID_PONTOS, buffer);
		
		_swprintf_p(buffer, 32, _T("Vidas: %d / 5"), map.numLifes);
		ModifyMenu(hMenu, ID_VIDA, MF_BYCOMMAND | MF_STRING, ID_VIDA, buffer);

		DrawMenuBar(hWndGlobal);
	}
	for(int i = 0; i < map.numFaixas + 2; i++){
		for (int j = 0; j < 20; j++) {
			if (map.map[i][j] == _T('S')) {
				drawBitmap(j * tamanhoBmp, i * tamanhoBmp, hBmpFrog, bmpFrog, hdc);
			}
			else if(map.map[i][j] == _T('_') || map.map[i][j] == _T('-')) {
				drawBitmap(j * tamanhoBmp, i * tamanhoBmp, hBmpSpecialLane, bmpSpecialLane, hdc);
			}
			else if (map.map[i][j] == _T(' ')) {
				drawBitmap(j * tamanhoBmp, i * tamanhoBmp, hBmpLane, bmpLane, hdc);
			}
			else if (map.map[i][j] == _T('C')) {
				drawBitmap(j * tamanhoBmp, i * tamanhoBmp, hBmpCar, bmpCar, hdc);
			}
			else if (map.map[i][j] == _T('O')) {
				drawBitmap(j * tamanhoBmp, i * tamanhoBmp, hBmpObstacle, bmpObstacle, hdc);
			}
		}
	}
	ReleaseMutex(hMutexUIElements);
}

DWORD WINAPI KillThread(LPVOID param) {
	TKILL* data = (TKILL*)param;

    HANDLE handles[2];
    DWORD nBytes;
    enum Movement action = END;
    handles[0] = data->hEventClose;
	handles[1] = data->hEventCloseClient;
    int index = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
    if (index - WAIT_OBJECT_0 == 0) {
        MessageBox(hWndGlobal, _T("O Servidor fechou."), _T("Informação"), MB_OK | MB_ICONINFORMATION);
		ExitProcess(0);
    }
    else{
        if(!WriteFile(hNamedPipeMovement, &action, sizeof(enum Movement), &nBytes, NULL)){
            MessageBox(hWndGlobal, _T("Falha ao enviar a mensagem ao servidor!"), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
        }
        ResetEvent(data->hEventCloseClient);
        MessageBox(hWndGlobal, _T("O Jogo terminou."), _T("Informação"), MB_OK | MB_ICONINFORMATION);
		ExitProcess(0);
    }
	free(data);
    ExitThread(0);
}

DWORD WINAPI ReceiveMapThread(LPVOID param){
	enum Movement action = END;
	DWORD nBytes;
	int errorCode = 0;
	while(1){
		if(!ReadFile(hNamedPipeMap, &map, sizeof(CLIENTMAP), &nBytes, NULL)){
			errorCode = GetLastError();
			if(errorCode != 109){
				MessageBox(hWndGlobal, _T("Erro ao receber o mapa!"), _T("Informação"), MB_OK | MB_ICONINFORMATION);
				ExitProcess(0);
			}
		}
		if(map.numLifes == 0 || errorCode == 109){
			MessageBox(hWndGlobal, _T("Ficou sem vidas."), _T("Informação"), MB_OK | MB_ICONINFORMATION);
			WriteFile(hNamedPipeMovement, &action, sizeof(enum Movement), &nBytes, NULL);
			ExitProcess(0);
		}
		else{
			InvalidateRect(hWndGlobal, NULL, FALSE);
		}
	}
	ExitThread(0);
}

void loadBMP(int index){
	TCHAR buffer[32];
	_swprintf_p(buffer, 32, _T("frog%d.bmp"), index);
	hBmpFrog = (HBITMAP)LoadImage(NULL, buffer, IMAGE_BITMAP, tamanhoBmp, tamanhoBmp, LR_LOADFROMFILE);
	if (hBmpFrog == NULL) {
		MessageBox(hWndGlobal, _T("Erro ao carregar o bmp do frog!"), _T("Informação"), MB_OK | MB_ICONINFORMATION);
	}

	_swprintf_p(buffer, 32, _T("car%d.bmp"), index);
	hBmpCar = (HBITMAP)LoadImage(NULL, buffer, IMAGE_BITMAP, tamanhoBmp, tamanhoBmp, LR_LOADFROMFILE);
	if (hBmpFrog == NULL) {
		MessageBox(hWndGlobal, _T("Erro ao carregar o bmp car!"), _T("Informação"), MB_OK | MB_ICONINFORMATION);
	}

	_swprintf_p(buffer, 32, _T("lane%d.bmp"), index);
	hBmpLane = (HBITMAP)LoadImage(NULL, buffer, IMAGE_BITMAP, tamanhoBmp, tamanhoBmp, LR_LOADFROMFILE);
	if (hBmpFrog == NULL) {
		MessageBox(hWndGlobal, _T("Erro ao carregar o bmp da lane!"), _T("Informação"), MB_OK | MB_ICONINFORMATION);
	}

	_swprintf_p(buffer, 32, _T("specialLane%d.bmp"), index);
	hBmpSpecialLane = (HBITMAP)LoadImage(NULL, buffer, IMAGE_BITMAP, tamanhoBmp, tamanhoBmp, LR_LOADFROMFILE);
	if (hBmpFrog == NULL) {
		MessageBox(hWndGlobal, _T("Erro ao carregar o bmp da special lane!"), _T("Informação"), MB_OK | MB_ICONINFORMATION);
	}

	_swprintf_p(buffer, 32, _T("obstacle%d.bmp"), index);
	hBmpObstacle = (HBITMAP)LoadImage(NULL, buffer, IMAGE_BITMAP, tamanhoBmp, tamanhoBmp, LR_LOADFROMFILE);
	if (hBmpFrog == NULL) {
		MessageBox(hWndGlobal, _T("Erro ao carregar o bmp frog"), _T("Informação"), MB_OK | MB_ICONINFORMATION);
	}

	GetObject(hBmpFrog, sizeof(BITMAP), &bmpFrog);
	GetObject(hBmpCar, sizeof(BITMAP), &bmpCar);
	GetObject(hBmpLane, sizeof(BITMAP), &bmpLane);
	GetObject(hBmpSpecialLane, sizeof(BITMAP), &bmpSpecialLane);
	GetObject(hBmpObstacle, sizeof(BITMAP), &bmpObstacle);

}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;		
	MSG lpMsg;		
	WNDCLASSEX wcApp;	

	wcApp.cbSize = sizeof(WNDCLASSEX);     
	wcApp.hInstance = hInst;		         
	wcApp.lpszClassName = szProgName;       
	wcApp.lpfnWndProc = TrataEventos;       
	wcApp.style = CS_HREDRAW | CS_VREDRAW;  
	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION);
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDC_SAPO);
	wcApp.cbClsExtra = 0;				
	wcApp.cbWndExtra = 0;				
	wcApp.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName,			
		TEXT("Sapo"),
		WS_OVERLAPPEDWINDOW,	
		CW_USEDEFAULT,		
		CW_USEDEFAULT,		
		CW_USEDEFAULT,		
		CW_USEDEFAULT,		
		(HWND)HWND_DESKTOP,	
		(HMENU)NULL,			
		(HINSTANCE)hInst,
		0);				

	HDC hdc;

	hdc = GetDC(hWnd);

	loadBMP(1);

	bmpDC = CreateCompatibleDC(hdc);


	ReleaseDC(hWnd, hdc);
	
	hWndGlobal = hWnd;


	ShowWindow(hWnd, nCmdShow);	
	UpdateWindow(hWnd);		

    
    int returnValue = checkIfCanRun();
    HANDLE hEventClose;
    HANDLE hEventCloseClients;
    if(returnValue >= 1){
        TCHAR msg[64];
        if(returnValue == 1)
            _tcscpy_s(msg, 63, _T("Servidor desligado."));
        else if(returnValue == 2 || returnValue == 3)
            _tcscpy_s(msg, 63, _T("Falha ao ligar ao servidor."));
        else if(returnValue == 4)
            _tcscpy_s(msg, 63, _T("Falha ao enviar mensagem ao servidor."));
        else
            _tcscpy_s(msg, 63, _T("Falha ao receber mensagem do servidor."));
        MessageBox(hWnd, msg, TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    else{

        hEventCloseClients = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_CLOSE_CLIENTS_EVENT);
        if (hEventCloseClients == INVALID_HANDLE_VALUE) {
            MessageBox(hWnd, _T("Erro ao abrir o evento de fechar os clientes."), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        
        hEventClose = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_CLOSE_EVENT);
        if (hEventClose == INVALID_HANDLE_VALUE) {
            MessageBox(hWnd, _T("Erro ao abrir o evento de fechar."), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        TCHAR buffer[64];
        _swprintf_p(buffer, 64, FIFOFROGMOVEMENT, (int) GetProcessId(GetCurrentProcess()));
        if (!WaitNamedPipe(buffer, 2000)) {
            MessageBox(hWnd, _T("Erro ao esperar pelo pipe de movimento."), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        hNamedPipeMovement = CreateFile(buffer, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if(hNamedPipeMovement == INVALID_HANDLE_VALUE) {
            MessageBox(hWnd, _T("Erro ao criar o pipe de movimento."), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        _swprintf_p(buffer, 64, FIFOFROGMAP, (int) GetProcessId(GetCurrentProcess()));
        if (!WaitNamedPipe(buffer, 2000)) {
            MessageBox(hWnd, _T("Erro ao esperar pelo pipe do mapa."), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        hNamedPipeMap = CreateFile(buffer, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        if(hNamedPipeMap == INVALID_HANDLE_VALUE) {
            MessageBox(hWnd, _T("Erro ao criar o pipe do mapa."), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        TKILL *dadosKillThread = malloc(sizeof(TKILL));

        dadosKillThread->hEventClose = hEventClose;
		dadosKillThread->hEventCloseClient = hEventCloseClients;
        if(CreateThread(NULL, 0, KillThread, dadosKillThread, 0, NULL) == NULL){
            MessageBox(hWnd, _T("Erro ao lançar a thread de fechar."), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

		if(CreateThread(NULL, 0, ReceiveMapThread, NULL, 0, NULL) == NULL){
			MessageBox(hWnd, _T("Erro ao lançar a thread de ler o mapa."), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
			return 0;
		}
    }

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);	
		DispatchMessage(&lpMsg);	
	}

	
	return((int)lpMsg.wParam);
}


LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	HBITMAP hBitmapDB;

    enum Movement action;
    enum ResponseMovement response;
    DWORD nBytes;

	switch (messg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case ID_TOGGLEICONS:
			WaitForSingleObject(hMutexUIElements, INFINITE);
			uiElements = (uiElements % 2) == 0 ? 1 : 2;
			loadBMP(uiElements);
			ReleaseMutex(hMutexUIElements);
			InvalidateRect(hWnd, NULL, FALSE);
		break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rect);

		if (memDC == NULL) {
			memDC = CreateCompatibleDC(hdc);

			hBitmapDB = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
			SelectObject(memDC, hBitmapDB);
			DeleteObject(hBitmapDB);
		}
		FillRect(memDC, &rect, CreateSolidBrush(RGB(255, 255, 255)));

		paintMap(memDC);

		BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

		EndPaint(hWnd, &ps);
		break;

	case WM_ERASEBKGND:
		return TRUE;
	case WM_SIZE:
		WaitForSingleObject(hMutexUIElements, INFINITE);
		tamanhoBmp = (LOWORD(lParam) / 20);
		loadBMP(uiElements);
		memDC = NULL;
		ReleaseMutex(hMutexUIElements);
		break;
    case WM_KEYUP:
        switch(wParam){
            case VK_LEFT:
                action = LEFT;
            break;
            case VK_RIGHT:
                action = RIGHT;
            break;
			case VK_UP:
				action = UP;
            break;
            case VK_DOWN:
                action = DOWN;
            break;
			case VK_ESCAPE:
				action = END;
			break;
		}
		if(!WriteFile(hNamedPipeMovement, &action, sizeof(enum Movement), &nBytes, NULL)){
			MessageBox(hWnd, _T("Falha ao enviar o movimento ao servidor!"), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            TrataEventos(hWnd, WM_DESTROY, wParam, lParam);
        }
        if(wParam != VK_ESCAPE){
            if(!ReadFile(hNamedPipeMovement, &response, sizeof(enum ResponseMovement), &nBytes, NULL)){
				MessageBox(hWnd, _T("Falha ao receber os dados do servidor!"), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            }
			if (response == LOSE) {
				MessageBox(hWnd, _T("Perdeste o jogo!"), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
                TrataEventos(hWnd, WM_DESTROY, wParam, lParam);
			}
            else if(response == WIN){
                MessageBox(hWnd, _T("Ganhou! Passando para o próximo nivel."), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            }
            else if(response == PAUSED){
                MessageBox(hWnd, _T("O jogo está pausado."), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
            }
        }
        else
            TrataEventos(hWnd, WM_CLOSE, wParam, lParam);
        break;
	case WM_CLOSE:
		if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"), TEXT("Confirmação"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
            action = END;
            if(!WriteFile(hNamedPipeMovement, &action, sizeof(enum Movement), &nBytes, NULL)){
                MessageBox(hWnd, _T("Falha ao enviar o movimento ao servidor!"), TEXT("Informação"), MB_OK | MB_ICONINFORMATION);
                TrataEventos(hWnd, WM_DESTROY, wParam, lParam);
            }
			DestroyWindow(hWnd);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}
	return(0);
}