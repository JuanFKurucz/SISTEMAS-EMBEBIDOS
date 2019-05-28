#define OS_TIME_DLY_HMSM_EN 1
#define OS_TASK_DEL_EN 1
#memmap xmem
#use "ucos2.lib"
#use "IO.lib"
#use "LED.lib"
#use GPS_Custom.LIB


void obtenerDatosGps(void *data){
  char algo[100];
  int resultado;
  while(1){
    resultado = GPS_gets(algo);
    printf("%d: %s\n",resultado,algo);
    OSTimeDlySec(1);
  }
}

int leerPuertoD(char received[], char waiting[]){
  OSTimeDlyHMSM(0,0,0,100);
  while( !serDrdUsed() )
  {
    OSTimeDlyHMSM(0,0,0,100);
  }
  memset(received, 0, sizeof(received));
  serDread( received, 100, 100 );
  printf("%s == %s",received,waiting);
  if(sizeof(waiting) == 0 || strcmpi(received,waiting)==0){
    return 1;
  } else {
    return 0;
  }
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

void comunicarseModem(char * texto){
  serDputs(texto);
  serDputc('\r');
}

void modem(void *data){
  char texto[10];
  int status;
  char received[100];

  while(1)
  {
    status = IO_getInput(PORT_E, BIT_1);
    while( !status )
    {
      BitWrPortI(PEDDR,&PEDDRShadow,OUTPUT_DIR,BIT_4);
      BitWrPortI(PEDR,&PEDRShadow,0,BIT_4);
      OSTimeDlySec(2);
      BitWrPortI(PEDDR,&PEDDRShadow,INPUT_DIR,BIT_4);
      status = IO_getInput(PORT_E, BIT_1);
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
    memset(received, 0, sizeof(received));
    serDread( received, 100, 100 );

    comunicarseModem("AT+CPIN?");

    if(leerPuertoD(received,"SIM PIN")){
      comunicarseModem("AT+CPIN=5454");
      if(leerPuertoD(received,"OK")){
        comunicarseModem("AT+CREG?");
        if(leerPuertoD(received,"AT+CREG:0.1")){
//AT+COPS=
        }
      }
    }

  }
}

main(){
  HW_init();
  OSInit();
  //OSTaskCreate(GPS_init, NULL, 512, 1);
  //OSTaskCreate(obtenerDatosGps, NULL, 512, 5);
  OSTaskCreate(modem,NULL,512,6);
  OSStart();
}