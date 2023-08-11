#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27,16,2);

uint8_t pinesFilas[4] = {23,25,27,29};
uint8_t pinesColumnas[4] = {31,33,35,37};
char keys[4][4] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

Keypad keypad = Keypad(makeKeymap(keys), pinesFilas, pinesColumnas, (const uint8_t)4, (const uint8_t)4);

const int cooldown = 250;
unsigned long cooldownFinished = 0;

const String filas = "ABCD";

struct nodoBarco {
  int columna;
  int fila;
};

struct posicion {
  nodoBarco casilla[3] = {{-1,-1},{-1,-1},{-1,-1}};
  String orientacion;
};

byte popaHoriz[] = {
  B00000,
  B00011,
  B01100,
  B10001,
  B10001,
  B01100,
  B00011,
  B00000
};

byte cuerpoHoriz[] = {
  B11111,
  B00000,
  B00010,
  B10001,
  B10001,
  B00010,
  B00000,
  B11111
};

byte proaHoriz[] = {
  B10000,
  B01000,
  B00110,
  B00011,
  B00011,
  B00110,
  B01000,
  B10000
};

byte popaVer[] = {
  B00000,
  B00000,
  B00100,
  B01010,
  B10001,
  B10101,
  B10001,
  B10101
};

byte cuerpoVer[] = {
  B10001,
  B10101,
  B10001,
  B11011,
  B10101,
  B11011,
  B10001,
  B10101
};

byte proaVer[] = {
  B10001,
  B10101,
  B10001,
  B10101,
  B11011,
  B01010,
  B00100,
  B00100
};

byte mar[] = {
  B01000,
  B10101,
  B00010,
  B01000,
  B10101,
  B00010,
  B01000,
  B10101
};

byte cursorBarcos[] = {
  B00000,
  B00000,
  B10001,
  B01110,
  B01010,
  B01110,
  B10001,
  B00000
};

byte direcciones[] = {
  B00100,
  B01010,
  B00000,
  B10001,
  B10001,
  B00000,
  B01010,
  B00100
};

byte misilPlantado[] = {
  B00000,
  B11011,
  B10001,
  B00100,
  B00100,
  B10001,
  B11011,
  B00000
};

posicion posicionesA[10], posicionesB[10];
int barcosA[10][4], barcosB[10][4];
boolean misilesA[10][4], misilesB[10][4];
boolean esPrimeraRonda;
int puntajeA, puntajeB, rondasA, rondasB;
int buzzerPin = 11;
int cantidadBarcos;
String eco;
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);
  lcd.init();
  lcd.backlight();
  for(int i = 0; i < 4; i++)
  {
    pinMode(pinesFilas[i], INPUT);
  }
  for(int i = 0; i < 4; i++)
  {
    pinMode(pinesColumnas[i], INPUT);
  }

  vaciarBarcos(barcosB);
  esPrimeraRonda=true;
  /*for(int i = 0; i < 10; i++)
  {
    Serial.print(barcosB[i][0]);Serial.print(',');
  }
  Serial.println(';');*/
   pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  puntajeA = 0;
  puntajeB = 0;

  pantallaInicioRonda(rondasA, rondasB);
  delay(2000);
  lcd.clear();
  vaciarBarcos(barcosA);
  vaciarMisiles(misilesA);
  vaciarPosiciones(posicionesA);

  cantidadBarcos = escogerCantidadBarcos();
  lcd.clear();

  int tamanos[cantidadBarcos];
  llenarTamanos(cantidadBarcos, tamanos);
  setCaracteresBarcos();
  inputMatrizBarcosA(tamanos, cantidadBarcos);
  
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("Barcos");
  lcd.setCursor(2,1); 
  lcd.print("Colocados");
  delay(2000);
  lcd.clear();

  setCaracteresMisiles();
  inputMatrizMisilesA(cantidadBarcos);

  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Misiles");
  lcd.setCursor(2,1); 
  lcd.print("Colocados");
  delay(2000);
  lcd.clear();


  //Ciclo de espera para sincronizar con la app de Java 

  if(esPrimeraRonda)
  {
    eco = esperarComunicacion();
  }
  //eco = Serial.readString();
  
  convertirMatrizInt(eco, barcosB);
  Serial.end();
  Serial.begin(115200);
  convertirMatrizBool(eco, misilesB);
  convertirPosiciones(eco, posicionesB);
  //imprimirVentanaMatrizBarcos(lcd, barcosA, 0);

  puntajeA = calcularPuntuacion(barcosB, posicionesB, misilesA);
  puntajeB = calcularPuntuacion(barcosA, posicionesA, misilesB);
  mostrarPuntuacion(puntajeA, puntajeB);
  delay(2000);
  if(puntajeA > puntajeB)
  {
    rondasA++;
  } else if(puntajeA < puntajeB)
  {
    rondasB++;
  }

  //enviarScore(eco, puntajeA, puntajeB);
  
  //Funcion para calcular la cantidad de rondas ganadas por cada jugador para determinar el fin de la partida
  if(rondasA >= 2)
  {
    //Reiniciamos las rondas y damos mensaje correspondiente
    rondasA = 0;
    rondasB = 0;

    lcd.clear();
    lcd.setCursor(4,0); lcd.print("Ganador");
    lcd.setCursor(3,1); lcd.print("Jugador A");
    VictoryMelody();
    delay(2000);

  }else if(rondasB >= 2)
  {
    //Reiniciamos las rondas y damos mensaje correspondiente
    rondasA = 0;
    rondasB = 0;

    lcd.clear();
    lcd.setCursor(4,0); lcd.print("Ganador");
    lcd.setCursor(3,1); lcd.print("Jugador B");
    DefeatMelody();
    delay(2000);
  }

  esPrimeraRonda = false;
  //to-do festejos y otras animaciones de victoria/derrota + comunicación de los eventos finales a la app de Java
}

