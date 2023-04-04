#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define TAM 200

void CreateVal(HKEY key) {
    TCHAR val_name[TAM], value[TAM];

    _tprintf_s(_T("\nIndique o nome do valor: "));
    _tscanf_s(_T("%s"), val_name, TAM - 1);
    _tprintf_s(_T("\nIndique o valor: "));
    _tscanf_s(_T("%s"), value, TAM - 1);

    if (RegSetValueEx(key, val_name, NULL, REG_SZ, value, (_tcslen(value) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS)
        _tprintf_s(_T("\nValor criado!"));
    else
        _tprintf_s(_T("\nErro ao criar o valor."));
    return;
}

void setNumFaixas(HKEY key, DWORD numFaixas) {

    if (RegSetValueEx(key, TEXT("numFaixas"), NULL, REG_DWORD, numFaixas, sizeof(numFaixas)) == ERROR_SUCCESS)
        _tprintf_s(_T("\nValor setado!"));
    else
        _tprintf_s(_T("\nErro ao setado o valor."));
    return;
}

void setVelIniCarros(HKEY key, DWORD velIniCarros) {

    if (RegSetValueEx(key, TEXT("velIniCarros"), NULL, REG_DWORD, velIniCarros, sizeof(velIniCarros)) == ERROR_SUCCESS)
        _tprintf_s(_T("\nValor setado!"));
    else
        _tprintf_s(_T("\nErro ao setado o valor."));
    return;
}

void ElimVal(HKEY key) {
    TCHAR val_name[TAM];

    _tprintf_s(_T("\nIndique o nome do valor: "));
    _tscanf_s(_T("%s"), val_name, TAM - 1);

    if (RegDeleteValue(key, val_name) == ERROR_SUCCESS)
        _tprintf_s(_T("\nO valor %s foi eliminado."), val_name);
    else
        _tprintf_s(_T("\nErro ao eliminar o valor."));
    return;
}

void CheckVal(HKEY key) {
    TCHAR val_name[TAM], value[TAM];
    DWORD size = sizeof(TCHAR) * TAM;

    _tprintf_s(_T("\nIndique o nome do valor: "));
    _tscanf_s(_T("%s"), val_name, TAM - 1);

    if (RegQueryValueEx(key, val_name, NULL, NULL, value, &size) == ERROR_SUCCESS)
        _tprintf_s(_T("\nO valor %s tem como valor: %s"), val_name, value);
    else
        _tprintf_s(_T("\nErro ao consultar o valor."));
    return;
}

int getNumFaixas(HKEY key) {
    TCHAR val_name[TAM] = TEXT("numFaixas");
    int value = 5;
    DWORD size = sizeof(TCHAR) * TAM;


    if (RegQueryValueEx(key, val_name, NULL, NULL, value, &size) == ERROR_SUCCESS)
        _tprintf_s(_T("\nO valor %s tem como valor: %s"), val_name, value);
    else
        _tprintf_s(_T("\nErro ao consultar o valor."));
    return value;
}

int getVelIniCarros(HKEY key) {
    TCHAR val_name[TAM] = TEXT("velIniCarros");
    int value = 1;
    DWORD size = sizeof(TCHAR) * TAM;

    if (RegQueryValueEx(key, val_name, NULL, NULL, value, &size) == ERROR_SUCCESS)
        _tprintf_s(_T("\nO valor %s tem como valor: %s"), val_name, value);
    else
        _tprintf_s(_T("\nErro ao consultar o valor."));
    return value;
}
