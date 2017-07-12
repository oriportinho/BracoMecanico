#include <VarSpeedServo.h>

// Flag de controle do debugger, se true ele vai imprimir todos so debugadores
bool flagDebugger = false;

// Controles de maximo e minimo de angulo
int motorAngMax0 =  90; // cintura     3
int motorAngMin0 =   0;
int motorAngMax1 = 130; // ombro      11
int motorAngMin1 =  60;
int motorAngMax2 = 150; // cotuvelo    9
int motorAngMin2 =  70;
int motorAngMax3 = 100; // pulso sobe  7
int motorAngMin3 =  10;
int motorAngMax4 = 150; // pulso gira  5
int motorAngMin4 =  10;
int motorAngMax5 = 140; // garra      13
int motorAngMin5 = 100;

// Instancia de motores
VarSpeedServo motor0; // cintura     3
VarSpeedServo motor1; // ombro      11
VarSpeedServo motor2; // cotuvelo    9
VarSpeedServo motor3; // pulso sobe  7
VarSpeedServo motor4; // pulso gira  5
VarSpeedServo motor5; // garra      13

int motorPin0 =  3;
int motorPin1 = 11;
int motorPin2 =  9;
int motorPin3 =  7;
int motorPin4 =  5;
int motorPin5 = 13;

// Velocidades de movimentacao
int rapido  = 254; // 0
int medio   = 175; // 1
int devagar =  70; // 2

// Tempo de espera para poder criar uma conexao serial
int temporizadorMax = 10000;
int temporizadorComando = 250;

int estadoSalvo [8][7];
int ponteiroEstado = 1;

void setup() {

  pinMode( 2, INPUT);

  // Inicializar servo motores
  motor0.attach(motorPin0);
  motor1.attach(motorPin1);
  motor2.attach(motorPin2);
  motor3.attach(motorPin3);
  motor4.attach(motorPin4);
  motor5.attach(motorPin5);

  // Iniciar comunicacao serial
  Serial.begin(9600);

  // Salvar as posicoes iniciais
  resetarPosicao();
  executarSalvo(0);
  digitalWrite( 2, HIGH);
}

