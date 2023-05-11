#include "registry.h"

void setNumFaixas(HKEY key, DWORD numFaixas) {

    if (RegSetValueEx(key, TEXT("numFaixas"), 0, REG_DWORD, (BYTE*)&numFaixas, sizeof(numFaixas)) == ERROR_SUCCESS)
        _tprintf_s(_T("\nValor setado!"));
    else
        _tprintf_s(_T("\nErro ao setado o valor."));
    return;
}

void setVelIniCarros(HKEY key, DWORD velIniCarros) {
    
    if (RegSetValueEx(key, TEXT("velIniCarros"), 0, REG_DWORD, (BYTE*)&velIniCarros, sizeof(velIniCarros)) == ERROR_SUCCESS)
        _tprintf_s(_T("\nValor setado!"));
    else
        _tprintf_s(_T("\nErro ao setado o valor."));
    return;
}

DWORD getNumFaixas(HKEY key) {
    TCHAR val_name[TAM] = TEXT("numFaixas");
	DWORD value = 0;
    DWORD size = sizeof(value);
    if (RegQueryValueEx(key, val_name, NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS)
        _tprintf_s(_T("\nO valor %s tem como valor: %d"), val_name, value);
    else
        _tprintf_s(_T("\nErro ao consultar o valor."));
    return value;
}

DWORD getVelIniCarros(HKEY key) {
    TCHAR val_name[TAM] = TEXT("velIniCarros");
    DWORD value;
    DWORD size = sizeof(value);
    if (RegQueryValueEx(key, val_name, NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS)
        _tprintf_s(_T("\nO valor %s tem como valor: %d"), val_name, value);
    else
        _tprintf_s(_T("\nErro ao consultar o valor."));
    return value;
}

HKEY getKey(){
    HKEY key;
    TCHAR keyPath[TAM] = _T("Software\\ProjetoSO");
	if (RegOpenKeyEx(HKEY_CURRENT_USER, keyPath, 0, KEY_ALL_ACCESS, &key) == ERROR_FILE_NOT_FOUND) {
		_tprintf_s(_T("\nKey not found, creating..."));
		if (RegCreateKeyEx(HKEY_CURRENT_USER, keyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL) == ERROR_SUCCESS){
			_tprintf_s(_T("\nKey has been created!"));
            setNumFaixas(key, 5);
            setVelIniCarros(key, 1);
        }
		else {
            _ftprintf_s(stderr, TEXT("\nFailed to create Key.\n"));
			return NULL;
		}
	}
	else
		_tprintf_s(_T("\nKey opened."));
    return key;
}

BOOL initRegistry(int argc, TCHAR **argv, DWORD *numFaixas, DWORD *velIniCarros, HANDLE hConsole){
    //buscar as cenas através da linha de comandos
    HKEY regKey = getKey();
    if (regKey == NULL) {
        return FALSE;
    }

    const DWORD limInfFaixas = 1, limSupFaixas = 8;
    const DWORD limInfVelIni = 1, limSupVelIni = 5;

    //num faixas: 1 a 8 inclusive
    //velocidade inicial: 1 a 5 inclusive
    if (argc == 3) {
        *numFaixas = _tcstoul(argv[1], NULL, 0);
        *velIniCarros = _tcstoul(argv[2], NULL, 0);
        _tprintf_s(_T("\nChecking lanes"));
        if (*numFaixas < limInfFaixas  || *numFaixas > limSupFaixas) {
            TCHAR bufferMessage[64];
            _tprintf_s(_T("\nIs not valid"));
            *numFaixas = getNumFaixas(regKey);
            errorMessage(TEXT("O número de faixas tem que ser entre 1 a 8!"), hConsole);
            _swprintf_p(bufferMessage, 64, _T("Usando os valores por default: %d"), *numFaixas);
            errorMessage(bufferMessage, hConsole);
        }
        else {
            setNumFaixas(regKey, *numFaixas);
        }
        _tprintf_s(_T("\nChecking speeds"));
        if (*velIniCarros < limInfVelIni || *velIniCarros > limSupVelIni) {
            TCHAR bufferMessage[64];
            _tprintf_s(_T("\nIs also not valid"));
            *velIniCarros = getVelIniCarros(regKey);
            errorMessage(TEXT("O número da velocidade inicial do carro tem que ser entre 1 e 5!"), hConsole);
            _swprintf_p(bufferMessage, 64, _T("Usando os valores por default: %d"), *velIniCarros);
            errorMessage(bufferMessage, hConsole);
        }
        else {
            setVelIniCarros(regKey, *velIniCarros);
        }
    }
    else {
        *velIniCarros = getVelIniCarros(regKey);  //registry
        if (argc == 2) {
            //numFaixas = (DWORD)argv[1];
            *numFaixas = _tcstoul(argv[1], NULL, 0);
            if (*numFaixas < limInfFaixas || *numFaixas > limSupFaixas) {
				TCHAR bufferMessage[64];
				*numFaixas = getNumFaixas(regKey);
				errorMessage(TEXT("O número de faixas tem que ser entre 1 a 8!"), hConsole);
				_swprintf_p(bufferMessage, 64, _T("Usando os valores por default: %d"), *numFaixas);
				errorMessage(bufferMessage, hConsole);
            }
            else {
                setNumFaixas(regKey, *numFaixas);
            }
        }
        else {
            *numFaixas = getNumFaixas(regKey);
        }
    }
    CloseHandle(regKey);
    return TRUE;
}
