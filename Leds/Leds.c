#class auto

enum portName { PA = PADR};
enum bitNumber {PA0,PA1,PA2,PA3,PA4,PA5,PA6,PA7};

void setOutput(enum portName p_port, enum bitNumber p_pin, unsigned char p_state){
   char* shadow;
   if(p_port == PADR){
    shadow=&PADRShadow;
   }
   BitWrPortI(p_port, shadow, p_state, p_pin);
}

configurar(){
	WrPortI(SPCR, &SPCRShadow, 0x084); // Puerto A (LEDS) como output
   //BitWrPortI(PBDR, &PBDRShadow,
}

main()
{
	configurar();

   setOutput(PADR,PA1,1);

   while(1==1){

   }
}


