/*
    SIECI KOMPUTEROWE 2021/22 -- grupa LF
    Projekt I: myls
    Autor: Dawid Odolczyk

    *** INFORMACJA ***
        Na filmie prezentujacym projekt plik zrodlowy nie zostal skompilowany z flaga -Wall. Jest to oczywiscie przeoczenie,
        lecz po sprawdzeniu dodanie tej flagi przy kompilacji NIE wypisuje zadnych ostrzezen.
*/

#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pwd.h>
#include<grp.h>
#include<string.h>
#include<time.h>
#include<ctype.h>
#include<strings.h>
#include<limits.h>
#include<fcntl.h>
#include<unistd.h>

#define MAX_DIR_SIZE 512        /* maksymalna liczba plikow w katalogu */
#define STR_SIZE 128            /* maksymalna dlugosc napisow (wlasciciel, grupa, nazwa pliku itp.) */

int howManyDigits(int n){       /* funkcja liczaca liczbe cyfr w danej liczbie */
    if(n==0) return 1;
    int dig=0;
    while(n){
        dig++;
        n/=10;
    }
    return dig;
}

char addChar(int n, char c){    /* funkcja wstawiajaca wybrany znak (do wypisywania dat w Trybie 2.) */
    if(n<10) return c;
    else return '\0';
}

