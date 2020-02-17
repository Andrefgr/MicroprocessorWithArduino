#define DEBOUNCETIME 500
unsigned long LastInterrupt;

word MemDados[256];
word MemCodigo[64];
word ModControlo[128];

boolean Zero = 0;
boolean CarryOut = 0;

byte Y1 = 0;
byte Y2 = 0;
byte Y3 = 0;
byte Y4 = 0;
byte Y5 = 0;
byte DB_MemDados = 0;

boolean EnR0, EnR1, EnF, WR, RD, EnA, SA1, SA0, JMP, JC, JNZ, EnRn, QFZ, PC0_1, PC0_2;

byte DFC, DFZ, DRA;

byte QPC, DPC, QRn, QR1, DRn, QR0, QRA, QFC;

boolean D8 = bitRead(MemCodigo[QPC], 8);
boolean D7 = bitRead(MemCodigo[QPC], 7);
boolean D6 = bitRead(MemCodigo[QPC], 6);
boolean D5 = bitRead(MemCodigo[QPC], 5);
boolean D4 = bitRead(MemCodigo[QPC], 4);
boolean D3 = bitRead(MemCodigo[QPC], 3);
boolean D2 = bitRead(MemCodigo[QPC], 2);
boolean D1 = bitRead(MemCodigo[QPC], 1);
boolean D0 = bitRead(MemCodigo[QPC], 0);



void setup() {
  Serial.begin(9600);
  QPC = 0;
  EPROM();
  Instruccoes();
  attachInterrupt(0, MCLK, RISING);
  interrupts();
}
void MCLK(){
  if(millis() - LastInterrupt < DEBOUNCETIME ){
     QPC= DPC;
     if(EnRn){
       if(D4){
         QR1= DRn;
       }
       else{
         QR0= DRn;
       }
     }
     if(EnA){
        QRA= DRA;
     }
     if(EnF){
        QFC= DFC; 
        QFZ= DFZ; 
     }
     PrintRegistos();
     PrintSinais();
   } 
  
  LastInterrupt = millis();
}

byte MUX_2x1(boolean S, byte In0, byte In1){
  if(!S)
    return In0;
  return In1;
}
    

byte MUX_4x1(boolean S1, boolean S0, byte In0, byte In1, byte In2, byte In3){
    
  if(!S0 && !S1)
    return In0;
      
  else if(S0 && !S1)
    return In1;
      
  else if(!S0 && S1)
    return In2;
    
  else    
    return In3;
}


byte ALU(byte Select, byte In0, byte In1, byte CarryIn){

  word help;

  if (Select == 0b100){
    
   //ADDC
    help = In0 + In1 + CarryIn;
    if(help & 0b100000000){
      CarryOut = 1;
    }
    else {
      CarryOut = 0;
    }
    Y5 = (byte)help;       
  }
   
  else if (Select == 0b101){
    //NOT
    Y5 = ~In1;
  }
   
  else if (Select == 0b110){
    //AND
    Y5 = (In0 & In1);
  }

  else if (Select == 0b111){
    //OR
    Y5 = (In0 | In1);
  }
   
  if (Y5 == 0) {
    Zero = 1;
  }
  
  else {
    Zero = 0;
  }
   
  return Y5;  
}

   
void ExtSinal(){
  
  byte I = MemCodigo[QPC];
  byte r = I & 0x1F;
  if (D4)
    r = r | 0xE0; // Nº Negativo
  else 
    r = r & 0x1F; // Nº Positivo
}
  

byte soma(byte A, byte B){
  return A + B; 
}

void RegistoRn(byte D4){
  
  if(EnRn == 1){
    if (D4 == 1) {
      EnR1 = 1;
      EnR0 = 0;
    }

    else {
      EnR1 = 0;
      EnR0 = 1;
    }
  }
  else {
    EnR0 = 0;
    EnR1 = 0;
  }
}

void EPROM(){
  for(byte b=0x20 ; b <=0x3F ; b++){
    ModControlo[b]= 0x1F0;
  }
  
  for(byte b=0x10 ; b <=0x17 ; b++){
    ModControlo[b]= 0x188;
  }
  
  for(byte b=0x08 ; b <=0x0F ; b++){
    ModControlo[b]= 0x184;
  }
  
  for(byte b=0x00 ; b <=0x07 ; b++){
    ModControlo[b]= 0x182;
  }
  
  ModControlo[0x1A]= 0x1C0;
  ModControlo[0x1B]= 0x181;
  ModControlo[0x1D]= 0x3D0;
  ModControlo[0x1E]= 0x3D0;
  ModControlo[0x1F]= 0x3D0;
  ModControlo[0x1C]= 0x3D0;
  ModControlo[0x18]= 0x160;
  ModControlo[0x19]= 0x080;
  
}

