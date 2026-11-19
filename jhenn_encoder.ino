#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//==============================================================
//DEFINICIONES
//*************************************************************
//PINES
//Para el sensor de Efecto Hall
#define PULSO_HALL 2

//*************************************************************
//CONSTANTES
#define NUM_IMANES 4
#define CANT_MUESTRAS 10 //todos los datos que se van a tomar para obtener un promedio
#define PERIODO_ACTUALIZAR_RPM 750 //[ms]
#define TIEMPO_DEBOUNCE 130

//==============================================================
//VARIABLES GLOBALES
volatile unsigned int cantidad_pulsos = 0;
volatile uint32_t tiempo_anterior_debounce = 0; //[ms]
volatile uint32_t suma_tiempos = 0; //[s]


//Objeto para el display LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);


//==============================================================
//FREERTOS | Solo se permiten dos tareas por el limitado hardware


//*************************************************************
//Handle de las Tareas
TaskHandle_t ProcesamientoPulsosTaskHandle = NULL;
TaskHandle_t GestorGraficoTaskHandle = NULL;


//*************************************************************
//Funciones de las Tareas
//Tarea 1. Procesamiento de los pulsos
void ProcesamientoPulsosTask(void *parameter){
	
		
	for (;;){
		//ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //pdTRUE -> limpiar el contador de notificaciones a 0 en cada notificacion
		unsigned int cantidad_pulsos_local = 0;
		uint32_t tiempo_anterior_debounce_local = 0; 

		taskENTER_CRITICAL();
			cantidad_pulsos_local = cantidad_pulsos;
		taskEXIT_CRITICAL();

		
	}
}

//Tarea 2. Gestor grafico LCD 
void GestorGraficoTask(void *parameter){
	for (;;){

	}
}



//==============================================================
void setup(){
	Serial.begin(115200);

	//*************************************************************
	//CONFIGURACION PARA LOS PINES
	pinMode(PULSO_HALL, INPUT);  //para el pin de Efecto Hall
	attachInterrupt(digitalPinToInterrupt(PULSO_HALL), recibirPulso, RISING);

	//Para el LCD
	lcd.init();
	lcd.backlight();

	//*************************************************************
	//FREERTOS: Creacion de las tareas
	//Tarea 1. Procesamiento de los pulsos
	xTaskCreate(
		ProcesamientoPulsosTask,  //la funcion tarea
		"ProcesamientoPulsos", //el nombre de la tarea
		128, //memoria en la pila
		NULL,
		2, //prioridad
		&ProcesamientoPulsosTaskHandle //el handle de la tarea
	);

	//Tarea 2. Gestor grafico LCD 
	xTaskCreate(
		GestorGraficoTask,  //la funcion tarea
		"GestorGrafico", //el nombre de la tarea
		128, //memoria en la pila
		NULL,
		1, //prioridad
		&GestorGraficoTaskHandle //el handle de la tarea
	);

}

//==============================================================
void loop(){
}

//==============================================================
//FUNCIONES
void recibirPulso(){
	uint32_t tiempo_actual_debounce = millis();
	//BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (tiempo_actual_debounce - tiempo_anterior_debounce > TIEMPO_DEBOUNCE){
		//Utilizando una Seccion Critica para modificar la variable dentro de un ISR
		//se guarda con el estado anterior y se devuelve con el mismo estado guardado (ISR)
		UBaseType_t uxEstadoInterrupcion = taskENTER_CRITICAL_FROM_ISR();
			++cantidad_pulsos;
			tiempo_anterior_debounce = tiempo_actual_debounce; //ya que el atmega no es 32 bits y necesita varios ciclos
		taskEXIT_CRITICAL_FROM_ISR(uxEstadoInterrupcion);

	//	vTaskNotifyGiveFromISR(ProcesamientoPulsosTaskHandle, &xHigherPriorityTaskWoken);

	//	if (xHigherPriorityTaskWoken){
	//		portYIELD_FROM_ISR();
	//	}
	}
}

