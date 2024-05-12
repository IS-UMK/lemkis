#pragma once

#include "matrix.hpp"
#include <cmath>
#include <cstdint>
#include <numeric>
#include <iostream>
#include <utility>
#include <typeinfo>


namespace algorithms::gaussian_elimination {

    enum class operation : std::uint8_t {
        none = 0,
        swap = 1 << 0,
        multiply = 1 << 1,
        add = 1 << 2,
    };

    template <std::integral I>
    struct fraction {
        I numerator{ 0 };
        I denominator{ 1 };

        constexpr fraction<I>& operator-=(I i) {
            numerator -= i * denominator;
            return *this;
        }
        /*
        description:
            reduce fraction
    */
        void reduce() {
            I divider = std::gcd(numerator, denominator);
            numerator /= divider;
            denominator /= divider;
        }

    };

    /*
        description:
        Operations on fractions
    */
    template <std::integral I>
    constexpr fraction<I> operator+(fraction<I> f, fraction<I> g) {
        fraction<I> result{};
        result.numerator =
                f.numerator * g.denominator + g.numerator * f.denominator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }

    template <std::integral I>
    constexpr fraction<I> operator+=(fraction<I>& f, fraction<I> g) {
        f.numerator =
                f.numerator * g.denominator + g.numerator * f.denominator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }

    template <std::integral I>
    constexpr fraction<I> operator-(fraction<I> f, fraction<I> g){
        fraction<I> result;
        result.numerator = f.numerator * g.denominator - g.numerator * f.denominator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }

    template <std::integral I>
    constexpr fraction<I> operator-=(fraction<I>& f, fraction<I> g){
        f.numerator = f.numerator * g.denominator - g.numerator * f.denominator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }

    template <std::integral I>
    constexpr fraction<I> operator*(fraction<I> f, fraction<I> g){
        fraction<I> result;
        result.numerator = f.numerator * g.numerator;
        result.denominator = f.denominator * g.denominator;
        result.reduce();
        return result;
    }

    template <std::integral I>
    constexpr fraction<I> operator*=(fraction<I>& f, fraction<I> g){
        f.numerator = f.numerator * g.numerator;
        f.denominator = f.denominator * g.denominator;
        f.reduce();
        return f;
    }

    template <std::integral I>
    constexpr fraction<I> operator/(fraction<I> f, fraction<I> g){
        fraction<I> result;
        result.numerator = f.numerator * g.denominator;
        result.denominator = f.denominator * g.numerator;
        result.reduce();
        return result;
    }

    template <std::integral I>
    constexpr fraction<I> operator/=(fraction<I>& f, fraction<I> g){
        fraction<I> result;
        f.numerator = f.numerator * g.denominator;
        f.denominator = f.denominator * g.numerator;
        f.reduce();
        return f;
    }

    template <std::integral I>
    constexpr fraction<I> operator*(fraction<I> f, int g){
        f.numerator = f.numerator * g;
        f.reduce();
        return f;
    }

    template <std::integral I>
    constexpr fraction<I> operator*=(fraction<I>& f, int g){
        f.numerator = f.numerator * g;
        f.reduce();
        return f;
    }

}
    /*
        description:
            enables formatting of output data
            for algorirthms::gaussian_elimination::fraction<I>
    */
    template <std::integral I>
    struct std::formatter<algorithms::gaussian_elimination::fraction<I>> {

        template <typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const algorithms::gaussian_elimination::fraction<I>& f,
                    FormatContext& ctx) const {
            return std::format_to(ctx.out(), "{}/{}", f.numerator, f.denominator);
        }
    };

namespace algorithms::gaussian_elimination {
    
    /*
        description:
            converts vector of ints to vector of fractions
    */
    template <std::ranges::range R>
    requires(std::integral<std::ranges::range_value_t<R>>)
    auto convert_to_vector_of_fractions(R& v) {
        return v | std::views::transform([](auto i) { return fraction{i, 1}; }) |
            std::ranges::to<std::vector>();
    }

