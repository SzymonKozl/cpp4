#ifndef __INVOKE_INTSEQ_H__
#define __INVOKE_INTSEQ_H__
#include <vector>
#include <functional>
#include <array>

template<class T>
concept is_intseq = requires (T a) {
    ([]<class U, U...args>(std::integer_sequence<U, args...>){})(a);
};

// na razie niepotrzebne, może kiedyś się przyda (ale wątpię tbh - wydaje mi się że is_intseq robi już decay implicite)
template<class T>
concept decays_to_intseq = is_intseq<std::decay_t<T>>;

template<class... Ts>
concept has_intseq = (false || ... || is_intseq<Ts>);

template<class... Ts>
concept has_not_intseq = !has_intseq<Ts...>;

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


template<class T, class... Args>
struct args_size_nz {
    using val = std::integral_constant<size_t, extract_size<std::decay_t<T>>::val::value * args_size_nz<Args...>::val::value>;
};

template <class T>
struct args_size_nz<T> {
    using val = typename extract_size<std::decay_t<T>>::val;
};

template <class... Args>
struct args_size {
    using val = typename args_size_nz<Args...>::val;
};

template<>
struct args_size<> {
    using val = std::integral_constant<size_t, 1>;
};

#define TRUE_ARGS(A) typename extract_type<A>::type...

template<class F>
concept return_void = (std::same_as<std::invoke_result_t<F>, void>);

template<class F, class T, class... Args>
constexpr void inner (F&& f, T&& arg1, Args&&... args){
    if constexpr (sizeof...(args) == 0){
        std::invoke(std::forward<F>(f), std::forward<T>(arg1));
    }
    else {
        decltype(auto) newf = [&f, &arg1]<class... U>(U&&... u) {
            std::invoke(std::forward<F>(f), std::forward<T>(arg1), std::forward<U>(u)...);
        };
        inner(std::forward<decltype(newf)>(newf), std::forward<Args>(args)...);
    }
}
template<class F, class T, class... Args, T... ints>
constexpr void inner(F&& f, std::integer_sequence<T, ints...>&&, Args&&... args) {
    (inner(std::forward<F>(f), std::forward<std::integral_constant<T, std::forward<T>(ints)>>(
        std::integral_constant<T, std::forward<T>(ints)>()), std::forward<Args>(args)...), ...);
}

// TODO - inner 2 nie jest do końca dopracowany, bo ma większy problem - w ogóle nie działa
template<class F, class T, class... Args, class U, size_t x>
constexpr void inner2(size_t index, std::array<U, x> &res, F&& f, T&& arg1, Args&&... args) {
    if constexpr (sizeof...(Args) == 0){
        res.at(index) = std::invoke(f, arg1);
    }
    else {
        inner2(index, res, std::bind_front(f, std::forward<T>(arg1)), std::forward<Args>(args)...);
    }
}

template<class F, class... Args, class T, class U, T... Ints, U... Ints2, class Y, size_t x>
constexpr void helper_tmp(size_t index, std::array<Y, x> &res, F&& f, std::integer_sequence<T, Ints...>&&, std::integer_sequence<U, Ints2...>, Args&&... args) {
    constexpr size_t step = args_size<Args...>::val::value;
    (inner2(index + Ints2 * step, res, std::forward<F>(f), std::integral_constant<T, Ints>(), std::forward<Args>(args)...), ...);
}

template<class F, class T, class... Args, T... ints, class U, size_t x>
constexpr void inner2(size_t index, std::array<U, x> &res, F&& f, std::integer_sequence<T, ints...>&& iseq, Args&&... args) {
    constexpr size_t range = std::integer_sequence<T, ints...>::size();
    auto iseq2 = std::make_index_sequence<range>();
    helper_tmp(index, res, std::forward<F>(f), std::forward<decltype(iseq)>(iseq), iseq2, std::forward<Args>(args)...);
}

template<class F, class... Args> requires has_not_intseq<Args...>
constexpr decltype(auto) invoke_intseq(F&& f, Args&&... args) {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

template<class F, class... Args> requires has_intseq<Args...> && std::same_as<std::invoke_result_t<F, TRUE_ARGS(Args)>, void>
constexpr void invoke_intseq(F&& f, Args&&... args) {
    inner(std::forward<F>(f), std::forward<Args>(args)...);
}

template<class F, class... Args> requires has_intseq<Args...>
constexpr decltype(auto) invoke_intseq(F&& f, Args&&... args) {
    constexpr size_t res_size = args_size<Args...>::val::value;
    std::array<std::invoke_result_t<F, TRUE_ARGS(Args)>, res_size> ans;
    size_t index = 0;
    inner2(index, ans, std::forward<F>(f), std::forward<Args>(args)...);
    return ans;
}


template <std::size_t... Is, typename T>
constexpr auto initializeArray(const T& ref, std::index_sequence<Is...>) {
    return std::array<T, sizeof...(Is)>{((void)Is, std::ref(ref))...};
}


template<class F, class... Args> requires has_intseq<Args...> && std::is_reference<std::invoke_result_t<F, TRUE_ARGS(Args)>>::value
constexpr decltype(auto) invoke_intseq(F&& f, Args&&... args) {
    constexpr size_t res_size = args_size<Args...>::val::value;
    using res_t = std::decay_t<std::invoke_result_t<F, TRUE_ARGS(Args)>>;
    res_t dummy{};
    std::reference_wrapper<res_t>dummy_ref(dummy);
    auto res = initializeArray(dummy_ref, std::make_index_sequence<res_size>{});
    inner2(0, res, std::forward<F>(f), std::forward<Args>(args)...);
    return res;
}
#endif