void loop() {
//debugger("funcionando");
  if(Serial.available() > 0) {
    String dado = Serial.readString();
    debugger("lido|" + dado + "|porta Serial");

    if(dado.length() >= 4 && dado.charAt(1) == '0') {
      debugger("dado com tamanho valido e primeiro digito = 0");
      if(dado.charAt(2) == '0' || dado.charAt(2) == '1') {
        debugger("recebido 0 ou 1");
        Serial.println("#00$");
        Serial.println("#01$");

      } else if(dado.charAt(2) == '2') { // Mover para posicao inicial
        debugger("recebido 2");
        Serial.println("#00$");
        executarSalvo(0);
        Serial.println("#01$");

      } else if(dado.charAt(2) == '3') { // Mover para a posicao dada
        debugger("recebido 3");
        Serial.println("#00$");
        int velocidadeMotor = (dado.substring(4, 6)).toInt();
        int j = 0;
        for(int i = 7; i < 37; i+=5) {
          int val = (dado.substring(i, i+4)).toInt();
          debugger("#velocidade" + String(velocidadeMotor) + "$");
          movimentarMotorSerial(j, val, velocidadeMotor);
          j++;
        }
        Serial.println("#01$");

      } else if(dado.charAt(2) == '4') { // Salvar ultima posicao
        debugger("recebido 4");
        Serial.println("#00$");

        if(ponteiroEstado >= 6) {
          ponteiroEstado = 1;
        }
        for(int i =0; i <= 5; i++) {
          salvarMotor(ponteiroEstado, estadoSalvo[7][i], estadoSalvo[7][6], i);
        }
        ponteiroEstado++;

        Serial.println("#01$");

      } else if(dado.charAt(2) == '5') { // Salvar ultima posciao em
        debugger("recebido 5");
        Serial.println("#00$");
        int posicaoSalvar = (int) (dado.charAt(5) - '0');
        for(int i =0; i <= 5; i++) {
          salvarMotor(ponteiroEstado, estadoSalvo[posicaoSalvar][i], estadoSalvo[posicaoSalvar][6], i);
        }
        Serial.println("#01$");

      } else if(dado.charAt(2) == '6') { // Executar todas as posicoes salvas
        debugger("recebido 6");
        Serial.println("#00$");

        for(int i = 1; i <= 6; i++) {
          executarSalvo(i);
          delay(temporizadorComando);
        }

        Serial.println("#01$");

      } else if(dado.charAt(2) == '7') { // Executar em ordem as posicoes
        debugger("recebido 7");
        Serial.println("#00$");

        for(int i = 5; i < (int) dado.length(); i++) {

          if( dado.charAt(i) == '0' || dado.charAt(i) == '1' || dado.charAt(i) == '2' ||
          dado.charAt(i) == '3' || dado.charAt(i) == '4' || dado.charAt(i) == '5' ||
          dado.charAt(i) == '6' || dado.charAt(i) == '7') {

            int numeroId = (int) (dado.charAt(i) - '0');
            executarSalvo(numeroId);
            i++;
          } else {
            break;
          }
        }
        Serial.println("#01$");

      } else if(dado.charAt(2) == '8') {
        debugger("recebido 8");
        Serial.println("#00$");
        resetarPosicao();
        Serial.println("#01$");

      } else if(dado.charAt(2) == '9') {
        debugger("recebido 9");
        Serial.println("#00$");
        /*int velocidadeMotor = (dado.substring(4, 6)).toInt();
        int j = 0;
        for(int i = 7; i < 37; i+=5) {
          int val = (dado.substring(i, i+4)).toInt();
          debugger("#velocidade" + String(velocidadeMotor) + "$");
          movimentarMotorSerial(j, val, velocidadeMotor);
          j++;
        }*/
        Serial.println("#01$");

      } else {
        debugger("erro de numero invalido");
        erro(); // numero invalido
      }
    } else {
      debugger("erro de tamanho invalido");
      erro(); // tamanho invalido
    }
  }
delay(500);
}

// Salva a posicao de um motor e a velocidade
void salvarMotor(int posicao, int movimento, byte velocidade, byte motor) {
  //debugger("salvar motor " + motor + " com velocidade " + velocidade + "em posicao " + posicao + "");
  estadoSalvo[posicao][motor] = movimento;
  estadoSalvo[posicao][6] = velocidade;
}

// Metodo para executar um movimento salvo na memoria do robo
void executarSalvo(int posicao) {
  debugger("executar posicao " + posicao);
  movimentarMotorAngulo(0, estadoSalvo[posicao][0], estadoSalvo[posicao][6]);
  movimentarMotorAngulo(1, estadoSalvo[posicao][1], estadoSalvo[posicao][6]);
  movimentarMotorAngulo(2, estadoSalvo[posicao][2], estadoSalvo[posicao][6]);
  movimentarMotorAngulo(3, estadoSalvo[posicao][3], estadoSalvo[posicao][6]);
  movimentarMotorAngulo(4, estadoSalvo[posicao][4], estadoSalvo[posicao][6]);
  movimentarMotorAngulo(5, estadoSalvo[posicao][5], estadoSalvo[posicao][6]);
}

// Metodo de aviso de erro dentro do sistema usando a led
void erro() {
  Serial.println("#99$");

  for(int i = 0; i < 5; i++) {
    digitalWrite( 2, HIGH);
    delay(250);
    digitalWrite( 2,  LOW);
    delay(250);
  }

  digitalWrite( 2, HIGH);
}

