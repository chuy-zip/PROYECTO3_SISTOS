# PROYECTO3_SISTOS
Simulación de algoritmos de calendarización y sincronización.

Para poder llevar a cabo este proyecto utilizamos C++ con la herramienta de QTcreator. Estes es un entorno de desarrollo multiplataforma que se utiliza para crear y modificar aplicaciones. En nuestro caso la configuración que nos funcinó a nosotros fue utilizar una versión menor de QTcreator utilizando el sistema operativo de Linux. Específicamente usamos Ubuntu 24.04. Para poder correr el proyecto es necesario correr los siguientes comandos de instalación:

### Actualizar lista de paquetes disponibles

```
sudo apt-get update
```

### Instalar herramientas esenciales de compilación y dependencias gráficas
```
sudo apt-get install build-essential libgl1-mesa-dev
```

### Instalar herramientas específicas para desarrollo con Qt
```
sudo apt install -y qtcreator qtbase5-dev qt5-qmake cmake
```

Luego de haber instalado estas librerias y programas es necesario clonar el repositorio.

## Pasos para abrir el proyecto y correrlo
1. Abrir QTCreator (desde la lista de aplicaciones o bien con el comando qtcreator en la terminal)
2. Elegir la opción de "Open Project"
3. Buscar donde se clonó el repositorio
4. Dentro del repositorio, navegar en las carpetas "sincronizacion_calendarizacion"
5. Seleccionar el archivo CMakeLists.txt, este archivo es la configuración del proyecto y debería de permitir a QTCreator abrir el proyecto correctamente
6. Presionar el boton verde de RUN en la esquina inferior izquierda

Con esto se construirá el proyecto y se podrá correr la simulación.

Tenemos 3 pantallas

* MainWindow
* Scheduling Window
* Sinchronization Window


### Main Window
Ventan de selección se simulación, donde se elige entre realizar la simulación de calendarización o bien la de sincronización. Estos botones abren ventanas nuevas para cada simulación.

### Scheduling Window (Ventana de Calendarización)

Simulación visual de algoritmos de planificación de procesos con diagramas de Gantt dinámicos.

#### Características principales:
Selección de algoritmos:

* FIFO (First In First Out)
* SJF (Shortest Job First)
* SRT (Shortest Remaining Time)
* Round Robin (con quantum configurable)
* Priority Scheduling con envejecimiento

Métricas:

* Tiempo promedio de espera (Avg Waiting Time)
* Tiempo promedio de finalización (Avg Completion Time)
* Desglose por proceso

Botones para cargar:

* Archivo de procesos

#### Formato de los archivos

**Archivo de procesos**

```
PID, BT, AT, Priority
Ejemplo: P1, 8, 0, 1
```

### Synchronization Window (Ventana de Sincronización)
Simulación de mecanismos de sincronización con visualización de estados de procesos y recursos a lo largo de del tiempo.

#### Características principales:
Modos de sincronización:

* Mutex Locks
* Semáforos

Indicadores visuales de estados:

* ACCESED (verde) - Recurso obtenido
* WAITING (rojo/naranja) - Proceso en espera

Botones para cargar:

* Archivo de recursos

* Archivo de acciones

#### Formato de los archivos

**Archivo de recursos**

```
NOMBRE_RECURSO, CONTADOR
Ejemplo: R1, 1
```

**Archivo de acciones**

```
PID, ACCION, RECURSO, CICLO
Ejemplo: P1, READ, R1, 0
Acciones válidas: READ, WRITE
```