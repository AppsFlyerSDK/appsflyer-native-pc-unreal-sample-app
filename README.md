---
title: Native PC Unreal
category: 6446526dddf659006c7ea807
order: 2
hidden: false
slug: native-pc-unreal
---

> Link to repository  
> [GitHub](https://github.com/AppsFlyerSDK/appsflyer-native-pc-unreal-sample-app)

## AppsFlyer Native PC Unreal SDK integration

AppsFlyer empowers gaming marketers to make better decisions by providing powerful tools to perform cross-platform attribution.

Game attribution requires the game to integrate the AppsFlyer SDK that records first opens, consecutive sessions, and in-app events. For example, purchase events.
We recommend you use this sample app as a reference for integrating the AppsFlyer SDK into your Unreal game. **Note**: The sample code that follows is currently only supported in a Windows environment.

**Prerequisites**:

- Unreal Engine 4.2x.

<hr/>

## AppsflyerModule - Interface

`AppsflyerModule.h`, included in the `appsflyer-native-pc-unreal-sample-app/AppsflyerPCModule` folder, contains the required code and logic to connect to AppsFlyer servers and report events.

### Init

This method receives your API key and app ID and initializes the AppsFlyer Module.

**Method signature**

```c++
void init(const char* devkey, const char* appID)
```

**Usage**:

```c++
AppsflyerModule()->init("DEV_KEY", "APP_ID");
```

<span id="app-details">**Arguments**:</span>

- `DEV_KEY`: Get from the marketer or [AppsFlyer HQ](https://support.appsflyer.com/hc/en-us/articles/211719806-App-settings-#general-app-settings).
- `APP_ID`: Your Appsflyer app id.

### Start

This method sends first open/session requests to AppsFlyer.

**Method signature**

```c++
void start(bool skipFirst = false)
```

**Usage**:

```c++
// without the flag
AppsflyerModule()->start();

// with the flag
bool skipFirst = [SOME_CONDITION];
AppsflyerModule()->start(skipFirst);
```

### LogEvent

This method receives an event name and JSON object and sends in-app events to AppsFlyer.

**Method signature**

```c++
void logEvent(std::string event_name, json event_values)
```

**Usage**:

```c++
//set event name
std::string event_name = "af_purchase";
//set json string
std::string event_values = "{\"af_currency\":\"USD\",\"af_price\":6.66,\"af_revenue\":24.12}";
AppsflyerModule()->logEvent(event_name, event_values);
```

### GetAppsFlyerUID

Get AppsFlyer's unique device ID. The SDK generates an AppsFlyer unique device ID upon app installation. When the SDK is started, this ID is recorded as the ID of the first app install.

**Method signature**

```c++
std::string getAppsFlyerUID()
```

**Usage**:

```c++
AppsflyerModule()->getAppsFlyerUID();
```

## Running the sample app

1. Open the UE4 engine.
2. Choose **New Project** > **Games** > **First Person**.
3. Choose C++ (instead of Blueprints).
4. Name the project `AppsFlyerSample` and click **Create project**.
5. Follow the [instructions to implement AppsFlyer in your game](#implementing-appsflyer-in-your-unreal-game).
6. Launch the sample app from the UE4 engine editor.
7. After 24 hours, the dashboard updates and shows organic and non-organic installs and in-app events.

## **Implementing AppsFlyer in your Unreal game**

### Setup

1. Open the project in your preferred C++ editor and in the `[YOUR-APP-NAME].Build.cs` file, add `OpenSSL` to your dependencies and `HTTP` as a private dependency:

```c#
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "OpenSSL" });
PrivateDependencyModuleNames.Add("HTTP");
```

2. In your Unreal Project files, under the `Source/[YOUR-APP-NAME]` directory, create a new directory named `AppsflyerPCModule`.
3. Copy the following files from `appsflyer-native-pc-unreal-sample-app/AppsflyerPCModule` to the new folder:

   - AppsflyerModule.cpp
   - AppsflyerModule.cpp
   - AppsflyerModule.h
   - DeviceID.h
   - RequestData.h

4. Generate project files in order to add OpenSSL. [Learn more](https://forums.unrealengine.com/t/how-to-use-included-openssl/670971/2)
5. In the `GameMode.h` file, add the `StartPlay()` function:

```c++
UCLASS(minimalapi)
class AAppsFlyerSampleGameMode : public AGameModeBase
{
 GENERATED_BODY()

public:
 AAppsFlyerSampleGameMode();
 virtual void StartPlay() override;
};

```

6. Open the `Source/[YOUR-APP-NAME]/AppsFlyerSampleGameMode.cpp` file and add the following include to your GameMode.cpp file.

```c++
#include "AppsflyerPCModule/AppsflyerPCModule.h"
using Super = AGameModeBase;
```

7. Add the following function, making sure to replace `DEV_KEY` and `APP_ID` in the [`init`](#init) function with your [app details](#app-details), and report [in-app events](#logevent)

```c++
void AAppsFlyerSampleGameMode::StartPlay()
{
 Super::StartPlay();
 // af module init
 AppsflyerPCModule()->init("DEV_KEY", "APP_ID");
 // af send firstopen/session
 AppsflyerPCModule()->start();
 //set event name
 std::string event_name = "af_purchase";
 //set json string
 std::string event_values = "{\"af_currency\":\"USD\",\"af_price\":6.66,\"af_revenue\":24.12}";
 // af send inapp event
 AppsflyerPCModule()->logEvent(event_name, event_values);
}
```
