# INICIALIZAR EL FLOOD
FUNCTION flood_reset()

    FOR x desde 0 hasta MAZE_WIDTH - 1
        FOR y desde 0 hasta MAZE_HEIGHT - 1
            maze[x][y].flood_value = INFINITY
        END FOR
    END FOR

END FUNCTION
## CELDAS META
FUNCTION flood_set_goals()

    PARA cada goal_cell
        maze[goal.x][goal.y].flood_value = 0
    FIN PARA

END FUNCTION
# FLOOD FILL
FUNCTION flood_fill()

    flood_reset()
    flood_set_goals()

    crear cola vacía

    PARA cada goal_cell
        enqueue(goal_cell)
    FIN PARA

    WHILE cola no vacía

        current = dequeue()

        current_value = maze[current.x][current.y].flood_value

        PARA cada dirección dir en {NORTH, EAST, SOUTH, WEST}

            neighbor_x = current.x + dx[dir]
            neighbor_y = current.y + dy[dir]

            SI is_valid_cell(neighbor_x, neighbor_y) == FALSE
                CONTINUE
            FIN SI

            SI get_wall(current.x, current.y, dir) == WALL_PRESENT
                CONTINUE
            FIN SI

            SI maze[neighbor_x][neighbor_y].flood_value > current_value + 1
                maze[neighbor_x][neighbor_y].flood_value = current_value + 1
                enqueue(neighbor_x, neighbor_y)
            FIN SI

        FIN PARA

    END WHILE

END FUNCTION
# DETERMINAR SIGUIENTE MOVIMIENTO
FUNCTION choose_next_direction(x, y)

    best_dir = NONE
    best_value = INFINITY

    PARA cada dirección dir en {NORTH, EAST, SOUTH, WEST}

        neighbor_x = x + dx[dir]
        neighbor_y = y + dy[dir]

        SI is_valid_cell(neighbor_x, neighbor_y) == FALSE
            CONTINUE
        FIN SI

        SI get_wall(x, y, dir) == WALL_PRESENT
            CONTINUE
        FIN SI

        neighbor_value = maze[neighbor_x][neighbor_y].flood_value

        SI neighbor_value < best_value
            best_value = neighbor_value
            best_dir = dir
            //
            (si dos valores son iguales, SEGUIR la dirección actual para girar menos)
            //
        FIN SI

    FIN PARA

    RETURN best_dir

END FUNCTION