# Ciastkarnia
## Informacje techniczne
Program stanowi projekt na zaliczenie przedmiotu Systemy Operacyjne. Celem projektu jest zaimplementowanie bezpiecznego i zoptymalizowanego systemu komunikacji wieloprocesowej w środowisku Linux z wykorzystaniem mechanizmów IPC Systemu V.
- **Wersja systemu operacyjnego**: Ubuntu 22.04.5 LTS (Jammy Jellyfish)
- **Wersja kompilatora**: gcc version 11.4.0 (Ubuntu 11.4.0-1ubuntu1~22.04.2)
- **Wersja kernel**: 6.8.0-90-generic

Uruchomienie:
```console
foo@bar:~$ chmod +x run.sh
foo@bar:~$ ./run.sh
```

## Założenia projektowe kodu
- Do tworzenia procesów użyto funkcji fork() i exec() - dzięki temu program jest zdecentralizowany
- Do komunikacji międzyprocesowej zostały wykorzystane mechanizmy IPC Systemu V - Te mechanizmy zostały zbudowane w większości w trybie blokującym, celem optymalizacji procesów (Chyba, że wymagane było użycie trybu nieblokującego)
- Program unika tzw. "Busy-waiting'u" i osiąga to za pomocą sygnałów i blokujących mechanizmów IPC

## Ogólny opis kodu
### Kierownik
- Proces nadrzędny, odpowiedzialny za tworzenie i czyszczenie struktur IPC 
- Tworzy i nadzoruje procesy potomne
- Odpowiada za wygenerowanie raportu przy sygnale inwentaryzacji (SIGUSR1)
- Zarządza ewakuacją podczas sygnału (SIGUSR2)
- Prowadzi cykl dnia
- Generuje procesy klientów
- Zarządza procesami zombie (SIG_IGN na SIGCHLD)

### Piekarz
- Proces łączy się ze strukturami IPC
- Generuje osobny wątek dla każdego produktu
- Dodaje losowe ilości produktów w losowych odstępach czasowych
- Prowadzi statystykę wypieczonych produktów
- Reaguje na ewakuacje

### Klient
- Proces łączy się ze strukturami IPC
- Decyduje, czy może wejść
- Zabiera produkty z podajników (Jeżeli podajnik nie jest pusty)
- Przesyła listę zabranych produktów do kasjera
- Oczekuje na potwierdzenie wystawienia paragonu od kasjera
- Wybiera kasę, jeżeli druga jest otwarta
- Reaguje na ewakuacje

### Kasjer
- Proces łączy się ze strukturami IPC
- Odbiera listę zakupów od klienta
- Przesyła potwierdzenie skasowania produktów do klienta
- Generuje paragon i zapisuje go do pliku
- Reaguje na ewakuacje

## Wykorzystane mechanizmy

