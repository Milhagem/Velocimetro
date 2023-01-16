  O objetivo desse código é fazer a medição da velocidade da roda. Velocidade é a derivada da posição, assim, seu calculo é dado pela variação da posição 
em um determinado tempo. Para isso, é necessário adquirir o valor das posições na roda que é a distancia entre dois imãs, podendo ser feito pelo uso de um
sensor optico ou pelo sensor hall. No caso deste ultimo imãs são posicionados na extremidade da roda e o sensor é posicionado de modo a identificar os imãs
passando por ele quando a roda gira adquirindo assim o elemento “posiçao”.
  Como os imãs estao fixos na roda e igualmente distanciados entre um e outro, o elemento “distancia” é fixo e já conhecido, desse modo o que muda é o tempo
que leva para a roda girar, sendo este o principal parâmetro a ser considerado no cálculo da velocidade.
  O código funciona da seguinte forma, toda vez que o sensor identifica o imã é ativada uma interrupção que recebe como entrada o tempo desde a última interrupção
e armazena seu valor num array.
  O cálculo da velocidade é feito dividindo a distancia entre os sensores pela média ultimos tempos armazenados no array e seu valor é exibido a uma taxa a ser 
definida pelo usuário (taxaDeExibicao no código).
  O que diferencia esse codigo do codigo de um tacometro normal é que o tacometro possui muitas amostras por rotação enquando os nossos sensores tem a limitação
do numero de imãs na roda ou o numero de dentes na roda (para o caso do sensor óptico), fazendo com que o valor a baixas velocidades fique muito impreciso e com 
alta variação. Assim, a solução foi utilizar a media dos tempos anteriores em vez de apenas o ultimo tempo adquirido para o cálculo.
