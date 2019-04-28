#define TCPCONFIG 0

#define USE_ETHERNET 1
#define MY_IP_ADDRESS "10.10.6.100"
#define MY_NETMASK "255.255.255.0"
#define MY_GATEWAY "10.10.6.2"
#define MAX_BUFSIZE 2048

#memmap xmem
#use "dcrtcp.lib"

#define PORT 7

tcp_Socket echosock;

#use IO.LIB
#use BTN.LIB
#use LED.LIB
#use UTILITIES.LIB
#use EVENTOS.LIB
#define MAX_TEXTO 10
#define preguntar(p, r, i) wfd hacerPregunta[i](p, r, i)

enum connectionModes
{
	CONNECTION_UNDEF = -1,
	CONSOLE,
	ETHERNET
};

//Funcion que en base al tipo pasado por parametro
//escribe por consola o por hercules
//En el caso de que sea un 1 será por Hercules
void imprimir(int tipo, char *s)
{
	if (tipo == ETHERNET)
	{
		sock_fastwrite(&echosock, s, strlen(s));
	}
	else
	{
		printf("%s", s);
	}
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

// Funcion que imprime los valores de las entradas analogicas dependiendo
// desde donde se pregunta (tipo)
cofunc void getInformacionEntradasAnalogicas[2](int tipo)
{
	char respuesta1[40];
	char respuesta2[40];
	int ana1;
	int ana2;
	wfd ana1 = IO_getAnalogInput(0);
	wfd ana2 = IO_getAnalogInput(1);
	sprintf(respuesta1, "Entrada analogica 1 = %d\n", ana1);
	imprimir(tipo, respuesta1);
	sprintf(respuesta2, "Entrada analogica 2 = %d\n", ana2);
	imprimir(tipo, respuesta2);
}

//Se imprime la fecha sumandole 1900 al a�o para mostrarlo humanamente
void printTime(struct tm *fecha, int tipo)
{
	char respuesta[20];
	sprintf(respuesta, "%d/%d/%d %d:%d:%d\n",
			(*fecha).tm_year + 1900,
			(*fecha).tm_mon,
			(*fecha).tm_mday,
			(*fecha).tm_hour,
			(*fecha).tm_min,
			(*fecha).tm_sec);
	imprimir(tipo, respuesta);
}

//Funcion que muestra los eventos en pantalla y retorna una lista de
void mostrarEventos(Event *eventos, int tipo)
{
	struct tm fecha;
	int i;
	char buffer[100];
	for (i = 0; i < MAX_EVENTOS; i++)
	{
		if (eventos[i].command != EVENTO_DESHABILITADO)
		{
			sprintf(buffer, "%d: Accion: %c, Bit: %c, Tiempo: ", i, eventos[i].command, eventos[i].param);
			imprimir(tipo, buffer);
			mktm(&fecha, eventos[i].time);
			printTime(&fecha, tipo);
			imprimir(tipo, "\n");
		}
	}
}

//Funcion que imprime una pregunta y espera por la respuesta cargando el texto al puntero de char respuesta
cofunc void hacerPregunta[2](char *pregunta, char *respuesta, int tipo)
{
	int bytes;
	imprimir(tipo, pregunta);
	if (tipo == CONSOLE)
	{
		waitfor(getswf(respuesta));
		return;
	}
	else if (tipo == ETHERNET)
	{
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
			yield;
		}
		while (1)
		{
			yield;
		}
	}
}

//Funcion encargada de pedir al usuario ingresar una fecha
cofunc void ingresarFecha[2](unsigned long *time, int tipo)
{
	char respuesta[MAX_TEXTO];
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

	preguntar("Ingrese el ano\n", respuesta, tipo);
	numeroAnio = atoi(respuesta);
	while (numeroMes <= 0 || numeroMes > 12)
	{
		preguntar("Ingrese el mes (Se espera un numero entre 1 y 12)\n", respuesta, tipo);
		numeroMes = atoi(respuesta);
	}
	while (numeroDia <= 0 || numeroDia >= 32)
	{
		preguntar("Ingrese el dia (Se espera un numero entre 1 y 31)\n", respuesta, tipo);
		numeroDia = atoi(respuesta);
	}
	while (numeroHora < 0 || numeroHora >= 24)
	{
		preguntar("Ingrese la hora(Se espera un numero entre 0 y 23)\n", respuesta, tipo);
		numeroHora = atoi(respuesta);
	}
	while (numeroMinuto < 0 || numeroMinuto >= 60)
	{
		preguntar("Ingrese los minutos (Se espera un num entre 0 y 59)\n", respuesta, tipo);
		numeroMinuto = atoi(respuesta);
	}
	while (numeroSegundo < 0 || numeroSegundo >= 60)
	{
		preguntar("Ingrese los segundos (Se espera un num entre 0 y 59)\n", respuesta, tipo);
		numeroSegundo = atoi(respuesta);
	}

	*time = convertir_time(numeroAnio, numeroMes, numeroDia, numeroHora, numeroMinuto, numeroSegundo);
	return;
}

//Funcion encargada de mostrar en pantalla el control de errores de la funcion convertir_time
//retorna 0 si hay un fallo retorna 1 si esta bien
int controlErroresFecha(unsigned long time, int tipo)
{
	int result;
	result = 0;
	if (time == -1)
	{
		imprimir(tipo, "El anio ingresado es incorrecto\n");
	}
	else if (time == -2)
	{
		imprimir(tipo, "El mes ingresado es incorrecto\n");
	}
	else if (time == -3)
	{
		imprimir(tipo, "El dia ingresado es incorrecto\n");
	}
	else if (time == -4)
	{
		imprimir(tipo, "Fecha erronea\n");
	}
	else if (time == -5)
	{
		imprimir(tipo, "La hora ingresada es incorrecta\n");
	}
	else if (time == -6)
	{
		imprimir(tipo, "Los minutos ingresados son incorrectos\n");
	}
	else if (time == -7)
	{
		imprimir(tipo, "Los segundos ingresados son incorrectos\n");
	}
	else if (time < 0)
	{
		imprimir(tipo, "Fecha incorrecta\n");
	}
	else
	{
		result = 1;
	}
	return result;
}