void pantallaEspera()
{
  lcd.setCursor(1,0);
  lcd.print("Esperando al");
  lcd.setCursor(2,1);
  lcd.print("Jugador 2");
}

void puntosEspera(int i)
{
  lcd.setCursor(11 + i, 1);
  if(i == 0)
  {
    lcd.print(".  ");
  }else 
  {
    lcd.print(".");
  }
}

void pantallaInicioRonda(int rondasA, int rondasB)
{
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Ronda ");
  lcd.setCursor(10, 0);
  lcd.print(rondasA + rondasB + 1);

  lcd.setCursor(5,1); lcd.print(rondasA);
  lcd.setCursor(7, 1); lcd.print("-");
  lcd.setCursor(9, 1); lcd.print(rondasB);
}

boolean calcularHit(int x, int y, int matrizBarcos[10][4], boolean matrizMisiles[10][4])
{
  if((matrizMisiles[x][y] == true) && ((matrizBarcos[x][y] >= 1) && (matrizBarcos[x][y] <= 3)))
  {
    return true;
  } else 
  {
    return false;
  }
}

int hundirBarco(int x, int y, int matrizBarcos[10][4], posicion posiciones[10])
{
  int tamanoBarco = 0;
  for(int i = 0; i < 10; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      if((posiciones[i].casilla[j].columna == x) && (posiciones[i].casilla[j].fila == y)) //Conseguimos al barco que le pegó el misil
      {
        for(int k = 0; k < 3; k++) //Hundir todo el barco
        {
          if(posiciones[i].casilla[k].columna != -1) //Casillas en -1 indican fin del barco
          {
            matrizBarcos[posiciones[i].casilla[k].columna][posiciones[i].casilla[k].fila] = 0;
            tamanoBarco++; //Guardamos el tamaño del barco para la puntuación
          }
        }
        
        return tamanoBarco; //Retornamos el tamaño para calcular la puntuación
      }
    }
  }
}

