#include <type_traits>
#include <functional>
#include <cstdio>

// #define PERFECT_CAPTURE_BREAKS_GCC

#ifdef PERFECT_CAPTURE_BREAKS_GCC
#   define FORWARD(VARIABLE) std::forward<decltype(VARIABLE)>(VARIABLE)
#   define CAPTURE_FORWARD(VARIABLE) detail::perfect_capture_t<decltype(VARIABLE)>{ FORWARD(VARIABLE) }
#else
#   define FORWARD(VARIABLE) VARIABLE
#   define CAPTURE_FORWARD(VARIABLE) VARIABLE
#endif

namespace detail {
    /// perfect_capture_t is taken from http://stackoverflow.com/a/31410880/2622629
    template <class T> using
    perfect_capture_t =
        std::conditional_t<std::is_lvalue_reference<T>::value,
                           std::reference_wrapper<std::remove_reference_t<T>>, T>;

    template< class, class = std::void_t<> >
    struct needs_unapply : std::true_type { };

    template< class T >
    struct needs_unapply<T, std::void_t<decltype(std::declval<T>()())>> : std::false_type { };
}

template <typename F>
decltype(auto) curry(F&& f) {
    if constexpr (detail::needs_unapply<decltype(f)>::value) {
        return [f=CAPTURE_FORWARD(f)](auto&& x) {
            return curry(
              [x=CAPTURE_FORWARD(x), f=CAPTURE_FORWARD(f)](auto&&...xs) -> decltype(f(FORWARD(x),FORWARD(xs)...)) {
                return f(FORWARD(x),FORWARD(xs)...);
              }
            );
        };
    }
    else return f();
}

struct call_t {} call;

template <typename F> decltype(auto)
curryX(F&& f) {
    return [f=CAPTURE_FORWARD(f)](auto&& x) {
      	if constexpr (std::is_same<std::decay_t<decltype(x)>,call_t>::value) {
          return f();
        }
        else {
          return curryX(
              [x=CAPTURE_FORWARD(x), f=CAPTURE_FORWARD(f)](auto&&...xs) -> decltype(f(FORWARD(x),FORWARD(xs)...)) {
                return f(FORWARD(x),FORWARD(xs)...);
              });
        };
    };
}

int main()
{
    auto f = [](auto...xs) -> decltype(auto) {
      	return sizeof...(xs);
    };

    curryX(printf)("Hello %s")("World")(call);
	return curryX(f)("Call")("Me")("With")("any")("number")("of")("args")(call);
    // return curryX(f)(args...)
}



