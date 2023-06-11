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

class CAppsflyerPCModule
{
public:
	// This method receives your api key and app id,
	// and initializes the AppsFlyer Connector
	void init(const char *devkey, const char *appID);
	// sends �first open/session� request to AppsFlyer.
	void start(bool skipFirst = false);
	// This method receives an event name and json object and sends an in-app event to AppsFlyer.
	void logEvent(std::string event_name, std::string event_values);
	// get AppsFlyer's unique device ID.
	std::string getAppsFlyerUID();

private:
	const char *devkey;
	const char *appID;
	friend CAppsflyerPCModule *AppsflyerPCModule();
	CAppsflyerPCModule();
	void SendHTTPReq(FHttpRequestRef pRequest, uint64 context);
	RequestData buildRequestData();
};

const int64 FIRST_OPEN_REQUEST = 100;
const int64 SESSION_REQUEST = 101;
const int64 INAPP_EVENT_REQUEST = 102;

CAppsflyerPCModule *AppsflyerPCModule();