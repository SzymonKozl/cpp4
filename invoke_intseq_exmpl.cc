#include "invoke_intseq.h"
#include <cstddef>
#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <tuple>
#include <utility>

namespace
{
    template <class... Args>
    void Print(const Args&... args)
    {
        ((std::cout << args << ' '), ...);
        std::cout << std::endl;
    }

    template <class T, T... t>
    struct Printer
    {
        static void Print()
        {
            ((std::cout << t << ' '), ...);
            std::cout << std::endl;
        }
    };

    struct Foo
    {
        static size_t instances_count;
        int id;

        void Init()
        {
            id = instances_count++;
        }

        Foo()
        {
            Init();
            std::cout << "Foo(): id = " << id << std::endl;
        }

        Foo(const Foo& o)
        {
            Init();
            std::cout << "Foo(const Foo&) id = " << o.id << " -> id = " << id << std::endl;
        }

        Foo(Foo&& o)
        {
            id = o.id;
            std::cout << "Foo(Foo&&) id = " << id << std::endl;
        }

        void Print()
        {
            std::cout << "Foo::Print() id = " << id << std::endl;
        }
    };

    size_t Foo::instances_count = 0;

    struct Caller
    {
        Foo foo;

        template <class... Args>
        void operator()(const Args&... args) const
        {
            Print(args...);
        }
    };

    std::ostream& operator<<(std::ostream& s, const Foo& a)
    {
        s << "[Foo: " << a.id << "]";
        return s;
    }
} // koniec anonimowej przestrzeni nazw

int main()
{
    Foo foo;
    constexpr auto make_number = [](auto x, auto y, auto z) {return 100 * x + 10 * y + z;};
    size_t s;

    // Sprawdzenie typów wyników wywołań
    static_assert(std::is_same_v<decltype(invoke_intseq([](auto...) {}, 1, 2, 3)), void>);
    static_assert(std::is_same_v<decltype(invoke_intseq([](auto...) {return 0;}, 1, 2, 3)), int>);
    static_assert(std::is_same_v<decltype(invoke_intseq([](auto...) {}, std::integer_sequence<int, 1, 2, 3>(), 4, 5)), void>);
    static_assert(std::ranges::range<decltype(invoke_intseq([](auto...) {return 0;}, std::integer_sequence<int, 1, 2, 3>(), 4, 5))>);
    static_assert(std::is_same_v<std::ranges::range_value_t<decltype(invoke_intseq([](auto...) {return 0;}, std::integer_sequence<int, 1, 2, 3>(), 4, 5))>, int>);

    std::cout << "empty" << std::endl;
    invoke_intseq([]() {std::cout << "nothing" << std::endl;});

    std::cout << "single with lambda" << std::endl;
    invoke_intseq([](auto... a) {Print(a...);}, 1, 2, 3);

    std::cout << "single with function pointer" << std::endl;
    invoke_intseq(Print<int, int, int>, 4, 5, 6);
    invoke_intseq(&Foo::Print, foo);

    std::cout << "size_t" << std::endl;
    invoke_intseq([](auto... a) {Print(a...);}, std::integer_sequence<int, 1, 2, 3>(), size_t(10), std::integer_sequence<int, 7, 8>());

    std::cout << "std::integral_constant, size_t" << std::endl;
    invoke_intseq([](auto... a) {Print(a...);}, std::integer_sequence<int, 1, 2, 3>(), std::integral_constant<size_t, 11>(), std::integer_sequence<int, 7, 8>());

    std::cout << "string" << std::endl;
    invoke_intseq([](auto... a) {Print(a...);}, "ala", size_t(10), std::integer_sequence<int, 7, 8>());

    std::cout << "template Print single" << std::endl;
    auto template_print = [=](auto... a) {Printer<int, make_number(a...)>::Print();};
    invoke_intseq(template_print, std::integer_sequence<int, 1, 2>(), std::integral_constant<int, 3>(), std::integer_sequence<int, 7, 8>());

    std::cout << "template Print all" << std::endl;
    invoke_intseq([](auto... a) {Printer<int, a...>::Print();}, std::integer_sequence<int, 1, 2>(), std::integral_constant<int, 3>(), std::integer_sequence<int, 7, 8>());

    std::cout << "single result" << std::endl;
    std::cout << invoke_intseq(make_number, 9, 8, 7) << std::endl;
    static_assert(invoke_intseq(make_number, 9, 8, 7) == 987);

    std::cout << "integer sequence single result" << std::endl;
    for (auto i : invoke_intseq(make_number, std::integer_sequence<int, 6>(), 5, 4))
        std::cout << i << std::endl;
    static_assert(std::ranges::equal(invoke_intseq(make_number, std::integer_sequence<int, 6>(), 5, 4), std::array{654}));

    std::cout << "integer sequence empty result" << std::endl;
    for (auto i : invoke_intseq(make_number, std::integer_sequence<int>(), 5, 4))
        std::cout << i << std::endl;
    static_assert(std::ranges::equal(invoke_intseq(make_number, std::integer_sequence<int>(), 5, 4), std::array<int, 0>{}));

    std::cout << "integer sequence multiple result" << std::endl;
    for (auto i : invoke_intseq(make_number, 1, std::integer_sequence<int, 2, 3>(), std::integer_sequence<int, 4, 5>()))
        std::cout << i << std::endl;
    static_assert(std::ranges::equal(invoke_intseq(make_number, 1, std::integer_sequence<int, 2, 3>(), std::integer_sequence<int, 4, 5>()), std::array{124, 125, 134, 135}));

    std::cout << "tuple result" << std::endl;
    for (const auto& t : invoke_intseq([](auto a, auto b, int c, auto d) {return std::make_tuple(a, b, c, d);}, "ala", "ma", std::integer_sequence<int, 5, 7, 9>(), "kotów"))
        std::apply([](auto... a) {Print(a...);}, t);

    std::cout << "Foo: const auto&" << std::endl;
    invoke_intseq([](const auto&... a) {Print(a...);}, Foo(), foo);

    std::cout << "Foo: auto" << std::endl;
    invoke_intseq([](auto... a) {Print(a...);}, Foo(), foo);

    std::cout << "Foo: auto&" << std::endl;
    // Nie powinno się kompilować: rvalue nie może być przekazywane jako auto&.
    // invoke_intseq([](auto&... a) {Print(a...);}, Foo(), foo, std::make_index_sequence<3>());
    invoke_intseq([](auto&... a) {Print(a...);}, foo, foo);

    std::cout << "Foo: const auto& integer_sequence" << std::endl;
    invoke_intseq([](const auto&... a) {Print(a...);}, std::make_index_sequence<2>(), Foo(), foo);

    std::cout << "Foo: auto integer_sequence" << std::endl;
    invoke_intseq([](auto... a) {Print(a...);}, std::make_index_sequence<2>(), Foo(), foo);

    std::cout << "reference passing with integer_sequence" << std::endl;
    s = 0;
    auto ref_seq = invoke_intseq([](size_t a, size_t& r) -> size_t& {r += a; return r;}, std::make_index_sequence<5>(), s);
    std::cout << s << std::endl;
    for (size_t& r : ref_seq)
        r++;
    std::cout << s << std::endl;

    std::cout << "reference result single" << std::endl;
    invoke_intseq([](size_t a, size_t& r) -> size_t& {r += a; return r;}, 200, s) += 3000;
    std::cout << s << std::endl;

    std::cout << "Caller" << std::endl;
    Caller caller;
    invoke_intseq(caller, std::integer_sequence<int, 3, 2, 1>(), "caller");
}
