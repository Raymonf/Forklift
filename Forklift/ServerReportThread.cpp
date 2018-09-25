#include "stdafx.h"
#include <vector>
#include <mutex>
#include <ostream>
#include <sstream>
#include <stdint.h>
#include "ServerReportThread.h"
#include "HashFile.h"
#ifndef _DEBUG
#include "curl/curl.h"
#endif
#include "VersionManager.h"

#define FORKLIFT_VERSION "1.0.3"
#define WULINSHU_URL "https://wulinshu.raymonf.me"

std::vector<HashFile *> hashes;
std::mutex mutex;
int64_t reportedIdx = 0;

void ServerReportThread::push(HashFile *hashFile)
{
	std::unique_lock<std::mutex> guard(mutex);
	hashes.push_back(hashFile);
	guard.unlock();
}

void ServerReportThread::work()
{
	// We're going to be doing work here, aren't we?
	// Let's initialise cURL.
#ifndef _DEBUG
	CURL *curl = curl_easy_init();
#endif

	// Check for updates
	{
		std::string version { };
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &version);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, static_cast<size_t(__stdcall *)(char*, size_t, size_t, void*)>(
			[](char* ptr, size_t size, size_t nmemb, void* resultBody) {
			*(static_cast<std::string*>(resultBody)) += std::string{ ptr, size * nmemb };
			return size * nmemb;
		}
		));
		curl_easy_setopt(curl, CURLOPT_URL, WULINSHU_URL "/hook/version");
		curl_easy_perform(curl);

		if (version != FORKLIFT_VERSION)
		{
			MessageBoxA(NULL, "It appears that there is an update for Forklift. Please check the original thread on Shenmue Dojo or Nexus.", "Forklift Loader", MB_OK);
		}
	}

	//PrintThread{} << "[ServerReportThread] Thread started!" << std::endl;
	while (true)
	{
		// Send every 10 seconds
		Sleep(10000);

		std::unique_lock<std::mutex> guard(mutex);

		auto goal = hashes.size() - 1;

		if (goal == reportedIdx) {
			// We don't need to report to the server,
			// there's no data.
			//PrintThread{} << "[ServerReportThread] No data to report" << std::endl;

			continue;
		}

		//PrintThread{} << "[ServerReportThread] I have data to report. Doing that now." << std::endl;

		std::stringstream data;
		data << "[";

		for (size_t i = reportedIdx; i < goal; i++)
		{
			std::string path(hashes[i]->path);

			// In case...
			size_t n = 0;
			while ((n = path.find("\"", n)) != std::string::npos)
			{
				path.insert(n, "\\");
				n += 2;
			}

			data << "{";
			data << "\"path\":\"" << path << "\",";
			data << "\"hash\":\"" << std::hex << hashes[i]->hash << "\"";
			data << "}";

			if (i + 1 < goal) {
				data << ",";
			}

			// free up the memory
			delete hashes[i]->path;
		}

		data << "]";

		reportedIdx = goal;

		guard.unlock();

		std::string json = data.str();
		std::string url(WULINSHU_URL "/api/storehash/");
		url += VersionManager::singleton()->getGameId();

#ifndef _DEBUG
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
		curl_easy_perform(curl);
#endif

		//PrintThread{} << json << std::endl;
		//PrintThread{} << "--- NOT POSTED ---" << std::endl;
	}
}