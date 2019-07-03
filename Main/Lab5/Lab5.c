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
#define TCPCONFIG 0
#define USE_ETHERNET 1
#define MY_IP_ADDRESS "10.10.6.100"
#define MY_NETMASK "255.255.255.0"
#define MY_GATEWAY "10.10.6.2"
#define MAX_BUFSIZE 2048
#define PORT 7
#define OS_TIME_DLY_HMSM_EN 1
#define OS_TASK_DEL_EN 1
#define STACK_CNT_1K 1
#define DINBUFSIZE 511
#define DOUTBUFSIZE 511
#define MINIMO_RITMO_CARDIACO 1500
#define MAXIMO_RITMO_CARDIACO 3500
#define MAX_TIMEOUT_KEEPALIVE 600		//10 minutos en segundos
#define PIN_ANALOGICO_CARDIACO 0

#memmap xmem
#use "ucos2.lib"
#use "dcrtcp.lib"
#use IO.LIB
#use BTN.LIB
#use LED.LIB
#use UTILITIES.LIB                                                                                                                             s
#use EVENTOS.LIB
#use GPS_Custom.LIB
#use MODEM_Custom.LIB

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

tcp_Socket echosock;
char bufferGPS[256];									// String donde se guarda la cadena que envia el GPS
char cadenaGPSFormateada[50];
GPSPosition posicionGPS;							// Variable que se utiliza para almacenar provisoriamente la posicion en formato GPS
unsigned long ultimaPresionadaBoton;

void obtenerDatosGps(void *data);
int leerPuertoD(char received[]);
int isEqual(char* received, char* waiting);

//Funcion que en base al tipo pasado por parametro
//escribe por consola o por hercules
//En el caso de que sea un 1 ser? por Hercules
void imprimirEthernet(char *s)
{
	sock_fastwrite(&echosock, s, strlen(s));
}

//Funcion que imprime una pregunta y espera por la respuesta cargando el texto al puntero de char respuesta
void preguntar(char *pregunta, char *respuesta)
{
	int bytes;

	imprimirEthernet(pregunta);
	while (tcp_tick(&echosock))
	{
		bytes = sock_dataready(&echosock);
		if (bytes > 0)
		{
			if (bytes > MAX_BUFSIZE)
			{
				bytes = MAX_BUFSIZE;
			}
			sock_fastread(&echosock, respuesta, bytes);
			sock_flush(&echosock);
			respuesta[bytes] = '\0';
			return;
		}
		OSTimeDlyHMSM(0,0,0,100);
	}
}

//Establecer la conexi?n
void iniciarConexion()
{
	sock_init();
	tcp_listen(&echosock, PORT, 0, 0, NULL, 0);
	sock_mode(&echosock, TCP_MODE_ASCII);
}

void cortarString(char * string, int inicio, int fin, char * resultado){
	int i;
	for(i=0;i<inicio;i++){
		string++;
	}
	for(i=0;i<fin-inicio;i++){
		(*resultado)=(*string);
		resultado++;
		string++;
	}
	(*resultado) = '\0';
}


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
		cortarString(respuesta,6*i,5*(i+1)+i,miCorte);
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

void iniciarMenuEthernet(void* data){
	INT8U err;
	char respuesta[255];

	while(1){
		while (!sock_established(&echosock))
		{
			OSTimeDlyHMSM(0,0,0,100);
		}
		memset(respuesta,0,sizeof(respuesta));
		printf("Esperando\n");
		preguntar("Ingrese",respuesta);
		printf("Calculando\n");
		convertirCheckpoint(respuesta);
		printf("Terminado\n");
		OSTimeDlyHMSM(0,0,0,100);
	}
}

//Funcion para pedir una trama al GPS
void PedidoDeInfoAGPS()
{
	GPS_gets(bufferGPS);
	printf("%s\n",bufferGPS);
}


int ActualizacionPosicion()
{
	int a;
	a=gps_get_position(&posicionGPS, bufferGPS);
	printf("\nResultado posicion: %d\n",a);
	return a;
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

//Funcion que chequea que el puerto D no este usado
//Y lee los datos que se encuentren en el
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

//Funcion que compara dos string
int isEqual(char* received, char* waiting){
	//printf("\nComparacion:\n%s==%s\n",received,waiting);
	if(strstr(received, waiting) != NULL){
		return 1;
	} else {
		return 0;
	}
}

void checkGps(void * data){
	char mensajeTexto[128];
	while(1){
		while (ActualizacionPosicion() < 0)		//Si la informacion es invalida no prosigo
		{
			PedidoDeInfoAGPS();
		}
		memset(mensajeTexto, 0, sizeof(mensajeTexto));
		strcat(mensajeTexto, "https://www.google.com/maps/?q=");
		formateoPosicion(posicionGPS);
		strcat(mensajeTexto, cadenaGPSFormateada);
		printf("\n");
		printf(mensajeTexto);
		printf("\n");
		OSTimeDlySec(1);
	}
}

// Funcion que imprime los valores de las entradas analogicas dependiendo
// desde donde se pregunta (tipo)
void chequearEstadoDeVida(void * data)
{
	int valorAnalogico;
	while(1){
		valorAnalogico = IO_getAnalogInput(PIN_ANALOGICO_CARDIACO);
		printf("Check cardiaco: %d\n",valorAnalogico);
		if(valorAnalogico<MINIMO_RITMO_CARDIACO || valorAnalogico>MAXIMO_RITMO_CARDIACO){
			printf("Agregando morido %d\n",valorAnalogico);
			OSQPost(mailBoxMensajeMuerteModem,"sepuku");
			OSTaskDel(OS_PRIO_SELF); //O usar un delay para dejar en espera la funcion por X cantidad de tiempo
		}
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
	}
}


void keepAlive(void * data){
	// Chequer de que toco el boton cada 10 min
	// Programar parpadeo de led cuando se aproxima el tiempo al timeout
	unsigned long timeNow;
	ultimaPresionadaBoton=read_rtc();
	while(1){
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

//Funcion que mantiene el Ethernet prendido
void matenerEthernet(){
	while(1){
		tcp_tick(NULL);
		OSTimeDlyHMSM(0,0,0,10);
	}
}


main(){
	int i;
	HW_init();
	OSInit();
	iniciarConexion();
	mailBoxMensajeMuerteModem = OSQCreate(&mensajeMailBox,1);

  	printf("Abrite consola\n");
	//storedInfo.checkpoints = listaCheckPoints;
	//storedInfo.lastPressTime = 0;
	//storedInfo.checker = 1;

	//writeUserBlock(1,&storedInfo,sizeof(storedInfo));
	//r=readUserBlock(&storedInfo,1,sizeof(storedInfo));
	// miFuncion();

	//OSTaskCreate(GPS_init, NULL, 512, 1);
	//OSTaskCreate(keepAlive,NULL, 512,2);
	//OSTaskCreate(checkGps, NULL, 512, 3);
	//OSTaskCreate(botonera, NULL, 512, 4);
	//OSTaskCreate(miFuncion, NULL, 512, 7);
	OSTaskCreate(matenerEthernet, NULL, 512, 5);
	OSTaskCreate(iniciarMenuEthernet, NULL, 512, 7);
	//OSTaskCreate(chequearEstadoDeVida,NULL,512,5);
	OSTaskCreate(MODEM_iniciar,NULL,1024,6);

	OSStart();
}
