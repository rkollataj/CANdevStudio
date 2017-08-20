#ifndef __CONTEXT_H
#define __CONTEXT_H

#include <memory>
#include <tuple>

class CanDeviceInterface;

template <typename... Args> struct Context {
    Context(Args*... args)
        // need explicitely mark unique_ptr type as GCC 5 fails to deduce type
        : _implsPtr(std::unique_ptr<Args>(args)...)
        // only to satisfy compiler. Will not be used.
        , _implsRef(*args...)
        , _usePtr(true)
    {
    }

    Context(Args&... args)
        : _implsRef(args...)
        , _usePtr(false)
    {
    }

    template <typename T> T& get()
    {
        if (_usePtr) {
            return *std::get<std::unique_ptr<T>>(_implsPtr).get();
        } else {
            return std::get<T&>(_implsRef);
        }
    }

private:
    std::tuple<std::unique_ptr<Args...>> _implsPtr;
    std::tuple<Args&...> _implsRef;
    const bool _usePtr;
};

typedef Context<CanDeviceInterface> CanDeviceCtx;

#endif /* !__CONTEXT_H */
