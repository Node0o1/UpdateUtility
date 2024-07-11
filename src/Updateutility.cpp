#include <windows.h>
#include <srrestoreptapi.h>
#include <iostream>
#include <iomanip>
#include <strsafe.h>
#include <shellapi.h>
#include <limits>
#include <chrono>
#include <thread>
#include <mutex>
#include <filesystem>
#include <cstring>
#include <vector>
#include "Updateutility.h"
#include<fstream>


UpdateUtility::UpdateUtility() 
{
	this->driverConfigList = {};
	this->fileCount = 0;
	this->errCount = 0;
}


BOOL UpdateUtility::srClientSystemRestorePT()
{
	RESTOREPOINTINFOW rpinfo;
	rpinfo.dwEventType = BEGIN_SYSTEM_CHANGE;
	rpinfo.dwRestorePtType = APPLICATION_INSTALL;
	rpinfo.llSequenceNumber = 0x00; //must be zero when creating RestorePoint
	StringCbCopyW(rpinfo.szDescription, sizeof(rpinfo.szDescription), L"pre-driver update/install restore point");
	STATEMGRSTATUS smgrstatus;

	typedef BOOL(WINAPI* PFN_SETRESTOREPTW) (PRESTOREPOINTINFOW, PSTATEMGRSTATUS);
	PFN_SETRESTOREPTW fnSRSetRestorePointW;
	HMODULE hSrClient = LoadLibraryW(L"srclient.dll");

	if (hSrClient == NULL) { return 0x00; }
	fnSRSetRestorePointW = (PFN_SETRESTOREPTW)GetProcAddress(hSrClient, "SRSetRestorePointW");

	if (!fnSRSetRestorePointW(&rpinfo, &smgrstatus)) { return 0x00; }
	return 0x01;
}


BOOL UpdateUtility::SystemRestoreEN() 
{
	DWORD dwErr = ERROR_SUCCESS;
	HKEY hkey = NULL;
	DWORD dwType = NULL;
	DWORD dwValue = NULL;
	DWORD dwBuffSize = sizeof(DWORD);
	BOOL flag = 0x00;
	const wchar_t* lpSubkey = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore";

	if (!RegOpenKeyExW(HKEY_LOCAL_MACHINE, lpSubkey, NULL, 0x20019, &hkey) == dwErr)
	{
		std::cout << "Error openning Registry Key." << std::endl;
		std::cout << "ERROR_CODE: " << GetLastError() << std::endl;
		goto EXIT;
	}
	if (!RegGetValueW(hkey, nullptr, (LPCWSTR)(L"RPSessionInterval"), RRF_RT_REG_DWORD, &dwType, &dwValue, &dwBuffSize) == dwErr)
	{
		std::cout << "Error getting System Restore Protection status." << std::endl;
		goto CLOSE_REG_AND_EXIT;
	}
	if (!dwValue == 0)
	{
		std::cout << "System Restore Protection is already enabled." << std::endl; 
		flag = 0x01;
		goto CLOSE_REG_AND_EXIT;
	}
	std::cout << "System Protection is currently disabled. Attempting to enable System Restore Protection...." << std::endl; 
	if (!(INT_PTR)ShellExecuteW(nullptr, (LPCWSTR)(L"open"), (LPCWSTR)(L"Powershell.exe"), (LPCWSTR)(L"Enable-ComputerRestore -Drive C:;exit;"), nullptr, 0x00) > 32)
	{
		std::cout << "Failed to enable Sytem Restore Protection automatically." << std::endl;
		std::cout << "ERROR_CODE: " << GetLastError() << std::endl;
		goto CLOSE_REG_AND_EXIT;
	}
	std::this_thread::sleep_for(std::chrono::seconds(0x05));
	std::cout << "System Restore Protection auto-enable successful" << std::endl;
	flag = 0x01;
	
CLOSE_REG_AND_EXIT:
	RegCloseKey(hkey);
EXIT:
	return flag;
}


unsigned int UpdateUtility::INFdriverConfigScrape(const char* rootDir)
{
	std::filesystem::path file;
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(rootDir))
	{
		try
		{
			file = dirEntry.path();
			if (!(file.extension() == ".inf")) { continue; }
			this->driverConfigList.push_back(file);
			this->fileCount++;
		}
		catch (const std::filesystem::filesystem_error& e) { std::cout << "FILE SYSTEM ERROR: " << e.what() << std::endl; }
		catch (...) { std::cout << "FILE SYSTEM ERROR: UNKNOWN!" << std::endl; }
	}
	return this->fileCount;
}


void UpdateUtility::CreateNewLogFile()
{
	std::ofstream errorLogFile;
	errorLogFile.open("./Failed-Driver-Installs.txt", std::ios::out);
	errorLogFile.close();
}


void UpdateUtility::SpawnDriverUpdates()
{
	UpdateUtility::CreateNewLogFile();
	std::vector<std::thread> pThreads;
	const unsigned int batchSize = 50;
	
	for(unsigned int i = 0; i < this->driverConfigList.size(); i++)
	{
		pThreads.emplace_back(&UpdateUtility::driverUpdate, this, this->driverConfigList[i]);
		if (i % batchSize == 0 || i == this->driverConfigList.size()-1)
		{
			for (unsigned int j = 0; j < pThreads.size(); j++)
			{
				pThreads[j].join();
			}
			pThreads.clear();
		}
	}
}


