#include <array>
#include <iostream>

namespace {
  constexpr int N = 6;

  constexpr int f(int x, int y) {
    return x * y;
  }

  template <typename T, T... t, typename... U>
  constexpr auto fs(U... u) {
    return std::array<T, sizeof... (t)>{{f(t, (0 + ... + u)) ...}};
  }

  template <int...>
  struct S {
    static constexpr auto gs() {
      return fs<int>();
    }
  };

  template <int... i>
  struct S<0, i...> {
    static constexpr auto gs() {
      return fs<int, 0, i...>(2);
    }
  };

  template <int i, int... j>
  struct S<i, j...> {
    static constexpr auto gs() {
      return S<i - 1, i, j...>::gs();
    }
  };

  constexpr auto X0 = fs<int>();
  constexpr auto X1 = fs<int, 3, 5>(2, 3);
  constexpr auto X2 = S<>::gs();
  constexpr auto X3 = S<N - 1>::gs();
  constexpr auto X4 = S<N, 17>::gs();
}

int main() {
  std::cout << "X0:";
  for (auto x : X0)
    std::cout << ' ' << x;
  std::cout << std::endl;

  std::cout << "X1:";
  for (auto x : X1)
    std::cout << ' ' << x;
  std::cout << std::endl;

  std::cout << "X2:";
  for (auto x : X2)
    std::cout << ' ' << x;
  std::cout << std::endl;

  std::cout << "X3:";
  for (auto x : X3)
    std::cout << ' ' << x;
  std::cout << std::endl;

  std::cout << "X4:";
  for (auto x : X4)
    std::cout << ' ' << x;
  std::cout << std::endl;
}
