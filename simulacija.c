#include <stdio.h>
#include <stdlib.h>
#ifdef __unix__
# include <unistd.h>
#elif defined _WIN32
# include <windows.h>
#define sleep(x) Sleep(1000 * x)
#endif

/// RASPOREDJIVANJE PREMA PRIORITETU

int t;   //simulacija vremena (trenutno vrijeme)
int zadnje = 0;

typedef struct {
	int vrijeme_prispijeca;
	int id; //1, 2, 3, ...
	int p;  //preostalo vrijeme rada
	int prio;
} dretva ;

#define MAX_DRETVI	7
dretva *P[MAX_DRETVI]; //red pripravnih dretvi, P[0] = aktivna dretva

/* podaci o dogadajima pojave novih poslova - dretvi */
#define DRETVI	6
int nove[DRETVI][5] =
{
	/* trenutak dolaska, id, p, prio, rasp (prio i rasp se koriste ovisno o rasporedivacu) */
	{ 1,  3, 5, 3, 1 }, /* rasp = 0 => PRIO+FIFO; 1 => PRIO+RR */
	{ 3,  5, 6, 5, 1 },
	{ 7,  2, 3, 5, 0 },
	{ 12, 1, 5, 3, 0 },
	{ 20, 6, 3, 6, 1 },
	{ 20, 7, 4, 7, 1 },
};

void ispis_stanja ( int ispisi_zaglavlje )
{
	int i;

	if ( ispisi_zaglavlje ) {
		printf ( "  t    AKT" );
		for ( i = 1; i < MAX_DRETVI; i++ )
			printf ( "     PR%d", i );
		printf ( "\n" );
		return;
	}

	printf ( "%3d ", t );
	for ( i = 0; i < MAX_DRETVI; i++ )
	{
	    if ( P[i] != NULL )
			printf ( "  %d/%d/%d ",
				 P[i]->id, P[i]->prio, P[i]->p ); // p je preostalo vrijeme rada
		else
			printf ( "  -/-/- " );
	}
	printf ( "\n");
	return;
}

void poslozi()
{
		int i, nered, prio1, prio2;
		dretva *temp;
		temp = (dretva*)malloc( sizeof(dretva));

		do{
			nered = 0;
			for ( i=0; i< zadnje; i++ )
			{
      	if ( P[i] == NULL )
          prio1 = 0;
        else
          prio1 = P[i]->prio;

        if ( P[i+1] == NULL )
          prio2 = 0;
        else
          prio2 = P[i+1]->prio;

				if ( prio1 < prio2 )
				{
							temp = P[i];
							P[i] = P[i+1];
							P[i+1] = temp;
							nered = 1;
				}
			}
		} while( nered );
}


void ima_li_novih( int t )
{
    int i;
    
    for( i=0; i<DRETVI; i++ )
        if( nove[i][0] == t )
        {
							P[zadnje] = (dretva *) malloc (sizeof(dretva));
							P[zadnje] -> vrijeme_prispijeca = nove[i][0];
							P[zadnje] -> id = nove[i][1];
							P[zadnje] -> p = nove[i][2];
							P[zadnje] -> prio = nove[i][3];

            printf("%3d -- nova dretva id=%d, prio=%d\n", t, P[zadnje]->id, P[zadnje]->prio);
            poslozi();
            ispis_stanja(0);
						++zadnje;
        }

    return;
}

int main ()
{
    int i;
    t = 0;

    for ( i=0; i<MAX_DRETVI; i++ )
        P[i] = NULL;

	ispis_stanja( 1 );

	while ( 1 )
    {
		if ( P[0] != NULL )  // P[0] je aktivna dretva
		{
            P[0]->p--; //odradjena jedinica vremena, smanji vrijeme izvrsavanja

            if ( P[0]->p == 0 )
            {
                printf("Dretva %d je gotova!\n", P[0]->id);
								free(P[0]);
                P[0] = NULL;
                --zadnje;  // zadnje ne-null mjesto u nizu P
            }
        }

        if ( P[0] == NULL || (P[1] != NULL && (P[0]->prio < P[1]->prio)) )
            poslozi(); // P[] po prioritetu dretvi

        ima_li_novih( t );
        ispis_stanja( 0 );

		sleep(1); // simulacija rada dretve
		t++; //prosla jedinica vremena
	}

    return 0;
}
