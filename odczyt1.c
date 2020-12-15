#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MAX 1024            /* Maksymalny rozmiar wczytywanego obrazu */
#define DL_LINII 1024      /* Dlugosc buforow pomocniczych */

/************************************************************************************
 * Funkcja wczytuje obraz PGM z pliku do tablicy       	       	       	       	    *
 *										    *
 * \param[in] plik_we uchwyt do pliku z obrazem w formacie PGM			    *
 * \param[out] obraz_pgm tablica, do ktorej zostanie zapisany obraz		    *
 * \param[out] wymx szerokosc obrazka						    *
 * \param[out] wymy wysokosc obrazka						    *
 * \param[out] szarosci liczba odcieni szarosci					    *
 * \return liczba wczytanych pikseli						    *
 ************************************************************************************/

int czytaj(FILE *plik_we,int obraz_pgm[][MAX],int *wymx,int *wymy, int *szarosci) {
  char buf[DL_LINII];      /* bufor pomocniczy do czytania naglowka i komentarzy */
  int znak;                /* zmienna pomocnicza do czytania komentarzy */
  int koniec=0;            /* czy napotkano koniec danych w pliku */
  int i,j;

  /*Sprawdzenie czy podano prawid�owy uchwyt pliku */
  if (plik_we==NULL) {
    fprintf(stderr,"Blad: Nie podano uchwytu do pliku\n");
    return(0);
  }

  /* Sprawdzenie "numeru magicznego" - powinien by� P2 */
  if (fgets(buf,DL_LINII,plik_we)==NULL)   /* Wczytanie pierwszej linii pliku do bufora */
    koniec=1;                              /* Nie udalo sie? Koniec danych! */

  if ( (buf[0]!='P') || (buf[1]!='2') || koniec) {  /* Czy jest magiczne "P2"? */
    fprintf(stderr,"Blad: To nie jest plik PGM\n");
    return(0);
  }

  /* Pominiecie komentarzy */
  do {
    if ((znak=fgetc(plik_we))=='#') {         /* Czy linia rozpoczyna sie od znaku '#'? */
      if (fgets(buf,DL_LINII,plik_we)==NULL)  /* Przeczytaj ja do bufora                */
	koniec=1;                   /* Zapamietaj ewentualny koniec danych */
    }  else {
      ungetc(znak,plik_we);                   /* Gdy przeczytany znak z poczatku linii */
    }                                         /* nie jest '#' zwroc go                 */
  } while (znak=='#' && !koniec);   /* Powtarzaj dopoki sa linie komentarza */
                                    /* i nie nastapil koniec danych         */

  /* Pobranie wymiarow obrazu i liczby odcieni szarosci */
  if (fscanf(plik_we,"%d %d %d",wymx,wymy,szarosci)!=3) {
    fprintf(stderr,"Blad: Brak wymiarow obrazu lub liczby stopni szarosci\n");
    return(0);
  }
  /* Pobranie obrazu i zapisanie w tablicy obraz_pgm*/
  for (i=0;i<*wymy;i++) {
    for (j=0;j<*wymx;j++) {
      if (fscanf(plik_we,"%d",&(obraz_pgm[i][j]))!=1) {
	fprintf(stderr,"Blad: Niewlasciwe wymiary obrazu\n");
	return(0);
      }
    }
  }
  return *wymx**wymy;   /* Czytanie zakonczone sukcesem    */
}                       /* Zwroc liczbe wczytanych pikseli */


/* Wyswietlenie obrazu o zadanej nazwie za pomoca programu "display"   */
void wyswietl(char *n_pliku) {
  char polecenie[DL_LINII];      /* bufor pomocniczy do zestawienia polecenia */

  strcpy(polecenie,"display ");  /* konstrukcja polecenia postaci */
  strcat(polecenie,n_pliku);     /* display "nazwa_pliku" &       */
  strcat(polecenie," &");
  printf("%s\n",polecenie);      /* wydruk kontrolny polecenia */
  system(polecenie);             /* wykonanie polecenia        */
}
/************************************************************************************
 * Funkcja zapisuje obraz PGM z tablicy do pliku       	       	       	       	    *
 *            *              *                   *             *
 * \param[in] "obraz_pgm" tablica, na ktorej zapisujemy obraz		    *
 * \param[in] "wymx" szerokosc obrazka						                   *
 * \param[in] "wymy" wysokosc obrazka						                      *
 * \param[in] "szarosci" liczba odcieni szarosci	                     *
 * \param[out] "plik_wy" uchwyt do pliku z obrazem w formacie PGM       *
 * \return zwracana wartoscia jest "0"                                      *
 ************************************************************************************/
