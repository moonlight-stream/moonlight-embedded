#pragma once

#include <switch.h>
#include <stdio.h>

template <typename T> struct future;
template <typename T> struct promise;

template <typename T>
struct future {
public:
    T get() {
        Result rc = waitSingle(waiterForUEvent(&promise_->resolve_event_), -1);

        if (R_SUCCEEDED(rc)) {
            return promise_->value_;
        }

        return nullptr;
    }

private:
    friend struct promise<T>;

    future(promise<T> *promise) : promise_(promise) {}
    promise<T> *promise_;
};

template <typename T>
struct promise {
public:
    promise() {
        // UEvent should not auto-clear, so that the promise will stay resolved and future
        // calls to `promise.get_future().get()` will not block.
        ueventCreate(&resolve_event_, false);
    }

    void resolve(T value) {
        // Signal that this promise was resolved
        value_ = value;
        ueventSignal(&resolve_event_);
    }

    future<T> get_future() {
        return future(this);
    }

private:
    friend struct future<T>;

    char *test_;
    UEvent resolve_event_;
    T value_;
};
