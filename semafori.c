#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <semaphore.h>

sem_t PISI; // binarni
sem_t PUN;
sem_t PRAZAN;

typedef struct {
    int id;
    char* poruka;
} podatak;

int ULAZ = 0, IZLAZ = 0, M[5];

// struktura procesa proizvođač:
void *proizvodac(void *arguments)
{
    podatak *args = (podatak*)arguments;

	char* s = args->poruka;  //pročitaj niz znakova s tipkovnice u polje s
	int i = 0, str_len = strlen( s );

    while ( i <= str_len )
   {
      sem_wait( &PUN ); //čekaj_PUN; opci semafor

      sem_wait( &PISI ); // binarni semafor
      M[ULAZ] = s[i];
      printf("proizvodac%d  -> %c\n", args->id, s[i]);
      ULAZ = (ULAZ+1) % 5;
      sem_post( &PISI );

      sem_post( &PRAZAN );
      i = i+1;
      sleep(1);
   }
}

//Struktura procesa potrošač:

void *potrosac( void *pbr )
{
    int i = 0, j = 0;
    char znak, s[200];

   while ( i < *((int*)pbr) ) // dok svi nizovi nisu dosli do kraja
   {
      sem_wait( &PRAZAN );
      s[j] = M[IZLAZ];
      printf("potrosac <- %c\n",  s[j]);
	  if ( s[j] != 0 )
		j++;
      IZLAZ = (IZLAZ+1) %5;
      sem_post( &PUN );
      i = i+1;
   }
   printf("%s\n", s);
}

//----------------------------------

int main(int argc, char **argv)
{
    printf("U mainu smo\n");
    if ( argc == 1 ) printf("Premalo argumenata!");

	int i, broj_dretvi = argc - 1, j=0; // broj dretvi je jednak broju nizova koje obrađujemo

	for( i=0; i<5; i++ ){ M[i] = 0; }  // inicijalizacija meduspremnika na 0

	sem_init(&PISI, 0, 1);  //binarni semafor, stiti M i ULAZ
	sem_init(&PUN, 0, 5); // osem, broji prazna mjesta u spremniku pa je pocetno 5
	sem_init(&PRAZAN, 0, 0); // broji poruke u spremniku

	podatak podaci[broj_dretvi];
    pthread_t dretva_proizvodac[broj_dretvi];
    pthread_t dretva_potrosac;

	for( i=0; i<broj_dretvi; i++ )
    {
        podaci[i].id = i+1;
        podaci[i].poruka = argv[i+1];

		j = j + strlen(argv[i+1]) + 1;
		if ( pthread_create( &dretva_proizvodac[i], NULL, &proizvodac, (void *)&podaci[i])  )
        {
			printf("Greska prilikom stvaranja dretve!\n");
			exit(1);
		}
	}

	if( pthread_create( &dretva_potrosac, NULL, &potrosac, (void*)&j) != 0 )  //potrosac u svojoj funkciji prima cjelokupan broj znakova
        {
            printf("Greska prilikom stvaranja dretve!\n");
            exit(1);
        }

	for( i=0; i<broj_dretvi; i++)
        pthread_join( dretva_proizvodac[i], NULL);

	pthread_join( dretva_potrosac, NULL);

	sem_destroy(&PISI);
	sem_destroy(&PUN);
	sem_destroy(&PRAZAN);

	return 0;
}
