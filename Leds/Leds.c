#class auto

enum portName { PA = PADR, PF = PFDR, PB = PBDR};
enum bitNumber {PA0,PA1,PA2,PA3,PA4,PA5,PA6,PA7};

#define prenderLed(x) setOutput(PADR,x,1)
#define apagarLed(x) setOutput(PADR,x,0)
#define leerBoton(puerto, bit) getInput(puerto, bit)
void setOutput(enum portName p_port, enum bitNumber p_pin, unsigned char p_state){
   char* shadow;
   if(p_port == PADR){
    shadow=&PADRShadow;
   }
   BitWrPortI(p_port, shadow, p_state, p_pin);
}

char getInput(enum portName p_port, enum bitNumber p_pin){
    return BitRdPortI(p_port, p_pin);
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
   getAnalogInput(58);
   //prender Led
   for(i = 0; i<8; i++ ) {
    prenderLed(i);
    delay(500);
   }
   for(i = 0; i<8; i++ ) {
    apagarLed(i);
   }

   while(1){
   	if (leerBoton(PB, 2) == 0){
      	prenderLed(PA0);
      }else {
        apagarLed(PA0);
      }

      if (leerBoton(PB, 3) == 0){
      	prenderLed(PA1);
      }else {
        apagarLed(PA1);
      }

      if (leerBoton(PB, 4) == 0){
      	prenderLed(PA2);
      }else {
        apagarLed(PA2);
      }
      if (leerBoton(PB, 5) == 0){
      	prenderLed(PA3);
      }else {
        apagarLed(PA3);
      }
      if (leerBoton(PF, 4) == 0){
      	prenderLed(PA4);
      }else {
        apagarLed(PA4);
      }
      if (leerBoton(PF, 5) == 0){
      	prenderLed(PA5);
      }else {
        apagarLed(PA5);
      }
      if (leerBoton(PF, 6) == 0){
      	prenderLed(PA6);
      }else {
        apagarLed(PA6);
      }
      if (leerBoton(PF, 7) == 0){
      	prenderLed(PA7);
      }else {
        apagarLed(PA7);
      }
   	printf("estado");
   	printf("%d\n", BitRdPortI(PBDR, 2));

   }
}


