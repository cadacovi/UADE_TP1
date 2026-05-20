# 17. Maquina de estados principal
```psc
FUNCTION navigation_loop_50hz()
    SWITCH robot.mode

        CASE BOOT:
            robot.mode = CALIBRATION

        CASE CALIBRATION:
            perform_calibration()

        CASE IDLE:
            stop_motors()
            IF start_button_pressed() THEN
                reset_run_state()
                robot.mode = EXPLORE
            END_IF

        CASE EXPLORE:
            explore_step()

        CASE SPEED_RUN:
            speed_run_step()

        CASE ERROR:
            stop_motors()
            signal_error()
    END_SWITCH
END_FUNCTION
```

# 18. Detección de paredes en una celda

Idealmente, las paredes se deciden cuando estás cerca del centro de una celda o en una posición repetible.
```psc
FUNCTION sense_current_cell_walls()
    front_wall = tof.front_wall_detected
    left_wall  = ir.left_wall_detected
    right_wall = ir.right_wall_detected

    heading = robot.maze_heading
    x = robot.maze_cell_x
    y = robot.maze_cell_y

    IF heading == NORTH THEN
        maze[x][y].wall_north = front_wall
        maze[x][y].wall_west  = left_wall
        maze[x][y].wall_east  = right_wall
    END_IF

    IF heading == EAST THEN
        maze[x][y].wall_east  = front_wall
        maze[x][y].wall_north = left_wall
        maze[x][y].wall_south = right_wall
    END_IF

    IF heading == SOUTH THEN
        maze[x][y].wall_south = front_wall
        maze[x][y].wall_east  = left_wall
        maze[x][y].wall_west  = right_wall
    END_IF

    IF heading == WEST THEN
        maze[x][y].wall_west  = front_wall
        maze[x][y].wall_south = left_wall
        maze[x][y].wall_north = right_wall
    END_IF

    maze[x][y].visited = TRUE
    maze[x][y].known = TRUE
END_FUNCTION
```

# 19. Actualización de celda actual

Después de avanzar una celda, se actualiza posición lógica:
```psc
FUNCTION advance_maze_coordinates_one_cell()
    IF robot.maze_heading == NORTH THEN
        robot.maze_cell_y = robot.maze_cell_y + 1
    END_IF

    IF robot.maze_heading == EAST THEN
        robot.maze_cell_x = robot.maze_cell_x + 1
    END_IF

    IF robot.maze_heading == SOUTH THEN
        robot.maze_cell_y = robot.maze_cell_y - 1
    END_IF

    IF robot.maze_heading == WEST THEN
        robot.maze_cell_x = robot.maze_cell_x - 1
    END_IF
END_FUNCTION
```

# 20. Exploración básica

flood fill.

## 20.1. meta
```psc
FUNCTION is_goal_cell(x, y)
    RETURN (x,y) pertenece al centro del laberinto
END_FUNCTION
```

## 20.2. flood fill
```psc
FUNCTION update_flood_fill(goal_cells)
    initialize_all_flood_values_to_large_number()

    queue = empty_queue()

    FOR each goal_cell IN goal_cells
        maze[goal_cell.x][goal_cell.y].flood_value = 0
        enqueue(goal_cell)
    END_FOR

    WHILE queue not empty
        current = dequeue()
        current_value = maze[current.x][current.y].flood_value

        FOR each accessible_neighbor of current
            IF maze[neighbor.x][neighbor.y].flood_value > current_value + 1 THEN
                maze[neighbor.x][neighbor.y].flood_value = current_value + 1
                enqueue(neighbor)
            END_IF
        END_FOR
    END_WHILE
END_FUNCTION
```

## 20.3. elegir siguiente movimiento
```psc
FUNCTION choose_next_direction()
    current = maze[robot.maze_cell_x][robot.maze_cell_y]

    best_dir = NONE
    best_value = VERY_LARGE

    FOR dir IN [FRONT, LEFT, RIGHT, BACK]
        IF path_open_in_direction(dir) THEN
            nx, ny = neighbor_from_direction(dir)
            value = maze[nx][ny].flood_value

            IF value < best_value THEN
                best_value = value
                best_dir = dir
            END_IF
        END_IF
    END_FOR

    RETURN best_dir
END_FUNCTION
```

## 20.4. paso de exploración
```psc
FUNCTION explore_step()
    IF robot.motion_state != STOPPED THEN
        RETURN
    END_IF

    sense_current_cell_walls()

    IF is_goal_cell(robot.maze_cell_x, robot.maze_cell_y) THEN
        mark_goal_reached()
        recompute_flood_for_start_or_best_path()
        robot.mode = SPEED_RUN
        RETURN
    END_IF

    update_flood_fill(goal_cells)

    next_dir = choose_next_direction()

    SWITCH next_dir
        CASE FRONT:
            move_one_cell()
            advance_maze_coordinates_one_cell()

        CASE LEFT:
            turn_left_90()
            update_heading_left()
            move_one_cell()
            advance_maze_coordinates_one_cell()

        CASE RIGHT:
            turn_right_90()
            update_heading_right()
            move_one_cell()
            advance_maze_coordinates_one_cell()

        CASE BACK:
            turn_back_180()
            update_heading_back()
            move_one_cell()
            advance_maze_coordinates_one_cell()

        CASE NONE:
            robot.mode = ERROR
    END_SWITCH
END_FUNCTION
```

# 21. Actualización de heading lógico
```psc
FUNCTION update_heading_left()
    robot.maze_heading = rotate_heading_left(robot.maze_heading)
END_FUNCTION

FUNCTION update_heading_right()
    robot.maze_heading = rotate_heading_right(robot.maze_heading)
END_FUNCTION

FUNCTION update_heading_back()
    robot.maze_heading = rotate_heading_back(robot.maze_heading)
END_FUNCTION
```

# 22. Modo speed-run

Cuando ya conoces el camino, usas una ruta precomputada.
```psc
FUNCTION speed_run_step()
    IF robot.motion_state != STOPPED THEN
        RETURN
    END_IF

    next_command = get_next_path_command()

    SWITCH next_command
        CASE STRAIGHT_1:
            move_one_cell()
            advance_maze_coordinates_one_cell()

        CASE STRAIGHT_2:
            start_move_straight(2 * CELL_SIZE_MM)
            wait_until_motion_complete()
            advance_maze_coordinates_one_cell()
            advance_maze_coordinates_one_cell()

        CASE TURN_LEFT_90:
            turn_left_90()
            update_heading_left()

        CASE TURN_RIGHT_90:
            turn_right_90()
            update_heading_right()

        CASE TURN_180:
            turn_back_180()
            update_heading_back()

        CASE END_RUN:
            robot.mode = IDLE
    END_SWITCH
END_FUNCTION
```
"Después, cuando afines mucho, puedes hacer giros curvos y diagonales, pero no ahora."