int calcularPuntuacion(int matrizBarcos[10][4], posicion posiciones[10], boolean matrizMisiles[10][4])
{
  int puntuacion = 0;
  int tamano;
  for(int i = 0; i < 10; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      if(calcularHit(i,j,matrizBarcos, matrizMisiles))
      {
        //Serial.print("Hit: "); Serial.print(i); Serial.print(","); Serial.println(j);
        tamano = hundirBarco(i,j, matrizBarcos, posiciones);

        puntuacion += 20 - 5*tamano; //Fórmula que resulta en diferencias de puntaje para cada barco
                                     //1 casilla = 15pts | 2 casillas = 10pts | 3 casillas = 5pts
        
      }
    }
  }
  return puntuacion;
}

void mostrarPuntuacion(int scoreA, int scoreB)
{
  lcd.clear();
  lcd.setCursor(3,0); lcd.print("Puntuacion");
  lcd.setCursor(4,1); lcd.print(scoreA);
  lcd.setCursor(7, 1); lcd.print("-");
  lcd.setCursor(9, 1); lcd.print(scoreB);
}

int escogerCantidadBarcos()
{
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Cantidad Barcos"); 
  //lcd.setCursor;

  while(true)
  {
    char key = keypad.getKey();
    
    if((key == '7'))
    {
      return 7;
    } 
    if((key == '8'))
    {
      return 8;
    }
    if((key == '9'))
    {
      return 9;
    }
    if (key == 'A')
    {
      return 10;
    }
  }
}

void llenarTamanos(int cantidadBarcos, int tamanos[])
{
  for(int i = 0; i < cantidadBarcos; i++)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tamano B"); lcd.print(i + 1);

    boolean input = false;
    while(!input)
    {
      char key = keypad.getKey();
      
      if((millis() > cooldownFinished) && (key == '1'))
      {
        cooldownFinished = millis() + cooldown;
        tamanos[i] = 1; 
        input = true;
      } else if((millis() > cooldownFinished) && (key == '2'))
      {
        cooldownFinished = millis() + cooldown;
        tamanos[i] = 2; 
        input = true;
      } else if((millis() > cooldownFinished) && (key == '3'))
      {
        cooldownFinished = millis() + cooldown;
        tamanos[i] = 3; 
        input = true;
      }
    }
  }
}

void setCaracteresBarcos()
{
  lcd.createChar(1, popaHoriz);
  lcd.createChar(2, cuerpoHoriz);
  lcd.createChar(3, proaHoriz);
  lcd.createChar(4, popaVer);
  lcd.createChar(5, cuerpoVer);
  lcd.createChar(6, proaVer);
  lcd.createChar(7, mar);
}

void setCaracteresMisiles()
{
  lcd.createChar(1, misilPlantado);
  lcd.createChar(7, mar);
}

void vaciarBarcos(int matrizBarcos[10][4]){
  for (int i = 0; i < 10; i++){
    for (int j = 0; j < 4; j++){
      matrizBarcos[i][j] = 0;
    }
  }
}

void vaciarMisiles(boolean matrizMisiles[10][4]){
  for (int i = 0; i < 10; i++){
    for (int j = 0; j < 4; j++){
      matrizMisiles[i][j] = false;
    }
  }
}

void vaciarPosiciones(posicion posiciones[10])
{
  for(int i = 0; i < 10; i++)
  {
    posiciones[i].orientacion = "";
    for(int j = 0; j < 3; j++)
    {
      posiciones[i].casilla[j] = {-1, -1};
    }
  }
}

void llenarMatriz(int matrizBarcos[10][4])
{
  for (int i = 0; i < 10; i++){
    for (int j = 0; j < 4; j++){
      matrizBarcos[i][j] = 1;
    }
  }
}

void buscarCaracter(int x, int y)
{
  //Serial.print("Llamada para: "); Serial.print(x);Serial.print(",");Serial.println(y);
  for(int i = 0; i < 10; i++)
  {
    if(posicionesA[i].orientacion != "")
    {
      for(int j = 0; j < 3; j++)
      {
        //Serial.print(posicionesA[i].casilla[j].columna); Serial.print(","); Serial.println(posicionesA[i].casilla[j].fila);
        if((posicionesA[i].casilla[j].columna == x) && (posicionesA[i].casilla[j].fila == y))
        {
          int valorCasilla = barcosA[x][y];
          if ((posicionesA[i].orientacion == "ARR") || (posicionesA[i].orientacion == "ABA"))
          {
            //lcd.setCursor(x,y);
            lcd.write(valorCasilla + 3);
          } else if((posicionesA[i].orientacion == "IZQ") || (posicionesA[i].orientacion == "DER"))
          {
            //lcd.setCursor(x,y);
            lcd.write(valorCasilla);
            
          } 
        }
      }
    }
  }
}

