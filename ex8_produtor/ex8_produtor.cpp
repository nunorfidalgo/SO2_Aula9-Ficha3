#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define MAX 256

#define FILE_PATH TEXT("FicheiroPartilhado.txt")

int _tmain(int argc, LPTSTR argv[])
{
	HANDLE hMutex, hEvento, hMem;
	TCHAR frase[MAX];
	TCHAR * ptrMem;

	hMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(frase), TEXT("shm"));

	hMutex = CreateMutex(NULL, FALSE, TEXT("MeuMutex"));

	hEvento = CreateEvent(NULL, TRUE, FALSE, TEXT("MeuEvento"));

	if (hMem == NULL || hMutex == NULL || hEvento == NULL)
	{
		_tprintf(TEXT("Erro a criar recursos de comunicação e/ou sincronização.\n"));
		return -1;
	}

	ptrMem = (TCHAR *)MapViewOfFile(hMem, FILE_MAP_WRITE, 0, 0, sizeof(frase));
	if (ptrMem == NULL)
	{
		_tprintf(TEXT("Erro ao mapear memória partilhada(%d).\n"), sizeof(frase));
		return -1;
	}

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	_tprintf(TEXT("[Processo Produtor %d] Vou começar a trabalhar...\n Prima \'fim\' para terminar...\n"), GetCurrentProcessId());

	do
	{
		_fgetts(frase, MAX, stdin);
		fflush(stdin);

		WaitForSingleObject(hMutex, INFINITE);

		_tcscpy_s(ptrMem, (_tcslen(frase) + 1) * sizeof(TCHAR), frase);

		SetEvent(hEvento);
		ReleaseMutex(hMutex);
		ResetEvent(hEvento);

	} while (_tcsncmp(frase, TEXT("fim"), 3));

	_tprintf(TEXT("[Processo Produtor %d] Finalmente vou terminar..."), GetCurrentProcessId());

	CloseHandle(hMutex);
	CloseHandle(hEvento);
	CloseHandle(hMem);

	return 0;
}
