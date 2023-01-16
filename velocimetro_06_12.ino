#include <Arduino.h>
#include <INA.h>
#include <U8glib.h>
#include <stdlib.h>
#include <Wire.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);
INA_Class INA;

// ========================================================================================================
// --- Variáveis Globais --- 
const float circunferenciaRoda = 1.596; // Roda do DT1
const float taxaDeExibicao = 1000; // intervalo de tempo (em microsegundos) entre a exibicao de um valor de velocidade e outro
const int velocidadeMaxima = 35; // velocidade maxima do carro em km/h
unsigned int pulsos_por_volta = 3; //imas ou medicoes do encoder por revolucao
const int amostras = 20; //quantidade de amostras para calcular periodo medio 
                           //Padrao:12

volatile unsigned long intervaloMaximoAmostragem;
volatile unsigned long intervaloMinimoAmostragem;

int           rpm, rpm_old = 1;
volatile byte pulsos;
volatile byte pulsosAux;

unsigned long timeold;
unsigned long timecontador;
unsigned long timecontadorold;
unsigned long tempoPulsoAtual;
unsigned long timenew;
const int zeroTimeout = 3000; // se passar desse tempo sem ter leitura de pulsos a velocidade eh igual a 0
unsigned int indiceDeposito;                                                                                                                                                                                                      
unsigned int indiceRetirada; 
volatile int somaPeriodos;
float velocidadeKm;               // Velocidade em km/h
float oldVelocidadeKm;               // Velocidade em km/h
float velocidadeMaxima_MetroPorSegundo;
volatile unsigned long periodoMedio;




//array numero de pulsos nos ultimos segundos
unsigned long historicoPulsos[amostras];
unsigned long historicoPeriodos[amostras];

// ========================================================================================================
// --- Interrupção ---
void contador()
{
  tempoPulsoAtual = micros(); 
  if((tempoPulsoAtual - timecontadorold)>intervaloMinimoAmostragem){ //evita o ruido
    //Incrementa contador
    pulsosAux++;
    pulsos++;
    timecontador = millis();
    historicoPeriodos[indiceDeposito] = timecontador - timecontadorold;
    indiceDeposito++;
    timecontadorold = timecontador;
    if (indiceDeposito >= amostras)  // If we're at the end of the array:
      {
        indiceDeposito = 0;  // Reset array index.
      }
    timecontadorold = timecontador;
  }
}

// ========================================================================================================
// --- Configurações Iniciais ---
void setup()
{
  Serial.begin(9600);                                             // Begin serial communication.
  attachInterrupt(digitalPinToInterrupt(2), contador, RISING); // Enable interruption pin 2 when going from LOW to HIGH.

  INA.begin(80, 1000);
  INA.setBusConversion(8500);
  INA.setShuntConversion(8500);
  INA.setAveraging(128);
  INA.alertOnConversion(true);
  INA.setMode(INA_MODE_CONTINUOUS_BOTH);
  INA.alertOnBusOverVoltage(true, 40000);

  u8g.begin();

  pulsosAux = 0;
  pulsos  = 0;
  rpm     = 0;
  timeold = 0;
  indiceDeposito = 0;                                                                                                                                                                                                      
  indiceRetirada = 0; 
  somaPeriodos = 0;
  velocidadeMaxima_MetroPorSegundo = velocidadeMaxima / 3.6;
  intervaloMaximoAmostragem = 1000*circunferenciaRoda/(pulsos_por_volta *0.2778); //em microsegundos
  intervaloMinimoAmostragem = 1000*circunferenciaRoda/(pulsos_por_volta *velocidadeMaxima_MetroPorSegundo); //em microsegundos
  
} //end setup


// ========================================================================================================
// --- Loop Infinito ---
void loop()
{
  //Atualiza contador a cada segundo
  if (millis() - timeold >= taxaDeExibicao)
  { 
    //Desabilita interrupcao durante o calculo
    detachInterrupt(digitalPinToInterrupt(2));

    timenew = millis();

    //determina quantos pulsos serao necessarios para calcular a velocidade
    int RemapedpulsosParaCalcularVelocidade = map(pulsos, 0, 12, 3, 7);  // Remap the period range to the reading range.
    RemapedpulsosParaCalcularVelocidade = constrain(RemapedpulsosParaCalcularVelocidade, 3, 7);  // Constrain the value so it doesn't go below or above the limits.
    
    //indice retirada sera igual ao local do armazenamento do ultimo pulso
    if(indiceDeposito == 0){
      indiceRetirada = amostras - 1;
    }else{
      indiceRetirada = indiceDeposito -1;
    }
	  
    //soma dos periodos dos pulsos necessarios para calcular a velocidade
    somaPeriodos = timenew - timecontador;
    for(int i=0; i<RemapedpulsosParaCalcularVelocidade; i++){
      somaPeriodos = somaPeriodos + historicoPulsos[indiceRetirada];  // Add the reading to the total.
      if (indiceRetirada ==0)  // If we're at the end of the array:
      {
        indiceRetirada = amostras;  // Reset array index.
      }
      indiceRetirada = indiceRetirada - 1;  // Advance to the next position in the array.
    }

    //se nao tiver registrado pulso em um determinado tempo a velocidade sera 0
    if((timenew-timecontador)>intervaloMaximoAmostragem){
      velocidadeKm = 0;
    }
    //caso contrario, sera feito a media dos periodos e o calculo da velocidade
    else{
      periodoMedio = somaPeriodos/ RemapedpulsosParaCalcularVelocidade;
      velocidadeKm = 3.6*circunferenciaRoda*1000/(pulsos_por_volta*periodoMedio);
    }

    /*//calculo de RPM
    pulsos = somaPeriodos/RemapedpulsosParaCalcularVelocidade;
    rpm = (60 * 1000 / (pulsos_por_volta) ) / (millis() - timeold) * pulsos;
    */
    
    Serial.print("RPM = ");
    Serial.println(rpm, DEC);

    //Mostra o valor de RPM no display
    if (velocidadeKm != oldVelocidadeKm) {
      u8g.firstPage();
      do {
        u8g.setFont(u8g_font_courR10);
        u8g.setPrintPos(0, 15);
        u8g.print(somaPeriodos);
        u8g.print("sum");
    
        u8g.setPrintPos(0, 37);
        u8g.print(pulsosAux);
        
        u8g.print(" PPCV");
        
        u8g.setPrintPos(0, 60);
        u8g.print(velocidadeKm);
        u8g.print("Vel_KM");
    
      } while ( u8g.nextPage() );
    
    }

   somaPeriodos = 0;
    oldVelocidadeKm = velocidadeKm;
   timeold = millis();
   pulsos = 0;
    //Habilita interrupcao
    attachInterrupt(digitalPinToInterrupt(2), contador, RISING);
  } 
} //end loop

