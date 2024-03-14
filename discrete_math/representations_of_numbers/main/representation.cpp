#include "representation.h"

using namespace representation;

int Rep::load_numerator() {
	int x;

	std::cout << "\nPodaj licznik ulamka:\t";
	std::cin >> x;

	if (std::cin.fail()) {
		std::cout << "\nPodana wartosc jest bledna.";
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		return 0;
	}

	return x;
}

int Rep::load_denominator() {
	int x;

	std::cout << "\nPodaj mianownik ulamka:\t";
	std::cin >> x;

	if (std::cin.fail() || x == 0) {
		std::cout << "\nPodana wartosc jest bledna.";
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		return 1;
	}

	return x;
}

int Rep::load_base() {
	int x;

	std::cout << "\nPodaj baze rozwiniecia ulamka:\t";
	std::cin >> x;

	if (std::cin.fail() || x < 2) {
		std::cout << "\nPodana wartosc jest bledna.";
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		
		return 10;
	}

	return x;
}

Fraction Rep::create_fraction(int x, int y) {
	Fraction fraction{ x, y};

	return fraction;
}

Expansion Rep::create_expansion(int x) {
	Expansion expansion{ x };

	return expansion;
}

std::string Rep::expansion_in_base(Fraction f, Expansion e) {
    std::string x, character;
    int integral_number, rest, index;
    std::vector<int> rs;

    if ((f.numerator > 0 && f.denominator > 0) || (f.numerator < 0 && f.denominator < 0)) {
        character = "";
    }
    else {
        character = "-";
    }

    f.numerator = abs(f.numerator);
    f.denominator = abs(f.denominator);
    x = "";

    integral_number = f.numerator / f.denominator;
    while (integral_number > 0) {
        rest = integral_number % e.base;
        if (rest < 10) {
            x = (char)(rest + 48) + x;
        }
        else {
            x = (char)(rest + 55) + x;
        }
        integral_number /= e.base;
    }
    if (f.numerator % f.denominator != 0) {
        x += ".";
        rest = f.numerator % f.denominator;

        while (rest != 0 && std::find(rs.begin(), rs.end(), rest) == rs.end()) {
            rs.push_back(rest);
            f.numerator = rest * e.base;
            if (f.numerator / f.denominator < 10) {
                x += (char)(f.numerator / f.denominator + 48);
            }
            else {
                x += (char)(f.numerator / f.denominator + 55);
            }
            rest = f.numerator % f.denominator;
        }

        if (rest != 0) {
            auto a = std::find(rs.begin(), rs.end(), rest);
            index = std::distance(rs.begin(), a);
            x.insert(index + x.find('.') + 1, "(");
            x += ')';
        }
    }
    x = character + x;

    return x;
}