// Metodo para controle de motor, numero do motor 0 a 5, movimento entre os limites de cada motor, velocidade entre 0 e 2
void movimentarMotorAngulo(int numeroServo, int val, int velocidade) {
  //debugger("movimentar motor " + numeroServo + " para  posicao " + val + " com velocidade de " + velocidade);
  estadoSalvo[7][numeroServo] = val;
  estadoSalvo[7][6] = velocidade;

  if(numeroServo == 0) {
    if(velocidade == 0) {
      motor0.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor0.write(val, medio, true);
    } else if(velocidade == 2) {
      motor0.write(val, devagar, true);
    } else {
      erro();
    }
  } else if(numeroServo == 1) {
    if(velocidade == 0) {
      motor1.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor1.write(val, medio, true);
    } else if(velocidade == 2) {
      motor1.write(val, devagar, true);
    } else {
      erro();
    }
  } else if(numeroServo == 2) {
    if(velocidade == 0) {
      motor2.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor2.write(val, medio, true);
    } else if(velocidade == 2) {
      motor2.write(val, devagar, true);
    } else {
      erro();
    }
  } else if(numeroServo == 3) {
    if(velocidade == 0) {
      motor3.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor3.write(val, medio, true);
    } else if(velocidade == 2) {
      motor3.write(val, devagar, true);
    } else {
      erro();
    }
  } else if(numeroServo == 4) {
    if(velocidade == 0) {
      motor4.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor4.write(val, medio, true);
    } else if(velocidade == 2) {
      motor4.write(val, devagar, true);
    } else {
      erro();
    }
  } else if(numeroServo == 5) {
    if(velocidade == 0) {
      motor5.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor5.write(val, medio, true);
    } else if(velocidade == 2) {
      motor5.write(val, devagar, true);
    } else {
      erro();
    }
  } else {
    erro();
  }
}

// Metodo para controle de motor, numero do motor 0 a 5, movimento entre 0 e 1023, velocidade entre 0 e 2
void movimentarMotorSerial(int numeroServo, int val, int velocidade) {
  //debugger("movimentar motor com serial " + numeroServo + " para  posicao " + val + " com velocidade de " + velocidade);
  if(numeroServo == 0) {
    val = map(val, 0, 1023, motorAngMin0, motorAngMax0);

    if(velocidade == 0) {
      motor0.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor0.write(val, medio, true);
    } else if(velocidade == 2) {
      motor0.write(val, devagar, true);
    } else {
      erro();
    }
    estadoSalvo[7][numeroServo] = val;
    estadoSalvo[7][6] = velocidade;

  } else if(numeroServo == 1) {
    val = map(val, 0, 1023, motorAngMin1, motorAngMax1);

    if(velocidade == 0) {
      motor1.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor1.write(val, medio, true);
    } else if(velocidade == 2) {
      motor1.write(val, devagar, true);
    } else {
      erro();
    }
    estadoSalvo[7][numeroServo] = val;
    estadoSalvo[7][6] = velocidade;

  } else if(numeroServo == 2) {
    val = map(val, 0, 1023, motorAngMin2, motorAngMax2);

    if(velocidade == 0) {
      motor2.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor2.write(val, medio, true);
    } else if(velocidade == 2) {
      motor2.write(val, devagar, true);
    } else {
      erro();
    }
    estadoSalvo[7][numeroServo] = val;
    estadoSalvo[7][6] = velocidade;

  } else if(numeroServo == 3) {
    val = map(val, 0, 1023, motorAngMin3, motorAngMax3);

    if(velocidade == 0) {
      motor3.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor3.write(val, medio, true);
    } else if(velocidade == 2) {
      motor3.write(val, devagar, true);
    } else {
      erro();
    }
    estadoSalvo[7][numeroServo] = val;
    estadoSalvo[7][6] = velocidade;

  } else if(numeroServo == 4) {
    val = map(val, 0, 1023, motorAngMin4, motorAngMax4);

    if(velocidade == 0) {
      motor4.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor4.write(val, medio, true);
    } else if(velocidade == 2) {
      motor4.write(val, devagar, true);
    } else {
      erro();
    }
    estadoSalvo[7][numeroServo] = val;
    estadoSalvo[7][6] = velocidade;

  } else if(numeroServo == 5) {
    val = map(val, 0, 1023, motorAngMin5, motorAngMax5);

    if(velocidade == 0) {
      motor5.write(val, rapido, true);
    } else if(velocidade == 1) {
      motor5.write(val, medio, true);
    } else if(velocidade == 2) {
      motor5.write(val, devagar, true);
    } else {
      erro();
    }
  } else {
    erro();
  }
  estadoSalvo[7][numeroServo] = val;
  estadoSalvo[7][6] = velocidade;

}

