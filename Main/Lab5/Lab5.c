/*
La funcion leerMensajes no se usa para el obligatorio ELIMINAR AL ENTREGAR

*/

#define OS_TIME_DLY_HMSM_EN 1
#define OS_TASK_DEL_EN 1
#memmap xmem
#define STACK_CNT_1K 1
#use "ucos2.lib"
#use IO.LIB
#use BTN.LIB
#use LED.LIB
#use UTILITIES.LIB
#use EVENTOS.LIB
#use GPS_Custom.LIB

#define DINBUFSIZE 511
#define DOUTBUFSIZE 511
#define MINIMO_RITMO_CARDIACO 1500
#define MAXIMO_RITMO_CARDIACO 3500

void * mensajeMailBox[1];
OS_EVENT * mailBoxMensajeMuerteModem;
char bufferGPS[256];						//String donde se guarda la cadena que envia el GPS
char cadenaGPSFormateada[50];
GPSPosition posicionGPS;					//Variable que se utiliza para almacenar provisoriamente la posicion en formato GPS

unsigned long ultimaPresionadaBoton;

void obtenerDatosGps(void *data);
int leerPuertoD(char received[]);
int isEqual(char* received, char* waiting);
void comunicarseModem(char * texto);
int ponerModoTexto(char* received);
int borrarMensajes(char * received);
int leerMensajes(char * received);
int enviarMensaje(char* received);
int ponerPin(char* received);
int registrarEnRed(char* received);
void prenderModem();
void modem(void *data);

//Funcion para pedir una trama al GPS
void PedidoDeInfoAGPS()
{
	GPS_gets(bufferGPS);
  printf("%s\n",bufferGPS);
}


int ActualizacionPosicion()
{
	return gps_get_position(&posicionGPS, bufferGPS);
}

void formateoPosicion(GPSPosition registro)
{
	//Asigno nuevas cadenas y numeros temporales sobre los cuales guardar la informacion que se va procesando
	float lon, lat;
	int gradosLatitud, gradosLongitud;
	char cadenaLongitud[10];
	char cadenaLatitud[10];
	char cadenaGradosLongitud[5];
	char cadenaGradosLatitud[5];
	//Fin asignacion

	//Se pasan los minutos de longitud a horas
	lon = (float)registro.lon_minutes / 60;
	ftoa(lon, cadenaLongitud);

	//Se pasan los minutos de latitud a horas
	lat = (float)registro.lat_minutes / 60;
	ftoa(lat, cadenaLatitud);

	//Se asignan los grados como positivos o negativos dependiendo el cuadrante
	gradosLatitud = registro.lat_degrees;
	if (registro.lat_direction == 'S')
	{
		gradosLatitud = gradosLatitud * (-1);
	}

	gradosLongitud = registro.lon_degrees;
	if (registro.lon_direction == 'W')
	{
		gradosLongitud = gradosLongitud * (-1);
	}

	//Se guardan como cadena los grados
	itoa(gradosLatitud, cadenaGradosLatitud);
	itoa(gradosLongitud, cadenaGradosLongitud);

	memset(cadenaGPSFormateada, 0, sizeof(cadenaGPSFormateada));	//Se limpia la cadena para asegurar que no se encuentre basura

	//Se concatenan las cadenas temporales en una cadena global para ser usadas posteriormente
	strcat(cadenaGPSFormateada, cadenaGradosLatitud);
	strcat(cadenaGPSFormateada, ".");
	strcat(cadenaGPSFormateada, cadenaLatitud);
	strcat(cadenaGPSFormateada, ",");
	strcat(cadenaGPSFormateada, cadenaGradosLongitud);
	strcat(cadenaGPSFormateada, ".");
	strcat(cadenaGPSFormateada, cadenaLongitud);
}

void obtenerDatosGps(void *data){
  char algo[100];
  int resultado;
  while(1){
    resultado = GPS_gets(algo);
    //printf("%d: %s\n",resultado,algo);
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
  //printf("\nComparacion:\n%s==%s\n",received,waiting);
  if(strstr(received, waiting) != NULL){
    return 1;
  } else {
    return 0;
  }
}

void comunicarseModem(char * texto){
  //printf("Comunicando: %s\n",texto);
  serDputs(texto);
  serDputc('\r');
}

int ponerModoTexto(char* received){
  //printf("Poner modo texto\n");
  comunicarseModem("AT+CMGF=1");
  memset(received, 0, sizeof(received));
  leerPuertoD(received);
  //printf("%s", received);
  if(isEqual(received,"OK") == 1){
    return 1;
  }
  return 0;
}

int leerMensajes(char * received){
  //printf("Leer mensajes");
  comunicarseModem("AT+CMGL=\"ALL\"");
  memset(received, 0, sizeof(received));
  leerPuertoD(received);
  //printf("%s", received);
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
    //printf("Mandando un mensaje");
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
    //printf("Enviado\n");
  }
}

int borrarMensajes(char * received){
  //printf("Borrar mensajes");
  comunicarseModem("AT+CMGDA=\"DEL ALL\"");
  memset(received, 0, sizeof(received));
  leerPuertoD(received);
  //printf("%s", received);
  if(isEqual(received,"OK") == 1){
    return 1;
  }
  return 0;
}

