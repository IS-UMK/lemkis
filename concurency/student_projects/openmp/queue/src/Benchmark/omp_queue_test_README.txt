OMPQueue vs ConcurrentQueue
    : Test Wydajno�ci Testuje wydajno�� trzech implementacji kolejek w
          r�nych konfiguracjach liczby producent�w i konsument�w,
      w celu por�wnania czas�w operacji push i pop w r�nych wersjach
    : bez OpenMP,
      z OpenMP,
      oraz z std::jthread.

          Opis
          OMPQueue(No OpenMP) -
          Kolejka OMPQueue,
      z sekwencyjnymi operacjami push i pop
          .Wszystkie operacje s� blokowane za pomoc� blokady omp_lock_t,
      co pozwala na synchronizacj� w�tk�w,
      ale nie wykorzystuje r�wnoleg�o�ci.

          OMPQueue(With OpenMP) -
          Kolejka OMPQueue,
      dzia�aj�ca z wykorzystaniem OpenMP.Operacje push i pop s� wykonywane
      r�wnolegle przez wielu producent�w i konsument�w,
      co pozwala na lepsz� skalowalno�� w przypadku wi�kszej liczby w�tk�w
              .Synchronizacja dost�pu
          do kolejki jest realizowana za pomoc� mechanizmu
          OpenMP(omp_set_lock i omp_unset_lock)
              .

          ConcurrentQueue(With std::jthread) -
          Kolejka ConcurrentQueue,
      z u�yciem std::jthread do r�wnoleg�ego zarz�dzania producentami i
      konsumentami.Wykorzystuje mechanizm std::mutex do synchronizacji dost�pu
      do kolejki,
      co pozwala na bezpieczne manipulowanie danymi w �rodowisku wielow�tkowym.

      Testy Ka�dy test jest wykonywany dla r�nych
      konfiguracji liczby producent�w i konsument�w,
      a wyniki obejmuj� czas wykonania operacji oraz liczb� przetworzonych
          element�w.

          No OpenMP(Bez r�wnoleg�o�ci) -
          Testuje przypadek,
      w kt�rym wszystkie operacje s� wykonywane sekwencyjnie bez OpenMP.

          Z OpenMP(R�wnolegle z OpenMP) -
          Testuje przypadek,
      w kt�rym operacje s� r�wnolegle wykonywane za pomoc� OpenMP,
      co umo�liwia
          przyspieszenie w przypadku wi�kszej liczby producent�w i konsument�w.

          Z std::jthread(R�wnolegle z std::jthread) -
          Testuje r�wnoleg�e zarz�dzanie producentami i
          konsumentami za pomoc� std::jthread,
      w celu por�wnania z tradycyjnymi podej�ciami OpenMP.