void Instruccoes(){
  MemDados[0] = 25;
  MemDados[1] = 5;
  
  // Armazenar o valor da Memória de Dados no índice 1
  
  MemCodigo[0] = 0b100000111; // MOV A, const8 => A = const8 = 0
  MemCodigo[1] = 0b011100011; // MOV R0, A => R0 = A = 0
  MemCodigo[2] = 0b011100000; // MOV A, @R0 => A = @R0 = 25
  MemCodigo[3] = 0b011110011; // MOV R1, A => R1 = A = 25
  
  // Armazenar o valor da Memória de Dados no índice 2
  
  MemCodigo[4] = 0b100000001; // MOV A, const8 => A = const8 = 1
  MemCodigo[5] = 0b011100011; // MOV R0, A => R0 = A = 1
  MemCodigo[6] = 0b011100000; // MOV A, @R0 => A = @R0 = 5
  
  // Realizar a soma
  
  MemCodigo[7] = 0b011110100; // ADDC A, R1 => A = A + R1 = 25 + 5 = 30
  
  // Mover o resultado da soma para a Memória de Dados

  MemCodigo[8] = 0b011110011; //MOV R1, A => R1 = A = 30
  MemCodigo[9] = 0b100000010; //MOV A, const8 => A = 2
  MemCodigo[10] = 0b011100011; //MOV R0, A => R0 = A = 2
  MemCodigo[11] = 0b011110010; //MOV A, R1 => A = R1 = 30
  MemCodigo[12] = 0b011100001; //MOV @R0, A -> @R0 = A <=> @2 = A = 30
  MemCodigo[13] = 0b001100110; //JNZ 6; QPC = 19
  
  // Forçar Carry = 1
  
  MemCodigo[19] = 0b111111111; // MOV A, const8 => A = 255
  MemCodigo[20] = 0b011100011; // MOV R0, A => R0 = A = 255
  MemCodigo[21] = 0b100000001; // MOV A, const8 => A = 1
  MemCodigo[22] = 0b011100100; // ADDC A, R0 => A = A + R0 = 1 + 255 = 256; Carry = 1
  
  MemCodigo[23] = 0b000100111; // JC 7; QPC = 30
  
  // HALT!
  MemCodigo[25] = 0b010011001; // JMP 0 => HALT!
  
  // Teste a NOT
  MemCodigo[30] = 0b011100101; // NOT A => A = -A = 254
  
  // Teste a AND
  MemCodigo[31] = 0b011110110; // AND A, R1 => A = A & R1 = 254 & 30 = 30
 
  MemCodigo[32] = 0b101011000; // MOV A, const8 => A = 88
  
  // Teste a OR
  MemCodigo[33] = 0b011110111; // OR A, R1 => A = A | R1 = 88 | 30 = 94
  
  // Teste a Zero
  MemCodigo[34] = 0b100000000; // MOV A, const8 => A = 0
  MemCodigo[35] = 0b011100011; // MOV R0, A => R0 = A = 0
  MemCodigo[36] = 0b011100100; // ADDC A, R0 => A = A + R0 = 0 + 0 = 0
  MemCodigo[37] = 0b001100010; // JNZ 2 => QPC = 39
  
  // Jump para trás
  MemCodigo[38] = 0b011110111; // ADDC A, R1 => A = A + R1 = 0 + 30 = 30
  MemCodigo[39] = 0b001110010; // JNZ -14 => QPC = 25
  

  
}  
  
  
void calcVarComb(){
   
  // leitura de bits da Memoria de Codigo
  
   D8= bitRead(MemCodigo[QPC], 8);
   D7= bitRead(MemCodigo[QPC], 7);
   D6= bitRead(MemCodigo[QPC], 6);
   D5= bitRead(MemCodigo[QPC], 5);
   D4= bitRead(MemCodigo[QPC], 4);
   D3= bitRead(MemCodigo[QPC], 3);
   D2= bitRead(MemCodigo[QPC], 2);
   D1= bitRead(MemCodigo[QPC], 1);
   D0= bitRead(MemCodigo[QPC], 0);

   // Modulo de Controlo
   byte Idx = D8 << 5 | D7 << 4 | D6 << 3 | D2 << 2 | D1 << 1 | D0;
  EnF  = bitRead(ModControlo[Idx], 9);
  WR   = bitRead(ModControlo[Idx], 8);
  RD   = bitRead(ModControlo[Idx], 7);
  EnA  = bitRead(ModControlo[Idx], 6);
  SA1  = bitRead(ModControlo[Idx], 5);
  SA0  = bitRead(ModControlo[Idx], 4);
  JMP  = bitRead(ModControlo[Idx], 3);
  JNZ  = bitRead(ModControlo[Idx], 2);
  JC = bitRead(ModControlo[Idx], 1); 
  EnRn = bitRead(ModControlo[Idx], 0);
  DFC= CarryOut;
  DFZ= Zero;  
  // se pc0_1 der mal faco = JMP
  PC0_2= JNZ && !QFZ || JC && QFC; 
  PC0_1= !JMP;
  
  RegistoRn(D4);
  
  DRn= QRA;
  
  QRn= MUX_2x1(D4, QR0, QR1);
  
  if(!WR){
    MemDados[QRn] = QRA;
  }
  if(!RD){
    DB_MemDados = MemDados[QRn];
  }
  
  byte Y5 = ALU((D2 << 2 | D1 << 1 | D0), QRn, QRA, QFC);
  
  DRA= MUX_4x1(SA1, SA0, QRn, Y5, DB_MemDados, MemCodigo[QPC] & 0xFF);
  
  byte rel5 = MemCodigo[QPC] & 0x1F;
  if (bitRead(rel5, 4)) rel5 |= 0xE0;
   
  DPC= MUX_2x1(PC0_1, MemCodigo[QPC] & 0x3F, soma(MUX_2x1(PC0_2, 1, (rel5) | ((MemCodigo[QPC] & 0x10) << 1 )), QPC));
   
}

