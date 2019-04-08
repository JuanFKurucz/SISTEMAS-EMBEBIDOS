#use IO.LIB
#use BTN.LIB
#use LED.LIB
#use UTILITIES.LIB
#define MAX_EVENTOS 10
#define MAX_TIME 32767

// Definimos los eventos
typedef struct Events{
	char command;
	char param;
	unsigned long time;
} Event;

// Funcion que inicializa los eventos con los valores time, param en 0 y command en 0xFF
iniciar_eventos(Event eventos[]){
	int i;
	for(i=0;i<MAX_EVENTOS;i++){
		eventos[i].time = 0;
		eventos[i].param = 0;
		eventos[i].command = 0xFF;
	}
}

// Funcion que corre durante toda la ejecucion de nuestro programa,
// buscando eventos que esten activos para ejecutarlos en el tiempo correspondiete
// Nuestros eventos se activan una vez que command deja de ser 0xFF
consumir_eventos(Event eventos[]){
	int i;
	for(i=0;i<MAX_EVENTOS;i++){
		if(eventos[i].command != 0xFF && eventos[i].time == read_rtc()){
			if(eventos[i].command=='1'){
				LED_SET(eventos[i].param);
			} else {
				LED_RESET(eventos[i].param);
			}
		}
	}
}

main()
{
	Event eventos[MAX_EVENTOS];
	int i;
	char texto[10];
	char texto2[10];
	int response;
	int ev;
	char command;
	char param;
	int time;
	int posicion;
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
			waitfor(getswf(texto)); //Esperar a que el usuario ingrese una opcion
			switch(texto[0]){
				//Para pasar el sting a time utilizamos la funcion getswf
				//Para fijar la hora del reloj utilizamos la funcion write_rtc
				case '1':
				printf("Ingrese la hora\n");
				waitfor(getswf(texto2));
				time=atoi(texto2);
				if(time>=0 && time <=MAX_TIME){
					write_rtc(atoi(texto2));
				}
				break;
				//Para consultar la hora de la placa utilizamos la funcion read_rtc
				case '2':
				printf("%d",read_rtc());
				break;
				case '3':
				posicion=-1;
				for(i=0;i<MAX_EVENTOS;i++){
					if(eventos[i].command == 0xFF){
						posicion=i;
						break;
					}
				}
				//Como definimos MAX_EVENTOS en 10, tenemos que controlar que el usuario no supere ese limite
				//Para que el evento quede correctamente definido, esperamos a tener todos los parametros que el usuario ingrese
				//verificando que sean correctos y luego lo creamos
				//De no realizar esto el programa prodria llevar a dar problemas, si se intenta listar un evento que todavia no tenga
				//todos sus datos
				if(posicion==-1){
					printf("Capacidad maxima de eventos alcanzada");
				} else {
					printf("Ingrese 1 para prender o ingrese 0 para apagar\n");
					waitfor(getswf(texto2));
					command = texto2[0];

					printf("Ingrese el numero de led\n");
					waitfor(getswf(texto2));
					param = texto2[0];

					printf("Ingrese el tiempo en el que ejecutar\n");
					waitfor(getswf(texto2));
					time = atoi(texto2);
					// Este if es para controlar los datos que el usuario ingresa
					if((command == '1' || command == '0') && (param>='0' && param <='7') && (time <= MAX_TIME)){
						eventos[posicion].command = command;
						eventos[posicion].param = param;
						eventos[posicion].time = time;
						command = 0;
						param = 0;
						time = 0;
					} else {
						printf("Datos erroneos\n");
					}
				}
				break;
				case '4':
				printf("Inserte el indice del evento a eliminar\n");
				posicion=-1;
				waitfor(getswf(texto2));
				posicion = atoi(texto2);
				//Controlamos que el usuario no se vaya de rango para eliminar un evento
				if(posicion>=0 && posicion < MAX_EVENTOS){
					//Lo que hacemos para eliminar nuestro evento es volver a setear los datos como en el estado inicial
					eventos[posicion].command = 0xFF;
					eventos[posicion].param = 0;
					eventos[posicion].time = 0;
				} else {
					printf("El indice se va de rango de la lista de eventos\n");
				}
				break;
				case '5':
				//Recorremos todos los eventos buscando unicamente los que se encuentren activos
				//y los imprimimos por consola
				for(i=0;i<MAX_EVENTOS;i++){
					if(eventos[i].command == 0xFF){
						printf("%d: Evento no definido\n",i);
					} else {
						printf("%d: Accion: %c, Bit: %c, Tiempo: %d\n",i,eventos[i].command,eventos[i].param,eventos[i].time);
					}
				}
				break;
				default:
				printf("Comando no encontrado");
			}
		}
	}

}
