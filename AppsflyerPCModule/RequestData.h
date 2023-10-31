#pragma once
#include <string>
#include <vector>
#include "DeviceID.h"

class RequestData
{
public:
	std::string timestamp;
	std::string ip;
	std::string device_os_version;
	std::string device_model;
	std::string app_version;
	std::vector<DeviceIDs> device_ids;
	std::string request_id;
	std::string limit_ad_tracking;
	std::string customer_user_id;
	std::string event_name;
	std::string event_parameters;
	std::string event_custom_parameters;
};