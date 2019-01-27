#pragma once

#include "promise.h"
#include <string>
#include <vector>
#include <utility>

extern "C" {
    #include "libgamestream/client.h"
}

typedef std::pair<char *, size_t> GameImageData;

struct ServerError;

struct ApplicationInfo {
    int id;
    std::string name;
    promise<GameImageData, ServerError> *image;

    static ApplicationInfo wrap(PAPP_LIST app);
    static std::vector<ApplicationInfo> wrapList(PAPP_LIST apps);
};

