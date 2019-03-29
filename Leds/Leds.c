#class auto

enum portName {PA,PF,PB};
enum bitNumber {Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7};

#define prenderLed(x) setOutput(PA,x,1)
#define apagarLed(x) setOutput(PA,x,0)
#define leerBoton(bit) getInputButton(bit)

void setOutput(enum portName p_port, enum bitNumber p_pin, unsigned char p_state){
   char* shadow;
   int port;
   if(p_port == PA){
		port = PADR;
   	shadow = &PADRShadow;
   } else if(p_port == PF){
		port = PFDR;
   	shadow = &PFDRShadow;
   } else if(p_port == PB){
		port = PBDR;
   	shadow = &PBDRShadow;
   } else {
      return;
   }
   BitWrPortI(port, shadow, p_state, p_pin);
}

char getInput(enum portName p_port, enum bitNumber p_pin){
    return BitRdPortI(p_port, p_pin);
}

char getInputButton(enum bitNumber p_pin){
	int bit;
   int port;
   switch(p_pin){
   	case 0:
   	case 1:
   	case 2:
   	case 3:
      	port = PBDR;
      	break;
   	case 4:
   	case 5:
   	case 6:
   	case 7:
      	port = PFDR;
     		break;
      default:
      	return -1;
   }
   if(p_pin == Bit0){
   	bit = 2;
   } else if(p_pin == Bit1){
		bit = 3;
   }  else if(p_pin == Bit2){
		bit = 4;
   } else if(p_pin == Bit3){
		bit = 5;
   } else if(p_pin == Bit4){
		bit = 4;
   } else if(p_pin == Bit5){
		bit = 5;
   } else if(p_pin == Bit6){
		bit = 6;
   } else if(p_pin == Bit7){
		bit = 7;
   } else {
     return -1;
   }
   return getInput(port, bit);
}

void delay(long milisegundos){
	long tiempoAhora;
   tiempoAhora = MS_TIMER;
   while((MS_TIMER - tiempoAhora) < milisegundos){
   }
}

int getAnalogInput(unsigned char p_input){
   char leer[5];
   char guardar[29];
   int i;
   leer[0] = 0x02;
   leer[1] = 0x48;
   leer[2] = 0x49;
   leer[3] = 0x03;
   leer[4] = 0x48;
   printf(leer);
   serCputs(leer);
   i=0;
   while(i < 29){
   	guardar[i] = serCgetc();
   }
   printf("%d",guardar);

}

configurar(){
	WrPortI(SPCR, &SPCRShadow, 0x084); // Puerto A (LEDS) como output
   BitWrPortI(PBDDR, &PBDDRShadow, 0x00, 2); // Puerto B bit2
   BitWrPortI(PBDDR, &PBDDRShadow, 0x00, 3); // Puerto B bit3
   BitWrPortI(PBDDR, &PBDDRShadow, 0x00, 4); // Puerto B bit4
   BitWrPortI(PBDDR, &PBDDRShadow, 0x00, 5); // Puerto B bit5

   BitWrPortI(PFDDR, &PFDDRShadow, 0x00, 4); // Puerto F bit4
   BitWrPortI(PFDDR, &PFDDRShadow, 0x00, 5); // Puerto F bit5
   BitWrPortI(PFDDR, &PFDDRShadow, 0x00, 6); // Puerto F bit6
   BitWrPortI(PFDDR, &PFDDRShadow, 0x00, 7); // Puerto F bit7
   serCopen(9600); //configurar serial port c
}

main()
{
	int i;
	configurar();
   //getAnalogInput(58);
   //prender Led
   for(i = 0; i<8; i++ ) {
    prenderLed(i);
    delay(500);
   }
   for(i = 0; i<8; i++ ) {
    apagarLed(i);
   }

   while(1){
   	for( i = Bit0; i <= Bit7; i++ ) {
      	if (leerBoton(i) == 0){
   	   	prenderLed(i);
	      }else {
	        	apagarLed(i);
	      }
      }
   	printf("estado");
   	printf("%d\n", BitRdPortI(PBDR, 2));                
   }
}


