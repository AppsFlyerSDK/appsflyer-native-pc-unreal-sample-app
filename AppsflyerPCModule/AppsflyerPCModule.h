#pragma once
#include "RequestData.h"
#include <iostream>
#include <string>

#define UI UI_ST
// THIRD_PARTY_INCLUDES_START
#include "openssl/evp.h"
#include "openssl/hmac.h"
// THIRD_PARTY_INCLUDES_END
#undef UI

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert

class CAppsflyerPCModule
{
public:
	// This method receives your api key and app id,
	// and initializes the AppsFlyer Connector
	void Init(const char *devkey, const char *appID);
	// sends "first open/session" request to AppsFlyer.
	void Start(bool skipFirst = false);
	void Stop();
	// This method receives an event name and json object and sends an in-app event to AppsFlyer.
	void LogEvent(std::string event_name, std::string event_parameters, std::string event_custom_parameters = "");
	// get AppsFlyer's unique device ID.
	std::string GetAppsFlyerUID();
	void SetCustomerUserId(std::string customerUserID);
	// This method receives a date string and returns true if the game folder modification date is older than the date string.
	bool IsInstallOlderThanDate(std::string datestring);
	std::string to_utf8(std::wstring& wide_string);

private:
	const char *devkey;
	const char *appID;
	bool isStopped;
	std::string cuid;
	friend CAppsflyerPCModule *AppsflyerPCModule();
	CAppsflyerPCModule();
	void SendHTTPReq(FHttpRequestRef pRequest, int64 context);
	RequestData buildRequestData();
};

const int64 FIRST_OPEN_REQUEST = 100;
const int64 SESSION_REQUEST = 101;
const int64 INAPP_EVENT_REQUEST = 102;

CAppsflyerPCModule *AppsflyerPCModule();