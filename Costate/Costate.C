#class auto
enum portName {PA,PF,PB};
enum bitNumber {Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7};

#define prenderLed(x) setOutput(PA,x,1)
#define apagarLed(x) setOutput(PA,x,0)

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


typedef struct Events{
	char command;
   char param;
   unsigned long time;
} Event;

#define MAX_EVENTOS 10
configurar(){
	WrPortI(SPCR, &SPCRShadow, 0x084); // Puerto A (LEDS) como output
  	BitWrPortI(PEDDR, &PEDDRShadow, 1, 5);
}

esperar_texto(char* texto){
   while(1){
    	costate{
      	waitfor(getswf(texto)); //Esperar a que el usuario ingrese algo
      	break;
   	}
	}
}

consumir_eventos(int max_i){
	for(i=0;i<ev;i++){
   	if(eventos[i].time == read_rtc()){
      	if(eventos[i].command=='1'){
      		prenderLed(eventos[i].param);
         } else {
         	apagarLed(eventos[i].param);
         }
   	}
	}
}

main()
{
	Event eventos[MAX_EVENTOS];
	int i;
   char uno;
   char texto[10];
   char texto2[10];
   int response;
   int ev;
   ev = 0;
   configurar();
   uno = '1';
   while(1){
  	 costate{
    BitWrPortI(PEDR, &PEDRShadow, 1, 5);
    waitfor(DelayMs(400));
     BitWrPortI(PEDR, &PEDRShadow, 0, 5);
     waitfor(DelayMs(800));
    }

    consumir_eventos(ev);

    costate{
      printf("\nIngrese 1 para Fijar la hora del reloj de tiempo real (RTC) del Rabbit\n");
      printf("Ingrese 2 para Consultar la hora del RTC del Rabbi\n");
      printf("Ingrese 3 para Agregar o quitar un evento de calendario.\n");
      printf("Ingrese 4 para Consultar la lista de eventos de calendario activos.\n");
	   waitfor(getswf(texto)); //Esperar a que el usuario ingrese algo
      printf(texto);
      switch(texto[0]){
         case '1':
            printf("Ingrese la hora\n");
            esperar_texto(texto2);
            write_rtc(atoi(texto2));
      		break;
         case '2':
      		printf("%d",read_rtc());
         	break;
         case '3':
            printf("Ingrese 1 para prender o ingrese 0 para apagar\n");
            esperar_texto(texto2);
            eventos[ev].command = texto2[0];

            printf("Ingrese el numero de led\n");
            esperar_texto(texto2);
            eventos[ev].param = texto2[0];

            printf("Ingrese el tiempo en el que ejecutar\n");
            esperar_texto(texto2);
            eventos[ev].time = atoi(texto2);
            ev++;
         	break;
         case '4':
            for(i=0;i<ev;i++){
               printf("Accion: %c, Bit: %c, Tiempo: %d\n",eventos[i].command,eventos[i].param,eventos[i].time);
            }
         	break;
      }
	 }
   }


}


