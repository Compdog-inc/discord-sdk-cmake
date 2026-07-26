#pragma once

#include <discordpp.h>
#include <string>
#include <string_view>

namespace discordpp
{
    bool IsAvailable();

    std::string_view GetLibrarySearchPath();
    void SetLibrarySearchPath(const std::string_view path);
};