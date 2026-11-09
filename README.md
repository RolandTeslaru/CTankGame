# CTankGame

Proiect SO, CTI anul 3 (tema 8): joc de tancuri in doi, la aceeasi tastatura.

## Cerinte
- Tancurile sunt litere, proiectilele sunt puncte, tabla are ziduri (labirint).
- Tancul se misca sus/jos/stanga/dreapta si trage in directia de deplasare.
- Proiectilul se misca animat pana loveste un tanc, un zid sau marginea tablei.
- Un tanc lovit pierde din viata; vietile se afiseaza in colturi; pierde cine ajunge la 0.
- Pe tabla pot fi mai multe proiectile simultan.
- Argumente in linia de comanda: literele jucatorilor, tastele de miscare si foc.
- Tabla = matrice in procesul gazda.
- Fiecare tanc = un thread; fiecare proiectil = un thread nou.
- Datele comune sunt protejate cu mutexuri sau semafoare.
- Interfata: ncurses.

## Cum se joaca
`./tancuri A wsadf B ikjlh` - pentru fiecare tanc: litera, apoi 5 taste
(sus, jos, stanga, dreapta, foc). `q` iese din joc. Fiecare tanc are 10 vieti.
Terminalul trebuie sa aiba cel putin 62 x 24 caractere.

## Thread-uri si sincronizare
- `main` citeste tastatura (`getch` cu timeout 100 ms) si pune fiecare tasta
  in coada tancului care o detine.
- Fiecare tanc are un thread care asteapta in `sem_wait` pe semaforul
  `keys_available` (producator-consumator), apoi se misca sau trage.
- Fiecare proiectil tras porneste un thread nou, detasat, care avanseaza cate
  o pozitie la 40 ms pana loveste un zid, marginea sau un tanc.
- `board_lock` (mutex) protejeaza tabla, pozitiile, vietile si starea jocului;
  `ui_lock` (mutex) protejeaza apelurile ncurses; ordinea: board, apoi ui.
- Semafoarele sunt create cu `sem_open` (merge si pe Linux, si pe macOS).
- La final, `main` trezeste tancurile, face join pe ele si asteapta
  (`pthread_cond_wait`) sa se termine toate proiectilele.

## Structura
- `include/` - headere (interfata fiecarui modul)
- `src/` - implementari: main, game, board, tank, bullet, ui
- `build/` - fisiere obiect (generat)

## Comenzi
- `make` - compileaza executabilul `tancuri`
- `make run` - compileaza si ruleaza cu `A wsadf B ikjlh`
- `make clean` - sterge fisierele generate