int zapisz(FILE *plik_wy,int obraz_pgm[][MAX],int wymx,int wymy, int szarosci) {
  int i,j;                                            // inicjacja zmiennych wymiaru x i y tablicy //
  fprintf(plik_wy, "P2 \n");                          // nadpisanie w pliku  "magicznej liczby" //
  fprintf(plik_wy, "%d %d", wymx, wymy);              // nadpisanie wymiarow obrazu //
  fprintf(plik_wy, "\n%d\n", szarosci);               // nadpisanie skali szarosci //    
  for(i = 0; i < wymy; i++)
    for(j = 0; j < wymx; j++)
      fprintf(plik_wy, "%d ",obraz_pgm[i][j]);        // stworzenie tablicy z pikseli obrazu wejsciowego //
return 0;
}

/************************************************************************************
 * Funkcja rozciaga histogram obrazu PGM        	       	       	       	    *
 * Operacja przydaje sie, gdy skala szarosci obrazu nie jest w pelni wykorzystana
    przez jego piksele, np. skala to 255, zas piksele przymuja jedynie wartosci
    od 10 do 150, wtedy od wartosci kazdego piksela odejmiemy wartosc 10, czyli MIN,
    a nastepnie wynik przemnozymy przez 1.8, czyli stosunek wartosci skali do
    sumy wartosci MIN i MAX, wtedy dolne wartosci wszystkich pikseli minimalnie sie zmniejsza,
    zas maksymalne wartosci zwieksza sie o duzo, np. z 20 zmniejszy sie na 18, 
    a ze 140 zwiekszy sie na 234, wiec skala jest rzeczywiscie lepiej wykorzystana.
 *            *              *                   *             *
 * \param[in] "obraz_pgm" tablica, ktora bedziemy modyfikowac		   *
 * \param[out] zmodyfikowana tablica "obraz_pgm"                    *
 * \param[in] "wymx" szerokosc obrazka						                   *
 * \param[in] "wymy" wysokosc obrazka						                      *
 * \param[in] "szarosci" liczba odcieni szarosci	                     *
 ************************************************************************************/
int histogram(int obraz_pgm[][MAX], int wymx, int wymy, int szarosci){ 
int i, j;                             
float SKALA = szarosci;     // zmienna odpowiadajaca za liczbe stopni szarosci w obrazie //
int MIN = 65536;          // zmienna minimalnego stopnia szarosci(ciemne) w obrazie //
int MAKS = 0;         // zmienna maksymalnego stopnia szarosci(jasne) w obrazie //
/* szukanie minimalnego i maksymalnego stopnia szarosci */
for(i = 0; i < wymy; i++){
  for(j = 0; j < wymx; j++){
    if(obraz_pgm[i][j] < MIN)
    MIN = obraz_pgm[i][j];
    if(obraz_pgm[i][j] > MAKS)
    MAKS = obraz_pgm[i][j];}
}
{
  /* rozciaganie histogramu obrazu */
  for (i = 0; i < wymy; i++){
    for (j = 0; j < wymx; j++)
    obraz_pgm[i][j] = (obraz_pgm[i][j] - MIN)*(SKALA/(MAKS - MIN));}
}
return 0;
}
/************************************************************************************
 * Funkcja tworzy negatyw obrazu PGM        	       	       	       	              *
 * Operacja polega na odjeciu wartosci pojedynczego piksela od wartosci skali
  szarosci, po zastosowanoiu dzialania dla kazdego elementu talicy(obrazu) 
  kolory zmieniaja sie na przeciwne sobie.                                          *
 *            *              *                   *             *
 * \param[in] "obraz_pgm" tablica, ktora bedziemy modyfikowac		   *
 * \param[out] zmodyfikowana tablica "obraz_pgm"                    *
 * \param[in] "wymx" szerokosc obrazka						                   *
 * \param[in] "wymy" wysokosc obrazka						                      *
 * \param[in] "szarosci" liczba odcieni szarosci	                     *
 ************************************************************************************/
