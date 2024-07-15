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
