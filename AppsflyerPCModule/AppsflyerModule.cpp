#include "AppsflyerPCModule.h"
#include <iostream>
#include "openssl/evp.h"
#include "openssl/hmac.h"
#include <string>
#include <Misc/Guid.h>
#include <iomanip>
#include <sstream>
#include <Containers/StringConv.h>
#include <random>
#include <HAL/PlatformFilemanager.h>
#include <Misc/FileHelper.h>
#include <Logging/LogMacros.h>
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#ifdef WIN32
#define stat _stat
#endif

using namespace std;

class AppsflyerModule
{
public:
	AppsflyerModule(const char *devkey, std::string appid)
	{
		_devkey = devkey;
		_appid = appid;
		AFInfoLoadFile();
	}

	// send curl with hmac auth and json data
	FHttpRequestRef send_http_post(std::string &url, std::string jsonData, int64 ulContextValue)
	{
		char *key = _strdup(_devkey);
		int keylen = strlen(key);
		const unsigned char *data = (const unsigned char *)_strdup(jsonData.c_str());

		int datalen = strlen((char *)data);
		unsigned char *result = NULL;
		unsigned int resultlen = -1;

		result = mx_hmac_sha256((const void *)key, keylen, data, datalen, result, &resultlen);

		// allocate memory for array
		std::stringstream ss;
		for (unsigned int i = 0; i < resultlen; i++)
		{
			ss << hex << setw(2) << setfill('0') << (int)static_cast<unsigned char>(result[i]);
		}
		std::string json_data_str = ss.str();

		FHttpModule &httpModule = FHttpModule::Get();
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> pRequest = httpModule.CreateRequest();

		FString urlFString(url.c_str());
		pRequest->SetURL(urlFString);
		pRequest->SetVerb("POST");
		FString OsVersion, OsSubVersion;
		FPlatformMisc::GetOSVersions(OsVersion, OsSubVersion);
		UE_LOG(LogTemp, Warning, TEXT("This is the OsVersion: %s"), *OsVersion);
		FString userAgent = "UnrealGamesLaucnher/2.2.0 (" + OsVersion + ")";
		pRequest->SetHeader(TEXT("User-Agent"), *userAgent);
		// pRequest->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
		pRequest->SetHeader("Content-Type", TEXT("application/json"));
		pRequest->SetHeader("Authorization", json_data_str.c_str());
		FString dataFString(UTF8_TO_TCHAR(jsonData.c_str()));
		pRequest->SetContentAsString(dataFString);

		return pRequest;
	}

	// report first open event to AppsFlyer (or session if counter > 2)
	FHttpRequestRef af_firstOpen_init(RequestData req, bool skipFirst = false)
	{
		// send requests
		int af_counter_int = get_AF_counter();
		if (af_counter_int < 2 && !skipFirst)
		{
			return af_firstOpenRequest(req);
		}
		else
		{
			return af_sessionRequest(req);
		}
	}

	// report inapp event to AppsFlyer
	FHttpRequestRef af_inappEvent(RequestData req)
	{
		std::string url = "https://events.appsflyer.com/v1.0/c2s/inapp/app/epic/" + _appid;

		/* Now specify the POST data */
		std::string jsonData = postDataStr(req, true);

		return send_http_post(url, jsonData, INAPP_EVENT_REQUEST);
	}

	std::string postDataStr(RequestData req, bool isEvent = false) {
		std::ostringstream oss;
		oss << "{\"device_ids\":[{\"type\":\"" << req.device_ids[0].type << "\",\"value\":\"" << req.device_ids[0].value << "\"}";
		oss << "],\"request_id\":\"" << req.request_id << "\",\"device_os_version\":\"" << req.device_os_version << "\",\"device_model\":\"" << req.device_model << "\",\"limit_ad_tracking\":" << req.limit_ad_tracking << ",\"app_version\":\"" << req.app_version << "\"";
		if (isEvent) {
			oss << ",\"event_parameters\":" << req.event_parameters << ",\"event_name\":\"" << req.event_name << "\"";
			if (req.event_custom_parameters != "") {
				oss << ",\"event_custom_parameters\":" << req.event_custom_parameters;
			}
		}
		if (!req.customer_user_id.empty()) {
			oss << ",\"customer_user_id\":\"" << req.customer_user_id << "\"";
		}
		oss << "}";

		return oss.str();
	}

	// return af uuid
	std::string get_AF_id()
	{
		if (af_guid.empty())
		{
			af_guid = uuid_gen();
			AFInfoSaveFile(af_guid, "0");
			return af_guid;
		}
		return af_guid;
	}

	// save the appsflyer counter and id info into a file
	void AFInfoSaveFile(std::string guid_str, std::string counter)
	{
		FString dataPath = "appsflyer_info.txt";
		TArray<FString> currentData;
		currentData.Add(UTF8_TO_TCHAR(guid_str.c_str()));
		currentData.Add(UTF8_TO_TCHAR(counter.c_str()));
		FString len = FString::FromInt(currentData.Num());
		UE_LOG(LogTemp, Warning, TEXT("Length of data array: %s"), *len);
		UE_LOG(LogTemp, Warning, TEXT("Saved File 1: %s"), UTF8_TO_TCHAR(guid_str.c_str()));
		UE_LOG(LogTemp, Warning, TEXT("Saved File 2: %s"), UTF8_TO_TCHAR(counter.c_str()));
		bool success = FFileHelper::SaveStringArrayToFile(currentData, *dataPath);
		FString result = success ? ("Successful") : ("Failed");
		UE_LOG(LogTemp, Warning, TEXT("Saved File: %s"), *result);
		FString dir = FPaths::ProjectDir() + dataPath;
		UE_LOG(LogTemp, Warning, TEXT("Saved to: %s"), *dir);
		AFInfoLoadFile();
	}

