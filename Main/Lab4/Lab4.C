#define DEPURANDO 0

#define MAX_TEXTO 200
#define MAX_CLIENTES 1

#define TASK_STK_SIZE 2K
#if 4+MAX_CLIENTES>10
	#define OS_MAX_TASKS 4+MAX_CLIENTES
#else
   #define OS_MAX_TASKS 10
#endif
#define STACK_CNT_2K 1+MAX_CLIENTES
#define OS_TIME_DLY_HMSM_EN	 1
#define OS_MEM_EN 1

#define TCPCONFIG 0
#define USE_ETHERNET 1
#define MY_IP_ADDRESS "10.10.6.100"
#define MY_NETMASK "255.255.255.0"
#define MY_GATEWAY "10.10.6.2"
#define MAX_BUFSIZE 2048
#define MAX_TCP_SOCKET_BUFFERS MAX_CLIENTES

#memmap xmem

#use "ucos2.lib"
#use "dcrtcp.lib"
#define PORT 7

OS_MEM *Memoria;

#use IO.LIB
#use BTN.LIB
#use LED.LIB
#use UTILITIES.LIB
#use EVENTOS.LIB


// Definimos los eventos
typedef struct Connections
{
	int tipo;
	Event *eventos;
	tcp_Socket *socket;
} Connection;

enum connectionModes
{
	CONNECTION_UNDEF = -1,
	CONSOLE,
	ETHERNET
};

tcp_Socket sockClientes[MAX_CLIENTES];

OS_EVENT* Semaforo;

void blinkRedLed(void *datos){
	while(1){
		#if DEPURANDO
   		printf("Console: blinkRedLed\n");
		#endif
		LED_ROJO_SET();
		OSTimeDlyHMSM(0,0,0,400);
		LED_ROJO_RESET();
		OSTimeDlyHMSM(0,0,0,800);
	}
}

OS_EVENT * semTellEth;
//Funcion que en base al tipo pasado por parametro
//escribe por consola o por hercules
//En el caso de que sea un 1 ser� por Hercules
void imprimir(Connection *conexion, char *s)
{
	char err;
	if ((*conexion).tipo == ETHERNET)
	{
	//	OSSemPend(semTellEth, 0, &err);
		sock_fastwrite((*conexion).socket, s, strlen(s));
	//	OSSemPost(semTellEth);
	}
	else if((*conexion).tipo == CONSOLE)
	{
		printf("%s", s);
	} else {
		#if DEPURANDO
			printf("Se corrompio: %d\n",(*conexion).tipo);
		#endif
	}
}


//Menu principal - se despliega en cuento comienza nuestro programa
void imprimirMenu(Connection *conexion)
{
	imprimir(conexion, "===MENU COMIENZO===\n");
	imprimir(conexion, "Ingrese 1 para Fijar la hora del reloj de tiempo real (RTC) del Rabbit\n");
	imprimir(conexion, "Ingrese 2 para Consultar la hora del RTC del Rabbit\n");
	imprimir(conexion, "Ingrese 3 para Agregar un evento de calendario.\n");
	imprimir(conexion, "Ingrese 4 para Quitar un evento de calendario.\n");
	imprimir(conexion, "Ingrese 5 para Consultar la lista de eventos de calendario activos.\n");
	imprimir(conexion, "Ingrese 6 para Consultar las entradas analogicas.\n");
	imprimir(conexion, "===MENU FIN===\n");
}

OS_EVENT * semAskEth;
//Funcion que imprime una pregunta y espera por la respuesta cargando el texto al puntero de char respuesta
void preguntar(char *pregunta, char *respuesta, Connection *conexion)
{
	int bytes;
   char err;
	imprimir(conexion, pregunta);

	if ((*conexion).tipo == CONSOLE)
	{
		#if DEPURANDO
			printf("Esperando respuesta de: %d\n",(*conexion).tipo);
		#endif
		while(getswf(respuesta)==0){
			OSTimeDlySec(1);
  	};
	}
	else if ((*conexion).tipo == ETHERNET)
	{
		while (tcp_tick((*conexion).socket))
		{
			#if DEPURANDO
				printf("Esperando respuesta de: %d\n",*(*conexion).socket);
			#endif
  //    OSSemPend(semAskEth, 0, &err);
			bytes = sock_dataready((*conexion).socket);
			if (bytes > 0)
			{
				if (bytes > MAX_BUFSIZE)
				{
					bytes = MAX_BUFSIZE;
				}
				sock_fastread((*conexion).socket, respuesta, bytes);
				sock_flush((*conexion).socket);
				respuesta[bytes] = '\0';
    //  	OSSemPost(semQueue);
				return;
			}
			OSTimeDlySec(1);
		}
		while(1){
			OSTimeDlySec(1);
		}
	}
}

