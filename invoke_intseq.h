#ifndef __INVOKE_INTSEQ_H__
#define __INVOKE_INTSEQ_H__
#include <vector>
#include <functional>

template<class T>
concept is_intseq = requires (T a) {
    ([] <class U> (std::integer_sequence<U>){})(a);
};

template<class... Args>
concept has_intseq = (true || ... || is_intseq<Args>);

template<class F>
concept return_void = (std::same_as<std::invoke_result<F>, void>);

template<class F, class T, class... Args>
constexpr std::function<std::invoke_result_t<F>(T, Args...)> curry (F f, T arg1) {
    return [&](Args... args) -> std::invoke_result_t<F> {
        return f(arg1, args...);
    };
}
template<class F, class T, class... Args>
constexpr void inner (F f, T arg1, Args... args){
    if constexpr (sizeof...(args) == 0){
        std::invoke(f, arg1);
    }
    else {
        inner(curry<F, T, Args...>(f, arg1), args...);
    }
}
template<class F, class T, class... Args, T... ints>
constexpr void inner (F f, std::integer_sequence<T, ints...>, Args... args) {
    if constexpr (sizeof...(args) == 0) {
        (std::invoke(f, ints), ...);
    }
    else {
        (inner(curry<F, T, Args...>(f, ints), args...), ...);
    }
}

template<class F, class T, class... Args>
constexpr void inner2 (std::vector<std::invoke_result<F>> &res, F f, T arg1, Args... args){
    if constexpr (sizeof...(args) == 0){
        res.push_back(std::invoke(f, arg1));
    }
    else {
        inner2(res, curry<F, T, Args...>(f, arg1), args...);
    }
}
template<class F, class T, class... Args, T... ints>
constexpr void inner2 (std::vector<std::invoke_result<F>> &res, F f, std::integer_sequence<T, ints...>, Args... args) {
    if constexpr (sizeof...(args) == 0) {
        (res.push_back(std::invoke(f, ints)), ...);
    }
    else {
        (inner2(res, curry<F, T, Args...>(f, ints), args...), ...);
    }
}
template<class F>
constexpr void inner2 (std::vector<std::invoke_result<F>> &res, F f) {
    res.push_back(std::invoke(f));
}

template<class F, class... Args>
constexpr std::invoke_result<F> invoke_intseq(F f, Args... args) {
    return std::invoke(f, args...);
}


template<class F, class... Args> requires has_intseq<Args...> && std::same_as<std::invoke_result<F>, void>
constexpr void invoke_intseq(F f, Args... args) {
    inner (f, args...);
}

template<class T>
constexpr size_t arg_size(T) {
    return 1;
}
template<class T, T... ints>
constexpr size_t arg_size(std::integer_sequence<T, ints...>) {
    return sizeof...(ints);
}

template<class... Args>
constexpr size_t result_size(Args... args) {
    return (1 * ... * arg_size(args));
}


template<class F, class... Args> requires has_intseq<Args...>
constexpr std::vector<std::invoke_result<F>> invoke_intseq(F f, Args... args) {
    std::vector<std::invoke_result<F>> ans;
    if (sizeof...(args))
        inner2 (ans, f, args...);
    else
        inner2(ans, f);
    return ans;
}

#endif