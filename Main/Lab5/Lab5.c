#define OS_TIME_DLY_HMSM_EN 1
#define OS_TASK_DEL_EN 1
#memmap xmem
#define STACK_CNT_1K 1
#use "ucos2.lib"
#use "IO.lib"
#use "LED.lib"
#use GPS_Custom.LIB

#define DINBUFSIZE 511
#define DOUTBUFSIZE 511


void obtenerDatosGps(void *data){
  char algo[100];
  int resultado;
  while(1){
    resultado = GPS_gets(algo);
    printf("%d: %s\n",resultado,algo);
    OSTimeDlySec(1);
  }
}

int leerPuertoD(char received[]){
  OSTimeDlyHMSM(0,0,0,100);
  while( !serDrdUsed() )
  {
    OSTimeDlyHMSM(0,0,0,100);
  }
  memset(received, 0, sizeof(received));
  serDread( received, 100, 100 );
  OSTimeDlyHMSM(0,0,0,100);
}

int isEqual(char* received, char* waiting){
  printf("\nComparacion:\n%s==%s\n",received,waiting);
  if(strstr(received, waiting) != NULL){
    return 1;
  } else {
    return 0;
  }
}

void comunicarseModem(char * texto){
  printf("Comunicando: %s\n",texto);
  serDputs(texto);
  serDputc('\r');
}

int ponerModoTexto(char* received){
  printf("Poner modo texto\n");
  comunicarseModem("AT+CMGF=1");
	    memset(received, 0, sizeof(received));
  leerPuertoD(received);
  printf("%s", received);
  if(isEqual(received,"OK") == 1){
    return 1;
  }
  return 0;
}
int borrarMensajes(char * received);
int leerMensajes(char * received){
  printf("Leer mensajes");
  comunicarseModem("AT+CMGL=\"ALL\"");
	    memset(received, 0, sizeof(received));
  leerPuertoD(received);
  printf("%s", received);
  OSTimeDlyHMSM(0,0,0,500);

  if(isEqual(received,"OK") == 1){
    //borrarMensajes(received);
    return 1;
  }
  return 0;
}
int enviarMensaje(char* received){
	 char buffer[255];
    if(ponerModoTexto(received) == 1){
   	 printf("Mandando un mensaje");
	    memset(received, 0, sizeof(received));
	    serDputs("AT+CMGS=\"091829233\"\r");
	    while(!serDrdUsed());
       memset(received, 0, sizeof(received));
	    leerPuertoD(received);
	    while (isEqual(received,">") != 1)
	    {
	      memset(received, 0, sizeof(received));
	      leerPuertoD(received);
	    }
       serDputc(0x0D);
	    serDputs("Hellowda");
	    serDputc(0x1A);
       serDputs("\r");
	    memset(received, 0, sizeof(received));
	    leerPuertoD(received);
    	 while(isEqual(received,"OK") != 1){
       	memset(received, 0, sizeof(received));
	    	leerPuertoD(received);
       }
       printf("Enviado\n");
    }
}

int borrarMensajes(char * received){
  printf("Borrar mensajes");
  comunicarseModem("AT+CMGDA=\"DEL ALL\"");
  memset(received, 0, sizeof(received));
  leerPuertoD(received);
  printf("%s", received);
  if(isEqual(received,"OK") == 1){
    return 1;
  }
  return 0;
}

int ponerPin(char* received){
  printf("Poner Pin");
  comunicarseModem("AT+CPIN?");
  leerPuertoD(received);
  printf("\nLectura: %s\n", received);
  if(isEqual(received,"SIM PIN") == 1){
    printf("Ingresando pin\n");
    comunicarseModem("AT+CPIN=1707");      //5454
    leerPuertoD(received);
    printf("%s", received);
    if(isEqual(received,"OK") == 1){
      printf("Pin ingresado correctamente");
      return 1;
    }
  }
  else if(isEqual(received,"OK") == 1){
    printf("El pin ya esta ingresado");
    return 1;
  } else {
    printf("Caso no definifido\n");
  }
  return 0;
}
int registrarEnRed(char* received){
  comunicarseModem("AT+CREG?");
  memset(received, 0, sizeof(received));
  leerPuertoD(received);
  if(isEqual(received,"0,1") == 1){
    printf("Esta registrado en la red Antel");
  	 return 1;
  }
  else{
    //Registro para Antel: "AT+COPS=1,2,\"74801\"\r"
    comunicarseModem("AT+COPS=1,2,\"74801\"");
    OSTimeDlySec(1);
    return registrarEnRed(received);
    printf("Se registro en la red Antel");
  }
  return 0;
}



void prenderModem(){
  if (!BitRdPortI(PBDR, 7)){
    BitWrPortI(PEDDR,&PEDDRShadow,0,4);
    OSTimeDlyHMSM(0,0,0,50);
    BitWrPortI(PEDDR,&PEDDRShadow,1,4);
    BitWrPortI(PEDR,&PEDRShadow,0,4);
    OSTimeDlyHMSM(0,0,2,0);
    BitWrPortI(PEDDR,&PEDDRShadow,0,4);
  }
  BitWrPortI(PEDR,&PEDRShadow, BitRdPortI(PBDR, 7), 0);
}

void apagarModem();

void modem(void *data){
  char texto[10];
  int status;
  char received[50];
  int h;
  h=0;
  while(1)
  {
    status = IO_getInput(PORT_E, BIT_1);
    printf("Status: %d\n",status);
    while(!status)
    {
    	printf("Entre\n");
      BitWrPortI(PEDDR,&PEDDRShadow,OUTPUT_DIR,BIT_4);
      BitWrPortI(PEDR,&PEDRShadow,0,BIT_4);
      OSTimeDlySec(2);
      BitWrPortI(PEDDR,&PEDDRShadow,INPUT_DIR,BIT_4);
      status = IO_getInput(PORT_E, BIT_1);
      printf("Status: %d\n",status);
    	if (status ) LED_SET(BIT_7);
      else LED_RESET(BIT_7);
    }
    comunicarseModem("A");
    OSTimeDlyHMSM(0,0,5,0);
    comunicarseModem("AT");
    OSTimeDlyHMSM(0,0,0,100);
    while( !serDrdUsed() )
    {
      OSTimeDlyHMSM(0,0,0,100);
    }

    leerPuertoD(received);
    if (ponerPin(received) == 1){
      OSTimeDlyHMSM(0,0,0,100);
      if(registrarEnRed(received)==1){
         OSTimeDlyHMSM(0,0,0,100);
      	if(ponerModoTexto(received)==1){
            OSTimeDlyHMSM(0,0,0,100);
            if(borrarMensajes(received)==1){
               OSTimeDlyHMSM(0,0,0,100);
            	enviarMensaje(received);
            }
         }
      }
    }
  }
}

void apagarModem(){
  if (BitRdPortI(PBDR, 7)){
    BitWrPortI(PEDDR,&PEDDRShadow,0,4);
    OSTimeDlyHMSM(0,0,0,50);
    BitWrPortI(PEDDR,&PEDDRShadow,1,4);
    BitWrPortI(PEDR,&PEDRShadow,0,4);
    OSTimeDlyHMSM(0,0,2,0);
    BitWrPortI(PEDDR,&PEDDRShadow,0,4);
  }
  BitWrPortI(PEDR,&PEDRShadow, BitRdPortI(PBDR, 7), 0);
}

main(){
  HW_init();
  OSInit();
  //OSTaskCreate(GPS_init, NULL, 512, 1);
  //OSTaskCreate(obtenerDatosGps, NULL, 512, 5);
  OSTaskCreate(modem,NULL,1024,6);
  OSStart();
}