#use IO.LIB
#use BTN.LIB
#use LED.LIB
#use UTILITIES.LIB
#define MAX_EVENTOS 10
#define MAX_TIME 32767

typedef struct Events{
	char command;
	char param;
	unsigned long time;
} Event;

iniciar_eventos(Event eventos[]){
 	int i;
	for(i=0;i<MAX_EVENTOS;i++){
		eventos[i].time = 0;
      eventos[i].param = 0;
      eventos[i].command = 0xFF;
	}
}

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
		costate{
			LED_ROJO_SET();
			waitfor(DelayMs(400));
			LED_ROJO_RESET();
			waitfor(DelayMs(800));
		}

      costate{
      	consumir_eventos(eventos);
      }

		costate{
			printf("\nIngrese 1 para Fijar la hora del reloj de tiempo real (RTC) del Rabbit\n");
			printf("Ingrese 2 para Consultar la hora del RTC del Rabbi\n");
         printf("Ingrese 3 para Agregar un evento de calendario.\n");
         printf("Ingrese 4 para Quitar un evento de calendario.\n");
			printf("Ingrese 5 para Consultar la lista de eventos de calendario activos.\n");
			waitfor(getswf(texto)); //Esperar a que el usuario ingrese algo
         switch(texto[0]){
				case '1':
					printf("Ingrese la hora\n");
					waitfor(getswf(texto2));
               time=atoi(texto2);
               if(time>=0 && time <=MAX_TIME){
	            	write_rtc(atoi(texto2));
               }
					break;
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
               if(posicion>=0 && posicion < MAX_EVENTOS){
               	eventos[posicion].command = 0xFF;
                  eventos[posicion].param = 0;
                  eventos[posicion].time = 0;
               } else {
                  printf("El indice se va de rango de la lista de eventos\n");
               }
            	break;
				case '5':
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