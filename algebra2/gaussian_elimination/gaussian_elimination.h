// In this task you will implement gaussian elimination algorithm.
// Important! You must use the methods from matrix.ixx file (previous project
// written by you classmates). USE C++20 MODULES READ e.g.
// https://learn.microsoft.com/en-us/cpp/cpp/modules-cpp?view=msvc-170
#pragma once

#include "matrix.hpp"
#include <cstdint>
#include <numeric>
#include <iostream>
#include <utility>



namespace algorithms::gaussian_elimination {

    // possible flags. User can indicate which operations are allowed
    // during the gaussian elimination use which operation
    enum class operation : std::uint8_t {
        none = 0,
        swap = 1 << 0,
        multiply = 1 << 1,
        add = 1 << 2,
    };

    /*you can combine opearations e.g. auto swap_or_add{operation::swap |
     * operation::add};*/
    constexpr auto operator|(operation o1, operation o2) -> operation {
        return static_cast<operation>(std::to_underlying(o1) |
                                      std::to_underlying(o2));
    }
    /*you can check which operations are allowed e.g.
    bool is_swap_allowed{operation::swap & swap_or_add != operation::none};*/
    constexpr auto operator&(operation o1, operation o2) -> operation {
        return static_cast<operation>(std::to_underlying(o1) &
                                      std::to_underlying(o2));
    }

    enum class axis {
        rows,
        columns,
    };

    /*If you perform (standard) gaussian elimination
    you obtain echelon form (see
    https://en.wikipedia.org/wiki/Gaussian_elimination#Echelon_form) Sometimes
    (e.g. when you look for an inverse) you need a diagonal form.*/
    enum class reducted_form {
        echelon,
        diagonal /*with ones or zeros on the diagonal and 0 otherwise*/
    };
    /*runs gaussian elimination on axis a for matrix m with allowed
    operations given by allowed_operations until the reducted form reducted is
    obtained*/
    template <std::integral T, std::integral LP>
    auto run(ranges::matrix_view<T, LP> m,
             axis a,
             operation allowed_operations = operation::swap | operation::add |
                                            operation::multiply,
             reducted_form reducted = reducted_form::echelon) -> void;


    /*runs gaussian elimination on axis a for matrix m with allowed
operations given by allowed_operations*/
    /*Demonstrate on screen intermediate steps of the elimination*/
    template<std::integral T, typename LP>
    auto show_steps(ranges::matrix_view<T, LP> m,
                    axis a,
                    operation allowed_operations = operation::swap | operation::add | operation::multiply,
                    reducted_form reducted = reducted_form::echelon) -> void;

    enum class error {
        not_invertible,
        not_square,
    };

    // returns inverse of matrix m
    template <typename T, typename LP>
    auto inverse(ranges::matrix_view<T, LP> m) -> std::expected<ranges::matrix_view<T, LP>, algorithms::gaussian_elimination::error>;

    /*calculates the determinant of matrix m*/
    template <typename T, typename LP>
    auto determinant(ranges::matrix_view<const T, LP> m) -> std::expected<T, error>;

    /*calculates the determinant of matrix m containing fractions*/
    template <std::integral T, typename LP>
    auto determinant(ranges::matrix_view<ranges::fraction<T>, LP> m) -> std::expected<ranges::fraction<T>, algorithms::gaussian_elimination::error>;

}  // namespace algorithms::gaussian_elimination


/*description:
 *      Performs gaussian elimination on matrix m containing fractions*/
template<std::integral T, typename LP>
auto run(ranges::matrix_view<ranges::fraction<T>, LP> m,
         algorithms::gaussian_elimination::axis a,
         algorithms::gaussian_elimination::operation allowed_operations = algorithms::gaussian_elimination::operation::swap | algorithms::gaussian_elimination::operation::add |
                 algorithms::gaussian_elimination::operation::multiply,
         algorithms::gaussian_elimination::reducted_form reducted = algorithms::gaussian_elimination::reducted_form::echelon) -> void {

    std::println("//////////Gaussian Elimination on matrix m containing fractions//////////\n");
    std::println("Matrix m = {}\n", m);
    ranges::fraction<T> res;

    for(int i = 0;  i < m.extent(0); i++){
        if(m[i][i].numerator == 0){
            for(int j = i+1; j < m.extent(0); j++){
                if(m[j][i].numerator != 0){
                    swap(m, i, j);
                }
            }
        }
        if(m[i][i].numerator != 0){
            for(int k = i+1; k < m.extent(0); k++){
                res = m[k][i]/m[i][i];
                if(res.numerator != 0){
                    subtract(m, k, i, res);
                }
            }
        }
    }

    if(reducted == algorithms::gaussian_elimination::reducted_form::diagonal){
        for(int i = 0;  i < m.extent(0); i++){
            if(m[i, i].numerator != 0){
                for(int k = i+1; k < m.extent(0); k++){
                    res = m[i, k]/m[k, k];
                    if(res.numerator != 0){
                        subtract(m, i, k, res);
                    }
                }
            }
        }
    }

    std::println("The reduced matrix is {}\n", m);
}

