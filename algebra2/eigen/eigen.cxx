module;
#include <cassert>
#include <cmath>
#include <ctime>

export module eigen;
import matrix;

template <typename T>
inline auto matrix_by_vector(matrix<T> m, matrix<T> v) -> matrix<T> {
    assert(m.number_of_columns() == v.number_of_rows() &&
           v.number_of_columns() == 1);
    matrix<T> multiply_result{m.number_of_rows(), 1, 0};
    for (std::size_t i = 0; i < multiply_result.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m.number_of_columns(); j++) {
            multiply_result[i, 0] += m[i, j] * v[j, 0];
        }
    }
    return multiply_result;
}

template <typename T>
inline auto lenght_of_vector(matrix<T> v) -> T {
    assert(v.number_of_columns() == 1);
    T lenght{0};
    for (std::size_t i = 0; i < v.number_of_rows(); i++) {
        lenght += v[i, 0] * v[i, 0];
    }
    return sqrt(lenght);
}

template <typename T>
inline auto normalize_vector(matrix<T> v) -> matrix<T> {
    assert(v.number_of_columns() == 1);
    const T lenght{lenght_of_vector(v)};
    for (std::size_t j = 0; j < v.number_of_rows(); j++) { v[j, 0] /= lenght; }
    return v;
}

template <typename T>
inline auto subtract_row(matrix<T> m, std::size_t r) -> matrix<T> {
    matrix<T> m_s{m.number_of_rows() - 1, m.number_of_columns(), 0};
    std::size_t k{0};
    for (std::size_t i = 0; i < m.number_of_rows(); i++) {
        if (i != r) {
            for (std::size_t j = 0; j < m.number_of_columns(); j++) {
                m_s[k, j] = m[i, j];
            }
            k++;
        }
    }
    return m_s;
}

template <typename T>
inline auto select_row(matrix<T> m, std::size_t r) -> matrix<T> {
    matrix<T> m_s{1, m.number_of_columns(), 0};
    for (std::size_t j = 0; j < m.number_of_columns(); j++) {
        m_s[0, j] = m[r, j];
    }
    return m_s;
}

template <typename T>
inline auto subtract_column(matrix<T> m, std::size_t c) -> matrix<T> {
    matrix<T> m_s{m.number_of_rows(), m.number_of_columns() - 1, 0};
    std::size_t k{0};
    for (std::size_t j = 0; j < m.number_of_columns(); j++) {
        if (j != c) {
            for (std::size_t i = 0; i < m.number_of_rows(); i++) {
                m_s[i, k] = m[i, j];
            }
            k++;
        }
    }
    return m_s;
}

template <typename T>
inline auto select_column(matrix<T> m, std::size_t c) -> matrix<T> {
    matrix<T> m_s{m.number_of_rows(), 1, 0};
    for (std::size_t i = 0; i < m.number_of_rows(); i++) {
        m_s[i, 0] = m[i, c];
    }
    return m_s;
}

template <typename T>
inline auto matrix_by_matrix(matrix<T> m1, matrix<T> m2) -> matrix<T> {
    assert(m1.number_of_columns() == m2.number_of_rows());
    matrix<T> m_m{m1.number_of_rows(), m2.number_of_columns(), 0};
    for (std::size_t i = 0; i < m_m.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m_m.number_of_columns(); j++) {
            m_m[i, j] =
                matrix_by_vector(select_row(m1, i), select_column(m2, j))[0, 0];
        }
    }
    return m_m;
}

template <typename T>
inline auto check_tolerance(const matrix<T>& m_diff) -> bool {
    assert(m_diff.number_of_columns() == 1);
    const T tolerance{1e-5};
    for (std::size_t i = 0; i < m_diff.number_of_rows(); i++) {
        if (abs(m_diff[i, 0]) > tolerance) { return false; }
    }
    return true;
}

template <typename T>
inline auto fuse_v_into_m(matrix<T> m,
                          matrix<T> v,
                          std::size_t c) -> matrix<T> {
    assert(m.number_of_rows() == v.number_of_rows());
    for (std::size_t i = 0; i < m.number_of_rows(); i++) { m[i, c] = v[i, 0]; }
    return m;
}