void UpdateUtility::driverUpdate(std::filesystem::path path)
{
	std::string lpApplicationName = "C:/Windows/System32/PNPUTIL.exe";
	//CrateProcess attributes
	std::vector<std::string> outputLines;
	std::string  lpCommandLineSTR = (lpApplicationName + " /add-driver " + static_cast<std::string>(path.string()) + " /install");
	std::wstring lpCommandLineWSTR = std::wstring(lpCommandLineSTR.begin(), lpCommandLineSTR.end());
	const wchar_t* lpCommandLine = lpCommandLineWSTR.c_str();
	STARTUPINFOW info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;

	//Create an anonymous pipe
	SECURITY_ATTRIBUTES lpPipeAttributes = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
	HANDLE hReadPipe;
	HANDLE hWritePipe;
	CreatePipe(&hReadPipe, &hWritePipe, &lpPipeAttributes, 4096);
	info.hStdOutput = hWritePipe;
	info.hStdError = hWritePipe;
	info.dwFlags |= STARTF_USESTDHANDLES;
		
	//ReadFile parameters
	char buffer[4096];
	memset(buffer, 0x00, sizeof(buffer));
	DWORD bytesRead;

	
	if (!(CreateProcessW(nullptr, (LPWSTR)(lpCommandLine), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &info, &processInfo)))
	{
		std::cout << "\n::PROCESS::FAILED::\nERROR_CODE: " << GetLastError() << "\nPROCESS_INFO: " << lpApplicationName << " " << lpCommandLine << std::endl << std::endl;
		goto CLOSE_PIPE;
	}
	WaitForSingleObject(processInfo.hProcess, INFINITE);

	
	if(ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead != 0)
	{
		std::unique_lock<std::mutex> printLock(this->thread_output_mutex);
		std::string buffStr(buffer, sizeof(buffer));
		if (!(buffStr.find("Failed") != std::string::npos))
		{
			std::cout << "\33[32m" << "[+]" << static_cast<std::string>(path.string()) << "\33[0m" << std::endl;
			goto EXIT;

		}
		std::cout << "\33[31m" << "[-]" << static_cast<std::string>(path.string()) << "\33[0m" << std::endl;
		this->errCount++;
		printLock.unlock();

		//split PIPE output into vector by line
		std::string bLine;
		for (auto& i : buffer)
		{
			if (!((i == '\n') || (i == '\r'))) { bLine += i; }
			if (i == '\n')
			{
				if (bLine == "") { continue; }
				outputLines.push_back(bLine);
				bLine = "";
			}
		}
		
		HANDLE hFile = CreateFileW(L"./Failed-Driver-Installs.txt", GENERIC_WRITE, 0x00000002, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		//ensure file opened and handle appropriately.
		if (hFile == INVALID_HANDLE_VALUE) 
		{ 
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
			{
				hFile = CreateFileW(L"./Failed-Driver-Installs.txt", GENERIC_WRITE, 0x00000002, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
			}
		}

		//error message to write
		std::string errMsgStr = (static_cast<std::string>(path.string()) + "\n" + outputLines[1] + "\n" + outputLines[2] + "\n\n");
		const char* errMsg = errMsgStr.c_str();
		std::cout << "\33[0m" << errMsg;

		//write to file //set OVERLAPPED structure to append data to the end of a file.
		OVERLAPPED lpOverlapped = { 0 };
		lpOverlapped.Offset = 0xFFFFFFFF;
		lpOverlapped.OffsetHigh = 0xFFFFFFFF;
		DWORD dwBytesWritten;
		int fResult = WriteFile(hFile, (LPCVOID)(errMsg), strlen(errMsg), &dwBytesWritten, &lpOverlapped);
		
		if (!((fResult != 0) || (GetLastError() == ERROR_IO_PENDING)))
		{
			std::cout << "\33[0m" << "File Write Error Failed." << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(0x01));
		}
		CloseHandle(hFile);
	}
EXIT:
	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);
CLOSE_PIPE:
	CloseHandle(hReadPipe);
	CloseHandle(hWritePipe);
}


void UpdateUtility::DisplayUpdateStats()
{	
	unsigned int maxWidth = 0x1D;
	const char* rTableTitle = "RESULTS OF DRIVER UPDATES";
	int padding = (maxWidth - strlen(rTableTitle)) / 2;
	unsigned int sCount = this->fileCount - this->errCount;
	std::cout << "\33[33m" << std::endl << std::setw(padding) << std::setfill(' ') << " " << rTableTitle << std::endl;
	std::cout << std::setw(maxWidth) << std::setfill('=') << "=" << std::endl;
	std::cout << "| " << std::setw(0x14) << std::left << std::setfill('.') << "Total Drivers:" << std::setw(0x05) << std::right << std::setfill('.') << this->fileCount << std::setw(0x02) << " |" <<std::endl;
	std::cout << "| " << std::setw(0x14) << std::left << std::setfill('.') << "Failed Installs:" << std::setw(0x05) << std::right << std::setfill('.') << this->errCount << std::setw(0x02) << " |" << std::endl;
	std::cout << "| " << std::setw(0x14) << std::left << std::setfill('.') << "Successful Installs:" << std::setw(0x05) << std::right << std::setfill('.') << sCount << std::setw(0x02) << " |" << std::endl;
	std::cout << std::setw(maxWidth) << std::setfill('=') << "=" << "\33[0m" << std::endl;
}


void UpdateUtility::OpenErrorLog()
{
	if (!this->errCount)
	{
		std::cout << "There were NO errors to LOG!" << std::endl;
		return;
	}
	std::cout << "Openning Error Log!" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(0x05));
	ShellExecuteW(nullptr, (LPCWSTR)(L"open"), (LPCWSTR)(L"notepad.exe"), (LPCWSTR)(L"./Failed-Driver-Installs.txt"), nullptr, 0x05);
}


UpdateUtility::~UpdateUtility(){}