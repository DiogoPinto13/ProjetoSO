#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <Tlhelp32.h>
#include "Sapo.h"
/* ===================================================== */
/* Programa base (esqueleto) para aplica��es Windows     */
/* ===================================================== */
// Cria uma janela de nome "Janela Principal" e pinta fundo de branco
// Modelo para programas Windows:
//  Composto por 2 fun��es: 
//	WinMain()     = Ponto de entrada dos programas windows
//			1) Define, cria e mostra a janela
//			2) Loop de recep��o de mensagens provenientes do Windows
//     TrataEventos()= Processamentos da janela (pode ter outro nome)
//			1) � chamada pelo Windows (callback) 
//			2) Executa c�digo em fun��o da mensagem recebida

// Fun��o de callback que ser� chamada pelo Windows sempre que acontece alguma coisa
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

// Nome da classe da janela (para programas de uma s� janela, normalmente este nome � 
// igual ao do pr�prio programa) "szprogName" � usado mais abaixo na defini��o das 
// propriedades do objecto janela
TCHAR szProgName[] = TEXT("Base");

// ============================================================================
// FUN��O DE IN�CIO DO PROGRAMA: WinMain()
// ============================================================================
// Em Windows, o programa come�a sempre a sua execu��o na fun��o WinMain()que desempenha
// o papel da fun��o main() do C em modo consola WINAPI indica o "tipo da fun��o" (WINAPI
// para todas as declaradas nos headers do Windows e CALLBACK para as fun��es de
// processamento da janela)
// Par�metros:
//   hInst: Gerado pelo Windows, � o handle (n�mero) da inst�ncia deste programa 
//   hPrevInst: Gerado pelo Windows, � sempre NULL para o NT (era usado no Windows 3.1)
//   lpCmdLine: Gerado pelo Windows, � um ponteiro para uma string terminada por 0
//              destinada a conter par�metros para o programa 
//   nCmdShow:  Par�metro que especifica o modo de exibi��o da janela (usado em  
//        	   ShowWindow()

//BITMAP
// uma vez que temos de usar estas vars tanto na main como na funcao de tratamento de eventos
// nao ha uma maneira de fugir ao uso de vars globais, dai estarem aqui

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

HDC bmpDC; // hdc do bitmap
HWND hWndGlobal; // handle para a janela

HDC memDC = NULL; // copia do device context que esta em memoria, tem de ser inicializado a null

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

	// operacoes de escrita da imagem - BitBlt
	BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
}