int ponerPin(char* received){
  //printf("Poner Pin");
  comunicarseModem("AT+CPIN?");
  leerPuertoD(received);
  //printf("\nLectura: %s\n", received);
  if(isEqual(received,"SIM PIN") == 1){
    //printf("Ingresando pin\n");
    comunicarseModem("AT+CPIN=5454");      //5454
    leerPuertoD(received);
    //printf("%s", received);
    if(isEqual(received,"OK") == 1){
      //printf("Pin ingresado correctamente");
      return 1;
    }
  }
  else if(isEqual(received,"OK") == 1){
    //printf("El pin ya esta ingresado");
    return 1;
  } else {
    //printf("Caso no definifido\n");
  }
  return 0;
}

int registrarEnRed(char* received){
  comunicarseModem("AT+CREG?");
  memset(received, 0, sizeof(received));
  leerPuertoD(received);
  if(isEqual(received,"0,1") == 1){
    //printf("Esta registrado en la red Antel");
    return 1;
  }
  else{
    //Registro para Antel: "AT+COPS=1,2,\"74801\"\r"
    comunicarseModem("AT+COPS=1,2,\"74801\"");
    OSTimeDlySec(1);
    return registrarEnRed(received);
    //printf("Se registro en la red Antel");
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

void modem(void *data){
  char texto[10];
  int status;
  char received[50];
  int h;
  void* punteroMensaje;
  INT8U timeout;
  INT8U err;
  h=0;
  while(1)
  {
    status = IO_getInput(PORT_E, BIT_1);
    //printf("Status: %d\n",status);
    while(!status)
    {
      //printf("Entre\n");
      BitWrPortI(PEDDR,&PEDDRShadow,OUTPUT_DIR,BIT_4);
      BitWrPortI(PEDR,&PEDRShadow,0,BIT_4);
      OSTimeDlySec(2);
      BitWrPortI(PEDDR,&PEDDRShadow,INPUT_DIR,BIT_4);
      status = IO_getInput(PORT_E, BIT_1);
      //printf("Status: %d\n",status);
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
    timeout = 100;
    printf("Comprobando cola\n");
    punteroMensaje = OSQPend(mailBoxMensajeMuerteModem, timeout,&err);
    if(punteroMensaje != (void *)0){
      printf("%s\n",punteroMensaje);
    }
    leerPuertoD(received);
    /*
    if (ponerPin(received) == 1){
    OSTimeDlyHMSM(0,0,0,100);
    if(registrarEnRed(received)==1){
    OSTimeDlyHMSM(0,0,0,100);
    if(ponerModoTexto(received)==1){
    OSTimeDlyHMSM(0,0,0,100);
    if(borrarMensajes(received)==1){
    OSTimeDlyHMSM(0,0,0,100);
    //        	enviarMensaje(received);
  }
}
}
} */
}
}


void retornarUbicacionGoogleMaps(char * buffer, char * cadena){
	memset(buffer, 0, sizeof(buffer));
  sprintf(buffer,"http://maps.google.com/?q=%s",cadena);
}

void checkGps(void * data){
  char buffer[200];
  while(1){
    PedidoDeInfoAGPS();
    ActualizacionPosicion();
    printf("Data: %s\n",cadenaGPSFormateada);
    retornarUbicacionGoogleMaps(buffer,cadenaGPSFormateada);
    printf("%s\n", buffer);
    OSTimeDlySec(1);
  }
}

// Funcion que imprime los valores de las entradas analogicas dependiendo
// desde donde se pregunta (tipo)
void chequearEstadoDeVida(void * data)
{
  int ana1;
  while(1){
    ana1 = IO_getAnalogInput(0);
    printf("Check cardiaco: %d\n",ana1);
    if(ana1<MINIMO_RITMO_CARDIACO || ana1>MAXIMO_RITMO_CARDIACO){
      printf("Agregando morido %d\n",ana1);
      OSQPost(mailBoxMensajeMuerteModem,"sepuku");
      OSTaskDel(OS_PRIO_SELF); //O usar un delay para dejar en espera la funcion por X cantidad de tiempo
    }
    OSTimeDlySec(1);
  }
}



void botonera(void * data){

	while(1){
      if(BTN_GET(0)==0){
      	LED_SET(0);
      }
      if(BTN_GET(1)==0){
      	LED_SET(1);
      }
      if(BTN_GET(2)==0){
      	LED_SET(2);
      }
      if(BTN_GET(3)==0){
      	LED_SET(3);
      }
      if(BTN_GET(4)==0){
      	LED_SET(4);
      }
      if(BTN_GET(5)==0){
      	LED_SET(5);
      }
      if(BTN_GET(6)==0){
   		printf("hellowda");
    		OSTimeDlySec(1);
      }
      if(BTN_GET(7)==0){    
         time = read_rtc();
      }
   }
}

main(){
  HW_init();
  OSInit();


  mailBoxMensajeMuerteModem = OSQCreate(&mensajeMailBox,1);
  OSTaskCreate(GPS_init, NULL, 512, 1);
  OSTaskCreate(checkGps, NULL, 512, 3);
  OSTaskCreate(botonera, NULL, 512, 4);
  OSTaskCreate(chequearEstadoDeVida,NULL,512,5);
  OSTaskCreate(modem,NULL,1024,6);
  OSStart();
}