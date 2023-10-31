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

void CAppsflyerPCModule::Init(const char *dkey, const char *appid)
{
	devkey = dkey;
	appID = appid;
	isStopped = true;
}

void CAppsflyerPCModule::Start(bool skipFirst)
{
	isStopped = false;

	AppsflyerModule afc(devkey, appID);

	RequestData req = buildRequestData();

	FHttpRequestRef reqH = afc.af_firstOpen_init(req, skipFirst);
	SendHTTPReq(reqH, FIRST_OPEN_REQUEST);
}

void CAppsflyerPCModule::Stop()
{
	isStopped = true;
}

void CAppsflyerPCModule::LogEvent(std::string event_name, std::string event_parameters, std::string event_custom_parameters)
{
	if (isStopped) {
		return;
	}

	AppsflyerModule afc(devkey, appID);

	RequestData req = buildRequestData();

	req.event_name = event_name;
	req.event_parameters = event_parameters;
	req.event_custom_parameters = event_custom_parameters;

	FHttpRequestRef reqH = afc.af_inappEvent(req);
	SendHTTPReq(reqH, INAPP_EVENT_REQUEST);
}

std::string CAppsflyerPCModule::GetAppsFlyerUID()
{
	AppsflyerModule afc(devkey, appID);
	return afc.get_AF_id();
}

void CAppsflyerPCModule::SetCustomerUserId(std::string customerUserID)
{
	if (!isStopped) {
		// Cannot set CustomerUserID while the SDK has started.
		return;
	}
	// Customer User ID has been set
	cuid = customerUserID;
}

bool CAppsflyerPCModule::IsInstallOlderThanDate(std::string datestring)
{
	AppsflyerModule afc(devkey, appID);
	return afc.isInstallOlderThanDate(datestring);
}

std::string CAppsflyerPCModule::to_utf8(std::wstring& wide_string)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	return utf8_conv.to_bytes(wide_string);
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

	if (!cuid.empty()) {
		req.customer_user_id = cuid;
	}

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