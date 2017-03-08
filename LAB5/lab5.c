#include <msp430.h>
#include <stdint.h>
#include <string.h>
#define DISPLAY_SIZE 7

char tab[]={"Lubie Nalesniki \r"};
unsigned int tab_size = sizeof(tab)/sizeof(tabb[0]); 
char display[DISPLAY_SIZE];
short int right	= 1;  // decyduje w ktora strone przsuwa napis
unsigned short wiekszy_disp = 0; //flaga mowi czy rozmiar wyswietlacza jest wiekszy od napisu
short int startShift= 0; // czy przesuwac
const short int refresh = 64; //  32678Hz
int counter=0;
int i=0;
char temp;
char transmiter;
char receiver;
int keypressed=0;
int attempts=0;
int size_tab = 0;


void init()
{
	//obliczanie rozmiaru tablicy
	UCTL0 |= SWRST;

    TACTL = MC_1 | ID_0 | TASSEL_1; //clock init ACLK, mode Up, no divider.
    BCSCTL1 &= ~XTS; //'slow mode' ACLK
    TACCR0=refresh;

    BCSCTL2 |= SELS;// Korzystamy z zewnetrznego kwarcu jako zrodla taktowania (XT2CLK: Optional high-frequency oscillator 450-kHz to
//  8-MHz range. )
    P3SEL |= 0x30; //ustawiamy pierwsza i druga nozke jako Rx i Tx // P3.4 UTXD0 oraz P3.5 URDX0
    ME1 |= UTXE0 + URXE0;// Wlaczamy modul do wysylania danych i odbierania w UART
    UCTL0 |= PEV + PENA + CHAR + SPB;// Ustawiamy bit parzystosci, dlugosc danych 8 bitow, i 2 bity stopu

// Ustawienia dla 115200Hz (dla UART: max 115200 hz)
    UTCTL0 |= SSEL1; //typical 1,048,576-Hz SMCLK. --> LPM1
    UBR00 = 0x40; // pierwszy dzielnik czestotliwosci
    UBR10 = 0x00; // drugi dzielnik
    UMCTL0 = 0x00;
    UCTL0 &= ~SWRST; // wylaczenie software reset

   TACCTL0 |= CCIE; //clock interrupts enabled
   IE1 |= URXIE0; //przerwania wlaczone dla odbierania/dla wysylania
   IE1 |= UTXIE0;
    _BIS_SR(LPM1_bits + GIE); //deep sleep with smclk running + interrupts enabled
}

void shift()
{
	if(right)
	{
		  temp=tab[size_tab-2];
		  for(i=size_tab-3;i>-1;i--)
	      {
	          tab[i+1]=tab[i];
	      }
	      tab[0]=temp;
	}
	else if(!right)
	{
	      temp=tab[0];
		  for(i=1;i<size_tab-1;i++)
	      {
	         tab[i-1]=tab[i];
	      }
	      tab[size_tab-2]=temp;
	}
}



int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// kill the watchdog
    init();

    while (1)
    {
    	if(keypressed && right) // przesun w lewo
    	{
    			right=0;
    			keypressed=0;
    	}
    	else if(keypressed && !right) // w prawo
    	{
    			right=1;
    			keypressed=0;
    	}


    	if(counter==512)  // jesli 1 sekunda minela
    	{
    		startShift=1; //moge przesunac
    		counter=0;

    	}
    	if(startShift==1)//jesli 1 sekunda..
    	{
    		startShift=0;
    		shift();//...to przesun napis
    		IE1 |= UTXIE0;
    	}
        _BIS_SR(LPM1_bits + GIE);
    }
    return 0;
}

#pragma vector=TIMERA0_VECTOR
__interrupt void timerA_int(void)
{
	counter++;//liczymy ile razy odswiezylismy

    _BIC_SR_IRQ(LPM1_bits);
}

#pragma vector=USART0TX_VECTOR
__interrupt void usart0_tx(void)
{
	TXBUF0 = tab[attempts];
	attempts++;

	if(attempts>tab_size-1){
		attempts=0;
		IE1 &= ~UTXIE0;
	}

}

#pragma vector=USART0RX_VECTOR

__interrupt void usart0_rx (void)
{
// przerwanie od ukladu UART, odbieramy znak
	receiver = (char)RXBUF0;
	if('q'==receiver)
		keypressed=1;

    _BIC_SR_IRQ(LPM1_bits);

}


