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



//Objeto para el display LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

//=====================================================
//PROTOTIPO DE LAS FUNCIONES



//=====================================================
//FREERTOS

//****************************************************
//Handle de los Semaforos
SemaphoreHandle_t PulsoHallSemaphoreBin = NULL; //semaforo binario


//****************************************************
//Handle de las Tareas
TaskHandle_t ProcesarPulsosHallTaskHandle = NULL;



//****************************************************
//Funciones de las Tareas
//Tarea 1. Procesamiento de los pulsos enviados desde la funcion de interrupcion
void ProcesarPulsosHallTask(void *parameter){
	for (;;){
		//esperar el semaforo binario hasta el infito
		if (xSemaphoreTake(PulsoHallSemaphoreBin, portMAX_DELAY) == pdTRUE){


		}

		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}


//=====================================================
void setup() {
	Serial.begin(9600);

	//****************************************************
	//CONFIGURACION PARA LOS PINES
	//Para el sensor de Efecto Hall
	pinMode(PULSO_HALL, INPUT);

	//Para el LCD
	lcd.init();
	lcd.backlight();

	//****************************************************
	//FREERTOS: Creacion de los semaforos
	PulsoHallSemaphoreBin = xSemaphoreCreateBinary();

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