int main(int argc, char *argv[]){

    /* === TRYB 1: Zawartosc biezacego katalogu === */
    if(argc<2){
        DIR *pDir;
        pDir = opendir(".");    /* otworz biezacy katalog */

        if(pDir == NULL){       /* zwroc blad w przypadku, gdy nie mozna otworzyc katalogu */
            perror("ERROR (opendir(): Nie udalo sie otworzyc katalogu)");
	        return 1;
        }

        struct dirent *d;
        int howManyFiles = 0;                           /* licznik elementow w katalogu */
        int octMode[MAX_DIR_SIZE] = {0};                /* osemkowa wartosc pola st_mode */
        char accessPerms[MAX_DIR_SIZE][11];             /* prawa dostepu dla konkretnego pliku */
        int hardLinks[MAX_DIR_SIZE] = {0};              /* liczba dowiazan twardych do pliku */
        char fileOwner[MAX_DIR_SIZE][STR_SIZE];         /* wlasciciel pliku */
        char fileGroup[MAX_DIR_SIZE][STR_SIZE];         /* nazwa grupy */
        int fileSize[MAX_DIR_SIZE] = {0};               /* rozmiar pliku w bajtach */
        int modifDay[MAX_DIR_SIZE];                     /* dzien ostatniej modyfikacji pliku */
        int modifMonth[MAX_DIR_SIZE];                   /* miesiac ostatniej modyfikacji pliku */
        int modifYear[MAX_DIR_SIZE];                    /* rok ostatniej modyfikacji pliku */
        int modifHour[MAX_DIR_SIZE];                    /* godzina ostatniej modyfikacji pliku */
        int modifMin[MAX_DIR_SIZE];                     /* minuta ostatniej modyfikacji pliku */
        char fileName[MAX_DIR_SIZE][STR_SIZE];          /* nazwa pliku */
        char linkPath[MAX_DIR_SIZE][STR_SIZE];          /* plik, na ktory wskazuje link symboliczny */

        int maxLenHL = 0;                               /* maksymalna szerokosc (bez spacji) kolumny z liczba dowiazan twardych */
        int maxLenFO = 0;                               /* maksymalna szerokosc (bez spacji) kolumny z nazwa wlasciciela pliku */
        int maxLenFG = 0;                               /* maksymalna szerokosc (bez spacji) kolumny z nazwa grupy */
        int maxLenFS = 0;                               /* maksymalna szerokosc (bez spacji) kolumny z rozmiarem pliku */

        while((d = readdir(pDir)) != NULL){                         /* przejrzyj caly katalog */
            
            strcpy(fileName[howManyFiles],d->d_name);               /* pozyskaj nazwe pliku */

            struct stat info;
            if(lstat(fileName[howManyFiles],&info) != 0){           /* zwroc blad, gdy nie da sie uzyskac danych nt. pliku */
                perror("ERROR (lstat(): Nie udalo sie otworzyc pliku)");
                return 1;
            }
            /* 
                Uwaga: Funkcja lstat() wywolana dla pliku, ktory NIE jest linkiem symbolicznym,
                       zachowa sie tak samo, jak funkcja stat().
            */

            octMode[howManyFiles] = info.st_mode;                   /* pozyskaj uprawnienia w postaci osemkowej */

            struct passwd *pwd;                                     /* pozyskaj nazwe wlasciciela pliku */
            pwd = getpwuid(info.st_uid);
            strcpy(fileOwner[howManyFiles],pwd->pw_name);
            if(strlen(fileOwner[howManyFiles])>maxLenFO){           /* wyznacz maksymalna szerokosc kolumny z nazwa wlasciciela pliku */
                maxLenFO = strlen(fileOwner[howManyFiles]);
            }

            struct group *grp;                                      /* pozyskaj nazwe grupy */
            grp = getgrgid(info.st_gid);
            strcpy(fileGroup[howManyFiles],grp->gr_name);
            if(strlen(fileGroup[howManyFiles])>maxLenFG){           /* wyznacz maksymalna szerokosc kolumny z nazwa grupy */
                maxLenFG = strlen(fileGroup[howManyFiles]);
            }

            fileSize[howManyFiles] = info.st_size;                  /* pozyskaj rozmiar pliku w bajtach */
            if(howManyDigits(fileSize[howManyFiles])>maxLenFS){     /* wyznacz maksymalna szerokosc kolumny z rozmiarem pliku */
                maxLenFS = howManyDigits(fileSize[howManyFiles]);
            }

            hardLinks[howManyFiles] = info.st_nlink;                /* pozyskaj liczbe dowiazan twardych do pliku */
            if(howManyDigits(hardLinks[howManyFiles])>maxLenHL){    /* wyznacz maksymalna szerokosc kolumny z liczba dow. twardych */
                maxLenHL = howManyDigits(hardLinks[howManyFiles]);
            }

            time_t date;                                
            date = info.st_mtime;                                   /* pozyskaj wartosc przechowujaca date i czas modyfikacji */
            struct tm *dateTime;                                    /* zamien time_t na powszechny format daty i czasu */        
            dateTime = localtime(&date);
            modifYear[howManyFiles] = 1900+(dateTime->tm_year);     /* tm_year przechowuje liczbe lat, ktore uplynely od 1900 roku */   
	        modifMonth[howManyFiles] = 1+(dateTime->tm_mon);        /* tm_mon jest z zakresu [0,11], dlatego trzeba dodac do wyniku 1 */
            modifDay[howManyFiles] = dateTime->tm_mday;
            modifHour[howManyFiles] = dateTime->tm_hour;
            modifMin[howManyFiles] = dateTime->tm_min;

            /*** Wyznacz uprawnienia do pliku ***/
            char rwx[4]={'x','w','r','\0'}, ugs[4]={'s','s','t','\0'};      /* tablice pomocnicze z symbolami praw dostepu */
            int mask=07, value, t=9, i, j;
            accessPerms[howManyFiles][10] = '\0';       /* na miejscu 0 bedzie stal symbol typu pliku... */
	        value = octMode[howManyFiles] & mask;       /* ...a na miejscach 1-9 symbole uprawnien z tablicy rwx */
            for(i=0; i<3; i++){
                for(j=0; j<3; j++){
                    if(value & 1){
                        accessPerms[howManyFiles][t--] = rwx[j%3];
                    } else {
                        accessPerms[howManyFiles][t--] = '-';
                    }
                    value >>= 1;
                }
                mask <<= 3;
                value = (octMode[howManyFiles] & mask) >> ((i+1)*3);
            }
            for(j=3; j>0; j--){                         /* ustawianie set-user-ID, set-group-ID i sticky bit */
                if(value & 1){
                    accessPerms[howManyFiles][j*3] = ugs[j%3];
                }
                value >>= 1;
            }
            mask <<= 4;         /* tworzenie maski = 17 (do odczytu typu pliku) */
            mask += 010000;
            value = (octMode[howManyFiles] & mask) >> 12;

            if(value == 04){                            /* ustalanie typu pliku (4 = katalog) */
                accessPerms[howManyFiles][0] = 'd';
                linkPath[howManyFiles][0] = '\0';       /* nie jest to link symboliczny - pomin te zmienna przy wypisywaniu */
            }
            if(value == 010){                           /* 10 = plik zwykły */
                accessPerms[howManyFiles][0] = '-';
                linkPath[howManyFiles][0] = '\0';       /* nie jest to link symboliczny - pomin te zmienna przy wypisywaniu */
            }
            if(value == 012){
                accessPerms[howManyFiles][0] = 'l';     /* 12 = link symboliczny */
                char buf[PATH_MAX];
                realpath(fileName[howManyFiles],buf);       /* pobierz sciezke do linku symbolicznego */
                linkPath[howManyFiles][0] = ' ';
                linkPath[howManyFiles][1] = '-';
                linkPath[howManyFiles][2] = '>';
                linkPath[howManyFiles][3] = ' ';
                strcat(linkPath[howManyFiles],buf);
                int fileNameLen = 0;
                int pathLen = strlen(linkPath[howManyFiles]);       /* skroc pelna sciezke do samej nazwy pliku... */
                for(i=pathLen-1; i>=0; i--){                        /* ...tj. od ostatniego slasha '/' do konca lancucha */
                    if(linkPath[howManyFiles][i] == '/') break;
                    else fileNameLen++;
                }
                for(i=0; i<fileNameLen; i++){
                    linkPath[howManyFiles][i+4] = linkPath[howManyFiles][i+(pathLen-fileNameLen)];
                }
                linkPath[howManyFiles][fileNameLen+4] = '\0';
            }

            howManyFiles++;
        }
        closedir(pDir);     /* zamknij katalog */

        int x,y;                /* iteratory petli */
        int isDiffYear = 0;     /* zmienna sprawdzajaca czy w ktorejs z dat jest inny rok niz w pozostalych */
        int yr = modifYear[0];
        for(x=1; x<howManyFiles; x++){
            if( yr != modifYear[x]){   /* jesli rok jest inny niz ten z pierwszego wiersza... */
                isDiffYear = 1;                 /* ...ustaw, ze jest roznica... */
	       	    break;                          /* ...i wyjdz z petli */
            }
        }
        
        /*** Posortuj alfabetycznie wiersze wzgledem nazw plikow ***/
        /*  Przygotowanie przed sortowaniem -- "oczyszczanie" nazw plikow z kropek na poczatku nazw */
        int i,j;                                        /* iteratory petli */
	    char fileNameSorted[MAX_DIR_SIZE][STR_SIZE];    /* tablica, ktora bedzie zawierac posortowane nazwy plikow */
        int whereChanged[MAX_DIR_SIZE] = {0};           /* tablica z informacja gdzie, w celu posortowania, pozbyto sie kropki z nazwy */
        for(x=0; x<howManyFiles; x++){
            strcpy(fileNameSorted[x],fileName[x]);
	        if(fileNameSorted[x][0] == '.'){                            /* jesli na poczatku nazwy jest kropka... */
                whereChanged[x] = 1;                                    /* ...odnotuj to na odp. pozycji w tablicy whereChanged... */
                for(i=0; i<strlen(fileNameSorted[x])-1; i++){           /* ...i ja faktycznie usun */
                    fileNameSorted[x][i] = fileNameSorted[x][i+1];
                }
		    fileNameSorted[x][i] = '\0';
	        }
        }
    
        /* Wlasciwe sortowanie */
	    for(i=0; i<howManyFiles; i++){                                      /* posortuj nazwy plikow w kolejnosci alfabetycznej */
	        for(j=howManyFiles-1; j>0; j--){                                /* nie zwazajac na kropki i wielkosc liter */
                if(strcasecmp(fileNameSorted[j-1],fileNameSorted[j])>=0){   /* jesli trzeba zamienic kolejnosc nazw... */
    		        char tempChar[STR_SIZE];                                /* ...dokonaj tej zamiany... */
                    strcpy(tempChar,fileNameSorted[j-1]);
                    strcpy(fileNameSorted[j-1],fileNameSorted[j]);
                    strcpy(fileNameSorted[j],tempChar);
                    int tempInt;                                            /* ...nie zapominajac o zamianie kolejnosci... */
                    tempInt = whereChanged[j-1];                            /* ...w tablicy whereChanged */
                    whereChanged[j-1] = whereChanged[j];
                    whereChanged[j] = tempInt;
                }
            }
     	}

        for(x=0; x<howManyFiles; x++){          /* tam, gdzie w nazwach zabrano kropki, przywroc je */
            if(whereChanged[x]){
	            char tmp[STR_SIZE]={'.'};
		        strcat(tmp,fileNameSorted[x]);
		        strcpy(fileNameSorted[x],tmp);
	        }
        }

        /*** Wyznacz odstepy miedzy kolumnami ***/
        char spacesHL[MAX_DIR_SIZE][STR_SIZE];      /* spacje przed liczba dowiazan twardych */
        char spacesFO[MAX_DIR_SIZE][STR_SIZE];      /* spacje przed nazwa wlasciciela pliku */
        char spacesFG[MAX_DIR_SIZE][STR_SIZE];      /* spacje przed nazwa grupy */
        char spacesFS[MAX_DIR_SIZE][STR_SIZE];      /* spacje przed rozmiarem pliku */
        for(x=0; x<howManyFiles; x++){
            for(y=0; y<maxLenHL-howManyDigits(hardLinks[x]); y++)   /* liczba spacji = (maks. szer. kolumny) - (szer. aktualnej komorki) */
                spacesHL[x][y]=' ';
            spacesHL[x][y]='\0';
            for(y=0; y<maxLenFO-strlen(fileOwner[x]); y++)
                spacesFO[x][y]=' ';
            spacesFO[x][y]='\0';
            for(y=0; y<maxLenFG-strlen(fileGroup[x]); y++)
                spacesFG[x][y]=' ';
            spacesFG[x][y]='\0';
            for(y=0; y<maxLenFS-howManyDigits(fileSize[x]); y++)
                spacesFS[x][y]=' ';
            spacesFS[x][y]='\0';
        }

        /*** Wypisz wszystkie wyznaczone informacje o plikach ***/
        for(x=0; x<howManyFiles; x++){
            for(y=0; y<howManyFiles; y++){
                if(!strcmp(fileNameSorted[x],fileName[y])){
                    if(!isDiffYear){        /* jesli rok sie nigdzie nie rozni, to wypisz dane bez niego... */
                        printf("%s %s%d %s%s %s%s %s%d %c%d-%c%d %c%d:%c%d %s%s\n",accessPerms[y],spacesHL[y],hardLinks[y],fileOwner[y],spacesFO[y],fileGroup[y],spacesFG[y],spacesFS[y],fileSize[y],addChar(modifMonth[y],'0'),modifMonth[y],addChar(modifDay[y],'0'),modifDay[y],addChar(modifHour[y],'0'),modifHour[y],addChar(modifMin[y],'0'),modifMin[y],fileName[y],linkPath[y]);
                    } else {                /* ...w przeciwnym razie dopisz rowniez rok w dacie ostatniej modyfikacji */
                        printf("%s %s%d %s%s %s%s %s%d %d-%c%d-%c%d %c%d:%c%d %s%s\n",accessPerms[y],spacesHL[y],hardLinks[y],fileOwner[y],spacesFO[y],fileGroup[y],spacesFG[y],spacesFS[y],fileSize[y],modifYear[y],addChar(modifMonth[y],'0'),modifMonth[y],addChar(modifDay[y],'0'),modifDay[y],addChar(modifHour[y],'0'),modifHour[y],addChar(modifMin[y],'0'),modifMin[y],fileName[y],linkPath[y]);
                    }
                    break;
                }
            }
        }
    }

    /* === TRYB 2: Informacje o wskazanym pliku === */
    else if(argc==2){
        struct stat info;
        if(lstat(argv[1],&info) != 0){
            perror("ERROR (lstat(): Nie udalo sie otworzyc pliku)");
            return 1;
        }

        char month[12][STR_SIZE];          /* tablica ze slownymi nazwami miesiecy w dopelniaczu */
        strcpy(month[0],"stycznia");
        strcpy(month[1],"lutego");
        strcpy(month[2],"marca");
        strcpy(month[3],"kwietnia");
        strcpy(month[4],"maja");
        strcpy(month[5],"czerwca");
        strcpy(month[6],"lipca");
        strcpy(month[7],"sierpnia");
        strcpy(month[8],"wrzesnia");
        strcpy(month[9],"pazdziernika");
        strcpy(month[10],"listopada");
        strcpy(month[11],"grudnia");

        char filePath[PATH_MAX];        /* pelna sciezka do pliku */
        char linkPath[PATH_MAX];        /* pelna sciezka do pliku, na ktory wskazuje link symboliczny */
        int fileSize;                   /* rozmiar pliku */
        char accessPerms[STR_SIZE];     /* uprawnienia */
        int accessDay, accessMonth, accessYear, accessHour, accessMin, accessSec;       /* data ostatniego dostepu */
        int modifDay, modifMonth, modifYear, modifHour, modifMin, modifSec;             /* data ostatniej modyfikacji */
        int changeDay, changeMonth, changeYear, changeHour, changeMin, changeSec;       /* data ostatniej zmiany */
        int isTxtFile = 1;              /* zmienna przechowujaca informacje czy plik moze byc plikiem tekstowym */

        /*** Wyznacz pelna sciezke dla przetwarzanego pliku ***/
        char tmp[PATH_MAX];
        getcwd(tmp,PATH_MAX);
        strcat(tmp,"/");
        strcat(tmp,argv[1]);
        strcpy(filePath,tmp);

        /*** Wyznacz uprawnienia do pliku ***/
        char rwx[4]={'x','w','r','\0'}, ugs[4]={'s','s','t','\0'};      /* tablice pomocnicze */
        int mask=07, value, t=9, i, j;
        accessPerms[10] = '\0';             /* na miejscu 0 bedzie stal symbol typu pliku, ...*/
	    value = info.st_mode & mask;        /* ...a na miejscach 1-9 symbole uprawnien z tablicy rwx */
        for(i=0; i<3; i++){
            for(j=0; j<3; j++){
                if(value & 1){
                    accessPerms[t--] = rwx[j%3];
                } else {
                    accessPerms[t--] = '-';
                }
                value >>= 1;
            }
            mask <<= 3;
            value = (info.st_mode & mask) >> ((i+1)*3);
        }
        for(j=3; j>0; j--){                         /* ustawianie set-user-ID, set-group-ID i sticky bit */
            if(value & 1){
                accessPerms[j*3] = ugs[j%3];
            }
            value >>= 1;
        }
        mask <<= 4;         /* tworzenie maski = 17 (do odczytu typu pliku) */
        mask += 010000;
        value = (info.st_mode & mask) >> 12;

        if(value == 04){                            /* ustalanie typu pliku (4 = katalog) */
            accessPerms[0] = 'd';
        }
        if(value == 010){                           /* 10 = plik zwykły */
            accessPerms[0] = '-';
        }
        if(value == 012){
            accessPerms[0] = 'l';     /* 12 = link symboliczny */
            char buf[PATH_MAX];
            realpath(argv[1],buf);            /* pobierz sciezke do linku symbolicznego */
	    strcpy(linkPath,buf);
        }

        fileSize = info.st_size;        /* pobierz rozmiar pliku w bajtach */

        /*** Pobierz i sformatuj daty dostepu, modyfikacji i zmiany ***/
        time_t date;
        struct tm *dateTime;                                
        
        date = info.st_atime;                       /* pozyskaj wartosc przechowujaca date i czas ostatniego dostepu */
        dateTime = localtime(&date);                /* zamien time_t na powszechny format daty i czasu */
        accessDay = dateTime->tm_mday;
        accessMonth = dateTime->tm_mon;
        accessYear = 1900+(dateTime->tm_year);
        accessHour = dateTime->tm_hour;
        accessMin = dateTime->tm_min;
        accessSec = dateTime->tm_sec;

        date = info.st_mtime;                       /* pozyskaj wartosc przechowujaca date i czas ostatniej modyfikacji */
        dateTime = localtime(&date);
        modifDay = dateTime->tm_mday;
        modifMonth = dateTime->tm_mon;
        modifYear = 1900+(dateTime->tm_year);
        modifHour = dateTime->tm_hour;
        modifMin = dateTime->tm_min;
        modifSec = dateTime->tm_sec;

        date = info.st_ctime;                       /* pozyskaj wartosc przechowujaca date i czas ostatniej zmiany */
        dateTime = localtime(&date);
        changeDay = dateTime->tm_mday;
        changeMonth = dateTime->tm_mon;
        changeYear = 1900+(dateTime->tm_year);
        changeHour = dateTime->tm_hour;
        changeMin = dateTime->tm_min;
        changeSec = dateTime->tm_sec;

        /*** Wypisz wszystkie informacje o wskazanym pliku ***/
        printf("Informacje o %s:\n",argv[1]);

        printf("Typ pliku:   ");
        if(accessPerms[0] == '-') printf("zwykly plik\n");
        if(accessPerms[0] == 'd') printf("katalog\n");
        if(accessPerms[0] == 'l') printf("link symboliczny\n");

        printf("Sciezka:     %s\n",filePath);
        if(accessPerms[0] == 'l') printf("Wskazuje na: %s\n",linkPath);
        
        printf("Rozmiar:     ");
        if(fileSize == 1){
            printf("%d bajt\n",fileSize);
        }
        else if(((fileSize%10 >= 5 && fileSize%10 <= 9) || fileSize%10 == 0) || (fileSize%100 >= 11 && fileSize%100 <= 14)){
            printf("%d bajtow\n",fileSize);
        }
        else{
            printf("%d bajty\n",fileSize);
        }

        printf("Uprawnienia: ");
        for(i=1; i<10; i++){
            printf("%c",accessPerms[i]);
        }
        printf("\n");

        printf("Ostatnio uzywany:        %c%d %s %d roku o %c%d:%c%d:%c%d\n",addChar(accessDay,' '),accessDay,month[accessMonth],accessYear,addChar(accessHour,'0'),accessHour,addChar(accessMin,'0'),accessMin,addChar(accessSec,'0'),accessSec);
        printf("Ostatnio modyfikowany:   %c%d %s %d roku o %c%d:%c%d:%c%d\n",addChar(modifDay,' '),modifDay,month[modifMonth],modifYear,addChar(modifHour,'0'),modifHour,addChar(modifMin,'0'),modifMin,addChar(modifSec,'0'),modifSec);
        printf("Ostatnio zmieniany stan: %c%d %s %d roku o %c%d:%c%d:%c%d\n",addChar(changeDay,' '),changeDay,month[changeMonth],changeYear,addChar(changeHour,'0'),changeHour,addChar(changeMin,'0'),changeMin,addChar(changeSec,'0'),changeSec);

        for(i=0; i<4; i++){                 /* petla sprawdzajaca czy plik moze byc plikiem tekstowym... */
            if(accessPerms[3*i] != '-'){
                isTxtFile = 0;
                break;
            }
        }

        if(isTxtFile){                      /* ...jesli tak, to wypisz dwie pierwsze linie jego zawartosci */
            printf("Poczatek zawartosci:\n");
            int file, n, newLine = 0;
            char buf[1];
            if((file = open(argv[1],O_RDONLY)) == -1){
                perror("ERROR (open(): Nie udalo sie otworzyc pliku)");
                return 1;
            } else {
                while((n = read(file,buf,sizeof(buf))) != 0){
                    if(newLine == 2) break;
                    write(1,buf,n);
                    if(buf[0] == '\n') newLine++;
                }
		close(file);
            }
        }
    }

    /* Ostrzezenie dla wiekszej liczby argumentow */
    else {
        printf("WARNING: Podano zbyt wiele argumentow!\n");
        printf("Program myls dziala w dwoch trybach:\n");
        printf(" * Tryb 1 (bez podawania argumentow): szczegolowe informacje o plikach w biezacym katalogu\n");
        printf(" * Tryb 2 (z jednym parametrem): szczegolowe informacje o pliku podanym jako argument\n");
        printf("Uruchom program ponownie, podajac wlasciwa liczbe argumentow.\n");
    }

    return 0;
}