void imprimirVentanaMatrizBarcos(LiquidCrystal_I2C lcd, int matrizBarcos[10][4] ,int filaIni)
{
  lcd.home();
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      if(matrizBarcos[i][filaIni + j] == 0)
      {
        lcd.setCursor(i, j);
        lcd.write(7); //Caracter de mar
      } else //Caracter para barco, se escoge la orientacion 
      {
        lcd.setCursor(i,j);
        buscarCaracter(i,filaIni + j);
      }
    }
  }
}

void imprimirDatosBarcos(int barcosRestantes, int tamano)
{
  lcd.setCursor(12, 0);
  lcd.print("B:" + String(barcosRestantes));
  lcd.setCursor(12, 1);
  lcd.print("T:" + String(tamano));
}

void imprimirFilas(int filaIni)
{
  lcd.setCursor(10, 0);
  lcd.print(filas.charAt(filaIni));
  lcd.setCursor(10,1);
  lcd.print(filas.charAt(filaIni + 1));
}

void imprimirCursor(int x, int y, byte caracter)
{
  lcd.createChar(0, caracter);
  lcd.setCursor(x,y);
  lcd.write(0);
}

void inputMatrizBarcosA(int tamanosBarcos[], int cantidadBarcos)
{
  int filaIni = 0;
  int cursX = 0;
  int cursY = 0;
  char key;

  int barcosColocados = 0;
  imprimirVentanaMatrizBarcos(lcd, barcosA, filaIni);
  imprimirDatosBarcos(cantidadBarcos, tamanosBarcos[0]);
  imprimirFilas(filaIni);
  lcd.setCursor(cursX, cursY - filaIni);
  lcd.blink();
  while(barcosColocados < cantidadBarcos)
  {
    key = keypad.getKey();
    if((millis() > cooldownFinished) && (key == '2'))
    {
      cooldownFinished = millis() + cooldown;
      cursY--;
      if(cursY <= 1)
      {
        filaIni = constrain(filaIni - 1, 0, 2);//Scroll hacia arriba
        imprimirVentanaMatrizBarcos(lcd, barcosA, filaIni);
        imprimirDatosBarcos(cantidadBarcos - barcosColocados, tamanosBarcos[barcosColocados]);
        imprimirFilas(filaIni);
      }
      cursY = constrain(cursY, 0, 3);
      lcd.setCursor(cursX, cursY - filaIni);
      lcd.blink();
    }

    if((millis() > cooldownFinished) && (key == '8'))
    {
      cooldownFinished = millis() + cooldown;
      cursY++;
      if(cursY >= 2)
      {
        filaIni = constrain(filaIni + 1, 0, 2); //Scroll hacia abajo
        imprimirVentanaMatrizBarcos(lcd, barcosA, filaIni);
        imprimirDatosBarcos(cantidadBarcos - barcosColocados, tamanosBarcos[barcosColocados]);
        imprimirFilas(filaIni);
      }
      cursY = constrain(cursY, 0, 3);
      lcd.setCursor(cursX, cursY - filaIni);
      lcd.blink();
    }

    if((millis() > cooldownFinished) && (key == '4'))
    {
      cooldownFinished = millis() + cooldown;
      cursX--;
      cursX = constrain(cursX, 0, 9);
      lcd.setCursor(cursX, cursY - filaIni);
      lcd.blink();
    }

    if((millis() > cooldownFinished) && (key == '6'))
    {
      cooldownFinished = millis() + cooldown;
      cursX++;
      cursX = constrain(cursX, 0, 9);
      lcd.setCursor(cursX, cursY - filaIni);
      lcd.blink();
    }
    
    if((millis() > cooldownFinished) && (key == '5'))
    {
      cooldownFinished = millis() + cooldown;
      int puntoA[2];
      puntoA[0] = cursX;
      puntoA[1] = cursY;
      
      posicionesA[barcosColocados] = llenarPosiciones(puntoA, tamanosBarcos[barcosColocados]);
      /*for(int i = 0; i < tamanosBarcos[barcosColocados]; i++) //Debug
      {
        //Serial.print("(");Serial.print(posicionesA[barcosColocados].casilla[i].columna); Serial.print(",");
        //Serial.print(posicionesA[barcosColocados].casilla[i].fila); Serial.println(")");
        //Serial.println(barcosA[posicionesA[barcosColocados].casilla[i].columna][posicionesA[barcosColocados].casilla[i].fila]);
      }*/
      if(posicionesA[barcosColocados].casilla[0].columna >= 0)
      {
        barcosColocados++;
      }
      imprimirVentanaMatrizBarcos(lcd, barcosA, filaIni);
      imprimirDatosBarcos(cantidadBarcos - barcosColocados, tamanosBarcos[barcosColocados]);
      imprimirFilas(filaIni);
      lcd.setCursor(cursX, cursY - filaIni);
      lcd.blink();
      
    }
    //Serial.print("(");Serial.print(cursX);Serial.print(",");Serial.print(cursY);Serial.println(")");
  }
}