template<typename T, typename LP>
auto run(ranges::matrix_view<T, LP> m,
         algorithms::gaussian_elimination::axis a,
         algorithms::gaussian_elimination::operation allowed_operations = algorithms::gaussian_elimination::operation::swap | algorithms::gaussian_elimination::operation::add |
                                                                          algorithms::gaussian_elimination::operation::multiply,
         algorithms::gaussian_elimination::reducted_form reducted = algorithms::gaussian_elimination::reducted_form::echelon) -> void {

    std::println("//////////Gaussian Elimination on matrix m//////////\n");
    std::println("Matrix m = {}\n", m);
    double res;

    for(int i = 0;  i < m.extent(0); i++) {
        if (m[i, i] == 0) {
            for (int j = i + 1; j < m.extent(0); j++) {
                if (m[j, i] != 0) {
                    swap(m, i, j);
                    break;
                }
            }
        }
        if (m[i, i] != 0) {
            for (int k = i + 1; k < m.extent(0); k++) {
                res = m[k, i] / m[i, i];
                if (res != 0) {
                    subtract(m, k, i, res);
                }
            }
        }
    }

    if(reducted == algorithms::gaussian_elimination::reducted_form::diagonal){
        for(int i = 0;  i < m.extent(0); i++){
            if(m[i, i] != 0){
                for(int k = i+1; k < m.extent(0); k++){
                    res = m[i, k]/m[k, k];
                    if(res != 0){
                        subtract(m, i, k, res);
                    }
                }
            }
        }
    }
    std::println("The reduced matrix is {}\n", m);
}


/*
 * description:
 *      Performs Gaussian elimination on matrix m containing fractions and shows steps after each operation*/
template<std::integral T, typename LP>
auto show_steps(ranges::matrix_view<ranges::fraction<T>, LP> m,
                algorithms::gaussian_elimination::axis a,
                algorithms::gaussian_elimination::operation allowed_operations = algorithms::gaussian_elimination::operation::swap | algorithms::gaussian_elimination::operation::add |
                                                                                 algorithms::gaussian_elimination::operation::multiply,
                algorithms::gaussian_elimination::reducted_form reducted = algorithms::gaussian_elimination::reducted_form::echelon) -> void{
    std::println("//////////Gaussian Elimination to row echelon form on fractions with steps//////////\n");
    std::println("Matrix m = {}\n", m);
    ranges::fraction<T> res;

    for(int i = 0;  i < m.extent(0); i++){
        if(m[i][i].numerator == 0){
            for(int j = i+1; j < m.extent(0); j++){
                if(m[j][i].numerator != 0){
                    swap(m, i, j);
                    std::println("↓ swap R{} with R{}", i + 1, j + 1);
                    std::println("{}\n", m);
                    break;
                }
            }
        }
        if(m[i][i].numerator != 0){
            for(int k = i+1; k < m.extent(0); k++){
                res = m[k][i]/m[i][i];
                if(res.numerator != 0){
                    subtract(m, k, i, res);
                    std::println("↓ R({}) - {}*R({})", k+1, res, i+1);
                    std::println("{}\n", m);
                }
            }
        }
    }

    if(reducted == algorithms::gaussian_elimination::reducted_form::diagonal) {
        for (int i = 0; i < m.extent(0); i++) {
            if (m[i, i].numerator != 0) {
                for (int k = i + 1; k < m.extent(0); k++) {
                    res = m[i, k] / m[k, k];
                    if (res.numerator != 0) {
                        subtract(m, i, k, res);
                        std::println("↓ R({}) - {}*R({})", i + 1, res, k + 1);
                        std::println("{}\n", m);
                    }
                }
            }
        }
    }
}
/*
 * description:
 *      Performs Gaussian elimination on matrix m and shows steps after each operation*/
