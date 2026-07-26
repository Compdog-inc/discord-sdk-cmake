# discord-sdk-cmake

A CMake wrapper for the Discord Social SDK that loads the SDK dynamically at runtime.

The wrapper allows applications to use the Discord SDK without requiring the SDK binaries to be installed beforehand. If the SDK is missing, Discord features are disabled and the application continues running normally.

The SDK can also be placed in a custom location or downloaded at runtime, then loaded without restarting the application.

## Features

- Runtime SDK loading
  - Missing Discord SDK binaries do not prevent the application from starting.
  - Supports `.dll`, `.so`, and `.dylib` libraries.

- Custom library paths
  - Configure where the wrapper searches for the SDK.
  - Useful for app data folders, plugin directories, or downloaded SDK files.

- CMake SDK download support
  - Optionally download and extract the Discord SDK during CMake configuration.

## Usage

### Check SDK availability

Set the SDK search path before creating your Discord client.

```cpp
#include <iostream>
#include <discordsdk/discord.h>

int main() {
    // First searches in this path, if not found falls back to default OS loading behavior
    discordpp::SetLibrarySearchPath("../../_external/discord_social_sdk/bin/debug");

    if (!discordpp::IsAvailable()) {
        std::cerr << "Discord SDK not found: "
                  << discordpp::GetLibrarySearchPath()
                  << "\n";

        // Continue without Discord features
    } else {
        std::cout << "Discord SDK loaded\n";

        // Initialize Discord client here
        auto client = std::make_shared<discordpp::Client>();
    }

    return 0;
}
````

### Add to CMake

Add the repository with `FetchContent`, then link the target.

```cmake
include(FetchContent)

# Optionally specify download url during configuration
set(DISCORDSDK_DOWNLOAD_URL "<url_to_discord_sdk.zip>")

FetchContent_Declare(
    discordsdk
    GIT_REPOSITORY https://github.com/Compdog-inc/discord-sdk-cmake.git
    GIT_TAG main
)

FetchContent_MakeAvailable(discordsdk)

target_link_libraries(my_project PRIVATE discordsdk::discordsdk)
```

Or to specify the download url from command line:

```bash
cmake -B build \
  -DDISCORDSDK_DOWNLOAD_URL="<url_to_discord_sdk.zip>"
```

## Examples

Examples can be found in the [examples](examples/) folder.
