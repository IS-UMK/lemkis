
namespace ranges {

    /*this class will be used to modify rows columns of a matrix. It vectorizes
     * operations on ranges*/
    template <std::ranges::viewable_range R>
    class numeric_view
        : public std::ranges::subrange<std::ranges::iterator_t<R>,
                                       std::ranges::sentinel_t<R>> {

      private:
        using value_type = std::ranges::range_value_t<R>;
        using base_type = std::ranges::subrange<std::ranges::iterator_t<R>,
                                                std::ranges::sentinel_t<R>>;

      public:
        constexpr explicit numeric_view(R&& v) : base_type{v} {}
        constexpr explicit numeric_view(R& v) : base_type{v} {}


      public:
        template <std::ranges::viewable_range Range>
        requires(
            std::convertible_to<std::ranges::range_value_t<Range>, value_type>)
        constexpr auto operator+=(const Range& v) -> auto& {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a += b; }
            return *this;
        }

        // Coordinate-wise subtraction
        template <std::ranges::viewable_range Range>
        requires(
            std::convertible_to<std::ranges::range_value_t<Range>, value_type>)
        constexpr auto operator-=(const Range& v) -> auto& {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a -= b; }
            return *this;
        }

        // Coordinate-wise multiplication
        template <std::ranges::viewable_range Range>
        requires(
            std::convertible_to<std::ranges::range_value_t<Range>, value_type>)
        constexpr auto operator*=(const Range& v) -> auto& {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a *= b; }
            return *this;
        }

        // Coordinate-wise division
        template <std::ranges::viewable_range Range>
        requires(
            std::convertible_to<std::ranges::range_value_t<Range>, value_type>)
        constexpr auto operator/=(const Range& v) -> auto& {
            for (auto&& [a, b] : std::views::zip(*this, v)) { a /= b; }
            return *this;
        }

        // Scalar addition to each coordinate
        constexpr auto operator+=(std::convertible_to<value_type> auto scalar)
            -> auto& {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val + scalar;
            });
            return *this;
        }

        // Scalar subtraction from each coordinate
        constexpr auto operator-=(std::convertible_to<value_type> auto scalar)
            -> auto& {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val - scalar;
            });
            return *this;
        }

        // Scalar multiplication to each coordinate
        constexpr auto operator*=(std::convertible_to<value_type> auto scalar)
            -> auto& {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val * scalar;
            });
            return *this;
        }

        // Scalar division from each coordinate
        constexpr auto operator/=(std::convertible_to<value_type> auto scalar)
            -> auto& {
            std::ranges::transform(*this, this->begin(), [scalar](auto val) {
                return val / scalar;
            });
            return *this;
        }
    };

}  // namespace ranges
