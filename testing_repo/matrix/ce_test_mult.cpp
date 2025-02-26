#include "matrix.hpp"
#include <array>

int main() {
	Matrix<1, 2> matrix_a(1);
	Matrix<3, 4> matrix_b(1);

	auto result = matrix_a * matrix_b;
}
