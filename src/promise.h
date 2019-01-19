#pragma once

#include <switch.h>
#include <stdio.h>

template <typename TResolve, typename TReject>
struct promise {
public:
    struct future {
    public:
        bool get();
        bool resolved();
        bool rejected();
        TResolve resolvedValue();
        TReject rejectedValue();

    private:
        future(promise<TResolve, TReject> *promise);

        promise<TResolve, TReject> *promise_;
        bool resolved_;
        bool rejected_;
    };

    promise();
    void resolve(TResolve value);
    void reject(TReject value);
    future *get_future();

private:
    UEvent resolve_event_;
    UEvent reject_event_;
    
    TResolve resolve_value_;
    TReject reject_value_;

    future future_;
};
