#pragma once

#include "util.h"
#include <switch.h>
#include <stdio.h>
#include <optional>

template <typename TResolve, typename TReject>
class promise {
public:
    promise() : 
        resolve_value_{}, 
        reject_value_{}
    {
        // UEvent should not auto-clear, so that the promise will stay resolved and future
        // calls to `promise.get_future().get()` will not block.
        ueventCreate(&resolve_event_, false);
        ueventCreate(&reject_event_, false);
    }

    inline bool get() {
        int index;
        Result rc = waitMulti(&index, -1,
            waiterForUEvent(&resolve_event_), 
            waiterForUEvent(&reject_event_)
        );

        if (R_SUCCEEDED(rc)) {
            if (index == 0) {
                return true;
            }
            else {
                return false;
            }
        }

        return false;
    }

    inline bool isResolved() {
        return resolve_value_.has_value();
    }

    inline bool isRejected(){
        return reject_value_.has_value();
    }

    inline std::optional<TResolve> resolvedValue() {
        return resolve_value_;
    }

    inline std::optional<TReject> rejectedValue() {
        return reject_value_;
    }

    inline void resolve(TResolve value) {
        // Signal that this ptromise was resolved
        resolve_value_ = value;
        ueventSignal(&resolve_event_);
    };

    inline void reject(TReject value) {
        // Signal that this promise was rejected
        reject_value_ = value;
        ueventSignal(&reject_event_);
    };

private:
    UEvent resolve_event_;
    UEvent reject_event_;
    
    std::optional<TResolve> resolve_value_;
    std::optional<TReject> reject_value_;
};
