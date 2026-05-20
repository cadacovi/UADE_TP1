# INICIALIZACION
FUNCTION maze_init()

    FOR x desde 0 hasta MAZE_WIDTH - 1
        FOR y desde 0 hasta MAZE_HEIGHT - 1

            maze[x][y].north = WALL_UNKNOWN
            maze[x][y].east  = WALL_UNKNOWN
            maze[x][y].south = WALL_UNKNOWN
            maze[x][y].west  = WALL_UNKNOWN

            maze[x][y].visited = FALSE
            maze[x][y].flood_value = UNKNOWN

        END FOR
    END FOR

END FUNCTION
## SE SUPONEN PAREDES EXTERIORES
FUNCTION set_outer_walls()

    FOR x desde 0 hasta MAZE_WIDTH - 1
        maze[x][0].south = WALL_PRESENT
        maze[x][MAZE_HEIGHT - 1].north = WALL_PRESENT
    END FOR

    FOR y desde 0 hasta MAZE_HEIGHT - 1
        maze[0][y].west = WALL_PRESENT
        maze[MAZE_WIDTH - 1][y].east = WALL_PRESENT
    END FOR

END FUNCTION
# FUNCIONES
## MARCAR CELDA VISITADA
FUNCTION mark_visited(x, y)
    maze[x][y].visited = TRUE
END FUNCTION
## CELDA VALIDA (DENTRO DEL MAZE)
FUNCTION is_valid_cell(x, y)

    SI x < 0 O x >= MAZE_WIDTH
        RETURN FALSE
    FIN SI

    SI y < 0 O y >= MAZE_HEIGHT
        RETURN FALSE
    FIN SI

    RETURN TRUE

END FUNCTION
## MARCAR PAREDES DE CELDA
FUNCTION set_wall(x, y, dir, state)

    SI dir == NORTH
        maze[x][y].north = state
    SI_NO SI dir == EAST
        maze[x][y].east = state
    SI_NO SI dir == SOUTH
        maze[x][y].south = state
    SI_NO SI dir == WEST
        maze[x][y].west = state
    FIN SI

END FUNCTION
### VERSION BIDIRECCIONAL (PAREDES COMPARTIDAS)
FUNCTION set_wall_bidirectional(x, y, dir, state)

    set_wall(x, y, dir, state)

    neighbor_x = x + dx[dir]
    neighbor_y = y + dy[dir]

    SI is_valid_cell(neighbor_x, neighbor_y)

        opposite_dir = (dir + 2) mod 4
        set_wall(neighbor_x, neighbor_y, opposite_dir, state)

    FIN SI

END FUNCTION
## OBTENER ESTADO DE PARED DE CELDA
FUNCTION get_wall(x, y, dir)

    SI dir == NORTH
        RETURN maze[x][y].north
    SI_NO SI dir == EAST
        RETURN maze[x][y].east
    SI_NO SI dir == SOUTH
        RETURN maze[x][y].south
    SI_NO SI dir == WEST
        RETURN maze[x][y].west
    FIN SI

END FUNCTION
## DEFINIR SI UNA CELDA ES GOAL
FUNCTION is_goal_cell(x, y)

    (...)

    RETURN FALSE

END FUNCTION
## ACTUALIZAR MAPA CON INFORMACION DE SENSORES
FUNCTION update_maze_from_sensors()

    x = robot_pose.x
    y = robot_pose.y
    dir = robot_pose.dir
// pasa de direcciones relativas (mouse) a absolutas (laberinto)
    left_dir  = (dir + 3) mod 4
    front_dir = dir
    right_dir = (dir + 1) mod 4
//
    mark_visited(x, y)

    SI sensor_data.wall_left == TRUE
        set_wall_bidirectional(x, y, left_dir, WALL_PRESENT)
    SI_NO
        set_wall_bidirectional(x, y, left_dir, WALL_OPEN)
    FIN SI

    SI sensor_data.wall_front == TRUE
        set_wall_bidirectional(x, y, front_dir, WALL_PRESENT)
    SI_NO
        set_wall_bidirectional(x, y, front_dir, WALL_OPEN)
    FIN SI

    SI sensor_data.wall_right == TRUE
        set_wall_bidirectional(x, y, right_dir, WALL_PRESENT)
    SI_NO
        set_wall_bidirectional(x, y, right_dir, WALL_OPEN)
    FIN SI

END FUNCTION
## FUNCION AUXILIAR OPCIONA LAPRA PASAR A DIRECCIONES ABSOLUTAS
FUNCTION get_left_dir(dir)
    RETURN (dir + 3) mod 4
END FUNCTION

FUNCTION get_right_dir(dir)
    RETURN (dir + 1) mod 4
END FUNCTION
## FUNCION EJEMPLO PARA USAR EN EL LOOP
FUNCTION sense_and_update_maze()
    update_sensors()
    update_maze_from_sensors()
END FUNCTION