template<typename T, typename LP>
auto show_steps(ranges::matrix_view<T, LP> m,
                algorithms::gaussian_elimination::axis a,
                algorithms::gaussian_elimination::operation allowed_operations = algorithms::gaussian_elimination::operation::swap | algorithms::gaussian_elimination::operation::add |
                                                                                 algorithms::gaussian_elimination::operation::multiply,
                algorithms::gaussian_elimination::reducted_form reducted = algorithms::gaussian_elimination::reducted_form::echelon) -> void{
    double res;

    std::println("//////////Gaussian Elimination to row echelon form with steps//////////\n");
    std::println("Matrix m = {}\n", m);

    for(int i = 0;  i < m.extent(0); i++) {
        if (m[i, i] == 0) {
            for (int j = i + 1; j < m.extent(0); j++) {
                if (m[j, i] != 0) {
                    swap(m, i, j);
                    std::println("↓ swap R{} with R{}", i + 1, j + 1);
                    std::println("{}\n", m);
                    break;
                }
            }
        }
        if (m[i][i] != 0) {
            for (int k = i + 1; k < m.extent(0); k++) {
                res = m[k, i] / m[i, i];
                if (res != 0) {
                    subtract(m, k, i, res);
                    std::println("↓ R({}) - {}*R({})", k + 1, res, i + 1);
                    std::println("{}\n", m);
                }
            }
        }
    }

    if(reducted == algorithms::gaussian_elimination::reducted_form::diagonal){
        for(int i = 0;  i < m.extent(0); i++){
            if(m[i, i] != 0){
                for(int k = i+1; k < m.extent(0); k++){
                    res = m[i, k]/m[k, k];
                    if(res != 0){
                        subtract(m, i, k, res);
                        std::println("↓ R({}) - {}*R({})", i + 1, res, k + 1);
                        std::println("{}\n", m);
                    }
                }
            }
        }
    }
}


/*
 * description:
 *      Calculates determinant of matrix m containing fractions*/
template <std::integral T, typename LP>
auto determinant(ranges::matrix_view<ranges::fraction<T>, LP> m) -> std::expected<ranges::fraction<T>, algorithms::gaussian_elimination::error>{
    ranges::fraction<T> det{1, 1}, res;

    std::println("//////////Determinant of a matrix m containing fractions//////////\n");
    std::println("Matrix m = {}\n", m);

    if(m.extent(0) == m.extent(1)){
        for(int i = 0;  i < m.extent(0); i++){
            if(m[i][i].numerator == 0){
                for(int j = i+1; j < m.extent(0); j++){
                    if(m[j][i].numerator != 0){
                        swap(m, i, j);
                        break;
                    }
                }
            }
            if(m[i][i].numerator != 0){
                for(int k = i+1; k < m.extent(0); k++){
                    res = m[k][i]/m[i][i];
                    if(res.numerator != 0){
                        subtract(m, k, i, res);
                    }
                }
            }
        }

        for(int i = 0; i < m.extent(0); i++){
            det *= m[i, i];
        }
        std::println("The determinant of m = {}\n", det);
        return det;
    }else{
        std::println("Cannot calculate a determinant of a non square matrix\n");
        return std::unexpected(algorithms::gaussian_elimination::error::not_square);
    }
}
/*
 * description:
 *      Calculates determinant of matrix m*/
template <typename T, typename LP>
auto determinant(ranges::matrix_view<T, LP> m) -> std::expected<T, algorithms::gaussian_elimination::error>{
    double det = 1, res;

    std::println("//////////Determinant of matrix m//////////\n");
    std::println("Matrix m = {}\n", m);

    if(m.extent(0) == m.extent(1)){

        for(int i = 0;  i < m.extent(0); i++){
            if(m[i, i] == 0){
                for(int j = i+1; j < m.extent(0); j++){
                    if(m[j, i] != 0){
                        swap(m, i, j);
                        break;
                    }
                }
            }
            if(m[i, i] != 0){
                for(int k = i+1; k < m.extent(0); k++){
                    res = m[k, i]/m[i, i];
                    if(res != 0){
                        subtract(m, k, i, res);
                    }
                }
            }
        }

        for(int i = 0; i < m.extent(0); i++){
            det *= m[i, i];
        }
        std::println("The determinant of m = {}\n", det);
        return det;
    }else{
        std::println("Cannot calculate a determinant of a non square matrix\n");
        return std::unexpected(algorithms::gaussian_elimination::error::not_square);
    }
}