    /*
        description:
            converts matrix of ints to vector of fractions
    */
    template <std::integral I, typename LP>
    auto convert_to_matrix_of_fractions(::ranges::matrix_view<I, LP>& m)
    {
        auto fracs{convert_to_vector_of_fractions(m)};
        ranges::matrix_view<fraction<I>, LP> matrix_of_fracs{fracs, m.extent(0), m.extent(1), LP{}};
        return std::pair{std::move(fracs), matrix_of_fracs};
    }

    /*
        description:
            combines two matrices into one augmented matrix
    */
    template <typename T, typename LP>
    auto combine_matrices(::ranges::matrix_view<T, LP>& m1,::ranges::matrix_view<T, LP>& m2){
        int combined_dims = m1.extent(0) + m2.extent(0);
        
        typename::std::vector<T>::iterator it;
        int iterator = 0;
        std::vector m1_vector = m1 | std::ranges::to<std::vector>();
        std::vector m2_vector = m2 | std::ranges::to<std::vector>();
        int size_m1 = m1_vector.size(),
            size_m2 = m2_vector.size();
        for(int i = 0; m1_vector.size() < size_m2 + size_m1; i += sqrt(size_m2)+ sqrt(size_m1)){
            for(int j = 0; j < sqrt(size_m2); j++){
                it = m1_vector.begin();
                m1_vector.insert(it + i+j, m2_vector[iterator]);
                iterator++;
            }
        }

        ranges::matrix_view<T, LP> combined_matrix(m1_vector, m1.extent(0), combined_dims, layout::row);
        return std::pair{std::move(m1_vector), combined_matrix};
    }

    /*
        description:
            generates an identity matrix using given matrix as a reference
    */
    template <typename T, typename LP>
    auto identity_generator(ranges::matrix_view<T, LP> m){
        int dimensions = m.extent(0) * m.extent(1);
        std::vector<T> identity_vector;
        identity_vector.reserve(dimensions);
        
        for(int i = 0; i < dimensions; i++){
            identity_vector[i] = 0;
        }
        for(int i = 0; i < dimensions; i += m.extent(0)+1){
            identity_vector[i] = 1;
        }

        ranges::matrix_view identity_matrix(identity_vector, m.extent(0), m.extent(1), layout::row);
        return std::pair{std::move(identity_vector), identity_matrix};
    }

    /*
        description:
            generates an identity matrix of fractions using given matrix as a reference
    */
    template <typename T, typename LP>
    auto identity_generator(ranges::matrix_view<fraction<T>, LP> m){
        int dimensions = m.extent(0) * m.extent(1);
        std::vector<fraction<T>> identity_vector;
        identity_vector.reserve(dimensions);
        
        for(int i = 0; i < dimensions; i++){
            identity_vector[i].numerator = 0;
            identity_vector[i].denominator = 1;
        }
        for(int i = 0; i < dimensions; i += m.extent(0)+1){
            identity_vector[i].numerator = 1;
        }

        ranges::matrix_view identity_matrix(identity_vector, m.extent(0), m.extent(1), layout::row);
        return std::pair{std::move(identity_vector), identity_matrix};
    }

    /*
        description:
            splits one matrix in half and returns the second half
    */
    template <typename T, typename LP>
    auto split_matrix(ranges::matrix_view<T, LP> m){
        
        int rows = m.extent(0),
            cols = m.extent(1),
            dimensions = rows * cols;
        typename::std::vector<T>::iterator it;
        std::vector m_vector = m | std::ranges::to<std::vector>();
        std::vector<T> split_vec;
        split_vec.reserve(dimensions/2);
        int iterator = 0;
        for(int i = rows; i < dimensions; i += cols){
            for(int j = i; j < i + rows; j++){
                split_vec[iterator] = m_vector[j];
                iterator++;
            }
        }
        ranges::matrix_view split_mat(split_vec, rows, cols/2, layout::row);
        return std::pair{std::move(split_vec), split_mat};
    }

    constexpr auto operator|(operation o1, operation o2) -> operation {
        return static_cast<operation>(std::to_underlying(o1) |
                                      std::to_underlying(o2));
    }

    constexpr auto operator&(operation o1, operation o2) -> operation {
        return static_cast<operation>(std::to_underlying(o1) &
                                      std::to_underlying(o2));
    }

    enum class axis {
        rows,
        columns,
    };

    enum class reducted_form {
        echelon,
        diagonal
    };

    enum class error {
        not_invertible,
        not_square,
    };
    
