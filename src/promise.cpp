#include "promise.h"

template <typename TResolve, typename TReject>
promise<TResolve, TReject>::promise() 
    : future_(this)
{
    // UEvent should not auto-clear, so that the promise will stay resolved and future
    // calls to `promise.get_future().get()` will not block.
    ueventCreate(&resolve_event_, false);
    ueventCreate(&reject_event_, false);
}

template <typename TResolve, typename TReject>
void promise<TResolve, TReject>::resolve(TResolve value) {
    // Signal that this promise was resolved
    resolve_value_ = value;
    ueventSignal(&resolve_event_);
}

template <typename TResolve, typename TReject>
void promise<TResolve, TReject>::reject(TReject value) {
    // Signal that this promise was rejected
    reject_value_ = value;
    ueventSignal(&reject_event_);
}

template <typename TResolve, typename TReject>
typename promise<TResolve, TReject>::future *promise<TResolve, TReject>::get_future() {
    return &future_;
}


template <typename TResolve, typename TReject>
promise<TResolve, TReject>::future::future(promise<TResolve, TReject> *promise)
    : promise_(promise),
      resolved_(false),
      rejected_(false)
{ }

template <typename TResolve, typename TReject>
bool promise<TResolve, TReject>::future::get() {
    int index;
    Result rc = waitMulti(&index, -1,
        waiterForUEvent(&promise_->resolve_event_), 
        waiterForUEvent(&promise_->reject_event_)
    );

    if (R_SUCCEEDED(rc)) {
        if (index == 0) {
            resolved_ = true;
            return true;
        }
        else {
            rejected_ = true;
            return false;
        }
    }

    return false;
}

template <typename TResolve, typename TReject>
bool promise<TResolve, TReject>::future::resolved() {
    return resolved_;
}

template <typename TResolve, typename TReject>
bool promise<TResolve, TReject>::future::rejected() {
    return rejected_;
}

template <typename TResolve, typename TReject>
TResolve promise<TResolve, TReject>::future::resolvedValue() {
    return promise_->resolve_value_;
}

template <typename TResolve, typename TReject>
TReject promise<TResolve, TReject>::future::rejectedValue() {
    return promise_->reject_value_;
}