/*
 * description:
 *      Finds inverse of matrix m containing fractions*/
template <std::integral T, typename LP>
auto inverse(ranges::matrix_view<ranges::fraction<T>, LP> m) -> std::expected<ranges::matrix_view<ranges::fraction<T>, LP>, algorithms::gaussian_elimination::error>{
    ranges::fraction<T> det{1, 1}, res;
    std::vector<ranges::fraction<T>> ID;
    ID.reserve(m.extent(0)*m.extent(0));

    std::print("//////////Inverse of matrix m containing fractions//////////\n");

    for(int i = 0; i < m.extent(0)*m.extent(0); i++){
        ID[i].numerator = 0;
        ID[i].denominator = 1;
    }
    for(int i = 0; i < m.extent(0)*m.extent(0); i+=m.extent(0)+1){
        ID[i].numerator = 1;
        ID[i].denominator = 1;
    }

    ranges::matrix_view<ranges::fraction<T>, LP> id(ID, m.extent(0), m.extent(1), layout::row);
    std::println("Matrix m = {}\n", m);
    if(m.extent(0) == m.extent(1)){
        for(int i = 0;  i < m.extent(0); i++){
            if(m[i, i].numerator == 0){
                for(int j = i+1; j < m.extent(0); j++){
                    if(m[j, i].numerator != 0){
                        swap(m, i, j);
                        break;
                    }
                }
            }
            if(m[i, i].numerator != 0){
                for(int k = i+1; k < m.extent(0); k++){
                    res = m[k][i]/m[i][i];
                    if(res.numerator != 0){
                        subtract(m, k, i, res);
                        subtract(id, k, i, res);
                    }
                }
            }

        }

        for(int i = 0; i < m.extent(0); i++){
            det *= m[i, i];
        }

        if(det.numerator != 0){
            for(int i = 0;  i < m.extent(0); i++){
                if(m[i, i].numerator != 0){
                    for(int k = i+1; k < m.extent(0); k++){
                        res = m[i, k]/m[k, k];
                        if(res.numerator != 0){
                            subtract(m, i, k, res);
                            subtract(id, i, k, res);
                        }
                    }
                }
            }

            for(int i = 0; i < m.extent(0); i++){
                res = m[i, i];
                m[i] /= res;
                id[i] /= res;
            }
            std::println("The inverse of matrix m = {}\n", id);
            return id;
        }else{
            std::println("Determinant of m is 0, therefore inverse of matrix m does not exist");
            return std::unexpected(algorithms::gaussian_elimination::error::not_invertible);
        }
    }else{
        std::println("Cannot find inverse of a non-square matrix");
        return std::unexpected(algorithms::gaussian_elimination::error::not_square);
    }
}
/*
 * description:
 *      Finds inverse of matrix m that contains fractions*/
template <typename T, typename LP>
auto inverse(ranges::matrix_view<T, LP> m) -> std::expected<ranges::matrix_view<T, LP>, algorithms::gaussian_elimination::error>{
    double res, det = 1;
    std::vector<double> ID;
    ID.reserve(m.extent(0)*m.extent(0));

    std::println("//////////Inverse of matrix m//////////\n");

    for(int i = 0; i < m.extent(0)*m.extent(0); i++){
        ID[i] = 0.0;
    }
    for(int i = 0; i < m.extent(0)*m.extent(0); i+=m.extent(0)+1){
        ID[i] = 1.0;
    }

    ranges::matrix_view id(ID, m.extent(0), m.extent(1), layout::row);
    std::println("Matrix m = {}\n", m);
    if(m.extent(0) == m.extent(1)){
        for(int i = 0;  i < m.extent(0); i++){
            if(m[i, i] == 0){
                for(int j = i+1; j < m.extent(0); j++){
                    if(m[j, i] != 0){
                        swap(m, i, j);
                        break;
                    }
                }
            }
            if(m[i, i] != 0){
                for(int k = i+1; k < m.extent(0); k++){
                    res = m[k, i]/m[i, i];
                    if(res != 0){
                        subtract(m, k, i, res);
                        subtract(id, k, i, res);
                    }
                }
            }
        }

        for(int i = 0; i < m.extent(0); i++){
            det *= m[i, i];
        }

        if(det != 0){
            for(int i = 0;  i < m.extent(0); i++){
                if(m[i, i] != 0){
                    for(int k = i+1; k < m.extent(0); k++){
                        res = m[i, k]/m[k, k];
                        if(res != 0){
                            subtract(m, i, k, res);
                            subtract(id, i, k, res);
                        }
                    }
                }
            }

            for(int i = 0; i < m.extent(0); i++){
                res = m[i, i];
                m[i] /= res;
                id[i] /= res;
            }
            std::println("The inverse of matrix m = {}\n", id);
            return id;
        }else{
            std::println("Determinant of m is 0, therefore inverse of matrix m does not exist");
            return std::unexpected(algorithms::gaussian_elimination::error::not_invertible);
        }
    }else{
        std::println("Cannot find inverse of a non-square matrix");
        return std::unexpected(algorithms::gaussian_elimination::error::not_square);
    }
}


