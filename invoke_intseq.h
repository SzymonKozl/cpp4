#ifndef __INVOKE_INTSEQ_H__
#define __INVOKE_INTSEQ_H__
#include <vector>
#include <functional>

template<class T>
concept is_intseq = requires (T a) {
    ([] <class U, U... ints> (std::integer_sequence<U, ints...>){})(a);
};

template<class... Args>
concept has_intseq = (false || ... || is_intseq<Args>);

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
constexpr void inner2(std::vector<std::invoke_result_t<F>> &res, F&& f, T&& arg1, Args&&... args) {
    if constexpr (sizeof...(args) == 0){
        res.push_back(std::invoke(f, arg1));
    }
    else {
        inner2(res, std::bind_front(f, arg1), args...);
    }
}
template<class F, class T, class... Args, T... ints>
constexpr void inner2(std::vector<std::invoke_result_t<F>> &res, F&& f, std::integer_sequence<T, ints...>&&, Args&&... args) {
    (inner2(res, std::forward<F>(f), std::integral_constant<T, ints>(), std::forward<Args>(args)...), ...);
}

template<class F, class... Args>
constexpr decltype(auto) invoke_intseq(F&& f, Args&&... args) {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

template<class F, class... Args> requires has_intseq<Args...> && std::same_as<std::invoke_result_t<F>, void>
// FIXME - same_as should, I believe, include Args as parameters, but trivially
// adding them will not solve the problem - we need to unwrap intseqs first
constexpr void invoke_intseq(F&& f, Args&&... args) {
    inner(std::forward<F>(f), std::forward<Args>(args)...);
}

template<class F, class... Args> requires has_intseq<Args...>
constexpr std::vector<std::invoke_result_t<F>> invoke_intseq(F&& f, Args&&... args) { // FIXME
    std::vector<std::invoke_result_t<F>> ans; // FIXME - should include Args (insert same comment as above)
    inner2(std::forward<decltype(ans)>(ans), std::forward<f>, std::forward<Args>(args)...);
    return ans;
}

#endif