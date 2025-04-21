OMPQueue vs ConcurrentQueue: Test Wydajnoœci
Testuje wydajnoœæ trzech implementacji kolejek w ró¿nych konfiguracjach liczby producentów i konsumentów, w celu porównania czasów operacji push i pop w ró¿nych wersjach: bez OpenMP, z OpenMP, oraz z std::jthread.

Opis
OMPQueue (No OpenMP) - Kolejka OMPQueue, dzia³aj¹ca bez wykorzystania OpenMP, z sekwencyjnymi operacjami push i pop. Wszystkie operacje s¹ blokowane za pomoc¹ blokady omp_lock_t, co pozwala na synchronizacjê w¹tków, ale nie wykorzystuje równoleg³oœci.

OMPQueue (With OpenMP) - Kolejka OMPQueue, dzia³aj¹ca z wykorzystaniem OpenMP. Operacje push i pop s¹ wykonywane równolegle przez wielu producentów i konsumentów, co pozwala na lepsz¹ skalowalnoœæ w przypadku wiêkszej liczby w¹tków. Synchronizacja dostêpu do kolejki jest realizowana za pomoc¹ mechanizmu OpenMP (omp_set_lock i omp_unset_lock).

ConcurrentQueue (With std::jthread) - Kolejka ConcurrentQueue, z u¿yciem std::jthread do równoleg³ego zarz¹dzania producentami i konsumentami. Wykorzystuje mechanizm std::mutex do synchronizacji dostêpu do kolejki, co pozwala na bezpieczne manipulowanie danymi w œrodowisku wielow¹tkowym.

Testy
Ka¿dy test jest wykonywany dla ró¿nych konfiguracji liczby producentów i konsumentów, a wyniki obejmuj¹ czas wykonania operacji oraz liczbê przetworzonych elementów.

No OpenMP (Bez równoleg³oœci) - Testuje przypadek, w którym wszystkie operacje s¹ wykonywane sekwencyjnie bez OpenMP.

Z OpenMP (Równolegle z OpenMP) - Testuje przypadek, w którym operacje s¹ równolegle wykonywane za pomoc¹ OpenMP, co umo¿liwia przyspieszenie w przypadku wiêkszej liczby producentów i konsumentów.

Z std::jthread (Równolegle z std::jthread) - Testuje równoleg³e zarz¹dzanie producentami i konsumentami za pomoc¹ std::jthread, w celu porównania z tradycyjnymi podejœciami OpenMP.