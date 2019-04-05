#use IO.LIB
#use BTN.LIB
#use LED.LIB
#use UTILITIES.LIB

typedef struct Events{
	char command;
	char param;
	unsigned long time;
} Event;

#define MAX_EVENTOS 10

esperar_texto(char* texto){
	while(1){
		costate{
			waitfor(getswf(texto)); //Esperar a que el usuario ingrese algo
			break;
		}
	}
}

consumir_eventos(Event eventos[], int max_i){
	int i;
	for(i=0;i<max_i;i++){
		if(eventos[i].time == read_rtc()){
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
	char uno;
	char texto[10];
	char texto2[10];
	int response;
	int ev;
	ev = 0;
	HW_init();
	uno = '1';
	while(1){
		costate{
			LED_ROJO_SET();
			waitfor(DelayMs(400));
			LED_ROJO_RESET();
			waitfor(DelayMs(800));
		}

		consumir_eventos(eventos,ev);

		costate{
			printf("\nIngrese 1 para Fijar la hora del reloj de tiempo real (RTC) del Rabbit\n");
			printf("Ingrese 2 para Consultar la hora del RTC del Rabbi\n");
			printf("Ingrese 3 para Agregar o quitar un evento de calendario.\n");
			printf("Ingrese 4 para Consultar la lista de eventos de calendario activos.\n");
			waitfor(getswf(texto)); //Esperar a que el usuario ingrese algo
			printf(texto);
			switch(texto[0]){
				case '1':
				printf("Ingrese la hora\n");
				esperar_texto(texto2);
				write_rtc(atoi(texto2));
				break;
				case '2':
				printf("%d",read_rtc());
				break;
				case '3':
				printf("Ingrese 1 para prender o ingrese 0 para apagar\n");
				esperar_texto(texto2);
				eventos[ev].command = texto2[0];

				printf("Ingrese el numero de led\n");
				esperar_texto(texto2);
				eventos[ev].param = texto2[0];

				printf("Ingrese el tiempo en el que ejecutar\n");
				esperar_texto(texto2);
				eventos[ev].time = atoi(texto2);
				ev++;
				break;
				case '4':
				for(i=0;i<ev;i++){
					printf("Accion: %c, Bit: %c, Tiempo: %d\n",eventos[i].command,eventos[i].param,eventos[i].time);
				}
				break;
			}
		}
	}

}