posicion llenarPosiciones(int puntoA[], int longitud)
{
  bool seleccion = false;
  posicion posicionBarco;
  char key;
  while(!seleccion)
  {
    key = keypad.getKey();
    if((millis() > cooldownFinished) && (key == '2'))
    {
      cooldownFinished = millis() + cooldown;
      for(int i = 0; i < longitud; i++)
      {
        if((puntoA[1] - i >= 0) && (barcosA[puntoA[0]][puntoA[1] - i] == 0))
        {
          posicionBarco.casilla[i].columna = puntoA[0];
          posicionBarco.casilla[i].fila = puntoA[1] - i;
          posicionBarco.orientacion = "ARR";
        } else 
        {
          posicionBarco.casilla[0] = {-1,-1}; 
          return posicionBarco;
        }
      }
      seleccion = true;
    }

    if((millis() > cooldownFinished) && (key == '8'))
    {
      cooldownFinished = millis() + cooldown;
      for(int i = 0; i < longitud; i++)
      {
        if((puntoA[1] + i <= 3) && (barcosA[puntoA[0]][puntoA[1] + i] == 0))
        {
          posicionBarco.casilla[i].columna = puntoA[0];
          posicionBarco.casilla[i].fila = puntoA[1] + i;
          posicionBarco.orientacion = "ABA";
        } else 
        {
          posicionBarco.casilla[0] = {-1,-1}; 
          return posicionBarco;
        }
      }
      seleccion = true;
    }

    if((millis() > cooldownFinished) && (key == '4'))
    {
      cooldownFinished = millis() + cooldown;
      for(int i = 0; i < longitud; i++)
      {
        if((puntoA[0] - i >= 0) && (barcosA[puntoA[0] - i][puntoA[1]] == 0))
        {
          posicionBarco.casilla[i].columna = puntoA[0] - i;
          posicionBarco.casilla[i].fila = puntoA[1];
          posicionBarco.orientacion = "IZQ";
        } else 
        {
          posicionBarco.casilla[0] = {-1,-1}; 
          return posicionBarco;
        }
      }
      seleccion = true;
    }

    if((millis() > cooldownFinished) && (key == '6'))
    {
      cooldownFinished = millis() + cooldown;
      for(int i = 0; i < longitud; i++)
      {
        if((puntoA[0] + i <= 9) && (barcosA[puntoA[0] + i][puntoA[1]] == 0))
        {
          posicionBarco.casilla[i].columna = puntoA[0] + i;
          posicionBarco.casilla[i].fila = puntoA[1];
          posicionBarco.orientacion = "DER";
        } else 
        {
          posicionBarco.casilla[0] = {-1,-1}; 
          return posicionBarco;
        }
      }
      seleccion = true;
    }
  }

  if((posicionBarco.orientacion == "DER") || (posicionBarco.orientacion == "ABA"))
  {
    for(int i = 0; i < longitud; i++)
    {
      if((longitud == 2) && (i == 1))
      {
        barcosA[posicionBarco.casilla[i].columna][posicionBarco.casilla[i].fila] = 3;
      } else
      {
        //Serial.println(i);
        barcosA[posicionBarco.casilla[i].columna][posicionBarco.casilla[i].fila] = i + 1;
      }
    }
  } else if ((posicionBarco.orientacion == "IZQ") || (posicionBarco.orientacion == "ARR"))
  {
    int j = longitud;
    for(int i = 0; i < longitud; i++)
    {
      if((longitud == 2) && (i == 0))
      {
        barcosA[posicionBarco.casilla[i].columna][posicionBarco.casilla[i].fila] = 3;
      } else
      {
        barcosA[posicionBarco.casilla[i].columna][posicionBarco.casilla[i].fila] = j;
      }
      j--;
    }
  }

  return posicionBarco;
}