//Se imprime la fecha sumandole 1900 al a�o para mostrarlo humanamente
void printTime(struct tm *fecha, Connection *conexion)
{
	char respuesta[20];
	sprintf(respuesta, "%d/%d/%d %d:%d:%d\n",
	(*fecha).tm_year + 1900,
	(*fecha).tm_mon,
	(*fecha).tm_mday,
	(*fecha).tm_hour,
	(*fecha).tm_min,
	(*fecha).tm_sec);
	imprimir(conexion, respuesta);
}


// Funcion que obtiene a partir de un anio, mes y dia los segundos
// usando la funcion mktime y la estructura tm para generarla
unsigned long convertir_time(int anio, int mes, int dia, int hora, int minuto, int segundo)
{
	unsigned long calculo;
	struct tm fecha;
	struct tm fechaCheck;

	if (mes <= 0 || mes > 12)
	{
		return -2;
	}
	if (dia <= 0 || dia >= 32)
	{
		return -3;
	}
	if (hora < 0 || hora >= 24)
	{
		return -5;
	}
	if (minuto < 0 || minuto >= 60)
	{
		return -6;
	}
	if (segundo < 0 || segundo >= 60)
	{
		return -7;
	}
	fecha.tm_year = anio - 1900;
	fecha.tm_mon = mes;
	fecha.tm_mday = dia;
	fecha.tm_hour = hora;
	fecha.tm_min = minuto;
	fecha.tm_sec = segundo;
	calculo = mktime(&fecha);
	mktm(&fechaCheck, calculo);
	if (memcmp(&fecha, &fechaCheck, 0) == 0)
	{
		return calculo;
	}
	else
	{
		return -4;
	}
}


//Funcion encargada de mostrar en pantalla el control de errores de la funcion convertir_time
//retorna 0 si hay un fallo retorna 1 si esta bien
int controlErroresFecha(unsigned long time, Connection *conexion)
{
	int result;
	result = 0;
	if (time == -1)
	{
		imprimir(conexion, "El anio ingresado es incorrecto\n");
	}
	else if (time == -2)
	{
		imprimir(conexion, "El mes ingresado es incorrecto\n");
	}
	else if (time == -3)
	{
		imprimir(conexion, "El dia ingresado es incorrecto\n");
	}
	else if (time == -4)
	{
		imprimir(conexion, "Fecha erronea\n");
	}
	else if (time == -5)
	{
		imprimir(conexion, "La hora ingresada es incorrecta\n");
	}
	else if (time == -6)
	{
		imprimir(conexion, "Los minutos ingresados son incorrectos\n");
	}
	else if (time == -7)
	{
		imprimir(conexion, "Los segundos ingresados son incorrectos\n");
	}
	else if (time < 0)
	{
		imprimir(conexion, "Fecha incorrecta\n");
	}
	else
	{
		result = 1;
	}
	return result;
}

//Funcion encargada de pedir al usuario ingresar una fecha
void ingresarFecha(unsigned long *time, Connection *conexion)
{
	char respuesta[2048];
	int numeroAnio;
	int numeroMes;
	int numeroDia;
	int numeroHora;
	int numeroMinuto;
	int numeroSegundo;

	numeroAnio = -1;
	numeroMes = -1;
	numeroDia = -1;
	numeroHora = -1;
	numeroMinuto = -1;
	numeroSegundo = -1;

	preguntar("Ingrese el ano\n", respuesta, conexion);
	numeroAnio = atoi(respuesta);
	while (numeroMes <= 0 || numeroMes > 12)
	{
		preguntar("Ingrese el mes (Se espera un numero entre 1 y 12)\n", respuesta, conexion);
		numeroMes = atoi(respuesta);
	}
	while (numeroDia <= 0 || numeroDia >= 32)
	{
		preguntar("Ingrese el dia (Se espera un numero entre 1 y 31)\n", respuesta, conexion);
		numeroDia = atoi(respuesta);
	}
	while (numeroHora < 0 || numeroHora >= 24)
	{
		preguntar("Ingrese la hora(Se espera un numero entre 0 y 23)\n", respuesta, conexion);
		numeroHora = atoi(respuesta);
	}
	while (numeroMinuto < 0 || numeroMinuto >= 60)
	{
		preguntar("Ingrese los minutos (Se espera un num entre 0 y 59)\n", respuesta, conexion);
		numeroMinuto = atoi(respuesta);
	}
	while (numeroSegundo < 0 || numeroSegundo >= 60)
	{
		preguntar("Ingrese los segundos (Se espera un num entre 0 y 59)\n", respuesta, conexion);
		numeroSegundo = atoi(respuesta);
	}

	*time = convertir_time(numeroAnio, numeroMes, numeroDia, numeroHora, numeroMinuto, numeroSegundo);
	return;
}

