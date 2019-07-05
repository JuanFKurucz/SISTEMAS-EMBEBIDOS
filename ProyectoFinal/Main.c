/*
Pendientes:
-Chequear mensajes enviados
*/

#define TESTING 1
#define CANTIDAD_CHECKPOINTS 6
#define TOLERANCIA_LATITUD 1.0
#define TOLERANCIA_LONGITUD 1.0
#define PRESICION_LATITUD 2
#define PRESICION_LONGITUD 2

#define OS_TIME_DLY_HMSM_EN 1
#define OS_MEM_EN 1
#define OS_TIME_DLY_HMSM_EN 1
#define OS_TASK_DEL_EN 1
#define OS_MAX_TASKS 12

#define STACK_CNT_512 8
#define STACK_CNT_1K 1
#define STACK_CNT_4K 1


#define DINBUFSIZE 511
#define DOUTBUFSIZE 511

#define MINIMO_RITMO_CARDIACO 50.0
#define MAXIMO_RITMO_CARDIACO 220.0
#define MAX_TIMEOUT_KEEPALIVE 600		//10 minutos en segundos
#define PIN_ANALOGICO_CARDIACO 0

//Estructura de Checkpoints
typedef struct CheckPoints
{
	float latitud;
	float longitud;
	int estado;
} CheckPoint;

typedef struct Information
{
	CheckPoint* checkpoints;
	int checksum;
} Info;

#if TESTING
CheckPoint coordenadasPrueba[3];
int posicionPrueba;
#endif

Info storedInfo;
CheckPoint listaCheckPoints[CANTIDAD_CHECKPOINTS];

#memmap xmem
#use "ucos2.lib"
#use ETHERNET_Config.LIB
#use "dcrtcp.lib"
#use IO.LIB
#use BTN.LIB
#use LED.LIB
#use UTILITIES.LIB
#use GPS_Custom.LIB
#use MODEM_Custom.LIB
#use ETHERNET.LIB
#use USERBLOCK_Custom.LIB



unsigned long ultimaPresionadaBoton;

// Funcion que imprime los valores de las entradas analogicas dependiendo
// desde donde se pregunta (tipo)
void chequearEstadoDeVida(void * data)
{
	float valorAnalogico;
	while(1){
		valorAnalogico = ((float)IO_getAnalogInput(PIN_ANALOGICO_CARDIACO))*0.073;
		if(valorAnalogico<MINIMO_RITMO_CARDIACO || valorAnalogico>MAXIMO_RITMO_CARDIACO){
			OSQPost(mailBoxMensajeMuerteModem,"sepuku");
			OSTimeDlySec(60);
		}
		OSTimeDlySec(1);
	}
}


int checkPosicion(int id_checkpoint){
	float coordenadas[2];
	if(id_checkpoint<0 || id_checkpoint>=CANTIDAD_CHECKPOINTS){
		return 0;
	}
	#if TESTING
	posicionPrueba = (posicionPrueba+1)%(sizeof(coordenadasPrueba)/sizeof(coordenadasPrueba[0]));
	coordenadas[0]=coordenadasPrueba[posicionPrueba].latitud;
	coordenadas[1]=coordenadasPrueba[posicionPrueba].longitud;
	#else
	GPS_cords(posicionGPS,coordenadas);
	#endif
	if(fabs(listaCheckPoints[id_checkpoint].latitud - coordenadas[0]) <= TOLERANCIA_LATITUD &&
	fabs(listaCheckPoints[id_checkpoint].longitud - coordenadas[1]) <= TOLERANCIA_LONGITUD){
		return 1;
	}
	return 0;
}

//Funcion que espera la interaccion con los botones marcados como checkpoints
void interaccionBotonCheckPoint(int id_checkpoint){
	if(BTN_GET(id_checkpoint)==0 && checkPosicion(id_checkpoint)){
		LED_SET(id_checkpoint);
	}
}

//Funcion que espera la interaccion con los botones
void botonera(void * data){
	int i;
	while(1){
		for(i=0;i<=5;i++){
			interaccionBotonCheckPoint(i);
		}
		if(BTN_GET(6)==0){
			OSQPost(mailBoxMensajeMuerteModem,"help");
			OSTimeDlySec(1);
		}
		if(BTN_GET(7)==0){
			ultimaPresionadaBoton=read_rtc();
			LED_RESET(7);
		}
		OSTimeDlyHMSM(0,0,0,50);
	}
}


void keepAlive(void * data){
	// Chequer de que toco el boton cada 10 min
	// Programar parpadeo de led cuando se aproxima el tiempo al timeout
	unsigned long timeNow;
	ultimaPresionadaBoton=read_rtc();
	while(1){
		timeNow = read_rtc();
		if(timeNow-ultimaPresionadaBoton >= MAX_TIMEOUT_KEEPALIVE){
			//Murio
			OSQPost(mailBoxMensajeMuerteModem,"keepAlive");
			LED_SET(7);
		}
		OSTimeDlySec(MAX_TIMEOUT_KEEPALIVE);
	}
}

init(){
	HW_init();
	OSInit();
	ETHERNET_iniciar();
	MODEM_init();
	memset(listaCheckPoints,0,sizeof(listaCheckPoints));
	#if TESTING
	memset(coordenadasPrueba,0,sizeof(coordenadasPrueba));
	posicionPrueba=0;
	coordenadasPrueba[0].latitud = 11.0;
	coordenadasPrueba[0].longitud = 11.0;

	coordenadasPrueba[1].latitud = 20.0;
	coordenadasPrueba[1].longitud = 20.0;

	coordenadasPrueba[2].latitud = 15.0;
	coordenadasPrueba[2].longitud = 15.0;
	#endif
}

main(){
	init();
	OSTaskCreate(USERBLOCK_main, NULL, 512, 4);
	OSTaskCreate(GPS_init, NULL, 512, 5);
	OSTaskCreate(keepAlive,NULL, 512, 6);
	OSTaskCreate(GPS_main, NULL, 512, 7);
	OSTaskCreate(chequearEstadoDeVida,NULL,512,8);
	OSTaskCreate(MODEM_main,NULL,1024,9);
	OSTaskCreate(ETHERNET_main, NULL, 4096, 10);
	OSTaskCreate(botonera, NULL, 512, 11);
	OSTaskCreate(ETHERNET_mantener, NULL, 512, OS_PRIORIDAD_ETHERNET_MANTENER);

	OSStart();
}