void inputMatrizMisilesA(int cantidadMisiles)
{
  int filaIni = 0;
  int cursX = 0;
  int cursY = 0;
  char key;

  int misilesColocados = 0;
  imprimirVentanaMatrizMisiles(misilesA, filaIni);
  imprimirDatosMisiles(cantidadMisiles);
  imprimirFilas(filaIni);
  lcd.setCursor(cursX, cursY - filaIni);
  lcd.blink();
  while(misilesColocados < cantidadMisiles)
  {
    key = keypad.getKey();
    if((millis() > cooldownFinished) && (key == '2'))
    {
      cooldownFinished = millis() + cooldown;
      cursY--;
      if(cursY <= 1)
      {
        filaIni = constrain(filaIni - 1, 0, 2);//Scroll hacia arriba
        imprimirVentanaMatrizMisiles(misilesA, filaIni);
        imprimirDatosMisiles(cantidadMisiles - misilesColocados);
        imprimirFilas(filaIni);
      }
      cursY = constrain(cursY, 0, 3);
      lcd.setCursor(cursX, cursY - filaIni);
      lcd.blink();
    }

    if((millis() > cooldownFinished) && (key == '8'))
    {
      cooldownFinished = millis() + cooldown;
      cursY++;
      if(cursY >= 2)
      {
        filaIni = constrain(filaIni + 1, 0, 2); //Scroll hacia abajo
        imprimirVentanaMatrizMisiles(misilesA, filaIni);
        imprimirDatosMisiles(cantidadMisiles - misilesColocados);
        imprimirFilas(filaIni);
      }
      cursY = constrain(cursY, 0, 3);
      lcd.setCursor(cursX, cursY - filaIni);
      lcd.blink();
    }

    if((millis() > cooldownFinished) && (key == '4'))
    {
      cooldownFinished = millis() + cooldown;
      cursX--;
      cursX = constrain(cursX, 0, 9);
      lcd.setCursor(cursX, cursY - filaIni);
      lcd.blink();
    }

    if((millis() > cooldownFinished) && (key == '6'))
    {
      cooldownFinished = millis() + cooldown;
      cursX++;
      cursX = constrain(cursX, 0, 9);
      lcd.setCursor(cursX, cursY - filaIni);
      lcd.blink();
    }
    
    if((millis() > cooldownFinished) && (key == '5'))
    {
      cooldownFinished = millis() + cooldown;
      
      if(!misilesA[cursX][cursY])
      {
        misilesA[cursX][cursY] = true;
        misilesColocados++;
      } else
      {
        misilesA[cursX][cursY] = false;
        misilesColocados--;
      }

      imprimirVentanaMatrizMisiles(misilesA, filaIni);
      imprimirDatosMisiles(cantidadMisiles - misilesColocados);
      imprimirFilas(filaIni);
      lcd.setCursor(cursX, cursY - filaIni);
      lcd.blink();
      
    }
    //Serial.print("(");Serial.print(cursX);Serial.print(",");Serial.print(cursY);Serial.println(")");
  }
}