void paintMap(HDC hdc) {
	WaitForSingleObject(hMutexUIElements, INFINITE);
	HMENU hMenu = GetMenu(hWndGlobal);
	if(hMenu == NULL){
		//deu merda
	}
	else {
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
				//mete os sapos
				//x = i*tamanhoBmp, y = 0
				drawBitmap(j * tamanhoBmp, i * tamanhoBmp, hBmpFrog, bmpFrog, hdc);
			}
			else if(map.map[i][j] == _T('_') || map.map[i][j] == _T('-')) {
				//mete tudo special lane
				//x = i*tamanhoBmp, y = 0
				drawBitmap(j * tamanhoBmp, i * tamanhoBmp, hBmpSpecialLane, bmpSpecialLane, hdc);
			}
			else if (map.map[i][j] == _T(' ')) {
				//mete as lanes
				//x = i*tamanhoBmp, y = i*tamanhoBmp
				drawBitmap(j * tamanhoBmp, i * tamanhoBmp, hBmpLane, bmpLane, hdc);
			}
			else if (map.map[i][j] == _T('C')) {
				//mete os cars
				//x = i*tamanhoBmp, y = i*tamanhoBmp
				drawBitmap(j * tamanhoBmp, i * tamanhoBmp, hBmpCar, bmpCar, hdc);
			}
			else if (map.map[i][j] == _T('O')) {
				//mete os obstaculos
				//x = i*tamanhoBmp, y = i*tamanhoBmp
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
        MessageBox(hWndGlobal, _T("O Servidor fechou."), _T("Informa��o"), MB_OK | MB_ICONINFORMATION);
		ExitProcess(0);
    }
    else{
        if(!WriteFile(hNamedPipeMovement, &action, sizeof(enum Movement), &nBytes, NULL)){
            MessageBox(hWndGlobal, _T("Falha ao enviar a mensagem ao servidor!"), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
        }
        ResetEvent(data->hEventCloseClient);
        MessageBox(hWndGlobal, _T("O Jogo terminou."), _T("Informa��o"), MB_OK | MB_ICONINFORMATION);
		ExitProcess(0);
    }

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
				MessageBox(hWndGlobal, _T("Erro ao receber o mapa!"), _T("Informa��o"), MB_OK | MB_ICONINFORMATION);
				ExitProcess(0);
			}
		}
		if(map.numLifes == 0 || errorCode == 109){
			MessageBox(hWndGlobal, _T("Ficou sem vidas."), _T("Informa��o"), MB_OK | MB_ICONINFORMATION);
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
		MessageBox(hWndGlobal, _T("Erro ao carregar o bmp do frog!"), _T("Informa��o"), MB_OK | MB_ICONINFORMATION);
	}

	_swprintf_p(buffer, 32, _T("car%d.bmp"), index);
	hBmpCar = (HBITMAP)LoadImage(NULL, buffer, IMAGE_BITMAP, tamanhoBmp, tamanhoBmp, LR_LOADFROMFILE);
	if (hBmpFrog == NULL) {
		MessageBox(hWndGlobal, _T("Erro ao carregar o bmp car!"), _T("Informa��o"), MB_OK | MB_ICONINFORMATION);
	}

	_swprintf_p(buffer, 32, _T("lane%d.bmp"), index);
	hBmpLane = (HBITMAP)LoadImage(NULL, buffer, IMAGE_BITMAP, tamanhoBmp, tamanhoBmp, LR_LOADFROMFILE);
	if (hBmpFrog == NULL) {
		MessageBox(hWndGlobal, _T("Erro ao carregar o bmp da lane!"), _T("Informa��o"), MB_OK | MB_ICONINFORMATION);
	}

	_swprintf_p(buffer, 32, _T("specialLane%d.bmp"), index);
	hBmpSpecialLane = (HBITMAP)LoadImage(NULL, buffer, IMAGE_BITMAP, tamanhoBmp, tamanhoBmp, LR_LOADFROMFILE);
	if (hBmpFrog == NULL) {
		MessageBox(hWndGlobal, _T("Erro ao carregar o bmp da special lane!"), _T("Informa��o"), MB_OK | MB_ICONINFORMATION);
	}

	_swprintf_p(buffer, 32, _T("obstacle%d.bmp"), index);
	hBmpObstacle = (HBITMAP)LoadImage(NULL, buffer, IMAGE_BITMAP, tamanhoBmp, tamanhoBmp, LR_LOADFROMFILE);
	if (hBmpFrog == NULL) {
		MessageBox(hWndGlobal, _T("Erro ao carregar o bmp frog"), _T("Informa��o"), MB_OK | MB_ICONINFORMATION);
	}

	GetObject(hBmpFrog, sizeof(BITMAP), &bmpFrog);
	GetObject(hBmpCar, sizeof(BITMAP), &bmpCar);
	GetObject(hBmpLane, sizeof(BITMAP), &bmpLane);
	GetObject(hBmpSpecialLane, sizeof(BITMAP), &bmpSpecialLane);
	GetObject(hBmpObstacle, sizeof(BITMAP), &bmpObstacle);

}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;		// hWnd � o handler da janela, gerado mais abaixo por CreateWindow()
	MSG lpMsg;		// MSG � uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp;	// WNDCLASSEX � uma estrutura cujos membros servem para
	// definir as caracter�sticas da classe da janela

