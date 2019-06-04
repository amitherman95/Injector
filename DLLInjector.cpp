#include "DLLInjector.h"
#include <qfiledialog.h>
#include <qmessagebox.h>

																/*Constructors*/
DLLInjector::DLLInjector(QWidget *parent)
	: QMainWindow(parent)
{
	setupUi(this);
	refreshProcList();
	this->hKernel32Module = GetModuleHandleA(this->dllKernel32);
	this->addrLoadLib = (DWORD)GetProcAddress(this->hKernel32Module, this->loadLibName);
	this->addrFreeLib = (DWORD)GetProcAddress(this->hKernel32Module, this->freeLibName);
}

																	/*Buttons fuctions*/
void DLLInjector::on_btnClose_clicked() {
	QApplication::exit();
}


void DLLInjector::on_btnBrowse_clicked() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Select DLL file"),
																									NULL, tr("DLL files (*.dll)"));
	if (!filename.isEmpty()) {
		txtDLL->setText(filename);
	}
}

void DLLInjector::on_btnRefresh_clicked() {
	refreshProcList();
}

void DLLInjector::on_btnAttach_clicked() {
	attachDLL();
}


													/*Memory and processes functions*/
QString DLLInjector::GetProcessName(DWORD procID) {
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
																PROCESS_VM_READ,
																FALSE, procID);
	HMODULE hMod;
	DWORD cbNeeded;

	if (hProcess != NULL) {
		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
													 &cbNeeded)) {
				GetModuleBaseName(hProcess, hMod, szProcessName,
												sizeof(szProcessName) / sizeof(TCHAR));
				QString procName = QString::fromWCharArray(szProcessName);
				return procName;
		}
	}
	return "";
}

bool DLLInjector::refreshProcList() {
	/*This one will be in almost pure C*/
	DWORD arrProc[1024]; //4KB should be enough;
	DWORD cbNeeded;
	HMODULE hMod;
	TCHAR szProcessName[MAX_PATH];

	procList.clear();
	procComboBox->clear();

	if (EnumProcesses(arrProc, sizeof(arrProc), &cbNeeded)) {
			int N = cbNeeded / sizeof(DWORD);
			for (int i = 0; i < N; i++) {
					QString name = GetProcessName(arrProc[i]);
					if (!name.isEmpty()) {
							this->procList.push_back(arrProc[i]);
							procComboBox->addItem(name + "(" + QString::number(arrProc[i]) + ")");
					}
			}
	}
	return true;
}
HANDLE DLLInjector::GetProcessHandle(DWORD ProcID) {
	// We create a handle with the following access privileges:
		// PROCESS_VM_OPERATION- Perform an operation on the address space of the target process
		// PROCESS_VM_WRITE - Write to the memory of a the target process
		// PROCESS_CREATE_THREAD - create thread in the target process
	//We combine several privilegs with | operator
	HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE |
																PROCESS_CREATE_THREAD, NULL, ProcID);
	return hProcess;
}

bool DLLInjector::attachDLL() {
	QMessageBox msgBox;
	//Check filename
	if (!isFileExist(txtDLL->text())) {
			msgBox.setText("File not found.");
			msgBox.exec();
			return false;
	}
			
	/*Open handle to target process*/
	HANDLE hTarget = GetProcessHandle(procList.at(procComboBox->currentIndex()));
	if (hTarget == NULL)
		return false;
	/*Allocate memory in target program for dll filename*/
	int dllPathLen = txtDLL->text().length();
	void*addrTargetAllocMemory = VirtualAllocEx(hTarget, NULL, dllPathLen,
																						 MEM_COMMIT, PAGE_READWRITE);
	if(addrTargetAllocMemory==NULL)
			return false;

	/*Write filename to target memory*/
	const char * lpStrPath = txtDLL->text().toStdString().c_str();
	if (!WriteProcessMemory(hTarget, addrTargetAllocMemory, lpStrPath, dllPathLen, NULL))
		return NULL;
	/*Create thread in target program that exceutes LoadLibraryA*/
	DWORD threadID = NULL;
	if (!CreateRemoteThread(hTarget,
													NULL,
													0x100,
													(LPTHREAD_START_ROUTINE)hKernel32Module,//Address to execute
													(void*)addrTargetAllocMemory, //Thread parameter	
													NULL,
													&threadID))
		return false;
		//else:
		return true;
}


bool DLLInjector::isFileExist(const QString &filename) {
	QFile file;
	file.setFileName(filename);
	return file.exists();
}