    /*
        description:
            performs gaussian elimination on matrix m and returns it's determinant, reduced matrix and steps
    */
    template <typename T, typename LP>
    auto gaussian_elimiantion_alg(ranges::matrix_view<T, LP> m, 
                                    axis a, 
                                    reducted_form reducted,
                                    operation allowed_operations = operation::swap | operation::add | operation::multiply){
        auto [fracs, matrix_of_fracs] = convert_to_matrix_of_fractions(m);

        int rows = matrix_of_fracs.extent(0),
                    cols = matrix_of_fracs.extent(1);
        fraction<int> factor;
        std::pair<std::vector<std::string>, fraction<int>> values;
        values.second = {1, 1};      

        for(int i = 0;  i < rows; i++){
            if(matrix_of_fracs[i, i].numerator == 0){
                for(int j = i+1; j < rows; j++){
                    if(matrix_of_fracs[j, i].numerator != 0){
                        swap(m, i, j);
                        values.first.push_back(std::format("swap R{} with R{}", j+1, i+1));
                        values.second *= -1; 
                    }
                }
            }
            if(matrix_of_fracs[i, i].numerator != 0){
                for(int k = i+1; k < rows; k++){
                    factor = matrix_of_fracs[k, i]/matrix_of_fracs[i, i];
                    if(factor.numerator != 0){
                        subtract(matrix_of_fracs, k, i, factor);
                        values.first.push_back(std::format("R{} - {} * R{}", k+1, factor, i+1)); 
                    }
                }
            }
        }
        if(reducted == reducted_form::diagonal){
            for(int i = 0;  i < rows; i++){
                if(matrix_of_fracs[i, i].numerator != 0){
                    for(int k = i+1; k < rows; k++){
                        factor = matrix_of_fracs[i, k]/matrix_of_fracs[k, k];
                        subtract(matrix_of_fracs, i, k, factor);
                    }
                }
            }

            for(int i = 0; i < rows; i++){
                factor = matrix_of_fracs[i, i];
                matrix_of_fracs[i] /= factor;
                values.second *= factor * -1; 
            }
        }
        return std::pair{std::pair{std::move(fracs), matrix_of_fracs}, values};
    }

    /*
        description:
            performs gaussian elimination on matrix m and returns it's determinant, reduced matrix and steps(overloader for matrix with fractions)
    */
    template <std::integral T, typename LP>
    auto gaussian_elimiantion_alg(ranges::matrix_view<fraction<T>, LP> m, 
                                    axis a, 
                                    reducted_form reducted,
                                    operation allowed_operations = operation::swap | operation::add | operation::multiply){
        int rows = m.extent(0),
            cols = m.extent(1);
        fraction<int> factor;
        std::pair<std::vector<std::string>, fraction<int>> values;
        values.second = {1, 1};
        for(int i = 0;  i < rows; i++){
            if(m[i, i].numerator == 0){
                for(int j = i+1; j < rows; j++){
                    if(m[j, i].numerator != 0){
                        swap(m, i, j);
                        values.first.push_back(std::format("swap R{} with R{}", j+1, i+1));
                        values.second *= -1; 
                    }
                }
            }
            if(m[i, i].numerator != 0){
                for(int k = i+1; k < rows; k++){
                    factor = m[k, i]/m[i, i];
                    if(factor.numerator != 0){
                        subtract(m, k, i, factor);
                        values.first.push_back(std::format("R{} - {} * R{}", k+1, factor, i+1)); 
                    }
                }
            }
        }

        if(reducted == reducted_form::diagonal){
            for(int i = 0;  i < rows; i++){
                if(m[i, i].numerator != 0){
                    for(int k = i+1; k < rows; k++){
                        factor = m[i, k]/m[k, k];
                        subtract(m, i, k, factor);   
                    }
                }
            }

            for(int i = 0; i < rows; i++){
                factor = m[i, i];
                m[i] /= factor;
                values.second *= factor * -1; 
            }
        }
        
        std::vector m_vector = m | std::ranges::to<std::vector>();
        return std::pair{std::pair{m_vector, m}, values};
    }