void loop(){
  calcVarComb();
}

void PrintRegistos(){
  Serial.println ("Registo A: " + (String) QRA + "; ");
  Serial.println ("Program Counter: " + (String) QPC + "; ");
  Serial.println ("Registo 0: " + (String) QR0 + "; ");
  Serial.println ("Registo 1: " + (String) QR1 + "; ");
  Serial.println ("Carry: " + (String) CarryOut  + "; ");
  Serial.println ("Zero: " + (String) Zero  + "; ");
  Serial.println ("Constante 1: " + (String)MemDados[0] + "; ");
  Serial.println ("Constante 2: " + (String)MemDados[1] + "; ");
  Serial.println ("Resultado: " + (String)MemDados[2] + "; ");
}

void PrintSinais() {
  Serial.print   ("Address: ");
  Serial.print   (D8 << 5 | D7 << 4 | D6 << 3 | D2 << 2 | D1 << 1 | D0, HEX);
  Serial.println ("; ");
  Serial.print   ("Data: ");
  Serial.print   (ModControlo[D8 << 5 | D7 << 4 | D6 << 3 | D2 << 2 | D1 << 1 | D0], HEX);
  Serial.println ("; ");
  Serial.println ("Selector 1 do Registo A: " + (String)SA0 + "; ");
  Serial.println ("Selector 2 do Registo A: " + (String)SA0 + "; ");
  Serial.println ("Bit posição 5: " + (String)D4 + "; ");
  Serial.println ("Enable do Registo A: " + (String)EnA + "; ");
  Serial.println ("Enable do Registo Rn: " + (String)EnRn + "; ");
  Serial.println ("Enable da Flags Carry e Zero: " + (String)EnF + "; ");
  Serial.println ("Selector 1 do Program Counter: " + (String)PC0_1 + "; ");
  Serial.println ("Selector 2 do Program Counter: " + (String)PC0_2 + "; ");
  Serial.println ("Condição do Jump: " + (String)JMP + "; ");
  Serial.println ("Condição do Jump if Carry: " + (String)JC + "; ");
  Serial.println ("Condição do Jump if Not Zero: " + (String)JNZ + "; ");
  Serial.println ("-----------------------------------------------------");
}
