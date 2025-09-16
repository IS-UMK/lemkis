module;
#include <cassert>
#include <cmath>
#include <ctime>
#include <iostream>
#include <set>
#include <tuple>
#include <type_traits>

export module eigen;
import matrix;

template <typename T>
inline auto convert_to_double(matrix<T> m) -> matrix<double> {
    matrix<double> converted_m{m.number_of_rows(), m.number_of_columns(), 0};
    for (std::size_t i = 0; i < m.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m.number_of_columns(); j++) {
            converted_m[i, j] = static_cast<double>(m[i, j]);
        }
    }
    return converted_m;
}

template <typename T>
inline auto m_by_v(matrix<T> m, matrix<T> v) -> matrix<T> {
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
inline auto normalize_v(matrix<T> v) -> matrix<T> {
    assert(v.number_of_columns() == 1);
    const T lenght{lenght_of_vector(v)};
    for (std::size_t j = 0; j < v.number_of_rows(); j++) { v[j, 0] /= lenght; }
    return v;
}

template <typename T>
inline auto subtract_row(matrix<T> m, std::size_t r) -> matrix<T> {
    matrix<T> m_s{m.number_of_rows() - 1, m.number_of_columns(), 0};
    std::size_t k{0};
    for (std::size_t j = 0; j < m.number_of_columns(); j++) {
        for (std::size_t i = 0; i < m.number_of_rows(); i++) {
            (i != r) ? m_s[k, j] = m[i, j], k++ : m[i, j];
        }
        k = 0;
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
    for (std::size_t i = 0; i < m.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m.number_of_columns(); j++) {
            (j != c) ? m_s[i, k] = m[i, j], k++ : m[i, j];
        }
        k = 0;
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
inline auto m_by_m(matrix<T> m1, matrix<T> m2) -> matrix<T> {
    assert(m1.number_of_columns() == m2.number_of_rows());
    matrix<T> m_m{m1.number_of_rows(), m2.number_of_columns(), 0};
    for (std::size_t i = 0; i < m_m.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m_m.number_of_columns(); j++) {
            m_m[i, j] = m_by_v(select_row(m1, i), select_column(m2, j))[0, 0];
        }
    }
    return m_m;
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
inline auto scalar_of_m(matrix<T> m1, matrix<T> m2) -> T {
    assert(m1.number_of_rows() == m2.number_of_rows() &&
           m1.number_of_columns() == m2.number_of_columns());
    T result{0};
    for (std::size_t i = 0; i < m1.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m1.number_of_columns(); j++) {
            result += m1[i, j] * m2[i, j];
        }
    }
    return result;
}

template <typename T>
inline auto projection_of_vs(matrix<T> v1, matrix<T> v2) -> matrix<T> {
    assert(v1.number_of_rows() == v2.number_of_rows() &&
           v1.number_of_columns() == 1 && v2.number_of_columns() == 1);
    return v2 * (scalar_of_m(v1, v2) / pow(lenght_of_vector(v2), 2));
}

template <typename T>
inline auto qr_decomposition(matrix<T> m) -> std::pair<matrix<T>, matrix<T>> {
    matrix<T> q{m.number_of_rows(), m.number_of_columns(), 0};
    matrix<T> u_k{select_column(m, 0)};
    q = fuse_v_into_m(q, normalize_v(u_k), 0);
    for (std::size_t k = 1; k < m.number_of_columns(); k++) {
        u_k = select_column(m, k);
        for (std::size_t j = 0; j < k; j++) {
            u_k -= projection_of_vs(select_column(m, k), select_column(q, j));
        }
        q = fuse_v_into_m(q, normalize_v(u_k), k);
    }
    return {m_by_m(utils::matrix::transpose(q), m), q};
}

template <typename T>
inline auto vector_from_diagonal(matrix<T> m) -> matrix<T> {
    assert(m.number_of_rows() == m.number_of_columns());
    matrix<T> v{m.number_of_rows(), 1, 0};
    for (std::size_t i = 0; i < m.number_of_rows(); i++) { v[i, 0] = m[i, i]; }
    return v;
}

template <typename T>
inline auto determinant(matrix<T> m) -> T {
    assert(m.number_of_rows() == m.number_of_columns());
    if (m.number_of_rows() == 1) { return m[0, 0]; }
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
inline auto inverse_matrix(const matrix<T> m) {
    assert(determinant(m) != 0);
    return utils::matrix::transpose(cofactor(m)) / determinant(m);
}

template <typename T>
inline auto calculate_mu(matrix<T> m, matrix<T> b_k) -> T {
    return m_by_v(::utils::matrix::transpose(b_k), m_by_v(m, b_k))[0, 0] /
           m_by_v(::utils::matrix::transpose(b_k), b_k)[0, 0];
}

template <typename T>
inline auto largest_non_diagonal_value(matrix<T> m)
    -> std::tuple<std::size_t, std::size_t, T> {
    assert(m.number_of_rows() == m.number_of_columns());
    std::tuple result{std::size_t{0}, std::size_t{0}, T{0}};
    for (std::size_t i = 0; i < m.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m.number_of_columns(); j++) {
            (i != j && m[i, j] > std::get<2>(result)) ? result = {i, j, m[i, j]}
                                                      : result;
        }
    }
    return result;
}

template <typename T>
inline auto theta(T m_ii, T m_jj, T m_ij) -> std::pair<T, T> {
    T t{0};
    T tau{(m_jj - m_ii) / (2 * m_ij)};
    (tau >= 0) ? t = 1.0 / (std::abs(tau) + std::sqrt(1.0 + tau * tau))
               : t = -1.0 / (std::abs(tau) + std::sqrt(1.0 + tau * tau));
    return {1.0 / std::sqrt(1.0 + t * t), t / std::sqrt(1.0 + t * t)};
}

template <typename T>
inline auto rotation_matrix(matrix<T> m) -> matrix<T> {
    matrix<T> rotation_m{utils::matrix::identity<T>(m.number_of_rows())};
    auto [r, c, val] = largest_non_diagonal_value(m);
    rotation_m[r, r] = theta(m[r, r], m[c, c], val).first;
    rotation_m[r, c] = theta(m[r, r], m[c, c], val).second;
    rotation_m[c, r] = -theta(m[r, r], m[c, c], val).second;
    rotation_m[c, c] = theta(m[r, r], m[c, c], val).first;
    return m_by_m(utils::matrix::transpose(rotation_m), m_by_m(m, rotation_m));
}

template <typename T>
inline auto almost_diagonal(matrix<T> m) -> bool {
    T current{0};
    for (std::size_t i = 0; i < m.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m.number_of_columns(); j++) {
            current = m[i, j];
        }
        if (current == 0) { return true; }
    }
    return false;
}

template <typename T>
inline auto abs_m(matrix<T> m) -> matrix<T> {
    for (std::size_t i = 0; i < m.number_of_rows(); i++) {
        for (std::size_t j = 0; j < m.number_of_columns(); j++) {
            m[i, j] = abs(m[i, j]);
        }
    }
    return m;
}

template <typename T>
inline auto check_tolerance(matrix<T> v1, matrix<T> v2) -> bool {
    assert(v1.number_of_columns() == 1 && v1.shape() == v2.shape());
    const double tolerance{1e-6};
    matrix<T> m_diff{abs_m(v1) - abs_m(v2)};
    for (std::size_t i = 0; i < m_diff.number_of_rows(); i++) {
        if (abs(m_diff[i, 0]) > tolerance) { return false; }
    }
    return true;
}

export namespace eigen {

    template <typename T>
    inline auto power_method(matrix<T> m) {
        static_assert(std::is_same_v<double, T>);
        matrix<T> b_k{m.number_of_rows(), 1, 1};
        while (!check_tolerance(normalize_v(m_by_v(m, b_k)), b_k)) {
            b_k = normalize_v(m_by_v(m, b_k));
        }
        return b_k;
    }

    template <typename T>
    inline auto inverse_power_method(matrix<T> m) {
        static_assert(std::is_same_v<double, T>);
        return power_method(inverse_matrix(
            m - utils::matrix::identity<T>(m.number_of_rows()) *
                    calculate_mu(m, matrix<T>{m.number_of_rows(), 1, 1})));
    }

    template <typename T>
    inline auto rayleigh(matrix<T> m) {
        static_assert(std::is_same_v<double, T>);
        matrix<T> b_k{m.number_of_rows(), 1, 1};
        matrix<T> helper_m{
            inverse_matrix(m - utils::matrix::identity<T>(m.number_of_rows()) *
                                   calculate_mu(m, b_k))};
        while (!check_tolerance(normalize_v(m_by_v(helper_m, b_k)), b_k)) {
            b_k = normalize_v(m_by_v(helper_m, b_k));
            helper_m = inverse_matrix(
                m - utils::matrix::identity<T>(m.number_of_rows()) *
                        calculate_mu(m, b_k));
        }
        return b_k;
    }

    template <typename T>
    inline auto qr(matrix<T> m) {
        static_assert(std::is_same_v<double, T>);
        matrix<T> u{utils::matrix::identity<T>(m.number_of_rows())};
        while (!check_tolerance(
            vector_from_diagonal(m_by_m(u, qr_decomposition(m).second)),
            vector_from_diagonal(u))) {
            u = m_by_m(u, qr_decomposition(m).second);
            m = m_by_m(qr_decomposition(m).first, qr_decomposition(m).second);
        }
        return vector_from_diagonal(u);
    }

    template <typename T>
    inline auto jacobi(matrix<T> m) {
        static_assert(std::is_same_v<double, T>);
        m = rotation_matrix(m);
        while (!almost_diagonal(m)) { m = rotation_matrix(m); }
        return vector_from_diagonal(m);
    }
}  // namespace eigen
