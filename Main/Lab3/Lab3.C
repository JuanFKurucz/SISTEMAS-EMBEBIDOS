#define TCPCONFIG 0
#define USE_ETHERNET		1
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
#define MAX_EVENTOS 10
#define MAX_TEXTO 10
#define EVENTO_DESHABILITADO 0xFF
#define preguntar(p,r,i) wfd hacerPregunta(p,r,i)

// Definimos los eventos
typedef struct Events{
	char command;
	char param;
	unsigned long time;
} Event;


//Función que en base al tipo pasado por parametro
//escribe por consola o por hercules
//En el caso de que sea un 1 será por Hercules
void imprimir(int tipo, char *s){
	if(tipo == 1){
		sock_fastwrite(&echosock, s, strlen(s));
	} else {
		printf("%s",s);
	}
}

// Función que inicializa los eventos con los valores time, param en 0 y command en EVENTO_DESHABILITADO
iniciar_eventos(Event eventos[]){
	int i;
	for(i=0;i<MAX_EVENTOS;i++){
		eventos[i].time = 0;
		eventos[i].param = 0;
		eventos[i].command = EVENTO_DESHABILITADO;
	}
}

//Función que borra los eventos
void borrar_evento(Event *evento){
	(*evento).command = EVENTO_DESHABILITADO;
	(*evento).param = 0;
	(*evento).time = 0;
}

// Función que corre durante toda la ejecucion de nuestro programa,
// buscando eventos que esten activos para ejecutarlos en el tiempo correspondiete
// Nuestros eventos se activan una vez que command deja de ser EVENTO_DESHABILITADO
consumir_eventos(Event eventos[]){
	int i;
	unsigned long timeNow;
	timeNow = read_rtc();
	for(i=0;i<MAX_EVENTOS;i++){
		if(eventos[i].command != EVENTO_DESHABILITADO && eventos[i].time == timeNow){
			if(eventos[i].command=='1'){
				LED_SET(eventos[i].param);
			} else {
				LED_RESET(eventos[i].param);
			}
			borrar_evento(&eventos[i]);
		}
	}
}

//función que obtiene a partir de un ano, mes y dia los segundos
// usando la funcion mktime y la estructura tm para generarla
unsigned long convertir_time(char* ano, char* mes, char* dia, char* hora, char* minuto, char* segundo){
	int numeroAno;
	int numeroMes;
	int numeroDia;
	int numeroHora;
	int numeroMinuto;
	int numeroSegundo;
	unsigned long calculo;
	struct tm fecha;
	struct tm fechaCheck;

	numeroAno = atoi(ano)-1900;
	numeroMes = atoi(mes);
	numeroDia = atoi(dia);
	numeroHora = atoi(hora);
	numeroMinuto = atoi(minuto);
	numeroSegundo = atoi(segundo);

	fecha.tm_year = numeroAno;
	fecha.tm_mon = numeroMes;
	fecha.tm_mday = numeroDia;
	fecha.tm_hour = numeroHora;
	fecha.tm_min = numeroMinuto;
	fecha.tm_sec = numeroSegundo;
	calculo = mktime(&fecha);
	mktm(&fechaCheck,calculo);
	if(memcmp(&fecha,&fechaCheck,0)==0){
		return calculo;
	} else {
		return -4;
	}
}

//Se imprime la fecha sumandole 1900 al a�o para mostrarlo humanamente
void printTime(struct tm *fecha,int tipo){
	char respuesta[20];
	sprintf(respuesta,"%d/%d/%d %d:%d:%d",
	(*fecha).tm_year+1900,
	(*fecha).tm_mon,
	(*fecha).tm_mday,
	(*fecha).tm_hour,
	(*fecha).tm_min,
	(*fecha).tm_sec
);
imprimir(tipo,respuesta);
}

//Funcion que retorna el primer indice vacio que encuentra de la lista de eventos
char encontrarEspacioParaEvento(Event *eventos){
	int i;
	for(i=0;i<MAX_EVENTOS;i++){
		if(eventos[i].command == EVENTO_DESHABILITADO){
			return i;
		}
	}
	return -1;
}

//Funcion que muestra los eventos en pantalla y retorna una lista de
void mostrarEventos(Event *eventos,int tipo){
	struct tm fecha;
	int i;
	char buffer[100];
	for(i=0;i<MAX_EVENTOS;i++){
		if(eventos[i].command != EVENTO_DESHABILITADO){
			sprintf(buffer,"%d: Accion: %c, Bit: %c, Tiempo: ",i,eventos[i].command,eventos[i].param);
			imprimir(tipo,buffer);
			mktm(&fecha,eventos[i].time);
			printTime(&fecha,tipo);
			imprimir(tipo,"\n");
		}
	}
}

