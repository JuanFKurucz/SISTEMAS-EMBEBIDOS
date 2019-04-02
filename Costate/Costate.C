#class auto

configurar(){
   BitWrPortI(PEDDR, &PEDDRShadow, 1, 5);
}

main()
{
	int i;
   char uno;
   char texto[10];
   int response;
   configurar();
   //prender Led
   uno = '1';

   while(1){
  	 costate{
    BitWrPortI(PEDR, &PEDRShadow, 1, 5);
    waitfor(DelayMs(400));
     BitWrPortI(PEDR, &PEDRShadow, 0, 5);
     waitfor(DelayMs(800));
    }
    costate{
    	printf("\nhellow there\n");
      printf("Ingrese 1 para Fijar la hora del reloj de tiempo real (RTC) del Rabbit\n");
      printf("Ingrese 2 para Consultar la hora del RTC del Rabbi\n");
      printf("Ingrese 3 para Agregar o quitar un evento de calendario.\n");
      printf("Ingrese 4 para Consultar la lista de eventos de calendario activos.\n");
	   waitfor(getswf(texto));

      costate{
      	waitfor(texto[0]=='1');
         printf("Ingrese la hora\n");
         waitfor(getswf(texto));
         write_rtc(atoi(texto));
      }
      printf("%d",read_rtc());
      costate{
      	waitfor(texto[0]=="2");
          printf("Ingrese la hora\n");
         waitfor(getswf(texto));
      }
      costate{
      	waitfor(texto[0]=="3");
      }
      costate{
      	waitfor(texto[0]=="4");
      }
      printf(texto);
	 }
   }


}


