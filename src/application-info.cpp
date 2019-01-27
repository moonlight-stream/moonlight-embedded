#include "application-info.h"
#include "server.h"

ApplicationInfo ApplicationInfo::wrap(PAPP_LIST app) {
    promise<GameImageData, ServerError> *image_promise = new promise<GameImageData, ServerError>();
    ApplicationInfo info = {
        app->id,
        app->name,
        image_promise
    }; 
    return info;
}

std::vector<ApplicationInfo> ApplicationInfo::wrapList(PAPP_LIST apps) {
    std::vector<ApplicationInfo> wrapped;
    PAPP_LIST current = apps;

    while (current) {
        wrapped.push_back(ApplicationInfo::wrap(current));
        current = current->next;
    }

    return wrapped;
}