void imprimirVentanaMatrizMisiles(bool matrizMisiles[10][4] ,int filaIni)
{
  lcd.home();
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      lcd.setCursor(i, j);
      if(matrizMisiles[i][filaIni + j])
      {
        lcd.write(1); //Caracter de misil colocado
      } else 
      {
        lcd.write(7);//Caracter de mar
      }
    }
  }
}

void imprimirDatosMisiles(int misilesRestantes)
{
  lcd.setCursor(12, 0);
  lcd.print("M:" + String(misilesRestantes));
}

void convertirMatrizInt(String eco, int matrizTarget[10][4])
{
  int recorrido;
  int i = 0;
  String datos;
  String caracter;
  vaciarBarcos(matrizTarget);
  for(int j = 0; j < 4; j++)
  {
    recorrido = 0;
    Serial.print(eco);
    datos = esperarComunicacion();
    //datos = Serial.readString();
    caracter = (String)datos.charAt(recorrido);
    while(caracter != ";")
    {
      i = 0;
      if((caracter != ",") && (caracter != ";"))
      {
        matrizTarget[i][j] = caracter.toInt();
        //Serial.print(matrizTarget[i][j]);
        i++;
      }
      recorrido++;
      caracter = (String)datos.charAt(recorrido);
    }
  }
  //Serial.println();
}

void convertirMatrizBool(String eco, boolean matrizTarget[10][4])
{
  int recorrido;
  int i = 0;
  String datos;
  String caracter;
  for(int j = 0; j < 4; j++)
  {
    recorrido = 0;
    Serial.print(eco);
    Serial.flush();
    lcd.setCursor(15,0); lcd.print(eco);
    datos = esperarComunicacion();
    //datos = Serial.readString();
    caracter = (String)datos.charAt(recorrido);
    while(caracter != ";")
    {
      i = 0;
      if((caracter != ",") && (caracter != ";"))
      {
        if(caracter == "0")
        {
          matrizTarget[i][j] = false;
        } else 
        {
          matrizTarget[i][j] = true;
        }
        Serial.print(matrizTarget[i][j]);
        i++;
      }
      recorrido++;
      caracter = (String)datos.charAt(recorrido);
    }
  }
  //Serial.println();
}

void convertirPosiciones(String eco, posicion posicionesTarget[10])
{
  int recorrido = 0;
  int i = 0;
  int j;
  String datos;
  String caracter;

  caracter = (String)datos.charAt(recorrido);
  do
  {
    recorrido = 0;
    Serial.print(eco);
    datos = esperarComunicacion();
    //datos = Serial.readString();
    caracter = (String)datos.charAt(recorrido);
    j = 0;
    while((caracter != "-") && (caracter != ";"))
    {
      
      if(caracter != "+")
      {
        posicionesTarget[i].casilla[j].columna = caracter.toInt();
        recorrido++;
        caracter = (String)datos.charAt(recorrido);
        posicionesTarget[i].casilla[j].fila = caracter.toInt();
        recorrido++;
        caracter = (String)datos.charAt(recorrido);
        //Serial.println(posicionesTarget[i].casilla[j].columna);//Debug
      } else 
      {
        j++;
        recorrido++;
        caracter = (String)datos.charAt(recorrido);
      }
      
    }
    i++;
  }while(caracter != ";");
  
}

void enviarScore(String eco, int scoreA, int scoreB)
{
  Serial.print(eco);
  esperarComunicacion();
  Serial.print(scoreA);
  Serial.print("-");
  Serial.print(scoreB);
}

String esperarComunicacion()
{
  int i = 0;
  String dato = "";
  pantallaEspera();

  /*while(!Serial.available())*/ 
  while(dato == "")
  {
    if(i >= 3)
    {
      i = 0;
    }
    puntosEspera(i);
    i++;
    dato = Serial.readString();
    delay(250);
  }
  return dato;
}
void music(int melody[],int noteDuration[],int c,int t){
  for (int i = 0; i < c; i++) {
    tone(buzzerPin, melody[i], noteDuration[i]);
    delay(noteDuration[i] * t);
  }
  noTone(buzzerPin);
  delay(1000);
}

