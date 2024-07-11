#include <windows.h>
#include <iostream>
#include "Updateutility.h"

#undef max;

int main(){
	const char* systemDriverPath = "C:\\Windows\\System32\\DriverStore\\";
	UpdateUtility Util;
	UpdateUtility* util = &Util;

	const char* MSG_SUCCESS = "System Restore Protection enabled and System Restore Point created successfully.";
	const char* MSG_ERR = "Please be sure you are running as administrator and /or contact support for assistance.";
	
	if (!(util->SystemRestoreEN())) { goto FAIL; }

	std::cout << "Creating System Restore Point. Please wait..." << std::endl;
	if (!(util->srClientSystemRestorePT())) { goto FAIL; }
	std::cout << std::endl << MSG_SUCCESS << std::endl;

	std::cout << "\nScanning for Drivers. Please wait..." << std::endl;
	std::cout << std::endl << "INF Files Found: " << util->INFdriverConfigScrape(systemDriverPath) << std::endl;

	std::cout << "Updating Drivers. Please Wait..." << std::endl;
	util->SpawnDriverUpdates();
	std::cout << "Driver Install\\Updates Complete." << std::endl;

	std::cout << "Gathering results of Driver Install\\Updates. Please wait..." << std::endl;
	util->DisplayUpdateStats();

	std::cout << std::endl << "Please review the Error Log for failed Install\\Updates." << std::endl;
	util->OpenErrorLog();

	std::cout << "Press [ENTER] to EXIT >";
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), (char)(0x0A));
	
	return EXIT_SUCCESS;

FAIL:
	std::cout << std::endl << MSG_ERR << std::endl;
	return EXIT_FAILURE;
}







/*
class
	void SFCscan();

void UpdateUtility::SFCscan()
{

	ShellExecuteW(nullptr, L"open", L"sfc.exe", L"/scannow", nullptr, 0x0A);
}
main(){
	std::cout << "\nSystem File Checker scanning system files for integrity violations. Please wait..." << std::endl;
	util->SFCscan();
}

*/



/*
#include <mutex>

std::vector<std::thread> pThreads;
	std::mutex thread_output_mutex;

	for (auto& INFpath : this->driverConfigList)
	{
		std::lock_guard<std::mutex> lock(thread_output_mutex);
		pThreads.emplace_back(&UpdateUtility::driverUpdate, this, INFpath);
	}
	for (auto& thread : pThreads)
	{
		thread.join();
	}
*/




/*
* class
	protected:
		void driverUpdate(std::filesystem::path path);
		static DWORD WINAPI StaticDriverUpdate(LPVOID lpParameter)
		{
			auto* params = reinterpret_cast<ThreadParams*>(lpParameter);
			if (!params) return 1;
			params->utility->driverUpdate(params->path);
			return 0;
		}
		struct ThreadParams
		{
			UpdateUtility* utility;
			std::filesystem::path path;
		};




	/*std::mutex thread_output_mutex;
	const unsigned int numThreads = 679;// this->driverConfigList.size() > 0 ? this->driverConfigList.size() : 1;
	unsigned int createdThreads = numThreads;

	HANDLE pThreads[numThreads];

	ThreadParams params = {};
	params.utility = this;

	std::cout << "running threads of count: " << numThreads << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(0x02));


	std::lock_guard<std::mutex> lock(thread_output_mutex);
	for (unsigned int i = 0; i < numThreads; i++)
	{
		params.path = this->driverConfigList[i];
		pThreads[i] = CreateThread(nullptr, 0, UpdateUtility::StaticDriverUpdate, &params, 0, nullptr);
		if (pThreads[i] == NULL) createdThreads--;
	}

	WaitForMultipleObjects(numThreads, pThreads, 0x00000001, INFINITE);

	for (int i = 0; i < createdThreads; ++i) {
		CloseHandle(pThreads[i]);
	}
*/

