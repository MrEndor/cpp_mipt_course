#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>

namespace entrails {
template <typename T = int64_t>
using VecMatrix = std::vector<std::vector<T>>;

template <std::size_t N, std::size_t M>
constexpr bool IsSquareMatrix() {
  return N == M;
}
}  // namespace entrails

template <std::size_t N, std::size_t M, typename T = int64_t>
class Matrix {
 public:
  Matrix() : Matrix(T()) {};

  Matrix(entrails::VecMatrix<T> values) : matrix_(values) {};

  Matrix(const T& elem)
      : matrix_(entrails::VecMatrix<T>(N, std::vector<T>(M, elem))) {};

  const T& operator()(std::size_t row, std::size_t column) const {
    return matrix_[row][column];
  };

  T& operator()(std::size_t row, std::size_t column) {
    return matrix_[row][column];
  }

  const T& operator[](std::size_t row) const { return matrix_[row]; };

  T& operator[](std::size_t row) { return matrix_[row]; }

  Matrix<N, M, T>& operator-=(const Matrix<N, M, T>& other);

  Matrix<N, M, T>& operator+=(const Matrix<N, M, T>& other);

  Matrix<N, M, T>& operator*=(const T& value);

  Matrix<M, N, T> Transposed();
  T Trace();

 private:
  entrails::VecMatrix<T> matrix_ = entrails::VecMatrix<T>(N, std::vector<T>(M));
};

template <std::size_t N, std::size_t M, typename T>
Matrix<M, N, T> Matrix<N, M, T>::Transposed() {
  Matrix<M, N, T> new_matrix;

  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = 0; j < M; ++j) {
      new_matrix(j, i) = matrix_[i][j];
    }
  }
  return new_matrix;
}

template <std::size_t N, std::size_t M, typename T>
T Matrix<N, M, T>::Trace() {
  static_assert(entrails::IsSquareMatrix<N, M>());

  T trace = T();
  for (std::size_t i = 0; i < N; ++i) {
    trace += matrix_[i][i];
  }
  return trace;
}

template <std::size_t N, std::size_t M, typename T = int64_t>
bool operator==(const Matrix<N, M, T>& lhs, const Matrix<N, M, T>& rhs) {
  for (std::size_t i = 0; i < N; ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

template <std::size_t N, std::size_t M, std::size_t F, typename T = int64_t>
Matrix<N, F, T> operator*(const Matrix<N, M, T>& lhs,
                          const Matrix<M, F, T>& rhs) {
  Matrix<N, F, T> new_matrix;
  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = 0; j < F; ++j) {
      for (std::size_t column = 0; column < M; ++column) {
        new_matrix(i, j) += (lhs(i, column) * rhs(column, j));
      }
    }
  }
  return new_matrix;
}

template <std::size_t N, std::size_t M, typename T = int64_t>
Matrix<N, M, T> operator*(const Matrix<N, M, T>& matrix, const T& value) {
  Matrix<N, M, T> new_matrix = matrix;
  new_matrix *= value;
  return new_matrix;
}

template <std::size_t N, std::size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator*=(const T& value) {
  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = 0; j < M; ++j) {
      matrix_[i][j] *= value;
    }
  }
  return *this;
}

template <std::size_t N, std::size_t M, typename T = int64_t>
Matrix<N, M, T> operator+(const Matrix<N, M, T>& lhs,
                          const Matrix<N, M, T>& rhs) {
  Matrix<N, M, T> new_matrix = lhs;
  new_matrix += rhs;
  return new_matrix;
}

template <std::size_t N, std::size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator+=(const Matrix<N, M, T>& other) {
  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = 0; j < M; ++j) {
      matrix_[i][j] += other(i, j);
    }
  }
  return *this;
}

template <std::size_t N, std::size_t M, typename T = int64_t>
Matrix<N, M, T> operator-(const Matrix<N, M, T>& lhs,
                          const Matrix<N, M, T>& rhs) {
  Matrix<N, M, T> new_matrix = lhs;
  new_matrix -= rhs;
  return new_matrix;
}

template <std::size_t N, std::size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator-=(const Matrix<N, M, T>& other) {
  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = 0; j < M; ++j) {
      matrix_[i][j] -= other(i, j);
    }
  }
  return *this;
}