/*
 * description:
 *      Provides examples of implementing different gaussian elimination operations*/
template <std::integral T, typename LP>
void gaussian_elimination_examples(){
    std::vector v1{1.0, 2.0, 3.0, 4.0, 3.0, 4.0, 7.0, 12.0, 9.0};
    std::vector v2{1.0, 2.0, 3.0, 4.0, 3.0, 4.0, 7.0, 12.0, 9.0};
    std::vector v3{1.0, 9.0, 3.0, 4.0, 3.0, 4.0, 7.0, 12.0, 9.0, 3.3, 2.43, 1.4, 9.8, 3.6, 4.3, 5.0};
    std::vector v4{1.0, 9.0, 3.0, 4.0, 3.0, 4.0, 7.0, 12.0, 9.0, 3.3, 2.43, 1.4, 9.8, 3.6, 4.3, 5.0};

    ranges::matrix_view m1(v1, 3, 3, layout::row);
    ranges::matrix_view m2(v2, 3, 3, layout::row);
    ranges::matrix_view m3(v3, 4, 4, layout::row);
    ranges::matrix_view m4(v4, 4, 4, layout::row);

    run(m1, algorithms::gaussian_elimination::axis::rows);
    show_steps(m2, algorithms::gaussian_elimination::axis::rows);
    determinant(m3);
    inverse(m4);
}

/*
 * description:
 *      Provides examples of implementing different gaussian elimination operations on matrix m containing fractions*/
template <std::integral T, typename LP>
void gaussian_elimination_examples_fractions(){
    std::vector<ranges::fraction<T>> v1{{1, 1}, {2, 1}, {3, 1}, {4, 1}, {3, 1}, {4, 1}, {7, 1}, {12, 1}, {9, 1}};
    std::vector<ranges::fraction<T>> v2{{1, 1}, {2, 1}, {3, 1}, {4, 1}, {3, 1}, {4, 1}, {7, 1}, {12, 1}, {9, 1}};
    std::vector<ranges::fraction<T>> v3{{3, 1}, {4, 3}, {5, 9}, {9, 8}, {3, 4}, {13, 2}, {12, 1}, {19, 3}, {4, 1}, {3, 2}, {1, 5}, {9, 2}, {14, 15}, {4, 5}, {7, 3}, {3, 4}};
    std::vector<ranges::fraction<T>> v4{{3, 1}, {4, 3}, {5, 9}, {9, 8}, {3, 4}, {13, 2}, {12, 1}, {19, 3}, {4, 1}, {3, 2}, {1, 5}, {9, 2}, {14, 15}, {4, 5}, {7, 3}, {3, 4}};

    ranges::matrix_view<ranges::fraction<T>, LP> m1(v1, 3, 3, layout::row);
    ranges::matrix_view<ranges::fraction<T>, LP> m2(v2, 3, 3, layout::row);
    ranges::matrix_view<ranges::fraction<T>, LP> m3(v3, 4, 4, layout::row);
    ranges::matrix_view<ranges::fraction<T>, LP> m4(v4, 4, 4, layout::row);

    run(m1, algorithms::gaussian_elimination::axis::rows);
    show_steps(m2, algorithms::gaussian_elimination::axis::rows);
    determinant(m3);
    inverse(m4);
}
