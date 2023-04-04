#include "registry.h"

void setNumFaixas(HKEY key, DWORD numFaixas) {

    if (RegSetValueEx(key, TEXT("numFaixas"), NULL, REG_DWORD, &numFaixas, sizeof(numFaixas)) == ERROR_SUCCESS)
        _tprintf_s(_T("\nValor setado!"));
    else
        _tprintf_s(_T("\nErro ao setado o valor."));
    return;
}

void setVelIniCarros(HKEY key, DWORD velIniCarros) {
    
    if (RegSetValueEx(key, TEXT("velIniCarros"), 0, REG_DWORD, &velIniCarros, sizeof(velIniCarros)) == ERROR_SUCCESS)
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
