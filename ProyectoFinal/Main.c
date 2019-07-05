/*
La funcion leerMensajes no se usa para el obligatorio ELIMINAR AL ENTREGAR

Pendientes:
- Memoria volatil: Almacenar informacion de checkpoints y datos fragiles del usuario
lista de checkPoints definida por Ethernet, checkPoints marcados
ultimaPresionadaBoton
- Ethernet (Obtener datos de checkpoints por Ethernet)
- GPS
Obtener latitud y longitud generando el link de google
Guardar ultima posicion de GPS constantemente
Todos los mensajes enviados deben incluir link de google maps con posicion actual de GPS
- Comparar posicion GPS con posiciones de checkpoints a demanda de presionar boton
*/

#define OS_TIME_DLY_HMSM_EN 1
#define OS_MEM_EN 1
#define OS_TIME_DLY_HMSM_EN 1
#define OS_TASK_DEL_EN 1

#define OS_MAX_TASKS 7

#define STACK_CNT_256 3
#define STACK_CNT_512 7
#define STACK_CNT_1K 1
#define STACK_CNT_2K 1


#define DINBUFSIZE 511
#define DOUTBUFSIZE 511

#define MINIMO_RITMO_CARDIACO 1500
#define MAXIMO_RITMO_CARDIACO 3500
#define MAX_TIMEOUT_KEEPALIVE 600		//10 minutos en segundos
#define PIN_ANALOGICO_CARDIACO 0

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
	unsigned long lastPressTime;
	int checker;
} Info;

unsigned long ultimaPresionadaBoton;



/*"1	2	.	4	5	;	1	2	.	4	5		/		1		2		.		4		5		;		1		2		.		4		5"
// 0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22
i=0	0,5			5
i=1 6,11		5*2+1
i=2	12,17		5*3+2
i=3 18,23		5*4+3
*/
void convertirCheckpoint(char * respuesta){
	int i;
	CheckPoint listaCheckPoints[6];
	char miCorte[6];
	float datos[12];
	int largoCorte;
	memset(listaCheckPoints, 0, sizeof(listaCheckPoints));
	memset(miCorte, 0, sizeof(miCorte));
	memset(datos, 0, sizeof(datos));
	printf(respuesta);
	printf("\n");
	for(i=0;i<12;i++){
		memset(miCorte, 0, sizeof(miCorte));
		UT_cortarString(respuesta,6*i,5*(i+1)+i,miCorte);
		printf(miCorte);
		printf("\n");
		datos[i] = atof(miCorte);
	}
	memset(listaCheckPoints, 0, sizeof(listaCheckPoints));
	printf("Datos guardados\n");
	for(i=0;i<12;i+=2){
		listaCheckPoints[i].latitud=datos[i];
		listaCheckPoints[i+1].longitud=datos[i+1];
	}
	printf("Estructura setteada\n");
	for(i=0;i<6;i++){
		printf("Cord %d: %f  %f\n",i,listaCheckPoints[i].latitud,listaCheckPoints[i].longitud);
	}
}

// Funcion que imprime los valores de las entradas analogicas dependiendo
// desde donde se pregunta (tipo)
void chequearEstadoDeVida(void * data)
{
	int valorAnalogico;
	while(1){
		printf("Task debugg: chequearEstadoDeVida start\n");
		valorAnalogico = IO_getAnalogInput(PIN_ANALOGICO_CARDIACO);
		printf("Check cardiaco: %d\n",valorAnalogico);
		if(valorAnalogico<MINIMO_RITMO_CARDIACO || valorAnalogico>MAXIMO_RITMO_CARDIACO){
			printf("Agregando morido %d\n",valorAnalogico);
			OSQPost(mailBoxMensajeMuerteModem,"sepuku");
			OSTaskDel(OS_PRIO_SELF); //O usar un delay para dejar en espera la funcion por X cantidad de tiempo
		}
		//printf("Task debugg: chequearEstadoDeVida end\n");
		OSTimeDlySec(1);
	}
}


int checkPosicion(int id_checkpoint){
	return 1;
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
		printf("Task debugg: botonera start\n");
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
		//printf("Task debugg: botonera end\n");
	}
}


void keepAlive(void * data){
	// Chequer de que toco el boton cada 10 min
	// Programar parpadeo de led cuando se aproxima el tiempo al timeout
	unsigned long timeNow;
	ultimaPresionadaBoton=read_rtc();
	while(1){
		printf("Task debugg: keepAlive start\n");
		timeNow = read_rtc();
		printf("\nKeepAlive check (%lu-%lu)\n",timeNow,ultimaPresionadaBoton);
		if(timeNow-ultimaPresionadaBoton >= MAX_TIMEOUT_KEEPALIVE){
			//Murio
			printf("\nKeepAlive timeout\n");
			OSQPost(mailBoxMensajeMuerteModem,"keepAlive");
			LED_SET(7);
		}
		//OSTimeDlyHMSM(0,10,0,0);
		OSTimeDlySec(MAX_TIMEOUT_KEEPALIVE);
		//printf("Task debugg: keepAlive end\n");
	}
}

void miFuncion(void *data){
	CheckPoint lcp[6];
	Info storedInfo;
	CheckPoint cp;
	int i;
	int r;

	while(1){
		/*	for(i=0;i<6;i++){
		lcp[i].latitud = 1.0;
		lcp[i].longitud = 1.0;
		lcp[i].estado = 0;
		printf("Latitud: %f, Longitud: %f, Estado: %d\n", lcp[i].latitud, lcp[i].longitud, lcp[i].estado);
	}

	storedInfo.checkpoints = lcp;
	storedInfo.lastPressTime = 0;
	storedInfo.checker = 1;

	writeUserBlock(1,&storedInfo,sizeof(storedInfo));
	*/
	OSTimeDlySec(2);

	r=readUserBlock(&storedInfo,1,sizeof(storedInfo)); //Que es el 1 arbitrario que elegi y si hay que poner otra cosa

	printf("Read: %d",r);

	for(i=0;i<6;i++){
		cp = (*storedInfo.checkpoints);
		printf("Latitud: %f, Longitud: %f, Estado: %d\n", cp.latitud, cp.longitud, cp.estado);

		storedInfo.checkpoints++;
	}
}


}

init(){
	HW_init();
	OSInit();
	ETHERNET_iniciar();
	MODEM_init();
}


main(){
	init();
	printf("Abrite consola\n");
	/*
		storedInfo.checkpoints = listaCheckPoints;
		storedInfo.lastPressTime = 0;
		storedInfo.checker = 1;
		writeUserBlock(1,&storedInfo,sizeof(storedInfo));
		r=readUserBlock(&storedInfo,1,sizeof(storedInfo));
		miFuncion();
		OSTaskCreate(miFuncion, NULL, 512, 7);
	*/

	OSTaskCreate(GPS_init, NULL, 512, 5);
	OSTaskCreate(keepAlive,NULL, 512, 6);
	OSTaskCreate(GPS_main, NULL, 512, 7);
	OSTaskCreate(chequearEstadoDeVida,NULL,512,8);
	OSTaskCreate(MODEM_main,NULL,1024,9);
	OSTaskCreate(ETHERNET_main, NULL, 2048, 10);
	OSTaskCreate(ETHERNET_mantener, NULL, 512, 11);
	OSTaskCreate(botonera, NULL, 256, 12);

	OSStart();
}
