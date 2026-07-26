#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <functional>
#include <csignal>

#include <discordsdk/discord.h>

const uint64_t APPLICATION_ID = 1530529364221890771;

std::atomic<bool> running = true;

void signalHandler(int signum)
{
    running.store(false);
}

int main()
{
    std::signal(SIGINT, signalHandler);

    std::cout << "Rich Presence Example" << std::endl;

    // Load SDK at runtime if available
    discordpp::SetLibrarySearchPath("../../_external/discord_social_sdk/bin/debug");
    if (!discordpp::IsAvailable())
    {
        std::cerr << "Discord SDK is not available. Make sure 'discord_partner_sdk.dll is present in '" << discordpp::GetLibrarySearchPath() << "'." << std::endl;
        return 1;
    }

    auto client = std::make_shared<discordpp::Client>();

    client->SetApplicationId(APPLICATION_ID);

    // Configure rich presence details
    discordpp::Activity activity;
    activity.SetType(discordpp::ActivityTypes::Playing);

    discordpp::ActivityAssets assets;
    assets.SetLargeImage("2026");
    assets.SetLargeText("2026 Rebuilt");
    assets.SetSmallImage("blue");
    assets.SetSmallText("Blue Alliance (Blue 1)");
    activity.SetAssets(assets);

    activity.SetName("Driver Sim");
    activity.SetDetails("Score: 118 : 0");
    activity.SetState("Driving Microwave (Teleop)");
    activity.SetStateUrl("https://github.com/recordrobotics/2026-robot");
    activity.SetStatusDisplayType(discordpp::StatusDisplayTypes::State);

    discordpp::ActivityTimestamps timestamps;
    timestamps.SetStart(time(nullptr));
    timestamps.SetEnd(time(nullptr) + 3 * 60);
    activity.SetTimestamps(timestamps);

    discordpp::ActivityButton downloadButton;
    downloadButton.SetLabel("Get Microwave");
    downloadButton.SetUrl("https://github.com/recordrobotics/2026-robot/releases/latest");
    activity.AddButton(downloadButton);

    discordpp::ActivityButton viewButton;
    viewButton.SetLabel("View on GitHub");
    viewButton.SetUrl("https://github.com/recordrobotics/driver-sim");
    activity.AddButton(viewButton);

    // Update rich presence
    client->UpdateRichPresence(activity, [](discordpp::ClientResult result)
                               {
        if(result.Successful()) {
            std::cout << "Rich Presence updated successfully!\n";
        } else {
            std::cerr << "Rich Presence update failed";
        } });

    // Keep application running to allow SDK to receive events and callbacks
    while (running)
    {
        discordpp::RunCallbacks();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}