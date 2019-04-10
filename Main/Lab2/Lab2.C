#use IO.LIB
#use BTN.LIB
#use LED.LIB
#use UTILITIES.LIB
#define MAX_EVENTOS 10
#define MAX_TEXTO 10
#define EVENTO_DESHABILITADO 0xFF
#define preguntar(p,r) wfd hacerPregunta(p,r)

// Definimos los eventos
typedef struct Events{
	char command;
	char param;
	unsigned long time;
} Event;

// Funcion que inicializa los eventos con los valores time, param en 0 y command en EVENTO_DESHABILITADO
iniciar_eventos(Event eventos[]){
	int i;
	for(i=0;i<MAX_EVENTOS;i++){
		eventos[i].time = 0;
		eventos[i].param = 0;
		eventos[i].command = EVENTO_DESHABILITADO;
	}
}

// Funcion que corre durante toda la ejecucion de nuestro programa,
// buscando eventos que esten activos para ejecutarlos en el tiempo correspondiete
// Nuestros eventos se activan una vez que command deja de ser EVENTO_DESHABILITADO
consumir_eventos(Event eventos[]){
	int i;
	for(i=0;i<MAX_EVENTOS;i++){
		if(eventos[i].command != EVENTO_DESHABILITADO && eventos[i].time == read_rtc()){
			if(eventos[i].command=='1'){
				LED_SET(eventos[i].param);
			} else {
				LED_RESET(eventos[i].param);
			}
		}
	}
}

//funcion que obtiene a partir de un ano, mes y dia los segundos
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
	if(numeroMes<=0 || numeroMes>12){
		return -2;
	}
	if(numeroDia<=0 || numeroDia >= 32){
		return -3;
	}
	if(numeroHora<0 || numeroHora>=24){
		return -5;
	}
	if(numeroMinuto<0 || numeroMinuto>=60){
		return -6;
	}
	if(numeroSegundo<0 || numeroSegundo>=60){
		return -7;
	}
	fecha.tm_year = numeroAno;
	fecha.tm_mon = numeroMes;
	fecha.tm_mday = numeroDia;
	fecha.tm_hour = numeroHora;
	fecha.tm_min = numeroMinuto;
	fecha.tm_sec = numeroSegundo;
	calculo = mktime(&fecha);
	mktm(&fechaCheck,calculo);
	if(fecha.tm_mon == fechaCheck.tm_mon && fecha.tm_mday == fechaCheck.tm_mday && fecha.tm_year == fechaCheck.tm_year){
		return calculo;
	} else {
		return -4;
	}
}

//Se imprime la fecha sumandole 1900 al a�o para mostrarlo humanamente
printTime(struct tm fecha){
	printf("%d/%d/%d %d:%d:%d",fecha.tm_year+1900,fecha.tm_mon,fecha.tm_mday,fecha.tm_hour,fecha.tm_min,fecha.tm_sec);
}

int encontrarEspacioParaEvento(Event *eventos){
	int posicion;
	int i;
	posicion=-1;
	for(i=0;i<MAX_EVENTOS;i++){
		if(eventos[i].command == EVENTO_DESHABILITADO){
			posicion=i;
			break;
		}
	}
	return posicion;
}

//Funcion que muestra los eventos en pantalla y retorna una lista de
void mostrarEventos(Event *eventos){
	int i;
	for(i=0;i<MAX_EVENTOS;i++){
		if(eventos[i].command != EVENTO_DESHABILITADO){
			printf("%d: Accion: %c, Bit: %c, Tiempo: %d\n",i,eventos[i].command,eventos[i].param,eventos[i].time);
		}
	}
}

//Funcion encarga de chequear si existe un evento creado
int existenEventos(Event *eventos){
	int i;
	for(i=0;i<MAX_EVENTOS;i++){
		if(eventos[i].command != EVENTO_DESHABILITADO){
			return 1;
		}
	}
	return 0;
}

cofunc void hacerPregunta(char *pregunta, char *respuesta){
	printf(pregunta);
	printf("\n");
	waitfor(getswf(respuesta));
	return;
}

//Funcion encargada de pedir al usuario ingresar una fecha
cofunc void ingresarFecha(unsigned long *time){
	char ano[MAX_TEXTO];
	char mes[MAX_TEXTO];
	char dia[MAX_TEXTO];
	char hora[MAX_TEXTO];
	char minuto[MAX_TEXTO];
	char segundo[MAX_TEXTO];

	preguntar("Ingrese el ano",ano);
	preguntar("Ingrese el mes",mes);
	preguntar("Ingrese el dia",dia);
	preguntar("Ingrese la hora",hora);
	preguntar("Ingrese los minutos",minuto);
	preguntar("Ingrese los segundos",segundo);

	*time = convertir_time(ano, mes, dia, hora, minuto, segundo);
	return;
}