int negatyw(int obraz_pgm[][MAX],int wymx,int wymy, int szarosci) {
  int i,j;
  {
    for(i = 0; i < wymy; i++)
    for(j = 0; j < wymx; j++)
      obraz_pgm[i][j] = szarosci - obraz_pgm[i][j];     // dzialanie tworzace negatyw z pikseli obrazu //
}
return 0;
}

/************************************************************************************
 * Funkcja proguje obraz PGM        	       	       	       	              *
 * Operacja progowania ma na celu odroznienie poszukiwanego obiektu
   od tla zdjecia. Polega na przyporzadkowaniu danemu pikselowi wartosci
   0, gdy jego wartosci jest mniejsza badz rowna wartosci progu.
  "Maks" (realna wartosc skali szarosci w obrazie), gdy jest wieksza niz prog.
   Im wiekszy ustalimy prog tym bardziej obiekt zleje sie z tlem. 
   Wartosc progu podajemy w procentach(najlepiej od 0 do 100).              
   W przypadku podania progu nizszego od 0 otrzymamy bialy obraz,
   zas w przypadku wyzszego - czarny obraz.                                 *
 *            *              *                   *             *
 * \param[in] "obraz_pgm" tablica, ktora bedziemy modyfikowac		   *
 * \param[out] zmodyfikowana tablica "obraz_pgm"                    *
 * \param[in] "wymx" szerokosc obrazka						                   *
 * \param[in] "wymy" wysokosc obrazka						                      *
 * \param[in] "szarosci" liczba odcieni szarosci	                     *
 * \param[in] "prog" wartosc progu ustalona przez uzytkownika           *
 ************************************************************************************/
int progowanie(int obraz_pgm[][MAX],int wymx,int wymy, int szarosci, int prog) {
  int i, j, maks = 0;
  for(i = 0; i < wymy; i++) {
    for(j = 0; j < wymx; j++) 
      if (obraz_pgm[i][j] > maks)      
        maks = obraz_pgm[i][j];
        }
  prog = (maks * prog)/100.0;
  for(i = 0; i < wymy; i++) {
    for(j = 0; j < wymx; j++)
      if (obraz_pgm[i][j] <= prog)      // piksele mniejsze od wartosci progu staja sie czarne //
        obraz_pgm[i][j]=0;
        else
          obraz_pgm[i][j]=maks;     // reszta pikseli przyjmuje maksymalny stopien szarosci //
  }
  return 0;
}
/************************************************************************************
 * Funkcja powoduje rozmycie poziome obrazu PGM        	       	       	            *
 * Operacja rozmycia poziomego bierze pod uwage po 2 piksele
   z kazdej strony w poziomie biezacego piksela oraz biezacy piksel srodkowy.
   Dodaje do siebie ich wartosci, a nastepnie dzieli przez ilosc tych pikseli
   tak, aby stworzyc srednia arytmetyczna, ktorej wartosc przyjmuje biezacy
   piksel. Funkcja uwzglednia po 2 piksele obok bierzacego, by zwiekszyc
   efekt rozmycia.                                                                  *
 *            *              *                   *             *
 * \param[in] "obraz_pgm" tablica, ktora bedziemy modyfikowac		   *
 * \param[out] zmodyfikowana tablica "obraz_pgm"                    *
 * \param[in] "wymx" szerokosc obrazka						                   *
 * \param[in] "wymy" wysokosc obrazka						                      *
 * \param[in] "szarosci" liczba odcieni szarosci	                     *
 ************************************************************************************/
