#include <iostream>

namespace {
  template <int x>
  struct X {
    static constexpr int value = x;
  };

  template <typename X1, typename X2>
  struct add {
    using result = X<X1::value + X2::value>;
  };
}

int main() {
  using X1 = X<1>;
  using X2 = X<2>;
  using X3 = add<X1, X2>::result;

  std::cout << X3::value << std::endl;
}
