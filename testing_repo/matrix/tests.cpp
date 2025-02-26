#include "matrix.hpp"

#include <gtest/gtest.h>

#include <complex>
#include <random>

template<typename T = int64_t>
using VecMatrix = std::vector<std::vector<T>>;

using Complex = std::complex<double>;

template<size_t N, size_t M, typename T = int64_t>
void AreEqual(const Matrix<N, M, T>& matrix, const VecMatrix<T>& expected) {
  EXPECT_EQ(N, expected.size());
  if (N != 0) {
    EXPECT_EQ(M, expected[0].size());
  }
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      EXPECT_EQ(matrix(i, j), expected[i][j]);
    }
  }
}

template<typename T>
T GenerateRandomElem(std::mt19937& gen) {
  std::uniform_int_distribution<T> distribution(std::numeric_limits<T>::min(),
                                                std::numeric_limits<T>::max());
  return distribution(gen);
}

template<>
Complex GenerateRandomElem(std::mt19937& gen) {;
  std::normal_distribution distribution;
  return {distribution(gen), distribution(gen)};
}

template<>
double GenerateRandomElem(std::mt19937& gen) {;
  std::normal_distribution distribution;
  return distribution(gen);
}


template<typename T>
VecMatrix<T> GenerateRandomMatrix(size_t rows_num, size_t columns_num) {
  std::mt19937 gen;
  VecMatrix<T> result(rows_num, std::vector<T>(columns_num));
  for (size_t i = 0; i < rows_num; ++i) {
    for (size_t j = 0; j < columns_num; ++j) {
      result[i][j] = GenerateRandomElem<T>(gen);
    }
  }
  return result;
}

template<typename T>
VecMatrix<T> GenerateRandomSymmetricMatrix(size_t size) {
  std::mt19937 gen;
  VecMatrix<T> result(size, std::vector<T>(size));
  for (size_t i = 0; i < size; ++i) {
    for (size_t j = i; j < size; ++j) {
      result[i][j] = result[j][i] = GenerateRandomElem<T>(gen);
    }
  }
  return result;
}

TEST(Constructors, Default) {
  Matrix<3, 2> matrix;
  VecMatrix<> expected(3, std::vector<int64_t>(2));
  AreEqual(matrix, expected);
}

TEST(Constructors, FromElement) {
  Complex elem(3.14, 2.71);
  VecMatrix<Complex> expected(3, std::vector<Complex>(2, elem));
  Matrix<3, 2, Complex> matrix(elem);
  AreEqual(matrix, expected);
}

TEST(Constructors, FromVector) {
  auto vector = GenerateRandomMatrix<Complex>(3, 2);
  Matrix<3, 2, Complex> matrix(vector);
  AreEqual(matrix, vector);
}

TEST(SelfOperators, PlusDiff) {
  auto vector = GenerateRandomMatrix<int64_t>(3, 2);
  VecMatrix<int64_t> zero(3, std::vector<int64_t>(2, 0));
  Matrix<3, 2> matrix_a(vector);
  Matrix<3, 2> matrix_b(zero);
  matrix_a += matrix_b += matrix_b;
  AreEqual(matrix_a, vector);
  AreEqual(matrix_b, zero);
}

TEST(SelfOperators, PlusSelf) {
  VecMatrix<Complex> zero(7, std::vector<Complex>(3));
  Matrix<7, 3, Complex> matrix;
  matrix += matrix;
  AreEqual(matrix, zero);
}

TEST(SelfOperators, MinusDiff) {
  auto vector = GenerateRandomMatrix<int64_t>(3, 2);
  VecMatrix<int64_t> zero(3, std::vector<int64_t>(2, 0));
  Matrix<3, 2> matrix_a(vector);
  Matrix<3, 2> matrix_b(zero);
  matrix_a -= matrix_b -= matrix_b;
  AreEqual(matrix_a, vector);
  AreEqual(matrix_b, zero);
}

TEST(SelfOperators, MinusSelf) {
  VecMatrix<Complex> zero(7, std::vector<Complex>(3));
  Matrix<7, 3, Complex> matrix;
  matrix -= matrix;
  AreEqual(matrix, zero);
}

TEST(Operators, Plus) {
  VecMatrix<Complex> zero(7, std::vector<Complex>(3));
  VecMatrix<Complex> vector = GenerateRandomMatrix<Complex>(7, 3);
  Matrix<7, 3, Complex> matrix(vector);
  AreEqual(matrix + matrix * Complex(-1., 0.), zero);
}

TEST(Operators, Minus) {
  VecMatrix<Complex> zero(7, std::vector<Complex>(3));
  VecMatrix<Complex> vector = GenerateRandomMatrix<Complex>(7, 3);
  Matrix<7, 3, Complex> matrix(vector);
  AreEqual(matrix - matrix, zero);
}

TEST(Multiplication, Zero) {
  auto vector = GenerateRandomMatrix<Complex>(7, 3);
  Matrix<7, 3, Complex> matrix(vector);

  VecMatrix<Complex> zero(3, std::vector<Complex>(5));
  Matrix<3, 5, Complex> zero_matrix;

   AreEqual(matrix * zero_matrix, VecMatrix<Complex>(7, std::vector<Complex>(5)));
}

TEST(Multiplication, ToElem) {
  const size_t kSize = 5;
  auto vector = GenerateRandomMatrix<Complex>(kSize, kSize);
  Matrix<kSize, kSize, Complex> matrix(vector);

  VecMatrix<Complex> ones(kSize, std::vector<Complex>(kSize));
  for (size_t i = 0; i < kSize; ++i) {
    ones[i][i] = {1., 0.};
  }
  Matrix<kSize, kSize, Complex> ones_matrix(ones);

  AreEqual(matrix * ones_matrix, vector);
  AreEqual(ones_matrix * matrix, vector);
}

TEST(Transpose, Symmetric) {
  const size_t kSize = 10;
  auto vector = GenerateRandomSymmetricMatrix<Complex>(kSize);
  Matrix<kSize, kSize, Complex> matrix(vector);

  AreEqual(matrix.Transposed(), vector);
}

TEST(Transpose, RectangleZero) {
  VecMatrix<> vector(10, std::vector<int64_t >(3));
  Matrix<10, 3> matrix(vector);
  VecMatrix<> expected(3, std::vector<int64_t >(10));

  AreEqual(matrix.Transposed(), expected);
}

TEST(Transpose, RectangleNotZero) {
  VecMatrix<> vector = {{1, 2, 3}, {4, 5, 6}};
  Matrix<2, 3> matrix(vector);
  VecMatrix<> expected = {{1, 4}, {2 ,5}, {3, 6}};

  AreEqual(matrix.Transposed(), expected);
}

TEST(Trace, Default) {
  std::mt19937 gen;
  auto elem = GenerateRandomElem<double>(gen);
  const size_t kSize = 100;
  VecMatrix<double> vector(kSize, std::vector<double>(kSize, elem));
  Matrix<kSize, kSize, double> matrix(vector);

  EXPECT_LE(std::abs(matrix.Trace() - kSize * elem), 1e-6);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
