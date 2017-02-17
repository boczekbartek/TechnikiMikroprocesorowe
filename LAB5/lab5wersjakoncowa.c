#include <msp430.h>
#include <stdint.h>
#include <string.h>

//rozmiar wyswietlacza
#define size_display 17

//tablica na napis
char tab[]={"Lubie Nalesniki "};

//zmienna przechowujaca rozmiar tablicy
int size_tab = 0;

//tablica na wyswietlacz
char display[size_display+1];

//kierunek przesuwania napisu
unsigned short int right	= 1;  

//flaga informujaca czy zaczac przesuwanie
unsigned short int startShift= 0; 

const short int refresh = 64; //  32768Hz

//licznik do odmierzania czasu
int counter=0;

char temp;
char receiver;
unsigned short int keypressed=0;
int attempts=0;

void init()
{
    size_tab=0;
	//obliczanie rozmiaru tablicy
	while (tab[size_tab] != '\0')
	{
		++size_tab;
	}

    int i=0;
	for(i=0;i<size_display;i++)
    {
		display[i]=tab[i];
    }
    display[size_display]='\r'; //na koncu wyswietlacza zawsze powrot karetki

	UCTL0 |= SWRST; //reset softwarowy


    TACTL = MC_1 | ID_0 | TASSEL_1; //clock init ACLK, mode Up, no divider.
    BCSCTL1 &= ~XTS; //Basic Clock odule Operation, bardzo nieskie złużycie energii, 'low frequency' ACLK
    TACCR0=refresh;  //wlaczenie timera, poprzez nadanie wartosci, timer liczy do tej wartosci

    //Basic clock system control 2
    BCSCTL2 |= SELS;// Korzystamy z zewnetrznego kwarcu jako zrodla taktowania, z SMCLK
        
    P3SEL |= 0x30; //ustawiamy pierwsza i druga nozke jako Rx i Tx // P3.4 UTXD0 oraz P3.5 URDX0

    //Module Enable Register
    ME1 |= UTXE0 + URXE0;// Wlaczamy modul do wysylania danych i odbierania w UART

    //  Parity Select + Parity Enable + character lenght + stop bit select
    UCTL0 |= PEV + PENA + CHAR + SPB;// Ustawiamy bit parzystosci, dlugosc danych 8 bitow, i 2 bity stopu


    // Ustawienia dla 115200Hz (dla UART: max 115200 hz)
    UTCTL0 |= SSEL1; //typowo 1,048,576-Hz SMCLK. --> LPM1
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
    int i = 0;
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
	for(i=0;i<size_display;i++)
		display[i]=tab[i];
	display[size_display]='\r';
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
    	if(startShift==1)
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
	TXBUF0 = display[attempts];
	attempts++; 

	if(attempts>size_display){
		attempts=0;
		IE1 &= ~UTXIE0;   //If UTXIE0 is set, the UTXIFG0 flag will not trigger a transfer
	}
}

#pragma vector=USART0RX_VECTOR

__interrupt void usart0_rx (void)
{
// przerwanie od ukladu UART, odbieramy znak
	receiver = (char)RXBUF0; //Receiver Buffer
	if('q'==receiver)
		keypressed=1;

    _BIC_SR_IRQ(LPM1_bits);
}