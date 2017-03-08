#include <msp430.h>
#include <stdint.h>

short int tab[8];
int size_tab=8;
short int direction =0x01; // przelacznik od zmiany kierunku
short int right	= 1;  // decyduje w ktora strone przsuwa napis
short int startShift= 0; // czy przesuwac
const short int refresh = 64; //  32678Hz
short int position=0;  //indeks tablicy do wyswietlenia
short int move=0;  //indeks elementu tablic od ktorego zaczyna sie napis
int counter=0;
int i=0;
short int temp;
int counter_odszum = 0;
int odszum = 0;
init()
{
			//tablica na slowo
	    tab[0]= 224+1;
		tab[1]= 224+2;
		tab[2]= 224+3;
		tab[3]= 224+4;
	    tab[4]= 224+5;
		tab[5]= 0;
		tab[6]= 0;
		tab[7]= 0;
	//	tab[8]= 0;
	 //   tab[9]= 0;
	//	tab[10]= 0;

	TACTL = MC_1 | ID_0 | TASSEL_1; //mode up ,bez dzielnika,init clock;
	BCSCTL1 &= ~ XTS; // wolny tryb ACLK

	TACCTL0 |= CCIE; //wlacz przerwania zegara
	TACCR0 = refresh; //zliczaj 1 sekunde

	P1DIR=0x00 ; // kieruenk wejsciowy
	P1IE |= direction;  // wlacz przerwania dla zmiany kierunku...
	P1IES &= ~direction;  // .. po rosnacym zboczu
	P1IFG &= ~direction;  //  wyczysc rejest flag

	P4DIR = 0xff; // pozycja to highlight
	P5DIR = 0xff; // pozycja do wyswietlenia

	_BIS_SR(LPM3_bits + GIE); // idz spac i wlacz przerwania
}


void refresh_display()
{
	P4OUT |= 0xFF; // wylacz wyswietlacz
	if(position>7)
	{
		position=0;
	}
	if(tab[position]>0)
	{
		P4OUT &= ~(1<<position); // wybierz nowa pozycje do wyswietlanie !!!
		P5OUT = tab[position]; // wyswietl to na pozycji wybranej powyzej
	}
	position++;
}

void shift()
{
	if(right)
		{
		    temp=tab[size_tab-1];
		    for(i=size_tab-2;i>-1;i--)
	        {
	            tab[i+1]=tab[i];
	        }
	        tab[0]=temp;
		}
		else if(!right)
	    {
	        temp=tab[0];
		    for(i=1;i<size_tab;i++)
	        {
	           tab[i-1]=tab[i];
	        }
	        tab[size_tab-1]=temp;
	    }
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD ; // wylacz watchdoga
	init();
	while(1)
	{
		counter++;//liczymy ile razy odswiezylismy
		if(counter==512)  // jesli 1 sekunda minela
		{
				startShift=1; //moge przesunac
				counter=0;
		}
		if(startShift)//jesli 1 sekunda..
		{
			startShift=0;
			shift();//...to przesun napis
		}
		_BIS_SR(LPM3_bits); // idz spac
	}
}

#pragma vector=TIMERA0_VECTOR
__interrupt void timerA_int(void)  // wchodzimy tu co 512 razy/sekunde
{
	refresh_display(); // odswiez ekran
	LPM3_EXIT;  // obudz sie
}

#pragma vector=TIMERA1_VECTOR
__interrupt void timerA1_int(void)  // wchodzimy tu co  razy/sekunde
{
	if(odszum)
		counter_odszum++;
	if (counter_odszum > 1000)
	{
		odszum=0;
		if(P1IN & right) // przesun w lewo
		{
			right=0;
		}
		else if(P1IN & !right) // w prawo
		{
			right=1;
		}
		P1IFG &= ~direction; // wyczysc flagi
		TACCTL1 &= ~CCIE;
	}
	LPM3_EXIT;  // obudz sie
}

#pragma vector=PORT1_VECTOR
__interrupt void switches_int(void) // wchodzimy tu jak zmienimy stan przelacznika
{
		if(P1IFG & direction)
		{
			counter_odszum=0;
			odszum=1;
			TACCTL1 |= CCIE | CM_1 | CCIS_0 ;
			TACCR1 = 8; //zliczaj do 8
		}
}
