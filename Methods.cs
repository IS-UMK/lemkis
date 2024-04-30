using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

namespace BasicNumTheoryPack
{
    public class Methods<T> where T : struct
    {
        public T mod { get; set; }
        public T modular_pow(T num, T exp)
        {
            if ((dynamic)mod == 1) return (dynamic)0;
            dynamic result = 1;
            num %= (dynamic)mod;

            while (exp > (dynamic)0)
            {
                if (exp % (dynamic)2 == 1)
                {
                    result = (result * num) % mod;
                }
                exp /= (dynamic)2;
                num = (num * (dynamic)num) % mod;
            }
            return result;
        }
        public T modular_inverse(T num) {
            T m = mod;
            T m0 = m;
            T y=(dynamic)0,x=(dynamic)1;
            if(m==(dynamic)1) { return (dynamic)0; }

            while (num > (dynamic)1)
            {

                T q = (dynamic)num / m;
                T t = m;

                m = (dynamic)num % m; num = t;
                t = y;


                y = x - (dynamic)q * y;
                x = t;
            }


            if (x < (dynamic)0) x += (dynamic)m0;

            return x;
        }
        public Tuple<T, T, T> ExtendedGCD(T a, T b)
        {
            if (a == (dynamic)0)
                return Tuple.Create(b, (dynamic)0, (dynamic)1);
            else
            {
                var temp = ExtendedGCD(b % (dynamic)a, a);
                int gcd = temp.Item1;
                int x = temp.Item2;
                int y = temp.Item3;
                return Tuple.Create(gcd, y - (b / (dynamic)a) * x, x);
            }
        }

        public T ModInverse(T a, T m)
        {
            var temp = ExtendedGCD(a, m);
            dynamic gcd = temp.Item1;
            dynamic x = temp.Item2;
            dynamic y = temp.Item3;
            if (gcd != 1)
                throw new Exception("Modular inverse does not exist");
            else
                return (x % m + m) % m;
        }

       public T SolveModularEquation(T a, T b)
        {
            if (GCD(a, mod) != (dynamic)1)
                return (dynamic)(-1); // No solution if a and mod are not coprime
            else
            {
                T invA = ModInverse(a, mod);
                return (invA * (dynamic)b) % mod;
            }
        }

        public T GCD(T a, T b)
        {
            while (b != (dynamic)0)
            {
                T temp = b;
                b = a % (dynamic)b;
                a = temp;
            }
            return a;
        }


        public List<T> GeneratePrimes(int up_to)
        {
            bool[] isPrime = new bool[up_to];
            List<T> primes = new List<T>();

            // Initialize all numbers as prime
            for (int i = 2; i < up_to; i++)
                isPrime[i] = true;

            // Apply the Sieve of Eratosthenes algorithm
            for (int p = 2; p * p < up_to; p++)
            {
                if (isPrime[p] == true)
                {
                    for (int i = p * p; i < up_to; i += p)
                        isPrime[i] = false;
                }
            }

            // Collect the primes into a list
            for (int p = 2; p < up_to; p++)
            {
                if (isPrime[p] == true)
                    primes.Add((dynamic)p);
            }

            return primes;
        }

        public Dictionary<T, T> DecomposeIntoPrimePowers(T number)
        {
            Dictionary<T, T> primePowers = new Dictionary<T, T>();

            // Start dividing by 2 until it's no longer divisible
            while (number % (dynamic)2 == 0)
            {
                if (primePowers.ContainsKey((dynamic)2))
                    primePowers[(dynamic)2]++;
                else
                    primePowers[(dynamic)2] = 1;
                number /= (dynamic)2;
            }

            // Try dividing by odd numbers
            for (T i = (dynamic)3; i <= Math.Sqrt((dynamic)number); i += (dynamic)2)
            {
                while (number % (dynamic)i == 0)
                {
                    if (primePowers.ContainsKey(i))
                        primePowers[(dynamic)i]++;
                    else
                        primePowers[i] = (dynamic)1;
                    number /= (dynamic)i;
                }
            }

            // If number is still greater than 2, it must be a prime number
            if (number > (dynamic)2)
            {
                if (primePowers.ContainsKey(number))
                    primePowers[(dynamic)number]++;
                else
                    primePowers[number] = (dynamic)1;
            }

            return primePowers;
        }
        public T EulerTotient(T n)
        {
            T result = n; // Initialize result as n

            // Consider all prime factors of n and subtract their multiples from result
            for (int p = 2; p * p <= (dynamic)n; ++p)
            {
                // Check if p is a prime factor of n
                if (n % (dynamic)p == 0)
                {
                    // Update result to exclude multiples of p
                    while (n % (dynamic)p == 0)
                        n /= (dynamic)p;
                    result -= result / (dynamic)p;
                }
            }

            // If n has a prime factor greater than sqrt(n), add it to result
            if (n > (dynamic)1)
                result -= result / (dynamic)n;

            return result;
        }

        public T LargestPowerOfPrimeDividing(T value, T prime)
        {
            T power = (dynamic)0;

            // Keep dividing value by prime until it's no longer divisible
            while (value % (dynamic)prime == 0)
            {
                power=power+(dynamic)1;
                value /= (dynamic)prime;
            }

            return power;
        }

        public T SolveLinearSystem(T[] asArray, T[] bs, T[] mods)
        {
            T n = (dynamic)asArray.Length;
            T M = (dynamic)1;
            for (int i = 0; i < (dynamic)n; i++)
                M *= (dynamic)mods[i];

            T result = (dynamic)0;
            for (int i = 0; i < (dynamic)n; i++)
            {
                T Mi = M / (dynamic)mods[i];
                T yi = ModInverse(Mi, mods[i]);
                result = (result + asArray[i] * (dynamic)bs[i] * yi) % M;
            }

            return result < (dynamic)0 ? result + (dynamic)M : result;
        }

    }

}
