#pragma once
#include <windows.h>
#include <srrestoreptapi.h>
#include <filesystem>
#include <vector>
#include <mutex>


class UpdateUtility {
	private:
		unsigned int fileCount;
		unsigned int errFile;
		unsigned int errCount;		

	protected:
		void driverUpdate(std::filesystem::path path);
		std::vector<std::filesystem::path> driverConfigList;
		std::mutex thread_output_mutex;

	public:
		UpdateUtility();
		BOOL SystemRestoreEN();
		BOOL srClientSystemRestorePT();
		unsigned int INFdriverConfigScrape(const char* rootDir);
		void SpawnDriverUpdates();
		void DisplayUpdateStats();
		void OpenErrorLog();
		void CreateNewLogFile();
		~UpdateUtility();
};


