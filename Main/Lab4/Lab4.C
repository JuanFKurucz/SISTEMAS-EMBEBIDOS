#use "ucos2.lib"

#define TCPCONFIG 0
#define USE_ETHERNET 1
#define MY_IP_ADDRESS "10.10.6.100"
#define MY_NETMASK "255.255.255.0"
#define MY_GATEWAY "10.10.6.2"
#define MAX_BUFSIZE 2048
#memmap xmem
#use "dcrtcp.lib"
#define PORT 7

#use IO.LIB
#use BTN.LIB
#use LED.LIB
#use UTILITIES.LIB
#use EVENTOS.LIB

#define MAX_TEXTO 10



tcp_Socket echosock;
OS_EVENT* Semaforo;

void blinkRedLed(void *datos){
	while(1){
      LED_ROJO_SET();
		OSTimeDlySec(1);
		LED_ROJO_RESET();
      OSTimeDlySec(1);
	}
}

//Funcion que en base al tipo pasado por parametro
//escribe por consola o por hercules
//En el caso de que sea un 1 será por Hercules
void imprimir(int tipo, char *s)
{
  /*	if (tipo == ETHERNET)
	{
		sock_fastwrite(&echosock, s, strlen(s));
	}
	else
	{       */
		printf("%s", s);
	//}
}


//Menu principal - se despliega en cuento comienza nuestro programa
void imprimirMenu()
{
	printf("===MENU COMIENZO===\n");
	printf("Ingrese 1 para Fijar la hora del reloj de tiempo real (RTC) del Rabbit\n");
	printf("Ingrese 2 para Consultar la hora del RTC del Rabbit\n");
	printf("Ingrese 3 para Agregar un evento de calendario.\n");
	printf("Ingrese 4 para Quitar un evento de calendario.\n");
	printf("Ingrese 5 para Consultar la lista de eventos de calendario activos.\n");
	printf("Ingrese 6 para Consultar las entradas analogicas.\n");
	printf("===MENU FIN===\n");
}

//Funcion que imprime una pregunta y espera por la respuesta cargando el texto al puntero de char respuesta
void preguntar(char *pregunta, char *texto, int tipo)
{
	printf("%s",pregunta);
   while(getswf(texto)==0);
}


//Se imprime la fecha sumandole 1900 al a?o para mostrarlo humanamente
void printTime(struct tm *fecha)
{
	char respuesta[20];
	sprintf(respuesta, "%d/%d/%d %d:%d:%d\n",
			(*fecha).tm_year + 1900,
			(*fecha).tm_mon,
			(*fecha).tm_mday,
			(*fecha).tm_hour,
			(*fecha).tm_min,
			(*fecha).tm_sec);
	printf("%s",respuesta);
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

//Funcion encargada de pedir al usuario ingresar una fecha
void ingresarFecha(unsigned long *time, int tipo)
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


//Funcion encargada de gestionar el menu y todas sus tareas
void menu(void* data)
{
   char texto[MAX_TEXTO];
	struct tm fecha;
	char command;
	char param;
	int i; //Posicion de indice, usada para fors y obtencion de posiciones con funciones
	unsigned long time;
	int status;
   int tipo;
   tipo=1;

   while(1){
		imprimirMenu();
	   preguntar("Ingrese una opcion\n",texto,tipo);

	   switch (texto[0])
	   {
      case '1':
         //Para pasar el sting a time utilizamos la funcion getswf
			//Para fijar la hora del reloj utilizamos la funcion write_rtc
	      ingresarFecha(&time, tipo);
	      if (controlErroresFecha(time, tipo) == 1)
	      {
	         write_rtc(time);
	         imprimir(tipo, "Fecha actualizada con exito\n");
	      }
      	break;
	   case '2':
	      //Para consultar la hora de la placa utilizamos la funcion read_rtc
	      mktm(&fecha, read_rtc());
	      printTime(&fecha);
	      break;
	   default:
	      printf("%s", "Comando no encontrado\n");
	   }
   }
}

void matenerEthernet(){
	while(1){
		tcp_tick(NULL);
   }
}

void consumir(void* data){
	while(1){
   	EVENTOS_consumir(data);
   }
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
	OSInit();
	HW_init();
	iniciarConexion();


	printf("Iniciando\n");

	OSTaskCreate(matenerEthernet, NULL, 512, 10);
	OSTaskCreate(blinkRedLed, NULL, 512, 6);
	OSTaskCreate(consumir, eventos, 512, 9);
	OSTaskCreate(menu, eventos, 512, 8);     
	OSStart();
}