//Funcion que muestra los eventos en pantalla y retorna una lista de
void mostrarEventos(Event *eventos, Connection *conexion)
{
	struct tm fecha;
	int i;
	char buffer[2048];
	for (i = 0; i < MAX_EVENTOS; i++)
	{
		if (eventos[i].command != EVENTO_DESHABILITADO)
		{
			sprintf(buffer, "%d: Accion: %c, Bit: %c, Tiempo: ", i, eventos[i].command, eventos[i].param);
			imprimir(conexion, buffer);
			mktm(&fecha, eventos[i].time);
			printTime(&fecha, conexion);
			imprimir(conexion, "\n");
		}
	}
}

// Funcion que imprime los valores de las entradas analogicas dependiendo
// desde donde se pregunta (conexion)
void getInformacionEntradasAnalogicas(Connection *conexion)
{
	char respuesta1[40];
	char respuesta2[40];
	int ana1;
	int ana2;
	ana1 = IO_getAnalogInput(0);
	ana2 = IO_getAnalogInput(1);
	sprintf(respuesta1, "Entrada analogica 1 = %d\n", ana1);
	imprimir(conexion, respuesta1);
	sprintf(respuesta2, "Entrada analogica 2 = %d\n", ana2);
	imprimir(conexion, respuesta2);
}

//Funcion encargada de gestionar el menu y todas sus tareas
void menu(Connection *conexion)
{
	char texto[2048];
	struct tm fecha;
	char command;
	char param;
	int i; //Posicion de indice, usada para fors y obtencion de posiciones con funciones
	unsigned long time;
	int status;
	Event* eventos;
	eventos = (*conexion).eventos;
	while(1){
		imprimirMenu(conexion);
		preguntar("Ingrese una opcion\n",texto,conexion);

		switch (texto[0])
		{
			case '1':
				//Para pasar el sting a time utilizamos la funcion getswf
				//Para fijar la hora del reloj utilizamos la funcion write_rtc
				ingresarFecha(&time, conexion);
				if (controlErroresFecha(time, conexion) == 1)
				{
					write_rtc(time);
					imprimir(conexion, "Fecha actualizada con exito\n");
				}
				break;
			case '2':
				//Para consultar la hora de la placa utilizamos la funcion read_rtc
				mktm(&fecha, read_rtc());
				printTime(&fecha, conexion);
				break;
			case '3':
				i = EVENTOS_buscarEspacio(eventos);
				//Como definimos MAX_EVENTOS en 10, tenemos que controlar que el usuario no supere ese limite
				//Para que el evento quede correctamente definido, esperamos a tener todos los parametros que el usuario ingrese
				//verificando que sean correctos y luego lo creamos
				//De no realizar esto el programa prodria llevar a dar problemas, si se intenta listar un evento que todavia no tenga
				//todos sus datos
				if (i == -1)
				{
					imprimir(conexion, "Capacidad maxima de eventos alcanzada\n");
				}
				else
				{
					command = 0xFF;
					param = 0xFF;
					eventos[i].command = EVENTO_CREANDOSE;

					while (command < '0' || command > '1')
					{
						preguntar("Ingrese 1 para prender un led o ingrese 0 para apagarlo\n", texto, conexion);
						command = texto[0];
					}

					while (param < '0' || param > '7')
					{
						preguntar("Ingrese el numero de led (0 al 7)\n", texto, conexion);
						param = texto[0];
					}

					imprimir(conexion, "Se asignara el tiempo del evento ahora:\n");
					ingresarFecha(&time, conexion);
					if (controlErroresFecha(time, conexion) == 1)
					{
						// Este if es para controlar los datos que el usuario ingresa
						if ((command == '1' || command == '0') && (param >= '0' && param <= '7'))
						{
							eventos[i].command = command;
							eventos[i].param = param;
							eventos[i].time = time;
						}
						else
						{
							eventos[i].command = EVENTO_DESHABILITADO;
							imprimir(conexion, "Datos erroneos\n");
						}
					}
					else
					{
						eventos[i].command = EVENTO_DESHABILITADO;
						imprimir(conexion, "Fecha erronea\n");
					}
				}
				break;
			case '4':
				mostrarEventos(eventos, conexion);
				if (EVENTOS_existen(eventos) == 1)
				{
					i = -1;
					preguntar("Inserte el indice del evento a eliminar\n", texto, conexion);
					i = atoi(texto);
					//Controlamos que el usuario no se vaya de rango para eliminar un evento
					if (i >= 0 && i < MAX_EVENTOS)
					{
						//Lo que hacemos para eliminar nuestro evento es volver a setear los datos como en el estado inicial
						if (eventos[i].command != EVENTO_DESHABILITADO)
						{
							EVENTOS_borrar(&eventos[i]);
						}
						else
						{
							imprimir(conexion, "Este evento no existe\n");
						}
					}
					else
					{
						imprimir(conexion, "El indice se va de rango de la lista de eventos\n");
					}
				}
				else
				{
					imprimir(conexion, "No hay eventos creados\n");
				}
				break;
			case '5':
				//Recorremos todos los eventos buscando unicamente los que se encuentren activos
				//y los imprimimos por consola
				if (EVENTOS_existen(eventos) == 0)
				{
					imprimir(conexion, "No hay eventos creados\n");
				}
				else
				{
					mostrarEventos(eventos, conexion);
				}
				break;
			case '6':
				getInformacionEntradasAnalogicas(conexion);
				break;
			default:
				imprimir(conexion, "Comando no encontrado");
		}
	}
}

