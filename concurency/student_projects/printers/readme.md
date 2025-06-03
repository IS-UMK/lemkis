### Opis

W budynku biurowym jest **N firm**. Jest również **0 < K ≤ N drukarek**, z których korzystają pracownicy tych firm. Zasady korzystania z drukarek są następujące:

Pracownicy różnych firm **nigdy nie używają tej samej drukarki jednocześnie** (zakładamy, że istnieje procedura `print()`, której wywołanie oznacza korzystanie z drukarki: wysłanie dokumentu, oczekiwanie na wydrukowanie i odebranie dokumentu z drukarki).

Pracownik danej firmy **nie może rozpocząć drukowania**, dopóki nie będzie pewien, że drukarka, do której chce wysłać dokument, **nie jest używana przez pracowników innych firm**. Pracownicy **tej samej firmy mogą korzystać tylko z jednej drukarki na raz**. Jeśli jest wiele wolnych drukarek, pracownik, którego firma aktualnie nie drukuje, **może wybrać dowolną z nich** (nie zależy nam na równomiernym wykorzystaniu drukarek).

---

### Zasady:

1. **Pracownicy różnych firm nie mogą korzystać z tej samej drukarki w tym samym czasie.**  
   Oznacza to, że jeżeli firma 0 korzysta z drukarki 0, to firma 1 nie może korzystać z tej drukarki w tym samym czasie. Firma 1 będzie mogła korzystać z tej drukarki dopiero po tym, jak firma 0 ją zwolni → `usage_count == 0`.

2. **Pracownik danej firmy nie może rozpocząć drukowania, dopóki nie ma pewności, że wybrana drukarka jest przydzielona do jego firmy** — czyli nie jest używana przez pracowników innej firmy.

3. **Pracownicy tej samej firmy mogą korzystać z tej samej drukarki w tym samym czasie.**

4. **Jeśli jest wiele wolnych drukarek, pracownik może wybrać dowolną z nich, pod warunkiem że jego firma nie korzysta już z innej drukarki.**