    /*
        description:
            runs gaussian elimination on axis a for matrix m until the reducted form reducted is obtained
    */
    template <typename T, typename LP>
    auto run(ranges::matrix_view<T, LP> m,
             axis a,
             operation allowed_operations = operation::swap | operation::add | operation::multiply,
             reducted_form reducted = reducted_form::echelon){
        auto [m_reduced_vector, m_reduced] = gaussian_elimiantion_alg(m, axis::rows, reducted).first;
        return std::pair{std::move(m_reduced_vector), m_reduced};
    }


    /*
        description:
            runs gaussian elimination on axis a for matrix m with allowed
            operations given by allowed_operations,
            demonstrates on screen intermediate steps of the elimination
    */
    template<typename T, typename LP>
    auto show_steps(ranges::matrix_view<T, LP> m,
                    axis a,
                    operation allowed_operations = operation::swap | operation::add | operation::multiply,
                    reducted_form reducted = reducted_form::echelon) -> void{
        std::vector<std::string> steps = gaussian_elimiantion_alg(m, axis::rows, reducted).second.first;
        std::print("{}\n", steps);
    }


    /*
        description:
            calculates the determinant of matrix m
    */
    template <typename T, typename LP>
    auto determinant(ranges::matrix_view<T, LP> m) -> std::expected<fraction<int>, error>{
        int rows = m.extent(0),
            cols = m.extent(1);

        if(rows == cols){
            fraction<int> det = gaussian_elimiantion_alg(m, axis::rows, reducted_form::diagonal).second.second;
            return det;
        }
        else{
            return std::unexpected(error::not_square);
        }
    }

    /*
        description:
            returns inverse of matrix m
    */ 
    template <typename T, typename LP>
    auto inverse(ranges::matrix_view<T, LP> m) -> std::expected<std::pair<std::vector<fraction<int>>, ranges::matrix_view<fraction<int>, LP>>, error>{
        int rows = m.extent(0),
            cols = m.extent(1);
        auto [identity_vector, identity_matrix] = identity_generator(m);
        auto [combined_vector, combined_matrix] = combine_matrices(identity_matrix, m);

        if(determinant(m).value().numerator != 0){
            auto [reduced_vector, reduced_matrix] = gaussian_elimiantion_alg(combined_matrix, axis::rows, reducted_form::diagonal).first;
            auto [inverse_vector, inverse_matrix] = split_matrix(reduced_matrix);
            return std::pair{std::move(inverse_vector), inverse_matrix};
        }else{
            return std::unexpected(error::not_invertible);
        }
    }

}  // namespace algorithms::gaussian_elimination

/*
    description:
        examples of using different gaussian elimination operations on matrices
*/
auto examples(){
    std::vector v1{1, 1, -9, 6, 7, 10, 0, 0, -6, 4, 2, 3, 0, 0, -3, 2};
    std::vector v2{1, -3, -3, -1, 1, 2, 2, -3, -14};
    std::vector<algorithms::gaussian_elimination::fraction<int>> v3{{1, 2}, {1, 2}, {1, 2}, {1, 2}, {2, 4}, {1, 3}, {5, 3}, {1, 2}, {9, 4}};
    std::vector<algorithms::gaussian_elimination::fraction<int>> v4{{1, 3}, {1, 5}, {9, 2}, {5, 6}, {6, 7}, {11, 14}, {15, 10}, {0, 1}, {9, 1}};

    ranges::matrix_view m1(v1, 4, 4, layout::row);
    ranges::matrix_view m2(v2, 3, 3, layout::row);
    ranges::matrix_view m3(v3, 3, 3, layout::row);
    ranges::matrix_view m4(v4, 3, 3, layout::row);

    std::println("//////Gaussian Elimination Examples//////\n");
    auto [m1_vector, m1_reduced] = algorithms::gaussian_elimination::run(m1, algorithms::gaussian_elimination::axis::rows);
    std::println("{}\n", m1_reduced);
    algorithms::gaussian_elimination::show_steps(m2, algorithms::gaussian_elimination::axis::rows);
    auto det = algorithms::gaussian_elimination::determinant(m3).value();
    std::println("Determinant of m3 = {}\n", det);
    auto [inverse_vector, inverse_m4] = inverse(m4).value();
    std::println("Inverse of matrix m4 = {}\n", inverse_m4);
}