void VictoryMelody(){
  int melody[] = {
  293, 329, 392, 369, 587, 587, 587,
  587, 659, 587, 739, 659, 587, 587,
  659, 784, 659, 739, 659, 587, 587,
  659, 587, 659, 587, 587, 659, 587,
  369, 369, 293, 329, 392, 369,
  587, 587, 587, 587, 659, 587, 739,
  659, 587, 587, 659, 784, 659, 739,
  659, 587, 587, 659, 587, 659, 587,
  369, 369, 293, 329, 392, 369
};

int noteDuration[] = {
  250, 250, 250, 250, 500, 250, 250,
  250, 250, 250, 250, 250, 250, 250,
  250, 250, 250, 250, 250, 250, 250,
  250, 250, 250, 250, 250, 250, 250,
  250, 250, 250, 250, 500, 500,
  250, 250, 250, 250, 250, 250, 250,
  250, 250, 250, 250, 250, 250, 250,
  250, 250, 250, 250, 250, 250, 250,
  250, 250, 250, 250, 500, 500
};
music(melody,noteDuration,61,0.9);
  
}
void GameMelody(){
  int melody[] = {
  784, 880, 988, 784, 784, 880, 988, 784, 988, 1047, 988, 880, 784, 784, 880, 988,
  784, 880, 988, 784, 784, 880, 988, 784, 988, 1047, 988, 880, 784, 784, 880, 988,
  784, 880, 988, 784, 784, 880, 988, 784, 988, 1047, 988, 880, 784, 784, 880, 988,
  784, 880, 988, 784, 784, 880, 988, 784, 988, 1047, 988, 880, 784, 784, 880, 988
};

int noteDuration[] = {
  300, 150, 150, 300, 150, 150, 300, 150, 150, 150, 150, 150, 300, 150, 150, 300,
  150, 150, 300, 150, 150, 150, 150, 150, 150, 150, 150, 300, 150, 150, 300, 150,
  150, 300, 150, 150, 150, 150, 150, 300, 150, 150, 300, 150, 150, 300, 150, 150,
  150, 150, 150, 300, 150, 150, 300, 150, 150, 150, 150, 150, 300, 150, 150, 300
};
music(melody,noteDuration,64,1);
}

void DefeatMelody(){
  int melody[] = {
  // Parte 1
  330, 330, 330, 392, 330, 494, 392,
  330, 294, 262, 262, 294, 330, 330, 330,
  392, 330, 523, 494, 392, 330, 294,
  262, 294, 330, 294, 262,
  // Parte 2
  330, 330, 330, 392, 330, 494, 392,
  330, 294, 262, 262, 294, 330, 330, 330,
  392, 330, 523, 494, 392, 330, 294,
  262, 294, 330, 294, 262,
  // Parte 3
  220, 247, 262, 220, 294, 262, 330,
  330, 330, 349, 294, 262, 262, 294, 262,
  220, 247, 262, 220, 294, 330, 294,
  262, 294, 330, 330,
  // Parte 4
  349, 392, 523, 494, 440, 349,
  392, 330, 262, 294, 330, 262,
  220
};

int noteDuration[] = {
  // Parte 1
  250, 250, 250, 500, 500, 250, 375,
  125, 250, 250, 250, 250, 250, 250, 125,
  375, 125, 250, 250, 250, 250, 250,
  250, 250, 250, 250, 500,
  // Parte 2
  250, 250, 250, 500, 500, 250, 375,
  125, 250, 250, 250, 250, 250, 250, 125,
  375, 125, 250, 250, 250, 250, 250,
  250, 250, 250, 250, 500,
  // Parte 3
  250, 250, 250, 500, 500, 500, 375,
  125, 250, 250, 250, 250, 250, 250, 125,
  375, 125, 250, 250, 250, 250, 250,
  250, 250, 250, 500,
  // Parte 4
  250, 250, 250, 250, 125, 375,
  125, 250, 250, 250, 250, 250,
  500
};
music(melody,noteDuration,92,1.1);
}