//Funcion encargada de mostrar en pantalla el control de errores de la funcion convertir_time
//retorna 0 si hay un fallo retorna 1 si esta bien
int controlErroresFecha(unsigned long time){
	int result;
	result = 0;
	if(time == -1){
		printf("El ano ingresado es incorrecto\n");
	} else if(time == -2){
		printf("El mes ingresado es incorrecto\n");
	} else if(time == -3){
		printf("El dia ingresado es incorrecto\n");
	} else if(time == -4){
		printf("Fecha erronea\n");
	} else if(time == -5){
		printf("La hora ingresada es incorrecta\n");
	} else if(time == -6){
		printf("Los minutos ingresados son incorrectos\n");
	} else if(time == -7){
		printf("Los segundos ingresados son incorrectos\n");
	} else if(time<0){
		printf("Fecha incorrecta\n");
	} else {
		result = 1;
	}
	return result;
}

main()
{
	struct tm fecha;
	Event eventos[MAX_EVENTOS];
	char texto[MAX_TEXTO];
	char command;
	char param;
	int i; //Posicion de indice, usada para fors y obtencion de posiciones con funciones
	unsigned long time;

	HW_init();
	iniciar_eventos(eventos);

	while(1){
		//Durante la ejecucion de nuestro programa prendemos y apagamos el led rojo
		costate{
			LED_ROJO_SET();
			waitfor(DelayMs(400));
			LED_ROJO_RESET();
			waitfor(DelayMs(800));
		}

		costate{
			consumir_eventos(eventos);
		}

		//En este costate tenemos un menu para el usuario, con un switch y los diferentes casos posibles
		costate{
			printf("\nIngrese 1 para Fijar la hora del reloj de tiempo real (RTC) del Rabbit\n");
			printf("Ingrese 2 para Consultar la hora del RTC del Rabbi\n");
			printf("Ingrese 3 para Agregar un evento de calendario.\n");
			printf("Ingrese 4 para Quitar un evento de calendario.\n");
			printf("Ingrese 5 para Consultar la lista de eventos de calendario activos.\n");
			preguntar("Ingrese una opcion",texto);
			switch(texto[0]){
				case '1':
               //Para pasar el sting a time utilizamos la funcion getswf
					//Para fijar la hora del reloj utilizamos la funcion write_rtc
					wfd ingresarFecha(&time);
	            if(controlErroresFecha(time) == 1){
	               write_rtc(time);
	               printf("Fecha actualizada con exito\n");
	            }
	            break;
				case '2':
            	//Para consultar la hora de la placa utilizamos la funcion read_rtc
					mktm(&fecha,read_rtc());
	            printTime(fecha);
	            break;
				case '3':
            	i = encontrarEspacioParaEvento(eventos);
	            //Como definimos MAX_EVENTOS en 10, tenemos que controlar que el usuario no supere ese limite
	            //Para que el evento quede correctamente definido, esperamos a tener todos los parametros que el usuario ingrese
	            //verificando que sean correctos y luego lo creamos
	            //De no realizar esto el programa prodria llevar a dar problemas, si se intenta listar un evento que todavia no tenga
	            //todos sus datos
	            if(i==-1){
	               printf("Capacidad maxima de eventos alcanzada");
	            } else {
	               preguntar("Ingrese 1 para prender o ingrese 0 para apagar",texto);
	               command = texto[0];

	               preguntar("Ingrese el numero de led",texto);
	               param = texto[0];

	               printf("Se asignara el tiempo del evento ahora:\n");
	               wfd ingresarFecha(&time);
	               if(controlErroresFecha(time) == 1){
	                  // Este if es para controlar los datos que el usuario ingresa
	                  if((command == '1' || command == '0') && (param>='0' && param <='7')){
	                     eventos[i].command = command;
	                     eventos[i].param = param;
	                     eventos[i].time = time;
	                  } else {
	                     printf("Datos erroneos\n");
	                  }
	               } else {
	                  printf("Fecha erronea\n");
	               }
	            }
	            break;
				case '4':
            	mostrarEventos(eventos);
	            if(existenEventos(eventos) == 1){
	               i=-1;
	               preguntar("Inserte el indice del evento a eliminar",texto);
	               i = atoi(texto);
	               //Controlamos que el usuario no se vaya de rango para eliminar un evento
	               if(i>=0 && i < MAX_EVENTOS){
	                  //Lo que hacemos para eliminar nuestro evento es volver a setear los datos como en el estado inicial
	                  if(eventos[i].command != EVENTO_DESHABILITADO){
	                     eventos[i].command = EVENTO_DESHABILITADO;
	                     eventos[i].param = 0;
	                     eventos[i].time = 0;
	                  } else {
	                     printf("Este evento no existe\n");
	                  }
	               } else {
	                  printf("El indice se va de rango de la lista de eventos\n");
	               }
	            } else {
	               printf("No hay eventos creados\n");
	            }
	            break;
				case '5':
            	//Recorremos todos los eventos buscando unicamente los que se encuentren activos
	            //y los imprimimos por consola
	            if(existenEventos(eventos) == 0){
	               printf("No hay eventos creados\n");
	            } else {
								mostrarEventos(eventos);	
							}
	            break;
	         default:
			  		printf("Comando no encontrado");
			}
		}
	}

}
