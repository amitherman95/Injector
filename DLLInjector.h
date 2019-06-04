#ifndef _DLLINJECTOR_H
#define _DLLINJECTOR_H


#include <QtWidgets/QMainWindow>
#include <qfile.h>
#include "ui_DLLInjector.h"
#include <iostream>
#include <windows.h>
#include <memory>
#include <Psapi.h>
	
using namespace Ui;

typedef struct processInfo {
	DWORD dProcessID;
} processInfo;


class DLLInjector : public QMainWindow, public DLLInjectorClass
{
																	/* Methods */
	Q_OBJECT

public:
	DLLInjector(QWidget *parent = Q_NULLPTR);
	bool refreshProcList();

private:
	bool attachDLL();
	QString GetProcessName(DWORD procID);
	HANDLE GetProcessHandle(DWORD ProcID);
	bool isFileExist(const QString &filename);

private slots:	
	void on_btnClose_clicked();
	void on_btnBrowse_clicked();
	void on_btnRefresh_clicked();
	void on_btnAttach_clicked();
	
														/*Data members*/
private:
	std::vector<DWORD> procList;
	/*Since WinApi uses c-style strings we'll define the names this way */
	const char loadLibName[13] = "LoadLibraryA";
	const char freeLibName[12] = "FreeLibrary";
	const char dllKernel32[13] = "kernel32.dll";

	/*Handles*/
	HMODULE hKernel32Module=NULL;
	/*Functions addressses*/
	DWORD addrLoadLib = NULL;
	DWORD addrFreeLib = NULL;
};

#endif