#### Tworzenie i obsługa plików
- [Tworzenie plików](https://github.com/Krisuniooo/SO_projekt/blob/9ae1b39fe100dad9cc8a22561113f18a48eae500/src/Utils.cpp#L4-L12)
- [Otwieranie plików](https://github.com/Krisuniooo/SO_projekt/blob/9ae1b39fe100dad9cc8a22561113f18a48eae500/src/PROCESS/Cashier.cpp#L24-L28)
- [Zapisywanie do plików](https://github.com/Krisuniooo/SO_projekt/blob/e488c0538f2b9f30c145ba7c099a5d5de8310ee6/src/PROCESS/Cashier.cpp#L50-L76)
- [Zamykanie plików](https://github.com/Krisuniooo/SO_projekt/blob/9ae1b39fe100dad9cc8a22561113f18a48eae500/src/PROCESS/Cashier.cpp#L166)

#### Obsługa procesów
- [Pełny przykład (fork, exec i exit)](https://github.com/Krisuniooo/SO_projekt/blob/e488c0538f2b9f30c145ba7c099a5d5de8310ee6/src/PROCESS/Manager.cpp#L170-L190)
#### Obsługa wątków
- [Tworzenie wątku](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/PROCESS/Baker.cpp#L108-L113)
- [Czekanie na zakończenie wątku](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/PROCESS/Baker.cpp#L119-L122)
- [pthread mutex](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/PROCESS/Manager.cpp#L66-L70)
#### Obsługa sygnałów
- [signal](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/PROCESS/Manager.cpp#L354-L359)
- [kill](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/PROCESS/Manager.cpp#L224-L228)
- [pause](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/PROCESS/Manager.cpp#L466)
- [customowe library](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/Signals.cpp#L7-L38)
#### Synchronizacja procesów
- [customowe library](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/IPC/Semaphore.cpp#L7-L150)
- [przykład](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/PROCESS/Baker.cpp#L27-L80)
#### Pamięć dzielona
- [customowe library](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/IPC/SharedMemory.cpp#L1-L116)
- [przykład](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/PROCESS/Client.cpp#L146-L151)
#### Kolejki komunikatów
- [customowe library](https://github.com/Krisuniooo/SO_projekt/blob/main/src/IPC/MessageQueue.cpp)
- [pobieranie id kolejki z uprawnieniami](https://github.com/Krisuniooo/SO_projekt/blob/e488c0538f2b9f30c145ba7c099a5d5de8310ee6/src/PROCESS/Cashier.cpp#L14-L15)
- [msgsnd](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/PROCESS/Cashier.cpp#L143-L148)
- [msgrcv](https://github.com/Krisuniooo/SO_projekt/blob/fcc070e562912c5e0e01d08baf76c3edfc62aa60/src/PROCESS/Cashier.cpp#L34-L47)

## Testy
### 1. Test sprawdzający działanie sygnału inwentaryzacji
Wywołanie tego testu następuje poprzez wysłanie sygnału SIGUSR1 do kierownika ciastkarni. Należy go wysłać w trakcie trwania cyklu dnia, aby potwierdzić, że raport generuje się po obsłużeniu wszystkich klientów. Użycie:
```console
foo@bar:~$ kill -SIGUSR1 <PID_KIEROWNIKA>
```
Wynik:
```
=== MANAGER RAPORT CREATED AT [2026-02-01 23:32:43] ===

BAKER - STATS:
produced 3948 of WZ-ka pieces
produced 4101 of Kremowka pieces
produced 3815 of Piegusek pieces
produced 3867 of Brownie pieces
produced 3970 of Chocolate Chip pieces
produced 3993 of Coconut cookie pieces
produced 3961 of Chocolate Crinkles pieces
produced 3960 of Amaretti pieces
produced 3965 of Piernik pieces
produced 3983 of Dubai Chocolate pieces

MANAGER - LEFT ON TRAYS:
left 84 WZ-ka pieces on tray
left 75 Kremowka pieces on tray
left 66 Piegusek pieces on tray
left 87 Brownie pieces on tray
left 58 Chocolate Chip pieces on tray
left 91 Coconut cookie pieces on tray
left 84 Chocolate Crinkles pieces on tray
left 121 Amaretti pieces on tray
left 84 Piernik pieces on tray
left 55 Dubai Chocolate pieces on tray

MANAGER - TOTAL TRASHED:
trashed 16 WZ-ka
trashed 17 Kremowka
trashed 16 Piegusek
trashed 19 Brownie
trashed 26 Chocolate Chip
trashed 14 Coconut cookie
trashed 14 Chocolate Crinkles
trashed 9 Amaretti
trashed 13 Piernik
trashed 13 Dubai Chocolate

CASHIER - STATS:
sold 3848 of WZ-ka pieces, total value of 21164.00$
sold 4009 of Kremowka pieces, total value of 34076.50$
sold 3733 of Piegusek pieces, total value of 9332.50$
sold 3761 of Brownie pieces, total value of 38550.25$
sold 3886 of Chocolate Chip pieces, total value of 17487.00$
sold 3888 of Coconut cookie pieces, total value of 13608.00$
sold 3863 of Chocolate Crinkles pieces, total value of 40561.50$
sold 3830 of Amaretti pieces, total value of 24895.00$
sold 3868 of Piernik pieces, total value of 26109.00$
sold 3915 of Dubai Chocolate pieces, total value of 32298.75$
```
Sprawdzenie dla pierwszych trzech produktów:
- WZ-ka: 3848 sprzedanych + 16 wyrzuconych + 84 na podajniku = 3948 (tyle ile wyprodukowano) ✅
- Kremowka: 4009 sprzedanych + 17 wyrzuconych + 75 na podajniku = 4101 (tyle ile wyprodukowano) ✅
- Piegusek: 3733 sprzedanych + 16 wyrzuconych + 66 na podajniku = 3815 (tyle ile wyprodukowano) ✅

Test przeprowadzony prawidłowo ✅
### 2. Test sprawdzający działanie sygnału ewakuacji
Wywołanie tego testu następuje poprzez wysłanie sygnału SIGUSR2 do kierownika ciastkarni. Użycie:
```console
foo@bar:~$ kill -SIGUSR2 <PID_KIEROWNIKA>
```
Wynik:
```
[2026-02-02 01:10:47] EVACUATION IN PROGRESS
[2026-02-02 01:10:47] Client 901910 goes away - shop closed
[2026-02-02 01:10:47] Client 901907 goes away - shop closed
[2026-02-02 01:10:47] Client 901905 goes away - shop closed
[2026-02-02 01:10:47] Client 901912 goes away - shop closed
[2026-02-02 01:10:47] Client 901906 goes away - shop closed
[2026-02-02 01:10:47] Client 901908 goes away - shop closed
[2026-02-02 01:10:47] Client 901902 goes away - shop closed
[2026-02-02 01:10:47] Client 901901 goes away - shop closed
[2026-02-02 01:10:47] Client 901898 goes away - shop closed
[2026-02-02 01:10:47] Client 901899 goes away - shop closed
[2026-02-02 01:10:47] Client 901894 goes away - shop closed
[2026-02-02 01:10:47] Client 901893 goes away - shop closed
[2026-02-02 01:10:47] Client 901900 goes away - shop closed
[2026-02-02 01:10:47] Client 901911 goes away - shop closed
[2026-02-02 01:10:47] Client 901909 goes away - shop closed
[2026-02-02 01:10:47] Client 901903 goes away - shop closed
[2026-02-02 01:10:47] Client 901895 goes away - shop closed
[2026-02-02 01:10:47] Client 901885 goes away - shop closed
[2026-02-02 01:10:47] Client 901890 goes away - shop closed
[2026-02-02 01:10:47] Client 901892 goes away - shop closed
[2026-02-02 01:10:47] Client 901897 goes away - shop closed
[2026-02-02 01:10:47] Client 901887 goes away - shop closed
[2026-02-02 01:10:47] Client 901883 goes away - shop closed
[2026-02-02 01:10:47] Client 901904 goes away - shop closed
[2026-02-02 01:10:47] Client 901889 goes away - shop closed
[2026-02-02 01:10:47] Client 901891 goes away - shop closed
[2026-02-02 01:10:47] Client 901880 skips waiting for checkout
[2026-02-02 01:10:47] Client 901878 skips waiting for checkout
[2026-02-02 01:10:47] Client 901888 goes away - shop closed
[2026-02-02 01:10:47] Client 901896 goes away - shop closed
[2026-02-02 01:10:47] Client 901876 skips waiting for checkout
[2026-02-02 01:10:47] Client 901867 skips waiting for checkout
[2026-02-02 01:10:47] Client 901865 skips waiting for checkout
[2026-02-02 01:10:47] Client 901882 skips waiting for checkout
[2026-02-02 01:10:47] Client 901884 goes away - shop closed
[2026-02-02 01:10:47] Client 901881 skips waiting for checkout
[2026-02-02 01:10:47] Client 901879 skips waiting for checkout
[2026-02-02 01:10:47] Client 901877 skips waiting for checkout
[2026-02-02 01:10:47] Client 901861 skips waiting for checkout
[2026-02-02 01:10:47] Client 901858 skips waiting for checkout
[2026-02-02 01:10:47] Client 901886 goes away - shop closed
[2026-02-02 01:10:47] Client 901863 skips waiting for checkout
[2026-02-02 01:10:47] Client 901875 skips waiting for checkout
[2026-02-02 01:10:47] Client 901866 skips waiting for checkout
[2026-02-02 01:10:47] Client 901864 skips waiting for checkout
[2026-02-02 01:10:47] Client 901859 skips waiting for checkout
[2026-02-02 01:10:47] Client 901857 skips waiting for checkout
[2026-02-02 01:10:47] Client 901862 skips waiting for checkout
[2026-02-02 01:10:47] Client 901860 skips waiting for checkout
[2026-02-02 01:10:47] Client 901856 skips waiting for checkout
[2026-02-02 01:10:47] Client 901880 trashed 2 Kremowka
[2026-02-02 01:10:47] Client 901878 trashed 4 Chocolate Chip
[2026-02-02 01:10:47] Client 901876 trashed 1 Kremowka
[2026-02-02 01:10:47] Client 901867 trashed 2 WZ-ka
[2026-02-02 01:10:47] Client 901865 trashed 2 Dubai Chocolate
[2026-02-02 01:10:47] Client 901865 goes away - ended shopping
[2026-02-02 01:10:47] Client 901882 trashed 2 WZ-ka
[2026-02-02 01:10:47] Client 901881 trashed 2 Piernik
[2026-02-02 01:10:47] Client 901881 goes away - ended shopping
[2026-02-02 01:10:47] Client 901879 trashed 1 WZ-ka
[2026-02-02 01:10:47] Client 901877 trashed 1 Coconut cookie
[2026-02-02 01:10:47] Client 901877 goes away - ended shopping
[2026-02-02 01:10:47] Client 901861 trashed 4 Piegusek
[2026-02-02 01:10:47] Client 901858 trashed 1 Piegusek
[2026-02-02 01:10:47] Client 901863 trashed 2 WZ-ka
[2026-02-02 01:10:47] Client 901875 trashed 1 Kremowka
[2026-02-02 01:10:47] Client 901866 trashed 1 WZ-ka
[2026-02-02 01:10:47] Client 901864 trashed 2 Kremowka
[2026-02-02 01:10:47] Client 901859 trashed 4 WZ-ka
[2026-02-02 01:10:47] Client 901857 trashed 4 WZ-ka
[2026-02-02 01:10:47] Client 901862 trashed 2 WZ-ka
[2026-02-02 01:10:47] Client 901860 trashed 4 Piegusek
[2026-02-02 01:10:47] Client 901860 goes away - ended shopping
[2026-02-02 01:10:47] Client 901856 trashed 2 Piegusek
[2026-02-02 01:10:47] Client 901880 trashed 2 Dubai Chocolate
[2026-02-02 01:10:47] Client 901880 goes away - ended shopping
[2026-02-02 01:10:47] Client 901878 trashed 2 Chocolate Crinkles
[2026-02-02 01:10:47] Client 901878 goes away - ended shopping
[2026-02-02 01:10:47] Client 901876 trashed 1 Piegusek
[2026-02-02 01:10:47] Client 901867 trashed 1 Chocolate Chip
[2026-02-02 01:10:47] Client 901882 trashed 1 Piegusek
[2026-02-02 01:10:47] Client 901879 trashed 2 Piegusek
[2026-02-02 01:10:47] Client 901861 trashed 2 Coconut cookie
[2026-02-02 01:10:47] Client 901861 goes away - ended shopping
[2026-02-02 01:10:47] Client 901858 trashed 3 Brownie
[2026-02-02 01:10:47] Client 901863 trashed 1 Kremowka
[2026-02-02 01:10:47] Client 901875 trashed 1 Piegusek
[2026-02-02 01:10:47] Client 901866 trashed 2 Coconut cookie
[2026-02-02 01:10:47] Client 901866 goes away - ended shopping
[2026-02-02 01:10:47] Client 901864 trashed 2 Piegusek
[2026-02-02 01:10:47] Client 901859 trashed 3 Kremowka
[2026-02-02 01:10:47] Client 901857 trashed 4 Kremowka
[2026-02-02 01:10:47] Client 901862 trashed 4 Kremowka
[2026-02-02 01:10:47] Client 901856 trashed 1 Brownie
[2026-02-02 01:10:47] Client 901856 goes away - ended shopping
[2026-02-02 01:10:47] Client 901876 trashed 2 Brownie
[2026-02-02 01:10:47] Client 901867 trashed 1 Coconut cookie
[2026-02-02 01:10:47] Client 901882 trashed 1 Brownie
[2026-02-02 01:10:47] Client 901879 trashed 1 Brownie
[2026-02-02 01:10:47] Client 901858 trashed 4 Amaretti
[2026-02-02 01:10:47] Client 901863 trashed 3 Piegusek
[2026-02-02 01:10:47] Client 901875 trashed 1 Brownie
[2026-02-02 01:10:47] Client 901864 trashed 2 Chocolate Crinkles
[2026-02-02 01:10:47] Client 901859 trashed 1 Piegusek
[2026-02-02 01:10:47] Client 901857 trashed 2 Piegusek
[2026-02-02 01:10:47] Client 901862 trashed 2 Piegusek
[2026-02-02 01:10:47] Client 901876 trashed 4 Chocolate Chip
[2026-02-02 01:10:47] Client 901867 trashed 4 Chocolate Crinkles
[2026-02-02 01:10:47] Client 901882 trashed 3 Chocolate Chip
[2026-02-02 01:10:47] Client 901882 goes away - ended shopping
[2026-02-02 01:10:47] Client 901879 trashed 2 Coconut cookie
[2026-02-02 01:10:47] Client 901858 trashed 3 Dubai Chocolate
[2026-02-02 01:10:47] Client 901858 goes away - ended shopping
[2026-02-02 01:10:47] Client 901863 trashed 2 Chocolate Chip
[2026-02-02 01:10:47] Client 901875 trashed 3 Chocolate Chip
[2026-02-02 01:10:47] Client 901864 trashed 2 Dubai Chocolate
[2026-02-02 01:10:47] Client 901864 goes away - ended shopping
[2026-02-02 01:10:47] Client 901859 trashed 4 Chocolate Chip
[2026-02-02 01:10:47] Client 901857 trashed 4 Brownie
[2026-02-02 01:10:47] Client 901862 trashed 1 Chocolate Chip
[2026-02-02 01:10:47] Client 901876 trashed 2 Coconut cookie
[2026-02-02 01:10:47] Client 901867 trashed 2 Amaretti
[2026-02-02 01:10:47] Client 901879 trashed 2 Chocolate Crinkles
[2026-02-02 01:10:47] Client 901863 trashed 1 Coconut cookie
[2026-02-02 01:10:47] Client 901875 trashed 4 Coconut cookie
[2026-02-02 01:10:47] Client 901859 trashed 2 Coconut cookie
[2026-02-02 01:10:47] Client 901857 trashed 3 Chocolate Crinkles
[2026-02-02 01:10:47] Client 901862 trashed 1 Coconut cookie
[2026-02-02 01:10:47] Client 901876 trashed 1 Amaretti
[2026-02-02 01:10:47] Client 901876 goes away - ended shopping
[2026-02-02 01:10:47] Client 901867 trashed 1 Dubai Chocolate
[2026-02-02 01:10:47] Client 901867 goes away - ended shopping
[2026-02-02 01:10:47] Client 901879 trashed 1 Amaretti
[2026-02-02 01:10:47] Client 901863 trashed 2 Chocolate Crinkles
[2026-02-02 01:10:47] Client 901875 trashed 2 Chocolate Crinkles
[2026-02-02 01:10:47] Client 901859 trashed 2 Amaretti
[2026-02-02 01:10:47] Client 901857 trashed 1 Amaretti
[2026-02-02 01:10:47] Client 901862 trashed 2 Chocolate Crinkles
[2026-02-02 01:10:47] Client 901879 trashed 1 Piernik
[2026-02-02 01:10:47] Client 901879 goes away - ended shopping
[2026-02-02 01:10:47] Client 901863 trashed 3 Amaretti
[2026-02-02 01:10:47] Client 901875 trashed 1 Amaretti
[2026-02-02 01:10:47] Client 901875 goes away - ended shopping
[2026-02-02 01:10:47] Client 901859 trashed 4 Piernik
[2026-02-02 01:10:47] Client 901857 trashed 4 Piernik
[2026-02-02 01:10:47] Client 901862 trashed 1 Amaretti
[2026-02-02 01:10:47] Client 901863 trashed 4 Dubai Chocolate
[2026-02-02 01:10:47] Client 901863 goes away - ended shopping
[2026-02-02 01:10:47] Client 901859 trashed 1 Dubai Chocolate
[2026-02-02 01:10:47] Client 901859 goes away - ended shopping
[2026-02-02 01:10:47] Client 901857 trashed 1 Dubai Chocolate
[2026-02-02 01:10:47] Client 901857 goes away - ended shopping
[2026-02-02 01:10:47] Client 901862 trashed 1 Dubai Chocolate
[2026-02-02 01:10:47] Client 901862 goes away - ended shopping
```
Klienci wyszli bez oczekiwania na paragon oraz odłożyli produkty do kosza ✅
### 3. Test obciążeniowy
Do wykonania tego testu musimy odpowiednio przygotować plik konfiguracyjny (config.h), w którym należy ustawić następujące opcje umożliwiające nam zwiększyć ilość generowanych klientów. Dodatkowo możemy dorzucić sobie sleepa w kasjerze, aby troche spowolnić klienta w sklepie
```c++
#define MAX_PROCESSES 5000
...
#define CUSTOMER_SPAWN_MIN_TIME 0.000f 
#define CUSTOMER_SPAWN_MAX_TIME 0.000f
```
![obraz](https://i.imgur.com/bQnvW3m.png)

[link alternatywny do obrazu](https://i.imgur.com/bQnvW3m.png)
Na screenie z htopa widać, że utworzono 5000 procesów, które usypiają na semaforze, dzięki czemu pomimo mamy niskie zużycie procesora ✅
### 4. Test sprawdzający czy klient ominie tacke, jezeli nie ma produktu
Klient powinien spróbować wziąć produkt ale jeżeli tacka jest pusta to go ominąć - w tym przypadku klient zauważa, że jedynym produktem na tackach z tych, które chce jest Dubai Chcocolate, reszte z pomimo braków pomija
```
[2026-02-02 09:56:22] Client 112525 tries to enter the shop - currently clients inside: 14/20
[2026-02-02 09:56:22] Client 112525 enters shop
[2026-02-02 09:56:22] Client 112525 tries to take product Coconut cookie if possible
[2026-02-02 09:56:22] Client 112525 could not take Coconut cookie goes to next
[2026-02-02 09:56:22] Client 112525 tries to take product WZ-ka if possible
[2026-02-02 09:56:22] Client 112525 could not take WZ-ka goes to next
[2026-02-02 09:56:22] Client 112525 tries to take product Piegusek if possible
[2026-02-02 09:56:22] Client 112525 could not take Piegusek goes to next
[2026-02-02 09:56:22] Client 112525 tries to take product Chocolate Chip if possible
[2026-02-02 09:56:22] Client 112525 could not take Chocolate Chip goes to next
[2026-02-02 09:56:22] Client 112525 tries to take product Dubai Chocolate if possible
[2026-02-02 09:56:22] Client 112525 is locking product Dubai Chocolate mutex
[2026-02-02 09:56:22] Client 112525 is locking shared data mutex
[2026-02-02 09:56:22] Client 112525 tries to take product Dubai Chocolate if possible
[2026-02-02 09:56:22] Client 112525 is locking product Dubai Chocolate mutex
[2026-02-02 09:56:22] Client 112525 is locking shared data mutex
[2026-02-02 09:56:22] Client 112525 tries to take product Dubai Chocolate if possible
[2026-02-02 09:56:22] Client 112525 could not take Dubai Chocolate goes to next
[2026-02-02 09:56:22] Client 112525 tries to take product Kremowka if possible
[2026-02-02 09:56:22] Client 112525 could not take Kremowka goes to next
[2026-02-02 09:56:22] Client 112525 tries to take product Piernik if possible
[2026-02-02 09:56:22] Client 112525 could not take Piernik goes to next
[2026-02-02 09:56:22] Client 112525 tries to take product Amaretti if possible
[2026-02-02 09:56:22] Client 112525 could not take Amaretti goes to next
[2026-02-02 09:56:22] Client 112525 joins queue to cashier 2 at 5 place
[2026-02-02 09:56:22] Client 112525 gives product list to 2
[2026-02-02 09:56:22] Client 112525 is waiting for checkout
```
```
==================================
    RECEIPT - CLIENT 112525
==================================
Dubai Chocolate 2 - 16.50
==================================
TOTAL: 16.50$
==================================
```
Kasjer poprawnie skasował jedyne dwa produkty, które chciał klient i mógł wziąć klient ✅
### 5. Test sprawdzający prawidłowe czyszczenie zasobów
Test polega na przerwaniu działania programu w dowolnym momencie (np. poprzez SIGINT) i sprawdzeniu czy struktury IPC sie wyczyscily 
```console
foo@bar:~$ ipcs
```
Oczekiwany wynik:
![image](https://i.imgur.com/ugE5Y4K.png)

[alternatywny link](https://i.imgur.com/ugE5Y4K.png)
Struktury zostały wyczyszczone poprawnie ✅

## Napotkane problemy
- Zastosowano Ring na pamięci dzielonej zamiast kolejek fifo - kolejka mkfifo generowała różne problemy w systemie ubuntu, takie jak np. ignorowanie i przekraczanie limitu PIPE_BUF. Ponadto z powodów natury mkfifo file descriptory się zamykały w przypadku braku otwartego file descriptora z drugiej strony (np. wszyscy klienci zostali obsłużeni, a cykl dnia dalej trwa), natomiast proste rozwiązania jak postawienie strażnika psuły optymalizacje i zabierały sens rozwiązaniom nieblokującym - teoretycznie dałoby się to rozwiązać, jednak dużo prostszym podejściem było zastosowanie ringa na shared memory opartego na 2 dodatkowych semaforach dla każdego produktu.
## Kluczowe pseudokody
- Dodanie produktu przez piekarza
```
LOSUJ liczba_wypieczonego_produktu
DLA KAŻDEGO liczba_wypieczonego_produktu ZRÓB:
	CZEKAJ I ZABLOKUJ WOLNE MIEJSCA PRODUKTU

	JEŚLI ewakuacja:
		ODBLOKUJ wolne_miejsca_produktu
		WYJDZ

	CZEKAJ I ZABLOKUJ TACE

	JEŚLI ewakuacja
		ODBLOKUJ TACE
		ODBLOKUJ wolne_miejsca_produktu
		WYJDZ

	CZEKAJ I ZABLOKUJ DANE DZIELONE
	
	JEŚLI ewakuacja
		ODBLOKUJ DANE DZIELONE
		ODBLOKUJ TACE
		ODBLOKUJ wolne_miejsca_produktu
		WYJDZ

	UTWORZ PRODUKT I NADAJ ATRYBUTY
	DODAJ PRODUKT NA KONIEC
	ZAKTUALIZUJ tail TACY I liczniki

	ODBLOKUJ DANE DZIELONE
	ODBLOKUJ TACE
	ODBLOKUJ LICZBE PRODUKTOW	

	WYSLIJ LOG
```

Zabranie produktu przez klienta
```
DLA KAŻDEGO produkt_lista_zakupów ZRÓB:
	DLA KAŻDEGO wymagana_liczba_sztuk ZRÓB:
		WYŚLIJ LOG

		SPRÓBUJ ZABLOKOWAĆ LICZBĘ PRODUKTÓW BEZ CZEKANIA
		JEŚLI sukces
			WYŚLIJ LOG
			CZEKAJ I ZABLOKUJ TACE

			WYŚLIJ LOG
			CZEKAJ I ZABLOKUJ DANE DZIELONE

			USUŃ PRODUKT Z POCZATKU
			ZAKTUALIZUJ head TACY I liczniki

			ODBLOKUJ DANE DZIELONE
			ODBLOKUJ TACE
			ODBLOKUJ ZABLOKUJ WOLNE MIEJSCA PRODUKTU

			ZWIĘKSZ liczba_zabranych_produktow
		W PRZECIWNYM RAZIE:
			WYŚLIJ LOG
			PRZERWIJ PĘTLE
```
## Temat 15 – Ciastkarnia
Ciastkarnia produkuje P różnych produktów (P>10), każdy w innej cenie i na bieżąco sprzedaje je w samoobsługowym sklepie firmowym. Produkty bezpośrednio po wypieku (losowa liczba sztuk różnych produktów co określony czas) trafiają do sprzedaży w sklepie – każdy rodzaj produktu Pi na oddzielny podajnik. Każdy podajnik może przetransportować w danej chwili maksymalnie Ki sztuk pieczywa. Ciastka z danego podajnika muszą być pobieranie w sklepie dokładnie w takiej kolejności jak zostało położone na tym podajniku w piekarni. Zasady działania ciastkarni przyjęte przez kierownika są następujące: 
-  Ciastkarnia jest czynna w godzinach od Tp do Tk; 
- Sklep jest czynny w godzinach od Tp+30min do Tk; 
- W sklepie w danej chwili może się znajdować co najwyżej N klientów (pozostali, jeżeli są czekają przed wejściem); 
- W sklepie są 2 stanowiska kasowe, zawsze działa min. 1 stanowisko kasowe. 
- Na każdych K (K=N/2) klientów znajdujących się na terenie supermarketu powinno przypadać min. 1 czynne stanowisko kasowe. 
- Jeśli liczba klientów jest mniejsza niż N/2, to jedna z kas zostaje zamknięta. 
- Jeśli w kolejce do kasy czekali klienci (przed ogłoszeniem decyzji o jej zamknięciu) to powinni zostać obsłużeni przez tę kasę. 

Klienci przychodzą do sklepu w losowych momentach czasu z losową listą zakupów (spośród P produktów każdy klient wybiera min. dwa różne, np.: kremówki 3szt., WZ-tka 1 szt., …). Jeżeli dany produkt jest niedostępny (podajnik jest pusty) klient nie kupuje tego towaru. Następnie klient udaje się do kasy, a kasjer wystawia paragon na zakupy (kasjer zapamiętuje ile sztuk danego produktu skasował). Na komunikat (sygnał1) o inwentaryzacji – sygnał wysyła kierownik - klienci kontynuują zakupy normalnie do zamknięcia sklepu. Po zamknięciu sklepu, każda kasa robi podsumowanie sprzedanych produktów (Pi – liczba szt.), kierownik sumuje towar na podajnikach, piekarz podaje ilość wytworzonych produktów – raport zapisany w pliku tekstowym. Na komunikat (sygnał2) o ewakuacji – sygnał wysyła kierownik - klienci natychmiast przerywają zakupy i opuszczają piekarnię omijając kasy – pobrany już z podajników towar odkładają do kosza przy kasach. Napisz program, kierownika, piekarza, kasjera i klienta. Raport z przebiegu symulacji zapisać w pliku (plikach) tekstowym
