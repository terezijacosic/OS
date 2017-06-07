#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // potrebno za sleep!!!

/*Svaka dretva nakon isteka jedne sekunde (van kriticna odsjecka) provjerava ima li slobodnih stolova te sluèajno odabire jedan od njih.
Nakon odabira, dretva ulazi u kritièni odsjeèak te ponovo provjerava je li odabrani stol slobodan. Ako jest,
oznaèava stol zauzetim i izlazi iz kritiènog odsjeèka. U oba slucaja, nakon obavljene operacije (još uvijek unutar kritièna odsjeèka)
ispisuje trenutno stanje svih stolova te podatke o obavljenoj rezervaciji. Prilikom ispisa za svaki stol mora biti vidljivo je li slobodan
ili broj dretvea koja je taj stol rezervirala.
Svaka dretva ponavlja isti postupak sve dok više nema slobodnih stolova. Program završava kada sve dretve završe. */

int broj_stolova, broj_dretvi;
int *TRAZIM, *BROJ, *stolovi, *slobodni_stolovi;

//zajednièke varijable: TRAZIM[0..n-1], BROJ[0..n-1]

int max( int prvi, int drugi)
{
    return ((prvi<drugi)? drugi : prvi );
}

int odaberi_stol()
{
	int i, brojac=0;
	for( i=0; i<broj_stolova; ++i)
	{
	    if(stolovi[i] == 0)
            slobodni_stolovi[brojac++] = i;
	}
	return slobodni_stolovi[rand()%brojac];
}


int ima_li_slobodnih()
{
	int slobodni = 0, i;
	for( i=0; i<broj_stolova; ++i )
	{
	    if(stolovi[i] == 0)
            return slobodni = 1;
	}
	return slobodni;
}

void ispis_stanja()
{
    int i;
	for( i=0; i < broj_stolova; ++i )
    {
        if(stolovi[i]) printf("%d", stolovi[i]);  //printf("%d"); ovo je tebi pisalo. Tu ti je bila gre¨ka pa nije valjao ispis.
        else printf("-");
    }
	printf("\n");
}

void udi_u_kriticni_odsjecak( int i)
{
    int j, pomocna= BROJ[0];
    TRAZIM[i] = 1;
    //BROJ[i] = BROJ[0];
    for( j=1; j<broj_dretvi; ++j )
        pomocna = max( pomocna, BROJ[j] );
    BROJ[i] = pomocna + 1;

    //BROJ[i] = max(*BROJ) + 1;
    TRAZIM[i] = 0;

    for( j=0; j<broj_dretvi; ++j )
     {
        while ( TRAZIM[j] ) {} // dok je TRAŽIM[j] <> 0 èini ništa
        while ( BROJ[j]!=0 && ((BROJ[j] < BROJ[i]) || (BROJ[j] == BROJ[i] && j < i))) {}
            //dok je BROJ[j] <> 0 && (BROJ[j] < BROJ[i] || (BROJ[j] == BROJ[i] && j < i)) èini ništa
     }
}


void izadi_iz_kriticnog_odsjecka(int i)
{
   BROJ[i] = 0;
}


void *dretva( void *x )
{
    int i = *( (int *)x ), stol;

    while(ima_li_slobodnih())
	{
		int odabrani = odaberi_stol();
		printf("Dretva %d: odabirem stol %d \n", i+1, odabrani+1);
		sleep(1);

		udi_u_kriticni_odsjecak(i);
		if( stolovi[odabrani]==0 )
		{
			printf("Dretva %d: rezerviram stol %d, stanje: \n", i+1, odabrani+1);
			stolovi[odabrani] = i + 1;
		}
		else
			printf("Dretva %d: neuspjela rezervacija stola %d, stanje: \n", i+1, odabrani+1);

		ispis_stanja();
		izadi_iz_kriticnog_odsjecka(i);
	}
 // return;
}

void zauzmi_memoriju()
{
    int i;

    stolovi = (int*)malloc(sizeof(int)*broj_stolova);
    slobodni_stolovi = (int*)malloc(sizeof(int)*broj_stolova);
    for (i=0; i<broj_stolova; ++i)
    {
        stolovi[i] = 0; slobodni_stolovi[i] = 0;
    }

    TRAZIM = (int*)malloc(sizeof(int)*broj_dretvi);
    BROJ = (int*)malloc(sizeof(int)*broj_dretvi);
    for (i=0; i<broj_dretvi; ++i)
    {
        TRAZIM[i]=0; BROJ[i]=0;
    }
}

void oslobodi_memoriju()
{
	free(stolovi);
	free(TRAZIM);
	free(BROJ);
	free(slobodni_stolovi);
}


int main(int argc, char **argv )
{
    if (argc < 2)
    {
        printf("Pokretanje s ./lamport broj_dretvi broj_stolova\n");
        return 1;
    }

    sscanf( argv[1], "%d", &broj_dretvi );
    sscanf( argv[2], "%d", &broj_stolova );

    printf("Broj dretvi: %d, broj stolova: %d\n", broj_dretvi, broj_stolova);

    zauzmi_memoriju();

    pthread_t thr_id[broj_dretvi];
    int i, id[broj_dretvi];

    for (i=0; i<broj_dretvi; ++i)
    {
        id[i] = i;
         if (pthread_create(&thr_id[i], NULL, dretva, &id[i]) != 0)
        {
            printf("Greska pri stvaranju dretve!\n");
            return 1;
        }
        sleep(1);
    }

    for (i=0; i<broj_dretvi; ++i)
        pthread_join(thr_id[i], NULL);

    oslobodi_memoriju();

   return 0;
}

