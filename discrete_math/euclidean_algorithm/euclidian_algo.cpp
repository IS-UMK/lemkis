#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define filename "numbers.txt"
#define line "-------------------------------------------------------------- \n"

void welcometext()
{
    printf("jest to program liczacy krok po kroku nwd dla n liczb oraz algorytm euklidesa dla tych liczb\n");
    printf("liczby ktore podasz zostana zapisane do pliku z ktorego potem zostana wczytane do programu\n");
    printf("wynikiem dzialania programu jest nwd z n liczb oraz x i y potrzebne do wzoru: nwd(n) = x*n1 + y*n2\n");
    printf("%s", line);
}

void getdata()
{
    std::ofstream document;
    document.open(filename);

    if (document.is_open())
    {
        printf("podaj liczby dla ktorych chcesz policzyc nwd: \n");
        printf("q konczy zapisanie liczb do pliku \n");
        int numbers;
        while (std::cin >> numbers)
        {
            document << numbers << std::endl;
        }
        printf("\nliczby zostaly zapisane w pliku %s \n", filename);
    }
    else
    {
        std::cerr << "nie mozna otworzyc pliku do odczytu \n";
    }
    document.close();
}

void readdata(std::vector<int>& usernumbers)
{
    std::ifstream document;
    document.open("numbers.txt");
    int arguments;

    if (document.is_open())
    {
        while (document >> arguments)
        {
            usernumbers.push_back(arguments);
        }
    }
    else
    {
        std::cerr << "plik jest pusty \n";
    }

    printf("%s", line);
    printf("liczby ktore podales do pliku %s\n", filename);
}

void printvector(std::vector<int>& usernumbers)
{
    int i;
    int vectorsize = usernumbers.size();
    for (i = 0; i < vectorsize; i++)
    {
        printf("liczba %d: %d\n", i + 1, usernumbers.at(i));
    }
    printf("%s", line);
}

std::vector<int> extendedgcd(const std::vector<int>& usernumber) {
    int a = usernumber.at(0);
    int b = usernumber.at(1);

    int x = 0, y = 1, x_prev = 1, y_prev = 0;

    printf("Poczatkowe wartosci: \n");
    printf("a = %d, b = %d\n", a, b);
    printf("x = %d, y = %d \n", x, y);
    printf("x_prev = %d, y_prev = %d \n", x_prev, y_prev);

    while (b != 0) {
        int quotient = a / b;
        int temp = b;
        b = a % b;
        a = temp;

        int temp_x = x;
        x = x_prev - quotient * x;
        x_prev = temp_x;

        int temp_y = y;
        y = y_prev - quotient * y;
        y_prev = temp_y;

        printf("Kolejna iteracja: \n");
        printf("a = %d, b = %d \n", a, b);
        printf("x = %d, y = %d \n", x, y);
        printf("x_prev = %d, y_prev = %d \n", x_prev, y_prev);
    }

    return { x_prev, y_prev };
}

int main()
{
    std::vector<int> usernumbers;
    welcometext();
    getdata();
    readdata(usernumbers);
    printvector(usernumbers);
    extendedgcd(usernumbers);

    std::vector<int> result = extendedgcd(usernumbers);
    printf("Wspolczynniki x i y: %d, %d \n", result[0], result[1]);
    return 0;
}