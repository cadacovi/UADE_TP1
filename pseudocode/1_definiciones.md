# DEFINICION DE CONSTANTES Y TIPOS/ESTRUCTURAS
## GENERALES
MAZE_WIDTH  = 8
MAZE_HEIGHT = 8

CELL_DISTANCE_MM = 120

GOAL_X_1 = 3
GOAL_Y_1 = 3
GOAL_X_2 = 3
GOAL_Y_2 = 4
GOAL_X_3 = 4
GOAL_Y_3 = 3
GOAL_X_4 = 4
GOAL_Y_4 = 4

UNKNOWN = -1

TRUE  = 1
FALSE = 0
## ESTADO DEL SISTEMA
ENUM SYS_STATE
    INIT
    WAIT_START
    EXPLORE
    GOAL_REACHED
    ERROR
END ENUM
## DIRECCIONES POSIBLES
ENUM DIRECTION
    NORTH
    EAST
    SOUTH
    WEST
END ENUM
## MOVIMIENTOS POSIBLES
ENUM MOVIMIENTO
    MOVE_FORWARD
    TURN_LEFT
    TURN_RIGHT
    TURN_BACK
    STOP
END ENUM
## ORIENTACION
STRUCT Pose
    x
    y
    direction
END STRUCT
## LECTURA DE SENSORES
STRUCT SensorReadings
    left_raw
    front_raw
    right_raw

    wall_left
    wall_front
    wall_right
END STRUCT
## LECTURA ENCODERS DEL MOTOR
STRUCT EncoderData
    left_ticks
    right_ticks
END STRUCT
## PRESENCIA DE PAREDES
ENUM WALL_STATE
    WALL_UNKNOWN
    WALL_OPEN
    WALL_PRESENT
END ENUM
## CELDA DEL LABERINTO
STRUCT Cell
    wall_north
    wall_east
    wall_south
    wall_west

    visited

    flood_value
END STRUCT
## MAPA/LABERINTO
maze[MAZE_WIDTH][MAZE_HEIGHT] _maze es una matriz de Cell (estructura para las celdas del laberinto definida arriba)_
## CALIBRACION/ESTANDARIZACION (los valores requieren calibracion experimental)
### SENSORES (limite hasta que encuentra una pared)
SENSOR_THRESHOLD_LEFT  = 2000
SENSOR_THRESHOLD_FRONT = 2200
SENSOR_THRESHOLD_RIGHT = 2000
### MOVIMIENTO (velocidades base)
PWM_BASE_SPEED = 120
PWM_TURN_SPEED = 100
### ENCODER (define cuanto se necesita avanzar hasta pasar una celda, y cuanto necesita avanazar una rueda para girar 90 grados)
TICKS_PER_CELL = 420
TICKS_TURN_90  = 180
## VARIABLES GLOBALES PRINCIPALES
robot_pose      : Pose
sensor_data     : SensorReadings
encoder_data    : EncoderData

maze            : matriz de Cell

system_state    : ESTADO_SISTEMA
goal_reached    : boolean
## DEFINICION DE PINES
PIN_MOTOR_LEFT_PWM
PIN_MOTOR_LEFT_IN1
PIN_MOTOR_LEFT_IN2

PIN_MOTOR_RIGHT_PWM
PIN_MOTOR_RIGHT_IN1
PIN_MOTOR_RIGHT_IN2

PIN_ENCODER_LEFT_A
PIN_ENCODER_LEFT_B
PIN_ENCODER_RIGHT_A
PIN_ENCODER_RIGHT_B

PIN_SENSOR_LEFT
PIN_SENSOR_FRONT
PIN_SENSOR_RIGHT
## VECTORES AUXILIARES PARA LA ACTUALIZACION DE LA POSICION
dx[4] = {  0,  1,  0, -1 }
dy[4] = {  1,  0, -1,  0 }