	// load the appsflyer counter and id info from the info file to the class
	void AFInfoLoadFile()
	{
		FString dataPath = "appsflyer_info.txt";
		TArray<FString> loadedData;
		bool success = FFileHelper::LoadFileToStringArray(loadedData, *dataPath);
		FString result = success ? ("Successful") : ("Failed");
		UE_LOG(LogTemp, Warning, TEXT("Loaded File: %s"), *result);
		if (success)
		{
			for (FString string : loadedData)
			{
				UE_LOG(LogTemp, Warning, TEXT("FileManipulation: Found string %s"), *string);
			}
			if (loadedData.Num() > 1)
			{
				char *af_guid_char = TCHAR_TO_ANSI(*loadedData[0]);
				af_guid = af_guid_char;
				char *af_counter_char = TCHAR_TO_ANSI(*loadedData[1]);
				af_counter = af_counter_char;
				UE_LOG(LogTemp, Warning, TEXT("FileManipulation: Found string %s"), *loadedData[0]);
				UE_LOG(LogTemp, Warning, TEXT("FileManipulation: Found string %s"), *loadedData[1]);
			}
		}
	}

	// increase the AF open counter in the registry
	void increase_AF_counter()
	{
		std::string new_counter = std::to_string(get_AF_counter() + 1);
		UE_LOG(LogTemp, Warning, TEXT("new_counter: %s"), UTF8_TO_TCHAR(new_counter.c_str()));
		AFInfoSaveFile(af_guid, new_counter);
	}

	// get the OS type
	std::string get_OS()
	{
#ifdef _WIN32
		return ("Windows_32-bit");
#elif _WIN64
		return ("Windows_64-bit");
#elif __APPLE__ || __MACH__
		return ("Mac_OSX");
#elif __linux__
		return ("Linux");
#elif __FreeBSD__
		return ("FreeBSD");
#elif __unix || __unix__
		return ("Unix");
#else
		return ("Other");
#endif
	}

	// generate UUID
	std::string uuid_gen()
	{
		static random_device dev;
		static mt19937 rng(dev());

		uniform_int_distribution<int> dist(0, 15);

		const char *v = "0123456789abcdef";
		const bool dash[] = {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

		string res;
		for (int i = 0; i < 16; i++)
		{
			if (dash[i])
				res += "-";
			res += v[dist(rng)];
			res += v[dist(rng)];
		}
		return res;
	}

	std::time_t to_time_t(const std::string &str, bool is_dst = false, const std::string &format = "%Y-%b-%d %H:%M:%S")
	{
		std::tm t = {0};
		t.tm_isdst = is_dst ? 1 : 0;
		std::istringstream ss(str);
		ss >> std::get_time(&t, format.c_str());
		return mktime(&t);
	}


	bool isInstallOlderThanDate(string date)
	{
		bool isInstallOlder = NULL;
		FString launchDir = FPaths::LaunchDir();
		UE_LOG(LogTemp, Warning, TEXT("launchDir: %s"), *launchDir);
		const char* folderPathCh = StringCast<ANSICHAR>(*launchDir).Get();

		struct stat result;
		if (stat(folderPathCh, &result) == 0)
		{
			struct tm* mod_time = localtime(&result.st_ctime);
			std::time_t mod_time_t = result.st_mtime;
			//printf("File time and date: %s", asctime(mod_time));

			//__time64_t mod_time = result.st_mtime;
			time_t excludeInstallDateBefore = to_time_t(date);
			//printf("File time and date 2: %s", asctime(localtime(&excludeInstallDateBefore)));
			double diff = difftime(mod_time_t, excludeInstallDateBefore);
			isInstallOlder = diff < 0;
		}

		return isInstallOlder;
	}

private:
	std::string _appid;

	// the AF app _devkey
	const char *_devkey;

	std::string af_counter;
	std::string af_guid;

	// get the AF app open counter from registry
	int get_AF_counter()
	{
		if (af_counter.empty())
		{
			af_counter = "0";
			// reg_writeKey(reg_key, reg_path, "AF_counter", counter);
			return std::stoi(af_counter);
		}
		return std::stoi(af_counter);
	}

	// report first open event to AppsFlyer
	FHttpRequestRef af_firstOpenRequest(RequestData req)
	{
		std::string url = "https://events.appsflyer.com/v1.0/c2s/first_open/app/epic/" + _appid;

		/* Now specify the POST data */
		std::string jsonData = postDataStr(req);

		return send_http_post(url, jsonData, FIRST_OPEN_REQUEST);
		// CURLcode res = send_http_post(url, jsonData);
	}

	// report session event (after the counter passes 2 opens) to AppsFlyer
	FHttpRequestRef af_sessionRequest(RequestData req)
	{
		std::string url = "https://events.appsflyer.com/v1.0/c2s/session/app/epic/" + _appid;

		/* Now specify the POST data */
		std::string jsonData = postDataStr(req);

		return send_http_post(url, jsonData, SESSION_REQUEST);
	}

	// encrypt data with _devkey
	unsigned char *mx_hmac_sha256(const void *key, int keylen, const unsigned char *data, int datalen,
								  unsigned char *result, unsigned int *resultlen)
	{
		return HMAC(EVP_sha256(), key, keylen, data, datalen, result, resultlen);
	}
};
