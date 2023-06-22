#include "AppsflyerPCModule.h"

#include <stdio.h>
#include <stdlib.h>

#include "AppsflyerModule.cpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

using namespace std;

CAppsflyerPCModule *AppsflyerPCModule()
{
	static CAppsflyerPCModule inv;
	return &inv;
}

CAppsflyerPCModule::CAppsflyerPCModule()
{
}

void CAppsflyerPCModule::init(const char *dkey, const char *appid)
{
	devkey = dkey;
	appID = appid;
}

void CAppsflyerPCModule::start(bool skipFirst)
{
	AppsflyerModule afc(devkey, appID);

	RequestData req = buildRequestData();

	FHttpRequestRef reqH = afc.af_firstOpen_init(req, skipFirst);
	SendHTTPReq(reqH, FIRST_OPEN_REQUEST);
}

void CAppsflyerPCModule::logEvent(std::string event_name, std::string event_values)
{
	AppsflyerModule afc(devkey, appID);

	RequestData req = buildRequestData();

	req.event_name = event_name;
	req.event_values = event_values;

	FHttpRequestRef reqH = afc.af_inappEvent(req);
	SendHTTPReq(reqH, INAPP_EVENT_REQUEST);
}

std::string CAppsflyerPCModule::getAppsFlyerUID()
{
	AppsflyerModule afc(devkey, appID);
	return afc.get_AF_id();
}

RequestData CAppsflyerPCModule::buildRequestData()
{
	AppsflyerModule afc(devkey, appID);

	// app build id
	std::string app_version = "1.0.0";

	// create timestamp
	std::time_t t = std::time(0);
	std::ostringstream oss;
	oss << t;
	std::string timestamp = oss.str();

	RequestData req;
	req.timestamp = timestamp;
	req.device_os_version = "1.0.0";
	req.app_version = app_version;
	req.device_model = afc.get_OS(); // TODO: check how to retreive device model.
	req.limit_ad_tracking = "false";
	req.request_id = afc.uuid_gen().c_str();

	// adding AF id to the request
	DeviceIDs af_id;
	af_id.type = "custom";
	af_id.value = afc.get_AF_id().c_str();
	req.device_ids.insert(req.device_ids.end(), af_id);

	return req;
}

void CAppsflyerPCModule::SendHTTPReq(FHttpRequestRef pRequest, int64 contextId)
{
	UE_LOG(LogTemp, Warning, TEXT("context is: %i"), contextId);
	if (contextId == FIRST_OPEN_REQUEST || contextId == SESSION_REQUEST)
	{
		// Set the callback, which will execute when the HTTP call is complete
		pRequest->OnProcessRequestComplete().BindLambda(
			// Here, we "capture" the 'this' pointer (the "&"), so our lambda can call this
			// class's methods in the callback.
			[&](
				FHttpRequestPtr pRequest,
				FHttpResponsePtr pResponse,
				bool connectedSuccessfully) mutable
			{
				if (connectedSuccessfully)
				{
					// We should have a JSON response - attempt to process it.
					AppsflyerModule afc(devkey, appID);
					UE_LOG(LogTemp, Warning, TEXT("HTTP ResponseCode: %i"), pResponse->GetResponseCode());
					if (contextId == FIRST_OPEN_REQUEST)
					{
						UE_LOG(LogTemp, Warning, TEXT("FIRST_OPEN_REQUEST event"));
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("SESSION_REQUEST event"));
					}

					if (pResponse->GetResponseCode() == EHttpResponseCodes::Ok || pResponse->GetResponseCode() == EHttpResponseCodes::Accepted)
					{
						afc.increase_AF_counter();
						UE_LOG(LogTemp, Warning, TEXT("AF counter increase"));
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("HTTP error: %i"), pResponse->GetResponseCode());
					}
				}
				else
				{
					switch (pRequest->GetStatus())
					{
					case EHttpRequestStatus::Failed_ConnectionError:
						UE_LOG(LogTemp, Error, TEXT("Connection failed."));
					default:
						UE_LOG(LogTemp, Error, TEXT("Request failed."));
					}
				}
			});
	}
	else if (contextId == INAPP_EVENT_REQUEST)
	{
		// Set the callback, which will execute when the HTTP call is complete
		pRequest->OnProcessRequestComplete().BindLambda(
			// Here, we "capture" the 'this' pointer (the "&"), so our lambda can call this
			// class's methods in the callback.
			[&](
				FHttpRequestPtr pRequest,
				FHttpResponsePtr pResponse,
				bool connectedSuccessfully) mutable
			{
				if (connectedSuccessfully)
				{
					// We should have a JSON response - attempt to process it.
					AppsflyerModule afc(devkey, appID);
					UE_LOG(LogTemp, Warning, TEXT("HTTP ResponseCode: %i"), pResponse->GetResponseCode());
					UE_LOG(LogTemp, Warning, TEXT("INAPP_EVENT_REQUEST event"));
				}
				else
				{
					switch (pRequest->GetStatus())
					{
					case EHttpRequestStatus::Failed_ConnectionError:
						UE_LOG(LogTemp, Error, TEXT("Connection failed."));
					default:
						UE_LOG(LogTemp, Error, TEXT("Request failed."));
					}
				}
			});
	}

	// Finally, submit the request for processing
	pRequest->ProcessRequest();
}