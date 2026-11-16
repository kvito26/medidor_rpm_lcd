#include <Arduino_FreeRTOS.h>
#include <semphr.h> //para la inclusion de semaforos con FreeRTOS
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//=====================================================
//DEFINICIONES
//****************************************************
//PINES
//Para el display 16x02
#define SCL 22
#define SDA 21
//Para el sensor de Efecto Hall
#define PULSO_HALL 2

//****************************************************
//CONSTANTES
#define NUM_IMANES 4
#define CANT_MUESTRAS 10 //cantidad de muestras


//=====================================================
//VARIABLES GLOBALES
unsigned long tiempoEntreImanes;
unsigned long tiempoImanAnterior = 0;
volatile int muestras[CANT_MUESTRAS] = {0};


//Objeto para el display LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

//=====================================================
//PROTOTIPO DE LAS FUNCIONES



//=====================================================
//FREERTOS

//****************************************************
//Handle de los Semaforos
SemaphoreHandle_t PulsoHallSemaphoreBin = NULL; //semaforo binario
SemaphoreHandle_t ArregloMuestrasMutex = NULL;


//****************************************************
//Handle de las Tareas
TaskHandle_t ProcesarPulsosHallTaskHandle = NULL;
TaskHandle_t ProcesamientoMuestrasTaskHandle = NULL;



//****************************************************
//Funciones de las Tareas
//Tarea 1. Procesamiento de los pulsos enviados desde la funcion de interrupcion
void ProcesarPulsosHallTask(void *parameter){
	int contador = 0;

	for (;;){
		//esperar el semaforo binario hasta el infito
		if (xSemaphoreTake(PulsoHallSemaphoreBin, portMAX_DELAY) == pdTRUE){
			unsigned long tiempoActual = millis();

			//guardar muestras en el arreglo
			if (contador < CANT_MUESTRAS){
				tiempoEntreImanes = tiempoActual - tiempoImanAnterior;
				Serial.println("\vPulso detectado");
				Serial.print("El tiempo entre imanes: ");
				Serial.println(tiempoEntreImanes);
				
				Serial.print("Contador: ");
				Serial.println(contador);

				if (xSemaphoreTake(ArregloMuestrasMutex, pdMS_TO_TICKS(150)) == pdTRUE){
					muestras[contador] = (int) tiempoEntreImanes;
					
					xSemaphoreGive(ArregloMuestrasMutex);
				}

				tiempoImanAnterior = tiempoActual;
				contador++;
			}
			else { //notificar a la tarea de procesamiento y reiniciar el contador
				xTaskNotifyGive(ProcesamientoMuestrasTaskHandle);
				contador = 0;
			}
		}

		vTaskDelay(5 / portTICK_PERIOD_MS);
	}
}

//Tarea 2. Procesamiento de las muestras
void ProcesamientoMuestrasTask(void *parameter){
	for (;;){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		Serial.println("\vHE RECIVIDO LA NOTIFICACION");
		int muestras_local[CANT_MUESTRAS] = {0};
		float suma = 0.0f, promedio;

		if (xSemaphoreTake(ArregloMuestrasMutex, pdMS_TO_TICKS(150)) == pdTRUE){
			for (int i = 0; i < CANT_MUESTRAS; i++){
				muestras_local[i] = muestras[i];
			}

			xSemaphoreGive(ArregloMuestrasMutex);
		}

		//procesamiento de datos
		for (int i = 0; i < CANT_MUESTRAS; i++){
			Serial.print("\tMuestra ");
			Serial.print(i);
			Serial.print(" :");
			Serial.println(muestras_local[i]);
			suma += muestras_local[i];
		}

		Serial.print("\v\tPromedio: ");
		Serial.println((float) suma / CANT_MUESTRAS);

		vTaskDelay(5 / portTICK_PERIOD_MS);
	}
}


//=====================================================
void setup() {
	Serial.begin(115200);

	//****************************************************
	//CONFIGURACION PARA LOS PINES
	//Para el sensor de Efecto Hall
	pinMode(PULSO_HALL, INPUT); //pull down

	//Para el LCD
	lcd.init();
	lcd.backlight();

	//****************************************************
	//FREERTOS: Creacion de los semaforos
	PulsoHallSemaphoreBin = xSemaphoreCreateBinary();
	ArregloMuestrasMutex = xSemaphoreCreateMutex();

	//****************************************************
	//CONFIGURACION LAS INTERRUPCIONES
	attachInterrupt(digitalPinToInterrupt(PULSO_HALL), notificarPulso, RISING);

	//****************************************************
	//FREERTOS: Creacion de las tareas
	//Tarea 1. Procesamiento de los pulsos enviados desde la funcion de interrupcion
	xTaskCreate(
		ProcesarPulsosHallTask, //la funcion de la tarea
		"ProcesarPulsosHallTask",  //el nombre de la tarea
		128, //memoria en la pila
		NULL,
		2, //prioridad
		&ProcesarPulsosHallTaskHandle //el handle de la tarea
	);

	//Tarea 2. Procesamiento de las muestras
	xTaskCreate(
		ProcesamientoMuestrasTask, //la funcion de la tarea
		"ProcesamientoMuestrasTask",  //el nombre de la tarea
		128, //memoria en la pila
		NULL,
		1, //prioridad
		&ProcesamientoMuestrasTaskHandle//el handle de la tarea
	);
}

//=====================================================
void loop() {
}

//=====================================================
//FUNCIONES
//Funcion Callback para la interrupcion del sensor de Efecto Hall
void notificarPulso(){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	//enviando una segnal mediante el semaforo binario
	xSemaphoreGiveFromISR(PulsoHallSemaphoreBin, &xHigherPriorityTaskWoken);

	//solicitando cambio de contexto (opcional)
	if (xHigherPriorityTaskWoken){
		portYIELD_FROM_ISR();
	}
}