void matenerEthernet(){
	while(1){
		#if DEPURANDO
			printf("Console: mantenerEthernet\n");
		#endif
		tcp_tick(NULL);
		OSTimeDly(5);
	}
}

void consumir(void* data){
	Event *eventos;
	eventos = (Event*) data;
	while(1){
		#if DEPURANDO
   		printf("Console: consumir\n");
		#endif
		EVENTOS_consumir(eventos);
		OSTimeDlySec(1);
	}
}

//Establecer la conexi�n
void iniciarConexion(tcp_Socket *socket)
{
	int result;
	#if DEPURANDO
		printf("Console: inicarConexion %d\n",socket);
	#endif
	result=0;
	while(result == 0){
		#if DEPURANDO
			printf("Console: escuchando conexion %d\n",socket);
		#endif
		result = tcp_listen(socket, PORT, 0, 0, NULL, 0);
		OSTimeDly(5);
	}
	sock_mode(socket, TCP_MODE_ASCII);
	while (!sock_established(socket))// && sock_bytesready(socket)==-1)
	{
		#if DEPURANDO
			printf("Console: esperando conexion %d\n",socket);
		#endif
		//tcp_tick(NULL);
		OSTimeDly(5);
	}
}

void iniciarMenuConsola(void* data){
	INT8U err;
	Connection *con;
  con = (Connection*) data;
	while(1){
		#if DEPURANDO
   		printf("Console: iniciarMenuConsola\n");
		#endif

		menu(data);
		//OSTimeDlyHMSM(0,0,0,750);
		OSTimeDly(5);
	}
}

void iniciarMenuEthernet(void* data){
	INT8U err;
	Connection *conexion;
	conexion = (Connection*) data;
	while(1){
		iniciarConexion((*conexion).socket);
		#if DEPURANDO
   		printf("Console: iniciarMenuEthernet %d\n",*(*conexion).socket);
		#endif
    menu(conexion);
		//OSTimeDlyHMSM(0,0,0,750);
		OSTimeDly(5);
   }
}

Connection conexiones[1+MAX_CLIENTES];
main()
{
	Event eventos[MAX_EVENTOS];
	INT8U err;
	Connection *con;
	int i;

	OSInit();
	Memoria = OSMemCreate(conexiones,1+MAX_CLIENTES,sizeof(Connection),&err);

	HW_init();
	sock_init();
	EVENTOS_iniciar(eventos);
	con = (Connection*)(OSMemGet(Memoria, &err));
	(*con).tipo = 0;
	(*con).eventos = eventos;
 	semAskEth = OSSemCreate(0);

	printf("Iniciando\n");

	OSTaskCreate(matenerEthernet, NULL, 512, 5);
	OSTaskCreate(consumir, eventos, 512, 6);
	OSTaskCreate(blinkRedLed, NULL, 512, 7);
	OSTaskCreate(iniciarMenuConsola, con, 2048, 8);
	for(i=0;i<MAX_CLIENTES;i++){
		con++;
		(*con).tipo = 1;
		(*con).eventos = eventos;
		(*con).socket = &sockClientes[i];
		OSTaskCreate(iniciarMenuEthernet, con, 2048, i+9);
	}

	OSStart();
}