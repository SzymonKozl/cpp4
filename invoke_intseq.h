#ifndef __INVOKE_INTSEQ_H__
#define __INVOKE_INTSEQ_H__
#include <vector>
#include <functional>

template<class T>
concept is_intseq = requires (T a) {
    ([]<class U, U...args>(std::integer_sequence<U, args...>){})(a);
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

template<class T>
struct extract_size
{
    using val = std::integral_constant<std::size_t, 1>;
};

template<class U, U... Ints>
struct extract_size<std::integer_sequence<U, Ints...>>
{
    using val = std::integral_constant<std::size_t, sizeof...(Ints)>;
};


template<class... Args>
constexpr int _args_size_nozero(Args...) {
    return (1 * ... * extract_size<Args>::val::value);
}

template<class... Args>
constexpr int args_size(Args... args) {
    return (sizeof...(Args) == 0) ? 1 : _args_size_nozero(args...);
}

#define TRUE_ARGS(A) typename extract_type<A>::type...

template<class F>
concept return_void = (std::same_as<std::invoke_result_t<F>, void>);

template<class F, class T, class... Args>
constexpr void inner (F&& f, T&& arg1, Args&&... args){
    if constexpr (sizeof...(args) == 0){
        std::invoke(std::forward<F>(f), std::forward<T>(arg1));
    }
    else {
        auto newf = std::bind_front(f, std::forward<T>(arg1));
        inner(std::forward<decltype(newf)>(newf), std::forward<Args>(args)...);
    }
}
template<class F, class T, class... Args, T... ints>
constexpr void inner(F&& f, std::integer_sequence<T, ints...>&&, Args&&... args) {
    (inner(std::forward<F>(f), std::forward<std::integral_constant<T, std::forward<T>(ints)>>(
        std::integral_constant<T, std::forward<T>(ints)>()), std::forward<Args>(args)...), ...);
}

// TODO - inner 2 nie jest do końca dopracowany, bo ma większy problem - w ogóle nie działa
template<class F, class T, class... Args>
constexpr void inner2(std::vector<std::invoke_result_t<F, T, TRUE_ARGS(Args)>> &res, F&& f, T&& arg1, Args&&... args) {
    if constexpr (sizeof...(args) == 0){
        res.push_back(std::invoke(f, arg1));
    }
    else {
        inner2(res, std::bind_front(f, arg1), args...);
    }
}
template<class F, class T, class... Args, T... ints>
constexpr void inner2(std::vector<std::invoke_result_t<F, T, TRUE_ARGS(Args)>> &res, F&& f, std::integer_sequence<T, ints...>&&, Args&&... args) {
    (inner2(res, std::forward<F>(f), std::integral_constant<T, ints>(), std::forward<Args>(args)...), ...);
}

template<class F, class... Args>
constexpr decltype(auto) invoke_intseq(F&& f, Args&&... args) {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

template<class F, class... Args> requires has_intseq<Args...> && std::same_as<std::invoke_result_t<F, TRUE_ARGS(Args)>, void>
// FIXME - same_as should, I believe, include Args as parameters, but trivially
// adding them will not solve the problem - we need to unwrap intseqs first
constexpr void invoke_intseq(F&& f, Args&&... args) {
    inner(std::forward<F>(f), std::forward<Args>(args)...);
}

template<class F, class... Args> requires has_intseq<Args...>
constexpr std::vector<std::invoke_result_t<F, TRUE_ARGS(Args)>> invoke_intseq(F&& f, Args&&... args) { // FIXME
    std::vector<std::invoke_result_t<F, TRUE_ARGS(Args)>> ans; // FIXME - should include Args (insert same comment as above)
    inner2(std::forward<decltype(ans)>(ans), std::forward<f>, std::forward<Args>(args)...);
    return ans;
}

#endif