// Metodo para reiniciar todas as posicoes dos motores para a posicao inicial
void resetarPosicao() {
  int ini1 =  10;
  int ini2 =  95;
  int ini3 = 120;
  int ini4 =  50;
  int ini5 = 140;
  int ini6 = 110;
  int ini7 =   0;

  estadoSalvo[0][0] = ini1;
  estadoSalvo[0][1] = ini2;
  estadoSalvo[0][2] = ini3;
  estadoSalvo[0][3] = ini4;
  estadoSalvo[0][4] = ini5;
  estadoSalvo[0][5] = ini6;
  estadoSalvo[0][6] = ini7;

  estadoSalvo[1][0] = ini1;
  estadoSalvo[1][1] = ini2;
  estadoSalvo[1][2] = ini3;
  estadoSalvo[1][3] = ini4;
  estadoSalvo[1][4] = ini5;
  estadoSalvo[1][5] = ini6;
  estadoSalvo[1][6] = ini7;

  estadoSalvo[2][0] = ini1;
  estadoSalvo[2][1] = ini2;
  estadoSalvo[2][2] = ini3;
  estadoSalvo[2][3] = ini4;
  estadoSalvo[2][4] = ini5;
  estadoSalvo[2][5] = ini6;
  estadoSalvo[2][6] = ini7;

  estadoSalvo[3][0] = ini1;
  estadoSalvo[3][1] = ini2;
  estadoSalvo[3][2] = ini3;
  estadoSalvo[3][3] = ini4;
  estadoSalvo[3][4] = ini5;
  estadoSalvo[3][5] = ini6;
  estadoSalvo[3][6] = ini7;

  estadoSalvo[4][0] = ini1;
  estadoSalvo[4][1] = ini2;
  estadoSalvo[4][2] = ini3;
  estadoSalvo[4][3] = ini4;
  estadoSalvo[4][4] = ini5;
  estadoSalvo[4][5] = ini6;
  estadoSalvo[4][6] = ini7;

  estadoSalvo[5][0] = ini1;
  estadoSalvo[5][1] = ini2;
  estadoSalvo[5][2] = ini3;
  estadoSalvo[5][3] = ini4;
  estadoSalvo[5][4] = ini5;
  estadoSalvo[5][5] = ini6;
  estadoSalvo[5][6] = ini7;

  estadoSalvo[6][0] = ini1;
  estadoSalvo[6][1] = ini2;
  estadoSalvo[6][2] = ini3;
  estadoSalvo[6][3] = ini4;
  estadoSalvo[6][4] = ini5;
  estadoSalvo[6][5] = ini6;
  estadoSalvo[6][6] = ini7;

  estadoSalvo[7][0] = ini1;
  estadoSalvo[7][1] = ini2;
  estadoSalvo[7][2] = ini3;
  estadoSalvo[7][3] = ini4;
  estadoSalvo[7][4] = ini5;
  estadoSalvo[7][5] = ini6;
  estadoSalvo[7][6] = ini7;
}

// Metodo para imprimir na tela o sistema de debugg
void debugger(String bug) {
  if(flagDebugger)
    Serial.println(bug);
}
