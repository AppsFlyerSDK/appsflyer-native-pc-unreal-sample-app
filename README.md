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

`AppsflyerModule.h`, included in the `appsflyer-unreal-sample-app/AppsflyerIntegrationFiles` folder, contains the required code and logic to connect to AppsFlyer servers and report events.

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

1. In your Unreal editor, go to **Plugins**, activate **Online Subsystem EOS**, and then restart the editor.
2. Open the project in your preferred C++ editor and in the `[YOUR-APP-NAME].Build.cs` file, add `OpenSSL` to your dependencies and `HTTP` as a private dependency:

```c#
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "OpenSSL" });
PrivateDependencyModuleNames.Add("HTTP");
```

5. In your Unreal Project files, under the **Source** directory, create a new directory named **AppsflyerModule**.
6. Copy the following files from `appsflyer-unreal-sample-app/AppsflyerIntegrationFiles` to the new folder:

   - AppsflyerModule.cpp
   - AppsflyerModule.cpp
   - AppsflyerModule.h
   - DeviceID.h
   - RequestData.h

7. Generate project files in order to add OpenSSL. [Learn more](https://forums.unrealengine.com/t/how-to-use-included-openssl/670971/2)
8. In the `GameMode.h` file, add the `StartPlay()` function:

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

9. Open the `Source/AppsFlyerSample/AppsFlyerSampleGameMode.cpp` file and add the following include to your GameMode.cpp file.

```c++
#include "AppsflyerModule.cpp"
```

10. Add the following function, making sure to replace `DEV_KEY` and `APP_ID` in the [`init`](#init) function with your [app details](#app-details) and initialize the EOS SDK with your EOS details.

```c++
void ATestGameMode::StartPlay()
{
 Super::StartPlay();

 EOS_InitializeOptions SDKOptions;
 SDKOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
 SDKOptions.AllocateMemoryFunction = NULL;
 SDKOptions.ReallocateMemoryFunction = NULL;
 SDKOptions.ReleaseMemoryFunction = NULL;
 SDKOptions.ProductName = "PRODUCT_NAME";
 SDKOptions.ProductVersion = "1.0";
 SDKOptions.Reserved = NULL;
 SDKOptions.SystemInitializeOptions = NULL;
 EOS_EResult res = EOS_Initialize(&SDKOptions);

 if (res == EOS_EResult::EOS_Success || res == EOS_EResult::EOS_AlreadyConfigured) {
  EOS_Platform_Options PlatformOptions;
  PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
  PlatformOptions.Reserved = NULL;
  PlatformOptions.bIsServer = false;
  PlatformOptions.EncryptionKey = NULL;
  PlatformOptions.OverrideCountryCode = NULL;
  PlatformOptions.OverrideLocaleCode = NULL;
  PlatformOptions.ProductId = "PRODUCT_ID";
  EOS_Platform_ClientCredentials cCreds;
  cCreds.ClientId = "CLIENT_ID";
  cCreds.ClientSecret = "CLIENT_SECRET";
  PlatformOptions.ClientCredentials = cCreds;
  PlatformOptions.SandboxId = "SANDBOX_ID";
  PlatformOptions.DeploymentId = "DEPLOYMENT_ID";
  PlatformOptions.EncryptionKey = "ENCRYPTION_KEY";
  EOS_HPlatform platform = EOS_Platform_Create(&PlatformOptions);

  // af module init
  AppsflyerModule()->init("DEV_KEY", "APP_ID");
  // af send firstopen/session
  AppsflyerModule()->start();

  //set event name
  std::string event_name = "af_purchase";
  //set json string
  std::string event_values = "{\"af_currency\":\"USD\",\"af_price\":6.66,\"af_revenue\":24.12}";
  // af send inapp event
  AppsflyerModule()->logEvent(event_name, event_values);
 }
 else {
  UE_LOG(LogTemp, Warning, TEXT("EOS FAIL"));
  return;
 }
}
```

11. Report [in-app events](#logevent)