int rozmycie(int obraz_pgm[][MAX], int wymx, int wymy, int szarosci){
  int i, j;
  for(i = 0; i < wymy; i++){ // dla poprawnosci dzialania operacji zachowano stosowna odleglosc od granic poziomych w obrazie //
    for(j = 2; j < wymx - 2; j++)
    obraz_pgm[i][j] = ((obraz_pgm[i][j-2] + obraz_pgm[i][j-1] + obraz_pgm[i][j] + obraz_pgm[i][j+1] + obraz_pgm[i][j+2])/(5.0)); // rozciaganie w poziomie //
  }
  return 0;
}
 /************************************************************************************
 * Funkcja sluzy do tworzenia konturu obrazu PGM        	          	   	            *
 * Dzialanie konturowania bierze pod uwage wartosci bezwzgledne
   roznicy sasiednich pikseli oddzielnie w pionie i poziomie, 
   a nastepnie sumuje je. W taki sposob piksele, ktore maja podobna.
   wartosc staja sie ciemne(gdyz po odjeciu ich od siebie otrzymujemy
   bardzo mala wartosc), zas te mocno rozniace sie staja sie jasniejsze.              *
 *            *              *                   *             *
 * \param[in] "obraz_pgm" tablica, ktora bedziemy modyfikowac		   *
 * \param[out] zmodyfikowana tablica "obraz_pgm"                    *
 * \param[in] "wymx" szerokosc obrazka						                   *
 * \param[in] "wymy" wysokosc obrazka						                      *
 * \param[in] "szarosci" liczba odcieni szarosci	                     *
 ************************************************************************************/
int konturowanie(int obraz_pgm[][MAX], int wymx, int wymy, int szarosci){
  int i, j;
  for(i = 0; i < wymy; i++){
    for(j = 0; j < wymx; j++){
      // rozjasnienie pikseli rozniacych sie od otoczenia i przyciemnienie podobnych od otoczenia //
      obraz_pgm[i][j] = abs(obraz_pgm[i+1][j] - obraz_pgm[i][j]) + abs(obraz_pgm[i][j+1] - obraz_pgm[i][j]);
    }
  }
  return 0;
}                 
/* funkcja main */

