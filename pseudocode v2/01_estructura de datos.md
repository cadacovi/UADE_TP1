# 4. Estructuras de datos
## 4.1. Constantes
```psc
CONST WHEEL_DIAMETER_MM = 28.0
CONST WHEEL_RADIUS_MM   = 14.0
CONST WHEEL_BASE_MM     = 72.0         // medir entre centros de ruedas
CONST CELL_SIZE_MM      = 180.0
CONST HALF_CELL_MM      = 90.0

CONST CONTROL_DT_MS     = 1
CONST SENSOR_DT_MS      = 5
CONST NAV_DT_MS         = 20

CONST PWM_MAX           = 255
CONST PWM_MIN_EFFECTIVE = 25

CONST MAX_LINEAR_SPEED_MM_S   = 800
CONST MAX_LINEAR_ACCEL_MM_S2  = 3000
CONST MAX_ANGULAR_SPEED_DPS   = 720
CONST MAX_ANGULAR_ACCEL_DPS2  = 3000

CONST FRONT_WALL_THRESHOLD_MM = 70
CONST FRONT_STOP_TARGET_MM    = 35

CONST SIDE_WALL_THRESHOLD_LEFT  = valor_calibrado
CONST SIDE_WALL_THRESHOLD_RIGHT = valor_calibrado

CONST IR_SAMPLES = 4
CONST TOF_SAMPLES = 2

CONST GYRO_BIAS_SAMPLES = 1000
CONST IR_CALIB_SAMPLES  = 200
```

## 4.2. Estado de sensores
``` psc
STRUCT EncoderState
    raw_angle_deg
    prev_raw_angle_deg
    delta_angle_deg
    total_angle_deg
    angular_velocity_dps
    distance_mm
    velocity_mm_s
END_STRUCT

STRUCT IMUState
    gyro_z_dps
    gyro_bias_z
    accel_x
    accel_y
    accel_z
    yaw_deg
END_STRUCT

STRUCT IRState
    ambient_left
    active_left
    signal_left
    filtered_left

    ambient_right
    active_right
    signal_right
    filtered_right

    left_wall_detected
    right_wall_detected
END_STRUCT

STRUCT TOFState
    raw_mm
    filtered_mm
    front_wall_detected
    valid
END_STRUCT
```

## 4.3. Estado de odometría
```psc
STRUCT OdomState
    x_mm
    y_mm
    theta_deg

    prev_left_distance_mm
    prev_right_distance_mm

    linear_distance_total_mm
    cell_progress_mm
END_STRUCT
```

## 4.4. Controladores
``` psc
STRUCT PID
    kp
    ki
    kd
    integral
    prev_error
    output
    output_min
    output_max
END_STRUCT
```
Al menos estos "PID":
PID velocidad rueda izquierda
PID velocidad rueda derecha
PID ángulo/rumbo
PID centrado lateral opcional

## 4.5. Estado del robot
``` psc
ENUM RobotMode
    BOOT
    CALIBRATION
    IDLE
    EXPLORE
    SPEED_RUN
    ERROR
END_ENUM

ENUM MotionState
    STOPPED
    MOVING_STRAIGHT
    TURNING_LEFT_90
    TURNING_RIGHT_90
    TURNING_180
    ALIGNING_FRONT
    BRAKING
END_ENUM

STRUCT RobotState
    mode
    motion_state

    target_linear_speed_mm_s
    target_angular_speed_dps

    target_heading_deg
    target_distance_mm

    left_motor_pwm
    right_motor_pwm

    maze_cell_x
    maze_cell_y
    maze_heading   // N,E,S,W

    run_requested
    calibrated
    error_flag
END_STRUCT
```

# 5. Mapa del laberinto

Para micromouse conviene una celda con paredes y estado.
``` psc
STRUCT MazeCell
    wall_north
    wall_east
    wall_south
    wall_west

    visited
    flood_value
    known
END_STRUCT

DECLARE maze[16][16] AS MazeCell
```