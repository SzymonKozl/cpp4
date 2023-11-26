#ifndef __INVOKE_INTSEQ_H__
#define __INVOKE_INTSEQ_H__
#include <vector>


template<class T>
concept is_intseq = requires (T a) {
    ([] <class U> (std::integer_sequence<U> x){})(a);
};

template<class... Args>
concept has_intseq = (true || ... || is_intseq<Args>);

template<class F>
concept return_void = (std::same_as<std::invoke_result<F>, void>);

template<class F, class... Args>
constexpr std::invoke_result<F> invoke_intseq(F f, Args... args) {
    std::invoke(f, args...);
}


template<class F, class... Args> requires has_intseq<Args...> && std::same_as<std::invoke_result<F>, void>
constexpr void invoke_intseq(F f, Args... args) {

}


template<class F, class... Args> requires has_intseq<Args...>
constexpr std::vector<std::invoke_result<F>> invoke_intseq(F f, Args... args) {

}

#endif