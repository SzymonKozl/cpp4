#include <utility>
#include<string>

#include "invoke_intseq.h"

// note: for compilation from project dir, add argument "-I." to compile command


namespace {
    // only for same length packs
    template<class... Pack1>
    struct pack_comparator {
        public:
        template<class... Pack2>
        struct inner_cmp {
            public:
            using val = std::integral_constant<bool, (true && ... && std::is_same_v<Pack1, Pack2>)>;
        };
        template<class... Pack2>
        struct test_TRUE_ARGS {
            public:
            using val = typename inner_cmp<TRUE_ARGS(Pack2)>::val;
        };
    };

}


int main() {
    // is_intseq
    static_assert(is_intseq<std::integer_sequence<int, 1, 1>>);
    static_assert(is_intseq<std::integer_sequence<int>>);
    static_assert(is_intseq<std::integer_sequence<long, 1, 2, 3>>);
    static_assert(is_intseq<std::integer_sequence<std::size_t>>);
    static_assert(!is_intseq<int>);
    static_assert(!is_intseq<long>);
    static_assert(!is_intseq<void>);
    static_assert(!is_intseq<std::integral_constant<int, 1>>);
    static_assert(!is_intseq<std::true_type>);
    static_assert(!is_intseq<typename std::true_type::type>);


    // has_intseq
    using T1 = std::integer_sequence<int, 1, 2, 3>;
    using T2 = std::integer_sequence<long>;
    static_assert(has_intseq<T1>);
    static_assert(has_intseq<T1, int>);
    static_assert(has_intseq<int, T1>);
    static_assert(has_intseq<int, T1, bool>);
    static_assert(has_intseq<T2>);
    static_assert(has_intseq<T2, int>);
    static_assert(has_intseq<int, T2>);
    static_assert(has_intseq<int, T2, bool>);

    static_assert(!has_intseq<>);
    static_assert(!has_intseq<int>);
    static_assert(!has_intseq<long>);
    static_assert(!has_intseq<int, int>);
    static_assert(!has_intseq<int, int, int>);

    // pack_comparator (metatest)

    using cmp1 = pack_comparator<int, bool>;

    static_assert(
        !std::is_same_v<
            typename cmp1::inner_cmp<int, int>::val,
            typename std::true_type::type
        >
    );
    static_assert(
        !std::is_same_v<
            typename cmp1::inner_cmp<void, void>::val,
            typename std::true_type::type
        >
    );
    static_assert(
        std::is_same_v<
            typename cmp1::inner_cmp<int, bool>::val,
            typename std::true_type::type
        >
    );

    using cmp2 = pack_comparator<>;
    static_assert(
        std::is_same_v<
            typename cmp2::inner_cmp<>::val,
            typename std::true_type::type
        >
    );

    // extract_type

    static_assert(std::is_same_v<int, typename extract_type<int>::type>);
    static_assert(std::is_same_v<std::integral_constant<int, 0>, typename extract_type<std::integer_sequence<int, 1, 2, 3>>::type>);
    static_assert(std::is_same_v<std::integral_constant<int, 0>, typename extract_type<std::integer_sequence<int>>::type>);
    static_assert(std::is_same_v<std::integral_constant<long, 0>, typename extract_type<std::integer_sequence<long, 1L, 2L, 3L>>::type>);


    using cmp3 = pack_comparator<int, std::integral_constant<int, 0>, int>;
    using cmp4 = pack_comparator<std::integral_constant<int, 0>>;
    using cmp5 = pack_comparator<std::integral_constant<int, 0>, std::integral_constant<int, 0>>;
    using cmp6 = pack_comparator<std::integral_constant<int, 0>, std::integral_constant<int, 0>, std::integral_constant<long, 0>>;
    using iseq1 = std::integer_sequence<int, 1, 2, 3>;
    using iseq2 = std::integer_sequence<int, 4, 5, 6>;
    using iseq3 = std::integer_sequence<int>;
    using iseq4 = std::integer_sequence<long, 1L>;


    static_assert(
        std::is_same_v<
            typename cmp1::test_TRUE_ARGS<int, bool>::val,
            typename std::true_type::type
        >
    );

    static_assert(
        std::is_same_v<
            typename cmp3::test_TRUE_ARGS<int, iseq2, int>::val,
            typename std::true_type::type
        >
    );


    static_assert(
        std::is_same_v<
            typename cmp4::test_TRUE_ARGS<iseq3>::val,
            typename std::true_type::type
        >
    );

    static_assert(
        std::is_same_v<
            typename cmp5::test_TRUE_ARGS<iseq1, iseq2>::val,
            typename std::true_type::type
        >
    );

    static_assert(
        std::is_same_v<
            typename cmp6::test_TRUE_ARGS<iseq2, iseq3, iseq4>::val,
            typename std::true_type::type
        >
    );

    // extract size
    static_assert(extract_size<int>::val::value == 1);
    static_assert(extract_size<std::string>::val::value == 1);
    static_assert(extract_size<bool>::val::value == 1);
    static_assert(extract_size<float>::val::value == 1);
    static_assert(extract_size<std::integer_sequence<int, 1, 2, 3>>::val::value == 3);
    static_assert(extract_size<std::integer_sequence<bool, true>>::val::value == 1);
    static_assert(extract_size<std::integer_sequence<int>>::val::value == 0);

    // args_size_nozero
    static_assert(_args_size_nozero(1, iseq1(), iseq4()) == 3);
    static_assert(_args_size_nozero(iseq1(), iseq1(), iseq1()) == 27);
    static_assert(_args_size_nozero(1, 2, 3, "xd") == 1);
    static_assert(_args_size_nozero(iseq1(), iseq2(), iseq3(), 2) == 0);
    static_assert(_args_size_nozero(iseq1()) == 3);
    static_assert(_args_size_nozero(iseq2()) == 3);
    
    //args_size
    static_assert(args_size() == 1);
    static_assert(args_size(1, iseq1(), iseq4()) == 3);
    static_assert(args_size(iseq1(), iseq1(), iseq1()) == 27);
    static_assert(args_size(1, 2, 3, "xd") == 1);
    static_assert(args_size(iseq1(), iseq2(), iseq3(), 2) == 0);
    static_assert(args_size(iseq1()) == 3);
    static_assert(args_size(iseq2()) == 3);
}