int main() {                    // zmienne uzyte w funkcji glownej to //
  int obraz[MAX][MAX];          // tablica obrazu //
  int wymx,wymy,odcieni;        // wymiar pionowy i poziomy oraz skala szarosci //
  int prog;                     // wartosc progu //
  int n;                        // zmienna potrzebna do wykonania funkcji switch //
  int s;                        // zmienna uzyta do funkcji zapisu obrazu na polecenie uzytkownika //
  int odczytano = 0;            // zmienna uzyta jako wartosc zwracana na koncu funkcji main //
  FILE *plik;                   // plik wgrywany przez uzytkownika //
  FILE *pliczek;                // plik potrzebny do wyswietlenia obrazu wejsciowego po operacji //
  char nazwa[100];              // zmienna znakowa uzyta do zapisu obrazu pod nazwa podana przez uzytkownika //

  /* Wczytanie zawartosci wskazanego pliku do pamieci */
  printf("Podaj nazwe pliku:\n");
  scanf("%s",nazwa);
   do{                      // polecenie do-while umozliwia wykonanie kilku operacji na obrazie podczas 1 cyklu dzialania programu //
  plik=fopen(nazwa,"r");
  if (plik != NULL) {       /* wczytanie zawartosci pliku spowoduje zakomentowanie tego warunku */
    odczytano = czytaj(plik,obraz,&wymx,&wymy,&odcieni);
    fclose(plik);}
    else{
    printf("\nplik niewlasciwy\n");
    return 0;}
  /* menu funkcji */
    pliczek=fopen("tmp.pgm", "w");      // otwarcie pliku tymczasowego uzytego do wyswietlenia obrazu po operacji //
      printf("\n Witaj w menu wyboru operacji, ktora ma byc wykonana na wczytanym obrazie.\n");
      printf("Wybierz jedna z ponizszych cyfr, aby wykonac przypisana jej operacje\n");
      printf(" 1 - negatyw\n 2 - progowanie\n 3 - konturowanie\n 4 - rozciaganie histogramu\n 5 - rozmycie poziome\n 6 - wyjscie\n");
        scanf("%d", &n);      // odczytanie wskazanej wartosci //
        if (n < 1 || n > 6){  //sprawdzenie prawidlowosci danej wejsciowej//
        printf("\nNie wybrano zadnej opcji. Program konczy dzialanie.\n");
        return 0;}
      /* funkcja wyboru scenariusza po wskazaniu wartosci przez uzytkownika */
        switch(n){
          /* przypadek 1. tworzenie negatywu*/
          case 1: negatyw(obraz, wymx, wymy, odcieni);      // wywolanie funkcji negatyw //
                   zapisz(pliczek, obraz, wymx, wymy, odcieni); // wyswietlenie obrazu //
                   wyswietl("tmp.pgm"); 
                   fclose(pliczek);   
                    printf(" \n Czy chcesz zapisac obraz?\n Jesli tak, wybierz 1\n Jesli nie, wybierz 2 \n"); // czy zapisac obraz? //
                     scanf("%d", &s);                    
                     if(s != 1 && s != 2){      // sprawdzenie czy uzytkownik wybral jedna z opcji //
                       printf("\nNie podano zadnej z opcji. Obraz nie zostaje zapisany.\n");      // jesli nie, to... //
                       n = 0;
                       s = 0;
                       return 0;} 
                     
                      if(s == 1){                                       // jesli tak to //
                      printf("Zapisz jako - podaj nazwe pliku:\n");     // podaj nazwe //
                       scanf("%s",nazwa);                               // odczytanie napisu //
                       plik=fopen(nazwa, "w");                          // otwarcie pliku pod nazwa //
                       zapisz(plik, obraz, wymx, wymy, odcieni);        // zapis pliku //
                       n=0;
                       s=0;
                       }
                       if(s==2)                                         // jesli nie to //
                       n=0;
                       s=0;
                        break;                                          // sprawdz warunek while i wroc do poczatku instrukcji do //
          /* przypadek 2. progowanie */
          case 2: printf("podaj prog(sama liczba procentow bez znaku procent):\n");
                   scanf("%d",&prog);                                   // odczytanie wartosci progu //
                    progowanie(obraz,wymx,wymy,odcieni,prog);           // wywolanie funkcji progowania //
                     zapisz(pliczek, obraz, wymx, wymy, odcieni);
                      wyswietl("tmp.pgm");
                      fclose(pliczek);
                    printf("\nCzy chcesz zapisac obraz?\n Jesli tak, wybierz 1\n Jesli nie, wybierz 2\n");
                     scanf("%d", &s);
                     if(s != 1 && s != 2){
                       printf("\nNie podano zadnej z opcji. Obraz nie zostaje zapisany.\n");
                       n = 0;
                       s = 0;
                       return 0;
                       }
                      if(s == 1){
                      printf("Zapisz jako - podaj nazwe pliku:\n");
                       scanf("%s",nazwa);
                       plik=fopen(nazwa, "w");
                       zapisz(plik, obraz, wymx, wymy, odcieni);
                       n = 0;
                       break;
                       }
                       if(s==2)
                       n = 0;
                        break;
          /* przypadek 3. konturowanie */
          case 3: konturowanie(obraz, wymx, wymy, odcieni);             // wywolanie funkcji konturowania //
                   zapisz(pliczek, obraz, wymx, wymy, odcieni);
                    wyswietl("tmp.pgm");
                    fclose(pliczek);
                    printf("\nCzy chcesz zapisac obraz?\n Jesli tak, wybierz 1\n Jesli nie, wybierz 2\n");
                     scanf("%d", &s);
                     if(s != 1 && s != 2){
                       printf("\nNie podano zadnej z opcji. Obraz nie zostaje zapisany.\n");
                       n = 0;
                       s = 0;
                       return 0;
                       }
                      if(s == 1){
                      printf("Zapisz jako - podaj nazwe pliku:\n");
                       scanf("%s",nazwa);
                       plik=fopen(nazwa, "w");
                       zapisz(plik, obraz, wymx, wymy, odcieni);
                       n = 0;
                       break;
                       }
                       if(s==2)
                       n = 0;
                        break; 
          /* przypadek 4. rozciaganie histogramu */
          case 4: histogram(obraz, wymx, wymy, odcieni);                // wywolanie funkcji rozciagania histogramu //
                   zapisz(pliczek, obraz, wymx, wymy, odcieni);
                    wyswietl("tmp.pgm");
                    fclose(pliczek);
                    printf("\nCzy chcesz zapisac obraz?\n Jesli tak, wybierz 1\n Jesli nie, wybierz 2\n");
                     scanf("%d", &s);
                     if(s != 1 && s != 2){
                       printf("\nNie podano zadnej z opcji. Obraz nie zostaje zapisany.\n");
                       n = 0;
                       s = 0;
                       return 0;
                       }
                      if(s == 1){
                      printf("Zapisz jako - podaj nazwe pliku:\n");
                       scanf("%s",nazwa);
                       plik=fopen(nazwa, "w");
                       zapisz(plik, obraz, wymx, wymy, odcieni);
                       n = 0;
                       break;
                       }
                       if(s==2)
                       n = 0;
                        break;
          /* przypadek 5. rozmycie */
          case 5: rozmycie(obraz, wymx, wymy, odcieni);                 // wywolanie funkcji rozmycia poziomego //
                   zapisz(pliczek, obraz, wymx, wymy, odcieni);
                    wyswietl("tmp.pgm");
                    fclose(pliczek);
                    printf("\nCzy chcesz zapisac obraz?\n Jesli tak, wybierz 1\n Jesli nie, wybierz 2\n");
                     scanf("%d", &s);
                     if(s != 1 && s != 2){
                       printf("\nNie podano zadnej z opcji. Obraz nie zostaje zapisany.\n");
                       n = 0;
                       s = 0;
                       return 0;
                       }
                      if(s == 1){
                      printf("Zapisz jako - podaj nazwe pliku:\n");
                       scanf("%s",nazwa);
                       plik=fopen(nazwa, "w");
                       zapisz(plik, obraz, wymx, wymy, odcieni);
                       n = 0;
                       break;
                       }
                       if(s==2)
                       n = 0;
                        break;
          /* przypadek 6. zakonczenie dzialania programu */
          case 6: printf("\n Wybrales opcje wyjscie.\n");
                   return odczytano;                                    // zwrocenie wartosci "odczytano", zakonczenie dzialania programu //
}
   }
   while (n != 6);
return 0;
}


