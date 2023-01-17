#include <Arduino.h>
#include <stdlib.h>
#include <Wire.h>

// ========================================================================================================
// --- Variáveis Globais ---
unsigned long tempoAtual; //tempo (em ms) desde o inicio do programa, obtido pela funcao millis 
unsigned long tempoAnterior; //tempo (em ms) desde o inicio do programa ate o loop anterior
const float taxaDeExibicao = 1000; // intervalo de tempo (em microsegundos) entre a exibicao de um valor de velocidade e outro
volatile byte leituras; //numero de leituras efetuadas pelo sensor
unsigned long tempoLeituraAtual;
unsigned long tempoLeituraAnterior;
volatile unsigned long intervaloMaximoLeitura;
volatile unsigned long intervaloMinimoLeitura;
bool resetVelocidade;
const float circunferenciaRoda = 1.596; // Roda do DT1
const int velocidadeMaxima = 35; // velocidade maxima do carro em km/h
unsigned int leiturasPorVolta = 3; //imas ou medicoes do encoder por revolucao
const int amostras = 20; //quantidade de amostras para calcular periodo medio 
                           //Padrao:12
unsigned int indiceDeposito;                                                                                                                                                                                                      
unsigned int indiceRetirada; 
volatile int somaPeriodos;
float velocidadeKm;               // Velocidade em km/h
float velocidadeMaxima_MetroPorSegundo;
volatile unsigned long periodoMedio;
unsigned long historicoLeituras[amostras];

// ========================================================================================================
// --- Interrupção ---
void contador()
{
  tempoLeituraAtual = micros(); 
  if((tempoLeituraAtual - tempoLeituraAnterior) > intervaloMinimoLeitura){ //evita o ruido
    detachInterrupt(digitalPinToInterrupt(2));
    resetVelocidade = 0;
    leituras++; //Incrementa o contador de leituras do sensor
    historicoLeituras[indiceDeposito] = tempoLeituraAtual - tempoLeituraAnterior;
    indiceDeposito++;
    tempoLeituraAnterior = tempoLeituraAtual;
    if (indiceDeposito >= amostras)  // If we're at the end of the array:
      {
        indiceDeposito = 0;  // Reset array index.
      }
    attachInterrupt(digitalPinToInterrupt(2), contador, RISING);
  }
}

// ========================================================================================================
// --- Configurações Iniciais ---
void setup()
{
  Serial.begin(115200);                                        // Begin serial communication.
  //Define o pino 2 como entrada. 
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), contador, RISING); // Enable interruption pin 2 when going from LOW to HIGH.
  delay(100);
  
  Serial.println(F("Setup done"));
  resetVelocidade = 1;
  leituras  = 0;
  indiceDeposito = 0;                                                                                                                                                                                                      
  indiceRetirada = 0; 
  tempoLeituraAnterior = 0;
  velocidadeMaxima_MetroPorSegundo = velocidadeMaxima / 3.6;
  intervaloMaximoLeitura = 1000*circunferenciaRoda/(leiturasPorVolta *0.2778); //em microsegundos
  intervaloMinimoLeitura = 1000*circunferenciaRoda/(leiturasPorVolta *velocidadeMaxima_MetroPorSegundo); //em microsegundos
  
} //end setup


// ========================================================================================================
// --- Loop Infinito ---
void loop()
{

  tempoAtual = millis();
  
  //Atualiza o valor de velocidade exibido
  if (tempoAtual - tempoAnterior >= taxaDeExibicao)
  {
    detachInterrupt(digitalPinToInterrupt(2)); //Desabilita interrupcao durante o calculo  
    tempoAnterior = tempoAtual;

    //se nao tiver registrado pulso em um determinado tempo a velocidade sera 0
    if((tempoAtual-tempoLeituraAnterior) > intervaloMaximoLeitura){
      resetVelocidade = 1;
      indiceRetirada = 0;
      indiceDeposito = 0;
      for(int i = 0; i<amostras; i++){
        historicoLeituras[i] = 0;
      }
    }
    
    if (resetVelocidade != 1){
      //determina quantos leituras do sensor serao necessarias para calcular a velocidade
      int RemapedLeiturasParaCalcularVelocidade = map(leituras, 0, 12, 3, 7);  // Remap the period range to the reading range.
      RemapedLeiturasParaCalcularVelocidade = constrain(RemapedLeiturasParaCalcularVelocidade, 3, 7);  // Constrain the value so it doesn't go below or above the limits.
      
      //indice retirada sera igual ao local do armazenamento do ultimo pulso
      if(indiceDeposito == 0){
        indiceRetirada = amostras - 1;
      }else{
        indiceRetirada = indiceDeposito -1;
      }
  
      //soma dos periodos dos pulsos necessarios para calcular a velocidade
      somaPeriodos = tempoAtual - tempoLeituraAnterior;
      for(int j=0; j<RemapedLeiturasParaCalcularVelocidade; j++){
        somaPeriodos = somaPeriodos + historicoLeituras[indiceRetirada];  // Add the reading to the total.
        if (indiceRetirada ==0)  // If we're at the end of the array:
        {
          indiceRetirada = amostras;  // Reset array index.
        }
        indiceRetirada = indiceRetirada - 1;  // Advance to the next position in the array.
      }

      //Calculo da velocidade em Km/h
      periodoMedio = somaPeriodos/ RemapedLeiturasParaCalcularVelocidade;
      velocidadeKm = 3.6*circunferenciaRoda*1000/(leiturasPorVolta*periodoMedio);
    }else velocidadeKm = 0;

    leituras = 0;

    Serial.print(F("indiceRetirada = "));
    Serial.println(indiceRetirada);
    Serial.print(F("velocidadeKm = "));
    Serial.println(velocidadeKm);
    Serial.print(F("indiceDeposito = "));
    Serial.println(indiceDeposito);
    Serial.print(F("interrupcao = "));
    Serial.println(digitalRead(2));
   
    //Habilita interrupcao
   attachInterrupt(digitalPinToInterrupt(2), contador, RISING);
  } 
} //end loop
