;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file

;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer


;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------

INITALIZATION:
					BIS.B 	#0FFh, &P3DIR	;okreslamy kierunek przeplywu => port P3 jest podłączony do modułu z wyświetlaczem 
					CLR.B 	&P3OUT          ;wyzeruj wyjscia
					
					MOV.B 	#003h, &P2IE    ;włączanie przerwań dla pinów 1 i 2 należących do portu 2
					MOV.B 	#003h, &P2IES   ;flaga przerwania (xIFG) ustawiana zboczem opadajacym
					BIS.B 	#000h, &P2DIR	;port P2 ustawiony jako wejściowy => post podłączony do modułu z przyciskami
					BIS.B 	#000h, &P1DIR   ;pirt P1 ustawiony jako wejsciowy => post podłączony do modułu z pokrętłami hex

					MOV.B 	&P1IN, R9  		;wczytuje wektor z hexa, rejestr R9 przechowuje liczbę którą będziemy chcieli wyświatlić 
					MOV.B	R9, &P3OUT		;inicjalizacja wyświetlacza


MAIN:				
					MOV.B #000H , R5 		;inicjalizacja flagi mówiącej czy przerwanie nastąpiło poprzez przycisk ładowania
					MOV.B 	R9, &P3OUT   	;wyświetlenie zawartości rejestu R9 na wyświetlaczu
					MOV.B 	#003h, &P2IE	;włączanie przerwań dla pinów 1 i 2 należących do portu 2
					BIS.W 	#GIE+CPUOFF+OSCOFF+SCG1+SCG0,SR ; wprowadzenie procesowa w tryb energooszczędny => LPM4
					NOP

					MOV.W   #00D00h, R15 	;inicjalizacja licznika potrzebnego do niwelacji drgań styków
					MOV.W   #00800h, R14 	;inicjalizacja drugiego z liczników potrzebnych do niwelacji drgań styków	

					BIT.B 	#001h,R5 		;sprawdzenie czy przewanie bylo spowodowanie przyciskiem ładowania
					JZ MAIN      			;jeżeli tak, to nie musimy unikać drgań styków


PREVENT_VIBRATIONS:	
					DEC.W   R15    			;zdekrementuj zawartość R15 => licznika
					JZ    	MAIN  			;jeżeli R15 == 0 skocz do MAIN
					BIT.B   #001h, &P2IN  	;sprawdz czy wciśnięty jest przycisk numer 1
					
					;;;NIE WIADOMO CZY TO DZIALA;;;
					MOV.B	#001, R10 		;flaga => zaczelismy zliczanie prawidlowego nacisniecia 
					JNZ 	TMP
					;;;NIE WIADOMO CZY TO DZIALA;;;

					JNZ 	PREVENT_VIBRATIONS   ;i jeżeli jest  != 0 to skocz do obsługi dragania styków
					DEC.W   R14   			;zdekrementuj zawartość w rejestrze R14
					JNZ 	PREVENT_VIBRATIONS


;;;NIE WIADOMO CZY TO DZIALA;;;
TMP:				
					MOV.W   #00800h, R14
					JMP PREVENT_VIBRATIONS
;;;NIE WIADOMO CZY TO DZIALA;;;




INCREMENT:			
;????????????????? tu powinno być raczej JZ MAIN jeśli dwa klawisze są wciśnięte ???????


					BIC.W 	#GIE, SR 	 	;nie przyjmuj przerwań
					;;jezeli spraw
					BIT.B   #001h, R6  		;czy oba klawisze są wciśnięte na raz
					MOV.B 	#000h, R6		;wyzeruj flage
					JNZ     MAIN 			;jeśli 2 guziki są wciśnięte to nie inkrementujemy
					INC.B   R9				;inkrementuj R9
					MOV.B   R9, &P3OUT  	;wyświetl aktualną liczbę (rejestr R9) na wyświetlacz
					JMP MAIN


INTERRUPT:
		 			BIT.B   #002h, &P2IN 	;czy drugi przycik (ładowania) jest wciśnięty
					JZ 		INT_LOAD   		;jeżeli tak, to rozpocznik operację ładowania
					CLR.B 	&P2IFG			
					BIC.W 	#CPUOFF, 0(SP)	;jeżeli nie, to powróć z przerwania po śladzie na stosie
					RETI

INT_LOAD:
		            MOV.B 	#000h, R6  		;wyzeruj R6 
					BIT.B   #001h, &P2IN 	;czy pierwszy przycisk jest wciśnięty
					JNZ 	INT_END   		;jezeli nie to skacz do INT_END
					MOV.B 	#001h, R6 		;jeżeli tak to ustaw flagę że dwa wciśnięte

INT_END:
					MOV.B 	&P1IN, R9  		;wczytaj wartość z pokręteł do rejestru R9
					MOV.B   R9, &P3OUT 		;a następnie wyświetl tą wartość na wyświetlaczu
					MOV.B #001h , R5		;ustaw flagę mówiącą, że zostało wykonane ładowanie
					CLR.B 	&P2IFG			
					BIC.W 	#CPUOFF+GIE, 0(SP);powróć z przerwania po śladzie na stosie
					RETI




;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack

;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            .sect	".int01"				;przypisanie przerwań odpowiednim portom
            .short	INTERRUPT