// ============================================================================
// 1. Defini��o das caracter�sticas da janela "wcApp"
//    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
// ============================================================================
	wcApp.cbSize = sizeof(WNDCLASSEX);      // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst;		         // Inst�ncia da janela actualmente exibida
	// ("hInst" � par�metro de WinMain e vem
		  // inicializada da�)
	wcApp.lpszClassName = szProgName;       // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos;       // Endere�o da fun��o de processamento da janela
	// ("TrataEventos" foi declarada no in�cio e
	// encontra-se mais abaixo)
	wcApp.style = CS_HREDRAW | CS_VREDRAW;  // Estilo da janela: Fazer o redraw se for
	// modificada horizontal ou verticalmente

	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);   // "hIcon" = handler do �con normal
	// "NULL" = Icon definido no Windows
	// "IDI_AP..." �cone "aplica��o"
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION); // "hIconSm" = handler do �con pequeno
	// "NULL" = Icon definido no Windows
	// "IDI_INF..." �con de informa��o
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);	// "hCursor" = handler do cursor (rato)
	// "NULL" = Forma definida no Windows
	// "IDC_ARROW" Aspecto "seta"
	wcApp.lpszMenuName = IDC_SAPO;			// Classe do menu que a janela pode ter
	// (NULL = n�o tem menu)
	wcApp.cbClsExtra = 0;				// Livre, para uso particular
	wcApp.cbWndExtra = 0;				// Livre, para uso particular
	wcApp.hbrBackground = CreateSolidBrush(RGB(125, 125, 125));
	//(HBRUSH)GetStockObject(WHITE_BRUSH);
// "hbrBackground" = handler para "brush" de pintura do fundo da janela. Devolvido por
// "GetStockObject".Neste caso o fundo ser� branco