//Funcion encarga de chequear si existe un evento creado
char existenEventos(Event *eventos){
	int i;
	for(i=0;i<MAX_EVENTOS;i++){
		if(eventos[i].command != EVENTO_DESHABILITADO){
			return 1;
		}
	}
	return 0;
}

//Funcion que imprime una pregunta y espera por la respuesta cargando el texto al puntero de char respuesta
cofunc void hacerPregunta(char *pregunta, char *respuesta, int tipo){
	imprimir(tipo,pregunta);
	imprimir(tipo,"\n");
	waitfor(getswf(respuesta));
}

//Funcion encargada de pedir al usuario ingresar una fecha
cofunc void ingresarFecha(unsigned long *time, int tipo){
	char ano[MAX_TEXTO];
	char mes[MAX_TEXTO];
	char dia[MAX_TEXTO];
	char hora[MAX_TEXTO];
	char minuto[MAX_TEXTO];
	char segundo[MAX_TEXTO];

	preguntar("Ingrese el ano",ano,tipo);

	preguntar("Ingrese el mes (Se espera un num entre 1 y 12)",mes,tipo);
	numeroMes = atoi(mes);
	while(numeroMes<=0 || numeroMes>12){
		preguntar("Ingrese nuevamente el mes (Se espera un num entre 1 y 12)",mes,tipo);
		numeroMes = atoi(mes);
	}
	preguntar("Ingrese el dia (Se espera un num entre 1 y 31)",dia,tipo);
	numeroDia = atoi(dia);
	while(numeroDia<=0 || numeroDia >= 32){
		preguntar("Ingrese nuevamente el dia (Se espera un num entre 1 y 31)",dia,tipo);
		numeroDia = atoi(dia);
	}
	preguntar("Ingrese la hora(Se espera un num entre 0 y 24)",hora,tipo);
	numeroHora = atoi(hora);
	while(numeroHora<0 || numeroHora>=24){
		preguntar("Ingrese nuevamente la hora(Se espera un num entre 0 y 24)",hora,tipo);
		numeroHora = atoi(hora);
	}
	preguntar("Ingrese los minutos (Se espera un num entre 1 y 59)",minuto,tipo);
	numeroMinuto = atoi(minuto);
	while(minuto<0 || minuto>=60){
		numeroMinuto = atoi(minuto);
		preguntar("Ingrese nuevamente los minutos (Se espera un num entre 1 y 59)",minuto,tipo);
	}
	preguntar("Ingrese los segundos (Se espera un num entre 1 y 59)",segundo,tipo);
	numeroSegundo = atoi(segundo);
	while(segundo<0 || segundo>=60){
		preguntar("Ingrese nuevamente los segundos (Se espera un num entre 1 y 59)",segundo,tipo);
		numeroSegundo = atoi(segundo);
	}

	*time = convertir_time(ano, mes, dia, hora, minuto, segundo);
	return;
}

//Funcion encargada de mostrar en pantalla el control de errores de la funcion convertir_time
//retorna 0 si hay un fallo retorna 1 si esta bien
int controlErroresFecha(unsigned long time, int tipo){
	int result;
	result = 0;
	if(time == -1){
		imprimir(tipo,"El ano ingresado es incorrecto\n");
	} else if(time<0){
		imprimir(tipo,"Fecha incorrecta\n");
	} else {
		result = 1;
	}
	return result;
}

//Menu principal - se despliega en cuento comienza nuestro programa
void imprimirMenu(int tipo){
	imprimir(tipo,"Ingrese 1 para Fijar la hora del reloj de tiempo real (RTC) del Rabbit\n");
	imprimir(tipo,"Ingrese 2 para Consultar la hora del RTC del Rabbi\n");
	imprimir(tipo,"Ingrese 3 para Agregar un evento de calendario.\n");
	imprimir(tipo,"Ingrese 4 para Quitar un evento de calendario.\n");
	imprimir(tipo,"Ingrese 5 para Consultar la lista de eventos de calendario activos.\n");
}

