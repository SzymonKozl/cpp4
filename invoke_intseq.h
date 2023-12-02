#ifndef __INVOKE_INTSEQ_H__
#define __INVOKE_INTSEQ_H__
#include <vector>
#include <functional>

template<class T>
concept is_intseq = requires (T a) {
    []<class U, U...args>(std::integer_sequence<U, args...>){}(a);
};

template<class... Ts>
concept has_intseq = (false || ... || is_intseq<Ts>);

template<class T>
struct extract_type {
    using type = T;
};

template<class U, U... Ints>
struct extract_type<std::integer_sequence<U, Ints...>> {
    using type = std::integral_constant<U, 0>;
};

#define TRUE_ARGS(A) typename extract_type<A>::type...

template<class F, class... Args>
concept ret_void = std::same_as<std::invoke_result_t<F, TRUE_ARGS(Args)>, void>;

template<class F, class... Args>
concept ret_ref = std::is_reference_v<std::invoke_result_t<F, TRUE_ARGS(Args)>>;

template<class F, class T, class... Args>
constexpr void inner (F&& f, T&& arg1, Args&&... args){
    if constexpr (sizeof...(args) == 0){
        std::invoke(std::forward<F>(f), std::forward<T>(arg1));
    }
    else {
        decltype(auto) newf = [&f, &arg1]<class... U>(U&&... u) {
            std::invoke(
                std::forward<F>(f), std::forward<T>(arg1), std::forward<U>(u)...
            );
        };
        inner(std::forward<decltype(newf)>(newf), std::forward<Args>(args)...);
    }
}
template<class F, class T, class... Args, T... Ints>
constexpr void inner(
    F&& f, std::integer_sequence<T, Ints...>&&, Args&&... args) {
    (inner(std::forward<F>(f),
        std::forward<std::integral_constant<T, std::forward<T>(Ints)>>(
            std::integral_constant<T, std::forward<T>(Ints)>()),
        std::forward<Args>(args)...), ...);
}

template<class F, class T, class... Args, class TRes>
constexpr void inner2(std::vector<TRes> &res, F&& f, T&& arg1, Args&&... args) {
    if constexpr (sizeof...(Args) == 0) {
        res.push_back(std::invoke(f, arg1));
    }
    else {
        decltype(auto) newf = [&f, &arg1]<class... U>(U&&... u)
            -> std::invoke_result_t<F, T, TRUE_ARGS(Args)> {
            return std::invoke(
                std::forward<F>(f), std::forward<T>(arg1), std::forward<U>(u)...
            );
        };
        inner2(res, std::forward<decltype(newf)>(newf),
            std::forward<Args>(args)...);
    }
}

template<class F, class T, class... Args, T... Ints, class TRes>
constexpr void inner2(std::vector<TRes> &res, F&& f,
    std::integer_sequence<T, Ints...>&&, Args&&... args) {
    (inner2(res, std::forward<F>(f), std::integral_constant<T, Ints>(),
        std::forward<Args>(args)...), ...);
}

template<class F, class... Args>
constexpr decltype(auto) invoke_intseq(F&& f, Args&&... args) {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

template<class F, class... Args>
    requires has_intseq<Args...> && ret_void<F, Args...>
constexpr void invoke_intseq(F&& f, Args&&... args) {
    inner(std::forward<F>(f), std::forward<Args>(args)...);
}

template<class F, class... Args> requires has_intseq<Args...>
constexpr decltype(auto) invoke_intseq(F&& f, Args&&... args) {
    std::vector<std::invoke_result_t<F, TRUE_ARGS(Args)>> res;
    inner2(res, std::forward<F>(f), std::forward<Args>(args)...);
    return res;
}

template<class F, class... Args>
    requires has_intseq<Args...> && ret_ref<F, Args...>
constexpr decltype(auto) invoke_intseq(F&& f, Args&&... args) {
    using res_t = std::decay_t<std::invoke_result_t<F, TRUE_ARGS(Args)>>;
    std::vector<std::reference_wrapper<res_t>> res;
    inner2(res, std::forward<F>(f), std::forward<Args>(args)...);
    return res;
}
#endif