/*
Sprawozdanie:
Imie i nazwisko autora: Arkadiusz Dyjas
Temat cwiczenia: Przetwarzanie obrazow 1
Data wykonania: 14.11.2019r.
Kod zrodlowy programu: odczyt.c

Testy programu:
w ramach sprawdzenia poprawnosci dzialania programu najpierw uruchomilem program w normalnych warunkach, tzn.
wczytalem do niego zwykly obrazek z roszerzeniem .pgm("saturn.ascii.pgm"), a nastepnie wybralem opcje wykonania negatywu.
Program wyslal pytanie: "czy zapisac obrazek?", odpowiedzialem twierdzaco i podalem nazwe pod jaka ma byc zapisany.
Nastepnie uzylem opcji wyjscia i program zakonczyl dzialanie. Test przeprowadzony pomyslnie.

Kolejnym krokiem sprawdzania poprawnosci dzialania programu bylo:

-Wgranie do niego pustego obrazka z formatem .pgm i magiczna liczba, program wyswietlil nastepujacy komunikat:
"brak wymiarow obrazu lub liczby stopni szarosci", jako ze plik byl w formacie .pgm mozna bylo wykonac na nim dalsze
operacje, lecz nie mialo to sensu, wiec za pomoca menu zakonczylem dzialanie programu. Test przeprowadzony pomyslnie.

-Wgranie do niego obrazu/pliku z niewlasciwym formatem(np .jpg), program ukazuje komunikat "plik niewlasciwy" i konczy 
swoje dzialanie. Test przeprowadzony pomyslnie.

-Wgranie do niego obrazu .pgm z magiczna liczba i wymiarami ale bez stopni szarosci wyswietla komunikat jak w 1. tescie:
"brak wymiarow obrazu lub liczby stopni szarosci". Jako ze format jest poprawny, mozna wykonywac dalsze dzialania,
 ja wybralem opcje zakonczenia dzialania programu. Test przeprowadzony pomyslnie.

-Wgranie do niego obrazu .pgm z magiczna liczba, wymiarami i skala szarosci, ale bez pikseli powoduje wyswietlenie
 komunikatu: "niewlasciwe wymiary obrazu". Mozna wykonywac operacje. Wybralem opcje wyjscie. Test przeprowadzony 
 pomyslnie.

 Kwestia warta podkreslenia jest fakt, iz odpowiadanie programowi niezgodnie z jego instrukcjami powoduje kazdorazowe
 zakonczenie dzialania programu.
 Wszystkie ponizsze operacje zostaly opisane w komentarzach nad ich funkcjami. 

 Do sprawdzenia zostala jeszcze poprawnosc dzialania nastepujacych funkcji programu: zapisywanie, rozciaganie histogramu,
 tworzenie negatywu, progowanie, rozmywanie poziome oraz konturowanie.
 
 Zapisywanie: do programu wgralem obraz "saturn.ascii.pgm", wykonalem na nim operacje tworzenia negatywu, zapisalem go, 
 a nastepnie powtorzylem krok. Zapisany obraz wynikowy byl dokladnie taki sam jak poczatkowy, to oznacza ze funkcja zapisu
 dziala poprawnie i nie gubi po drodze zadnych informacji obrazu.

 ROZCIAGANIE HISTOGRAMU:
 w ramach testu wprowadzilem do programu plik .pgm z nastepujacymi danymi:

 P2
5 5
10
1 2 3 4 5
1 2 3 4 5
1 2 3 4 5
1 2 3 4 5
1 2 3 4 5

Wynik jaki otrzymalem to:

P2 
5 5
10
0 2 5 7 10
0 2 5 7 10
0 2 5 7 10
0 2 5 7 10
0 2 5 7 10 

Dzialanie wykonane prawidlowo w 100%.

TWORZENIE NEGATYWU:
w ramach testu wprowadzilem do programu plik .pgm z nastepujacymi danymi:

 P2
5 5
10
1 2 3 4 5
1 2 3 4 5
1 2 3 4 5
1 2 3 4 5
1 2 3 4 5

Wynik jaki otrzymalem to:

P2 
5 5
10
9 8 7 6 5 
9 8 7 6 5 
9 8 7 6 5 
9 8 7 6 5 
9 8 7 6 5 

Dzialanie wykonane prawidlowo w 100%.

PROGOWANIE:
Tak jak napisalem w komentarzu nad funkcja progowania w programie
przyjecie progu ponizej 0% jest rowne przjeciu progu 0%, obraz 
bedzie zupelnie jasny(kazdy piksel ma wartosci max szarosci),
zas w przypadku progu powyzej 100% spowoduje, ze obraz bedzie
zupelnie ciemny(kazdy piksel ma wartosc 0).
w ramach testu wprowadzilem do programu plik .pgm z nastepujacymi danymi:

 P2
5 5
10
1 2 3 4 5
1 2 3 4 5
1 2 3 4 5
1 2 3 4 5
1 2 3 4 5

oraz PROGIEM o wartosci 50%

Wynik jaki otrzymalem to:

P2 
5 5
10
0 0 5 5 5
0 0 5 5 5 
0 0 5 5 5 
0 0 5 5 5 
0 0 5 5 5 

Dzialanie wykonane prawidlowo w 100%.

ROZMYWANIE POZIOME:

W razie nie czytania komentarzy przy funkcjach warto wspomniec tu, ze
w funkcji zastosowalem srednia arytmetyczna 5 sasiednich pikseli w poziomie,
a nie 3, poniewaz pozwala to na uzyskanie bardziej zauwazalnego efektu rozmycia.

w ramach testu wprowadzilem do programu plik .pgm z nastepujacymi danymi:

P2
10 5
10
0 1 2 3 9 5 6 7 8 9
0 1 2 3 8 5 6 7 8 9
0 1 2 3 8 5 6 7 8 9
0 1 2 3 8 5 6 7 8 9
0 1 2 3 8 5 6 7 8 9

Wynik jaki otrzymalem to:

P2 
10 5
10
0 1 3 4 5 5 6 7 8 9 
0 1 2 3 4 5 6 7 8 9 
0 1 2 3 4 5 6 7 8 9 
0 1 2 3 4 5 6 7 8 9 
0 1 2 3 4 5 6 7 8 9 

Dzialanie wykonane prawidlowo w 100%.

KONTUROWANIE:

W ramach testu wprowadzilem do programu plik .pgm z nastepujacymi danymi:

P2
10 5
10
0 1 2 3 9 5 6 7 8 9
0 1 2 3 8 5 6 7 8 9
0 1 2 3 8 5 6 7 8 9
0 1 2 3 8 5 6 7 8 9
0 1 2 3 8 5 6 7 8 9

Wynik jaki otrzymalem to:

P2 
10 5
10
1 1 1 6 5 1 1 1 1 9 
1 1 1 5 3 1 1 1 1 9 
1 1 1 5 3 1 1 1 1 9 
1 1 1 5 3 1 1 1 1 9 
1 2 3 8 11 6 7 8 9 18 

Dzialanie wykonane prawidlowo w 100%.

NEGATYW I PROGOWANIE NARAZ:

W ramach testu wprowadzilem do programu plik .pgm z nastepujacymi danymi i progiem 50%:

P2
10 5
10
0 1 2 3 4 5 6 7 8 9
0 1 2 3 4 5 6 7 8 9
0 1 2 3 4 5 6 7 8 9
0 1 2 3 4 5 6 7 8 9
0 1 2 3 4 5 6 7 8 9

Wynik jaki otrzymalem to:

P2 
10 5
10
10 10 10 10 10 0 0 0 0 0 
10 10 10 10 10 0 0 0 0 0 
10 10 10 10 10 0 0 0 0 0 
10 10 10 10 10 0 0 0 0 0 
10 10 10 10 10 0 0 0 0 0 

Dzialanie wykonane prawidlowo w 100%.

WNIOSKI, UWAGI:

Na wstepie chcialbym uswiadomic problem zaistnialy podczas dzialania programu, 
kiedy uzytkownik wpisywal znak zamiast liczby, w miejscu gdzie powinna byc liczba,
program wpadal w nieskonczona petle, w celu unikniecia jej, program konczy dzialanie
w przypadku wpisania nieprawidlowej danej przy wyborze opcji na kazdym etapie dzialania.
Jednoczesnie nie ma po co sie martwic utrata postepow w modyfikowaniu obrazu w przypadku
wykonwania kilku operacji na raz, poniewaz po kazdej operacji uzytkownik ma do wyboru opcje zapisu.

Jesli uzytkownik chcialby wykonac kilka operacji nalozonych na siebie na obrazie w czasie
 1 cyklu dzialania programu, po kazdej operacji musi zapisac obraz wynikowy na tym samym pliku.
W razie niezapisania obrazu mozna wykonywac kolejne operacje na obrazie, ktory uzytkownik poczatkowo
 wgral do programu, o ile nie nadpisal go po udanej operacji.

Nalezy wspomniec, ze w ramach umozliwienia wyswietlania uzytkownikowi obrazu
po wykonanej operacji program tworzy w komputerze uzytkownika plik o nazwie tmp.pgm, ktory caly czas 
jest nadpisywany przy okazji wykonywania kolejnych dzialan.

Doszedlem rowniez do wniosku, ze aby uwidocznic efekt rozmywania poziomego nalezy zwiekszyc
liczbe punktow otaczajacych dana liczbe w poziomie, wiec promien wynosi 2, a nie 1.

Do programu nie mozna wgrac nieistniejacego obrazu lub obrazu z niewlasciwym formatem,
program konczy wtedy dzialanie wyswietlajac stosowny komunikat "plik niewlasciwy".

W przypadku funkcji switch wystepuje powtarzalnosc pewnych elementow, wiec zostaly
opisane tylko w case 1.
*/