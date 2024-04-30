using NUnit.Framework;
using NUnit.Framework.Legacy;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BasicNumTheoryPack.UnitTest
{
    internal class MethodsTests
    {
        [Test]
        public void Modular_Pow_return() 
        {
            //arange
            var methods = new Methods<int>();
            methods.mod = 3;

            //act
            var modular_pow = methods.modular_pow(2,4);

            //assert
            ClassicAssert.AreEqual(1, modular_pow);
        }

        [Test]
        public void Modular_inverse_return()
        {
            //arange
            var methods = new Methods<int>();
            methods.mod = 3;

            //act
            var modular_inv = methods.modular_inverse(2);

            //assert
            ClassicAssert.AreEqual(2, modular_inv);
        }

        [Test]
        public void SolveMod_return()
        {
            //arange
            var methods = new Methods<int>();
            methods.mod = 3;

            //act
            var solve = methods.SolveModularEquation(1,2);

            //assert
            ClassicAssert.AreEqual(2, solve);
        }

        [Test]
        public void Generate_Primes_return()
        {
            //arange
            var methods = new Methods<int>();
            methods.mod = 3;

            //act
            var primes = methods.GeneratePrimes(11);

            //assert
            List<int> primes_list = new List<int>() {2,3,5,7};
            ClassicAssert.AreEqual(primes_list, primes);
        }

        [Test]
        public void Decompose_return()
        {
            //arange
            var methods = new Methods<int>();
            methods.mod = 3;

            //act
            var primes = methods.DecomposeIntoPrimePowers(24);

            //assert
            Dictionary<int,int> dict= new Dictionary<int, int>() { {2,3},{3,1} };
            ClassicAssert.AreEqual(dict, primes);
        }

        [Test]
        public void Totient_return()
        {
            //arange
            var methods = new Methods<int>();
            methods.mod = 3;

            //act
            var primes = methods.EulerTotient(2);

            //assert
            ClassicAssert.AreEqual(1, primes);
        }

        [Test]
        public void Largest_return()
        {
            //arange
            var methods = new Methods<int>();
            methods.mod = 3;

            //act
            var primes = methods.LargestPowerOfPrimeDividing(16,2);

            //assert
            ClassicAssert.AreEqual(4, primes);
        }

        [Test]
        public void Linear_return()
        {
            //arange
            var methods = new Methods<int>();
            methods.mod = 3;

            //act
            var primes = methods.SolveLinearSystem([2, 3], [1, 2], [5, 7]);

            //assert
            ClassicAssert.AreEqual(24, primes);
        }
    }
}