cofunc void menu(char *texto, Event *eventos, int tipo){
	struct tm fecha;
	char command;
	char param;
	int i; //Posicion de indice, usada para fors y obtencion de posiciones con funciones
	unsigned long time;
	int status;
	imprimirMenu(tipo);
	preguntar("Ingrese una opcion",texto,tipo);
	switch(texto[0]){
		case '1':
		//Para pasar el sting a time utilizamos la funcion getswf
		//Para fijar la hora del reloj utilizamos la funcion write_rtc
		wfd ingresarFecha(&time,tipo);
		if(controlErroresFecha(time,tipo) == 1){
			write_rtc(time);
			imprimir(tipo,"Fecha actualizada con exito\n");
		}
		break;
		case '2':
		//Para consultar la hora de la placa utilizamos la funcion read_rtc
		mktm(&fecha,read_rtc());
		printTime(&fecha,tipo);
		break;
		case '3':
		i = encontrarEspacioParaEvento(eventos);
		//Como definimos MAX_EVENTOS en 10, tenemos que controlar que el usuario no supere ese limite
		//Para que el evento quede correctamente definido, esperamos a tener todos los parametros que el usuario ingrese
		//verificando que sean correctos y luego lo creamos
		//De no realizar esto el programa prodria llevar a dar problemas, si se intenta listar un evento que todavia no tenga
		//todos sus datos
		if(i==-1){
			imprimir(tipo,"Capacidad maxima de eventos alcanzada");
		} else {
			preguntar("Ingrese 1 para prender un led o ingrese 0 para apagarlo",texto,tipo);
			command = texto[0];

			preguntar("Ingrese el numero de led",texto,tipo);
			param = texto[0];

			imprimir(tipo,"Se asignara el tiempo del evento ahora:\n");
			wfd ingresarFecha(&time,tipo);
			if(controlErroresFecha(time,tipo) == 1){
				// Este if es para controlar los datos que el usuario ingresa
				if((command == '1' || command == '0') && (param>='0' && param <='7')){
					eventos[i].command = command;
					eventos[i].param = param;
					eventos[i].time = time;
				} else {
					imprimir(tipo,"Datos erroneos\n");
				}
			} else {
				imprimir(tipo,"Fecha erronea\n");
			}
		}
		break;
		case '4':
		mostrarEventos(eventos,tipo);
		if(existenEventos(eventos) == 1){
			i=-1;
			preguntar("Inserte el indice del evento a eliminar",texto,tipo);
			i = atoi(texto);
			//Controlamos que el usuario no se vaya de rango para eliminar un evento
			if(i>=0 && i < MAX_EVENTOS){
				//Lo que hacemos para eliminar nuestro evento es volver a setear los datos como en el estado inicial
				if(eventos[i].command != EVENTO_DESHABILITADO){
					borrar_evento(&eventos[i]);
				} else {
					imprimir(tipo,"Este evento no existe\n");
				}
			} else {
				imprimir(tipo,"El indice se va de rango de la lista de eventos\n");
			}
		} else {
			imprimir(tipo,"No hay eventos creados\n");
		}
		break;
		case '5':
		//Recorremos todos los eventos buscando unicamente los que se encuentren activos
		//y los imprimimos por consola
		if(existenEventos(eventos) == 0){
			imprimir(tipo,"No hay eventos creados\n");
		} else {
			mostrarEventos(eventos,tipo);
		}
		break;
		default:
		imprimir(tipo,"Comando no encontrado");
	}
}


//Establecer la conexión
cofunc int tcp_connect(tcp_Socket *s, int port, Event *eventos){
	auto int length, space_avaliable;
	char tmpBuff[MAX_BUFSIZE];
	char buf[MAX_BUFSIZE];
	int i;
	int bufferIndex;
	bufferIndex=0;
	tcp_listen(s, port, 0, 0, NULL, 0);
	// Espera por la conexión
	while((-1 == sock_bytesready(s)) && (0 == sock_established(s)))
	yield;

	imprimirMenu(1);
	while(sock_established(s)) {
		space_avaliable = sock_tbleft(s);
		if(space_avaliable > (MAX_BUFSIZE-1))
		space_avaliable = (MAX_BUFSIZE-1);

		// Obtener la informacióm
		length = sock_fastread(s, tmpBuff, space_avaliable);
		if(length > 0) {
			tmpBuff[length] = '\0';
			for(i = 0; tmpBuff[i] != '\0'; i++) {
				if(bufferIndex>MAX_BUFSIZE-1 || tmpBuff[i]==10){
					sock_fastwrite(s, buf, bufferIndex);

					break;
				} else {
					buf[bufferIndex]=tmpBuff[i];
					bufferIndex++;
				}
			}
		}
		yield; // give other tasks time to run
	}
	sock_close(s);
	return 1;
}

main()
{
	Event eventos[MAX_EVENTOS];
	char texto[MAX_TEXTO];

	HW_init();
	iniciar_eventos(eventos);

	printf("Iniciando Socket\n");
	sock_init();
	printf("Iniciado\n");

	while(1){
		//Durante la ejecucion de nuestro programa prendemos y apagamos el led rojo
		costate{
			LED_ROJO_SET();
			waitfor(DelayMs(400));
			LED_ROJO_RESET();
			waitfor(DelayMs(800));
		}

		costate {
			// Go do the TCP/IP part, on the first socket
			wfd tcp_connect(&echosock, PORT, eventos);
		}
		costate {
			// Para mantener la coneccion Ethernet
			tcp_tick(NULL);
		}

		costate{
			consumir_eventos(eventos);
		}

		//En este costate tenemos un menu para el usuario, con un switch y los diferentes casos posibles
		costate{
			wfd menu(texto,eventos,0);
		}
	}

}