//Menu principal - se despliega en cuento comienza nuestro programa
void imprimirMenu(int tipo)
{
	imprimir(tipo, "===MENU COMIENZO===\n");
	imprimir(tipo, "Ingrese 1 para Fijar la hora del reloj de tiempo real (RTC) del Rabbit\n");
	imprimir(tipo, "Ingrese 2 para Consultar la hora del RTC del Rabbit\n");
	imprimir(tipo, "Ingrese 3 para Agregar un evento de calendario.\n");
	imprimir(tipo, "Ingrese 4 para Quitar un evento de calendario.\n");
	imprimir(tipo, "Ingrese 5 para Consultar la lista de eventos de calendario activos.\n");
	imprimir(tipo, "Ingrese 6 para Consultar las entradas analogicas.\n");
	imprimir(tipo, "===MENU FIN===\n");
}

//Funcion encargada de gestionar el menu y todas sus tareas
cofunc void menu[2](Event *eventos, int tipo)
{
	char texto[MAX_TEXTO];
	struct tm fecha;
	char command;
	char param;
	int i; //Posicion de indice, usada para fors y obtencion de posiciones con funciones
	unsigned long time;
	int status;

	imprimirMenu(tipo);
	wfd hacerPregunta[tipo]("Ingrese una opcion\n", texto, tipo);

	switch (texto[0])
	{
	case '1':
		//Para pasar el sting a time utilizamos la funcion getswf
		//Para fijar la hora del reloj utilizamos la funcion write_rtc
		wfd ingresarFecha[tipo](&time, tipo);
		if (controlErroresFecha(time, tipo) == 1)
		{
			write_rtc(time);
			imprimir(tipo, "Fecha actualizada con exito\n");
		}
		break;
	case '2':
		//Para consultar la hora de la placa utilizamos la funcion read_rtc
		mktm(&fecha, read_rtc());
		printTime(&fecha, tipo);
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
			imprimir(tipo, "Capacidad maxima de eventos alcanzada\n");
		}
		else
		{
			command = 0xFF;
			param = 0xFF;
			eventos[i].command = EVENTO_CREANDOSE;

			while (command < '0' || command > '1')
			{
				preguntar("Ingrese 1 para prender un led o ingrese 0 para apagarlo\n", texto, tipo);
				command = texto[0];
			}

			while (param < '0' || param > '7')
			{
				preguntar("Ingrese el numero de led (0 al 7)\n", texto, tipo);
				param = texto[0];
			}

			imprimir(tipo, "Se asignara el tiempo del evento ahora:\n");
			wfd ingresarFecha[tipo](&time, tipo);
			if (controlErroresFecha(time, tipo) == 1)
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
					imprimir(tipo, "Datos erroneos\n");
				}
			}
			else
			{
				eventos[i].command = EVENTO_DESHABILITADO;
				imprimir(tipo, "Fecha erronea\n");
			}
		}
		break;
	case '4':
		mostrarEventos(eventos, tipo);
		if (EVENTOS_existen(eventos) == 1)
		{
			i = -1;
			preguntar("Inserte el indice del evento a eliminar\n", texto, tipo);
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
					imprimir(tipo, "Este evento no existe\n");
				}
			}
			else
			{
				imprimir(tipo, "El indice se va de rango de la lista de eventos\n");
			}
		}
		else
		{
			imprimir(tipo, "No hay eventos creados\n");
		}
		break;
	case '5':
		//Recorremos todos los eventos buscando unicamente los que se encuentren activos
		//y los imprimimos por consola
		if (EVENTOS_existen(eventos) == 0)
		{
			imprimir(tipo, "No hay eventos creados\n");
		}
		else
		{
			mostrarEventos(eventos, tipo);
		}
		break;
	case '6':
		wfd getInformacionEntradasAnalogicas[tipo](tipo);
		break;
	default:
		imprimir(tipo, "Comando no encontrado");
	}
	return;
}

//Establecer la conexión
void iniciarConexion()
{
	sock_init();
	tcp_listen(&echosock, PORT, 0, 0, NULL, 0);
	sock_mode(&echosock, TCP_MODE_ASCII);
}

main()
{
	Event eventos[MAX_EVENTOS];
	HW_init();
	EVENTOS_iniciar(eventos);
	iniciarConexion();
	printf("Iniciando\n");
	while (1)
	{
		//Durante la ejecucion de nuestro programa prendemos y apagamos el led rojo
		costate
		{
			LED_ROJO_SET();
			waitfor(DelayMs(400));
			LED_ROJO_RESET();
			waitfor(DelayMs(800));
		}

		costate
		{
			// drive the tcp stack
			tcp_tick(NULL);
		}

		costate
		{
			EVENTOS_consumir(eventos);
		}

		//En este costate tenemos un menu para el usuario, con un switch y los diferentes casos posibles
		costate
		{
			wfd menu[CONSOLE](eventos, CONSOLE);
		}
		costate
		{
			while (!sock_established(&echosock))
			{
				yield;
			}
			wfd menu[ETHERNET](eventos, ETHERNET);
		}
	}
}