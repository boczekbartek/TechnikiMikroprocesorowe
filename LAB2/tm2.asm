# File lab2.asm 
org 1800h    
 ;program do ładownia i zwiększania licznkia w kodzie Gray'a.     
 ;Rejestr D - licznik w binary; F - licznik w Gray;     
 ;E - słuzy do rozpoznawania czy inkrementujemy podczas przerwania wywołanego przez przyciski 1 (wczytujący z hexów na lampki); 
 ;H i B - czekanie podczas drgań styków    
 ;C - adres lampek;
 START:  
	LD SP,1900h  
	jr INITIALIZATION 
 
ds 0x1838-$,0 
 
INT:        ;przerwanie: wciśiety klawisz nr 2, użytkownik wcytuje cyfrę z hex
	EX AF, AF' 
	IN A,(01)      ;wczytujemy wejścia do A
	LD D,A       ;poczatek procedury zamiany binary -> gray   
	SRL A       ;przesuwa bity A o jeden w prawo bez przeniesienia  
	AND 7       ;upewniamy się że pierwszy bit jest zerem 
	XOR D       ;xor A z D, zapis do A, teraz już powinen być w A w gray  
	AND 15         
	LD L,A       ;wynik w gray przechowujemy w L 
	OUT (01), A 	;wyświetlanie na lampkach stanu 01 
	IN A, (01) 	;      
	BIT 4, A      ;sprawdzamy, czy klawisz inkrementacji jest wciśnięty (rejesrt 573->wejście numer 4, czyli 5 z kolei) BIT -> testuj bit 
	LD E, 0  		;zerujemy flagę ;LD nie wyþływa na zmianę znacznika flag
	JR NZ, KONIEC_INT ;jeśli instrukcja BIT dała 1-guzik nie jest wciśnięty (w rejestrze A na 4 bicie jest 1) wykonuje się skok warunkowy
	LD E, 1 		;jeżeli jump (JR) się nie wykona to ustawiamy flagę na 1
	 
KONIEC_INT:       ;wychodzimy z INT  
	EX AF, AF'  ;wymień ze sobą pary rejstrów
	EI 			;włączamy przerwania
	RETI 		;powrót ze śladu na stosie 
 
ds 0x1860-$,0  
 
INITIALIZATION: 
	IM 1      ;tryb 1 przerwań 
	
	IN A,(01) ;ładujemy to co jest w hexie na lampki  

	LD D,A      ;zamiana w gray   
	SRL A       
	AND 7      
	XOR D  
	AND 15         
	LD L,A      ;koniec zamiany, w L jest liczba z pokręteł hex   
	LD E, 0      ;zerujemy rejestr E, inicjalizujemy flagę E na 0, czyli nie jesteśmy w przerwaniu
 	EI        ;przerwania włączone 

GLOWNA_PETLA: 
	LD A,L        
	OUT (01), A      ;wyswietla na lampkach zawartosc L  
	IN A, (01)     ;ładujemy do A stan przycisków 
	BIT 4, A     ;sprawdzamy, czy został wciśnięty przycisk od inkremetnacji 
	JR NZ, GLOWNA_PETLA   ;jeśli nie, powtarzamy pętle  
	LD H, 270     ;licznik prób  
	LD B, 230     ;licznik udanych prób 
 
DRGANIA_STYKOW:        
	DEC H                      ;zmniejsza H i ustawia znacznik Z na 1 jeśli wynikiem dekrementacji jest 0  
	JR Z, GLOWNA_PETLA          ;jeśli H = 0 - klawisz nie wciśnięty i wracamy do głównej pętli  
	IN A, (01)         		;wczytujemy guzik
	BIT 4, A              ;sprawdzenie czy guzik 2(od inkrementacji) jest wciśnięty ustawiamy znaczik Z  
	JR NZ, TMP       ;jeśli not Z switch w tym momencie niewciśnięty, DRGANIA_STYKOW od początku  wraz 
						;inicjalizacją licznika 
	DEC B                     ;klawisz wciśnięty, dekremenmtuje B i ustawia znacznik Z  
	JR Z, INKREMENTUJ_LICZNIK ;jeśli B = 0 -> klawis NAPRAWDĘ wciśniety i idź do etykiety INKREMENTUJ_LICZNIK  
	JR DRGANIA_STYKOW; 	

TMP:
	LD B, 230 		;ponowna inicjalizacja licznika
	JR DRGANIA_STYKOW

INKREMENTUJ_LICZNIK: 
	DI  
	BIT 0, E     ;sprawdzam, czy wciśniety podczas przerwania  
	LD E, 0      ;zeruje flage  
	JR NZ, DDD     ;jeśli flaga = 1, nie inkrementuj i wyjdz do głwonej petli  
	INC D 
	LD A,D      ;poczatek zamiany w gray : kopiuje D do A 
	SRL A      ;przesuwa bity A o jeden w prawo bez przeniesienia 
	AND 7       
	XOR D 
	AND 15        
	LD L,A  
	OUT (01), A       ;wyswietla na lampkach zawartosc L    
	LD A,D  
	CP 16      ;gdy D==16, Z=1 ładuj na nowo  sprawdzamy czy w A i 16 (CP) jest to samo
	JR NZ, DDD     ;gdy nie ma potrzeby ładowania wróć do głównej pętli 
	LD D,0 
	LD A,0 
	LD L,A  
	OUT (01), A          

DDD:  
	EI      ;aby po wcisnieciu klawisza nie zliaczlalo  
	IN A,(01)  ;pobranie tego co jest w latchu
	BIT 4,A  
	JR Z, DDD  
	JP GLOWNA_PETLA     ;wracamy do głównej pętli 