// ============================================================================
// 2. Registar a classe "wcApp" no Windows
// ============================================================================
	if (!RegisterClassEx(&wcApp))
		return(0);

	// ============================================================================
	// 3. Criar a janela
	// ============================================================================
	hWnd = CreateWindow(
		szProgName,			// Nome da janela (programa) definido acima
		TEXT("Sapo"),// Texto que figura na barra do t�tulo
		WS_OVERLAPPEDWINDOW,	// Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT,		// Posi��o x pixels (default=� direita da �ltima)
		CW_USEDEFAULT,		// Posi��o y pixels (default=abaixo da �ltima)
		CW_USEDEFAULT,		// Largura da janela (em pixels)
		CW_USEDEFAULT,		// Altura da janela (em pixels)
		(HWND)HWND_DESKTOP,	// handle da janela pai (se se criar uma a partir de
		// outra) ou HWND_DESKTOP se a janela for a primeira,
		// criada a partir do "desktop"
		(HMENU)NULL,			// handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst,		// handle da inst�ncia do programa actual ("hInst" �
		// passado num dos par�metros de WinMain()
		0);				// N�o h� par�metros adicionais para a janela

	HDC hdc; // representa a propria janela

	hdc = GetDC(hWnd);

	// carregar os bitmaps
	loadBMP(1);
	// criamos copia do device context e colocar em memoria
	bmpDC = CreateCompatibleDC(hdc);
	// aplicamos o bitmap ao device context
	//SelectObject(bmpDC, hBmpFrog);

	ReleaseDC(hWnd, hdc);
	
	hWndGlobal = hWnd;

	// Cria a thread de movimenta��o
	//CreateThread(NULL, 0, MovimentaImagem, NULL, 0, NULL);


	// ============================================================================
	// 4. Mostrar a janela
	// ============================================================================
	ShowWindow(hWnd, nCmdShow);	// "hWnd"= handler da janela, devolvido por
	// "CreateWindow"; "nCmdShow"= modo de exibi��o (p.e.
	// normal/modal); � passado como par�metro de WinMain()
	UpdateWindow(hWnd);		// Refrescar a janela (Windows envia � janela uma
	// mensagem para pintar, mostrar dados, (refrescar)

    //Check pipe for availability
    //se o coiso devolver falso ou 0 ou wtv, mandamos uma message box para informar e depois fechamos o programa
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
        MessageBox(hWnd, msg, TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    else{

        hEventCloseClients = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_CLOSE_CLIENTS_EVENT);
        if (hEventCloseClients == INVALID_HANDLE_VALUE) {
            MessageBox(hWnd, _T("Erro ao abrir o evento de fechar os clientes."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        
        hEventClose = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_CLOSE_EVENT);
        if (hEventClose == INVALID_HANDLE_VALUE) {
            MessageBox(hWnd, _T("Erro ao abrir o evento de fechar."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        TCHAR buffer[64];
        _swprintf_p(buffer, 64, FIFOFROGMOVEMENT, (int) GetProcessId(GetCurrentProcess()));
        if (!WaitNamedPipe(buffer, 2000)) {
            MessageBox(hWnd, _T("Erro ao esperar pelo pipe de movimento."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        hNamedPipeMovement = CreateFile(buffer, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if(hNamedPipeMovement == INVALID_HANDLE_VALUE) {
            MessageBox(hWnd, _T("Erro ao criar o pipe de movimento."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        _swprintf_p(buffer, 64, FIFOFROGMAP, (int) GetProcessId(GetCurrentProcess()));
        if (!WaitNamedPipe(buffer, 2000)) {
            MessageBox(hWnd, _T("Erro ao esperar pelo pipe do mapa."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        hNamedPipeMap = CreateFile(buffer, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        if(hNamedPipeMap == INVALID_HANDLE_VALUE) {
            MessageBox(hWnd, _T("Erro ao criar o pipe do mapa."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        TKILL *dadosKillThread = malloc(sizeof(TKILL));

        dadosKillThread->hEventClose = hEventClose;
		dadosKillThread->hEventCloseClient = hEventCloseClients;
        if(CreateThread(NULL, 0, KillThread, dadosKillThread, 0, NULL) == NULL){
            MessageBox(hWnd, _T("Erro ao lan�ar a thread de fechar."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            return 0;
        }

		if(CreateThread(NULL, 0, ReceiveMapThread, NULL, 0, NULL) == NULL){
			MessageBox(hWnd, _T("Erro ao lan�ar a thread de ler o mapa."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
			return 0;
		}
		
        //MessageBox(hWnd, _T("App started successfully."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
    }

// ============================================================================
// 5. Loop de Mensagens
// ============================================================================
// O Windows envia mensagens �s janelas (programas). Estas mensagens ficam numa fila de
// espera at� que GetMessage(...) possa ler "a mensagem seguinte"
// Par�metros de "getMessage":
// 1)"&lpMsg"=Endere�o de uma estrutura do tipo MSG ("MSG lpMsg" ja foi declarada no
//   in�cio de WinMain()):
//			HWND hwnd		handler da janela a que se destina a mensagem
//			UINT message		Identificador da mensagem
//			WPARAM wParam		Par�metro, p.e. c�digo da tecla premida
//			LPARAM lParam		Par�metro, p.e. se ALT tamb�m estava premida
//			DWORD time		Hora a que a mensagem foi enviada pelo Windows
//			POINT pt		Localiza��o do mouse (x, y)
// 2)handle da window para a qual se pretendem receber mensagens (=NULL se se pretendem
//   receber as mensagens para todas as
// janelas pertencentes � thread actual)
// 3)C�digo limite inferior das mensagens que se pretendem receber
// 4)C�digo limite superior das mensagens que se pretendem receber

// NOTA: GetMessage() devolve 0 quando for recebida a mensagem de fecho da janela,
// 	  terminando ent�o o loop de recep��o de mensagens, e o programa

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);	// Pr�-processamento da mensagem (p.e. obter c�digo
		// ASCII da tecla premida)
		DispatchMessage(&lpMsg);	// Enviar a mensagem traduzida de volta ao Windows, que
		// aguarda at� que a possa reenviar � fun��o de
		// tratamento da janela, CALLBACK TrataEventos (abaixo)
	}

	// ============================================================================
	// 6. Fim do programa
	// ============================================================================
	return((int)lpMsg.wParam);	// Retorna sempre o par�metro wParam da estrutura lpMsg
}

// ============================================================================
// FUN��O DE PROCESSAMENTO DA JANELA
// Esta fun��o pode ter um nome qualquer: Apenas � neces�rio que na inicializa��o da
// estrutura "wcApp", feita no in�cio de // WinMain(), se identifique essa fun��o. Neste
// caso "wcApp.lpfnWndProc = WndProc"
//
// WndProc recebe as mensagens enviadas pelo Windows (depois de lidas e pr�-processadas
// no loop "while" da fun��o WinMain()
// Par�metros:
//		hWnd	O handler da janela, obtido no CreateWindow()
//		messg	Ponteiro para a estrutura mensagem (ver estrutura em 5. Loop...
//		wParam	O par�metro wParam da estrutura messg (a mensagem)
//		lParam	O par�metro lParam desta mesma estrutura
//
// NOTA:Estes par�metros est�o aqui acess�veis o que simplifica o acesso aos seus valores
//
// A fun��o EndProc � sempre do tipo "switch..." com "cases" que descriminam a mensagem
// recebida e a tratar.
// Estas mensagens s�o identificadas por constantes (p.e.
// WM_DESTROY, WM_CHAR, WM_KEYDOWN, WM_PAINT...) definidas em windows.h
// ============================================================================



LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	//handle para o device context
	HDC hdc;
	PAINTSTRUCT ps;
	//RECT rect;

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
		// evento que � disparado sempre que o sistema pede um refrescamento da janela
	case WM_PAINT:
		// Inicio da pintura da janela, que substitui o GetDC
		hdc = BeginPaint(hWnd, &ps);
		//GetClientRect(hWnd, &rect);

		// se a copia estiver a NULL, significa que � a 1� vez que estamos a passar no WM_PAINT e estamos a trabalhar com a copia em memoria
		/*if (memDC == NULL) {
			// cria copia em memoria
			memDC = CreateCompatibleDC(hdc);
			hBitmapDB = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
			// aplicamos na copia em memoria as configs que obtemos com o CreateCompatibleBitmap
			SelectObject(memDC, hBitmapDB);
			DeleteObject(hBitmapDB);
		}*/
		// opera��es feitas na copia que � o memDC
		//FillRect(memDC, &rect, CreateSolidBrush(RGB(125, 125, 125)));

		//memDC = CreateCompatibleDC(hdc);

		paintMap(hdc);

		/*SelectObject(bmpDC, hBmpFrog);

		// operacoes de escrita da imagem - BitBlt
		BitBlt(hdc, 0, 0, bmpFrog.bmWidth, bmpFrog.bmHeight, bmpDC, 0, 0, SRCCOPY);*/

		//old Bitmap
		//SelectObject(bmpDC, hBitmapDB);

		// bitblit da copia que esta em memoria para a janela principal - � a unica opera��o feita na janela principal
		//BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);


		// Encerra a pintura, que substitui o ReleaseDC
		EndPaint(hWnd, &ps);
		break;

	case WM_ERASEBKGND:
		hdc = (HDC)wParam;
		RECT rcClient;
		GetClientRect(hWnd, &rcClient);

		// Fill the background with a solid color
		HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // White color
		FillRect(hdc, &rcClient, hBrush);
		DeleteObject(hBrush);

		return TRUE; 

	case WM_SIZE:
		InvalidateRect(hWnd, NULL, FALSE);
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
			MessageBox(hWnd, _T("Falha ao enviar o movimento ao servidor!"), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            TrataEventos(hWnd, WM_DESTROY, wParam, lParam);
        }
        if(wParam != VK_ESCAPE){
            if(!ReadFile(hNamedPipeMovement, &response, sizeof(enum ResponseMovement), &nBytes, NULL)){
				MessageBox(hWnd, _T("Falha ao receber os dados do servidor!"), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            }
			if (response == LOSE) {
				MessageBox(hWnd, _T("Perdeste o jogo!"), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
                TrataEventos(hWnd, WM_DESTROY, wParam, lParam);
			}
            else if(response == WIN){
                MessageBox(hWnd, _T("Ganhou! Passando para o pr�ximo nivel."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            }
            else if(response == PAUSED){
                MessageBox(hWnd, _T("O jogo est� pausado."), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
            }
        }
        else
            TrataEventos(hWnd, WM_CLOSE, wParam, lParam);
        break;
	case WM_CLOSE:
		// handle , texto da janela, titulo da janela, configura��es da MessageBox(botoes e icons)
		if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"), TEXT("Confirma��o"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			// o utilizador disse que queria sair da aplica��o
            action = END;
            if(!WriteFile(hNamedPipeMovement, &action, sizeof(enum Movement), &nBytes, NULL)){
                MessageBox(hWnd, _T("Falha ao enviar o movimento ao servidor!"), TEXT("Informa��o"), MB_OK | MB_ICONINFORMATION);
                TrataEventos(hWnd, WM_DESTROY, wParam, lParam);
            }
			DestroyWindow(hWnd);
		}
		break;
	case WM_DESTROY:	// Destruir a janela e terminar o programa
		// "PostQuitMessage(Exit Status)"
		PostQuitMessage(0);
		break;
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// n�o � efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;  // break tecnicamente desnecess�rio por causa do return
	}
	return(0);
}