template <typename T>
inline auto projection_of_vs(matrix<T> v1, matrix<T> v2) -> matrix<T> {
    assert(v1.number_of_rows() == v2.number_of_rows() &&
           v1.number_of_columns() == 1 && v2.number_of_columns() == 1);
    return v2 * (matrix_by_vector(::utils::matrix::transpose(v2), v1)[0, 0] /
                 pow(lenght_of_vector(v2), 2));
}

template <typename T>
inline auto qr_decomposition(matrix<T>& m) -> matrix<T> {
    matrix<T> q{m.number_of_rows(), m.number_of_columns(), 0};
    matrix<T> u{m.number_of_rows(), 1, 0};
    u = select_column(m, 0);
    q = fuse_v_into_m(q, normalize_vector(u), 0);
    for (std::size_t j = 1; j < m.number_of_columns(); j++) {
        u = select_column(m, j) - projection_of_vs(select_column(m, j), u);
        q = fuse_v_into_m(q, normalize_vector(u), j);
    }
    return matrix_by_matrix(matrix_by_matrix(utils::matrix::transpose(q), m),
                            q);
}

template <typename T>
inline auto vector_from_diagonal(matrix<T>& m) -> matrix<T> {
    assert(m.number_of_rows() == m.number_of_columns());
    matrix<T> v{m.number_of_rows(), 1, 0};
    for (std::size_t i = 0; i < m.number_of_rows(); i++) { v[i, 0] = m[i, i]; }
    return v;
}
template <typename T>
inline auto determinant(matrix<T> m) -> T {
    assert(m.number_of_rows() == m.number_of_columns());
    if (m.number_of_rows() == 2) {
        return m[0, 0] * m[1, 1] - m[0, 1] * m[1, 0];
    }
    T det{0};
    for (std::size_t i = 0; i < m.number_of_columns(); i++) {
        det += pow(-1, i) * m[0, i] *
               determinant(subtract_column(subtract_row(m, 0), i));
    }
    return det;
}

template <typename T>
inline auto cofactor(matrix<T> m) -> matrix<T> {
    matrix<T> cofactor{m.number_of_rows(), m.number_of_columns(), 0};
    for (std::size_t i = 0; i < m.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m.number_of_columns(); j++) {
            cofactor[i, j] = pow(-1, i + j) * determinant(subtract_column(
                                                  subtract_row(m, i), j));
        }
    }
    return cofactor;
}

template <typename T>
inline auto inverse_matrix(const matrix<T>& m) {
    assert(determinant(m) != 0);
    return utils::matrix::transpose(cofactor(m)) / determinant(m);
}

template <typename T>
inline auto calculate_mu(matrix<T>& m, matrix<T>& b_k) -> T {
    return matrix_by_vector(::utils::matrix::transpose(b_k),
                            matrix_by_vector(m, b_k))[0, 0] /
           matrix_by_vector(::utils::matrix::transpose(b_k), b_k)[0, 0];
}

export namespace eigen {

    template <typename T>
    inline auto power_method(matrix<T>& m) {
        matrix<T> b_k{m.number_of_rows(), 1, 1};
        while (!check_tolerance(normalize_vector(matrix_by_vector(m, b_k)) -
                                b_k)) {
            b_k = normalize_vector(matrix_by_vector(m, b_k));
        }
        return b_k;
    }

    template <typename T>
    inline auto inverse_power_method(matrix<T>& m) {
        matrix<T> b_k{m.number_of_rows(), 1, 1};
        m = inverse_matrix(m - utils::matrix::identity<T>(m.number_of_rows()) *
                                   calculate_mu(m, b_k));
        return power_method(m);
    }

    template <typename T>
    inline auto rayleigh(matrix<T>& m) {
        matrix<T> b_k{m.number_of_rows(), 1, 1};
        while (!check_tolerance(normalize_vector(matrix_by_vector(m, b_k)) -
                                b_k)) {
            b_k = normalize_vector(matrix_by_vector(
                inverse_matrix(m -
                               utils::matrix::identity<T>(m.number_of_rows()) *
                                   calculate_mu(m, b_k)),
                b_k));
        }
        return b_k;
    }

    template <typename T>
    inline auto qr(matrix<T>& m) {
        matrix<T> q{m.number_of_rows(), m.number_of_columns(), 0};
        matrix<T> r{m.number_of_rows(), m.number_of_columns(), 0};
        for (std::size_t i = 0; i < 1; i++) { m = qr_decomposition(m); }
        return vector_from_diagonal(m);
    }
}  // namespace eigen