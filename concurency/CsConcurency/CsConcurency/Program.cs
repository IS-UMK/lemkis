//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                          //                  Task                     //                     Parallel                        //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Usage                    //  Asynchronous (I/O-bound), API operations //  CPU-bound loops/workloads                          //
// Supports await           //  YES                                      //  NO                                                 //
// Supports cancellation    //  YES with cancellation token              //  YES with cancellation token                        //    
// Supports return values   //  YES using Task<T>                        //  NO                                                 //
// Control and flexibility  //  High - supports delays continuation etc. //  Limited - mainly useful for loop-based scenarios   //                           
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using CsConcurency;

var exampleManager = new ExampleManager();

while (true)
{
    exampleManager.PrintMenu();

    var input = Console.ReadLine();
    if (string.IsNullOrEmpty(input))
        break;

    input = input.Trim();

    if (input.Equals("exit", StringComparison.OrdinalIgnoreCase))
        break;

    await exampleManager.RunExample(input);

    Console.WriteLine();
}