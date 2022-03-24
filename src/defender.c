// Standard headers
#include <stdio.h>
#include <stdlib.h>

// Internal headers
#include "direction.h"
#include "position.h"
#include "spy.h"

// Main header
#include "defender.h"

// Macros
#define UNUSED(x) (void)(x) // Auxiliary to avoid error of unused parameter

/*----------------------------------------------------------------------------*/
/*                         PRIVATE VARIABLES                                  */
/*----------------------------------------------------------------------------*/

enum DefenseState {START, ADVANCE, RETREAT, HOLD_GROUND, PATROL};
static enum DefenseState state = START;

static position_t previous_position;
static direction_t current_direction;

static size_t height_estimate; // Either height or (height - 1)
static size_t width; // Exactly the field width

static size_t rounds_stuck = 0;
static size_t rotations_clockwise = 0;
static size_t rotations_counterclockwise = 0;

/*----------------------------------------------------------------------------*/
/*                          PRIVATE FUNCTIONS HEADERS                         */
/*----------------------------------------------------------------------------*/

direction_t reverse_direction(direction_t current_direction);
direction_t rotate_clockwise(direction_t direction, size_t rotations);
direction_t rotate_counterclockwise(direction_t direction, size_t rotations);

bool is_stuck(position_t current_position);
direction_t execute_evasion_strategy();

/*----------------------------------------------------------------------------*/
/*                              PUBLIC FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

direction_t execute_defender_strategy(
    position_t defender_position, Spy attacker_spy) {

  /* Check if defender is stuck */
  if (is_stuck(defender_position)) {
    return execute_evasion_strategy();
  }

  switch (state) {
    case START :
      height_estimate = defender_position.i * 2;
      width = defender_position.j + 2;

      current_direction = (direction_t) DIR_LEFT;
      state = ADVANCE;
      break;

    case ADVANCE :
      /* Go forward until you reach the center, then Spy */
      if (defender_position.j == width / 2) {
        size_t attacker_i_at_spy = get_spy_position(attacker_spy).i;

        if (attacker_i_at_spy > defender_position.i) {
          /* The attacker is below the defender in the field,
           * so the defender will chase it that way
           */
          current_direction = (direction_t) DIR_DOWN_RIGHT;
        }

        else if (attacker_i_at_spy < defender_position.i) {
          /* The attacker is above the defender in the field,
           * so the defender will chase it that way
           */
          current_direction = (direction_t) DIR_UP_RIGHT;
        }

        else {
          /* The attacker is coming from the centre line,
           * so the defender will retreat in the same line
           */
          current_direction = (direction_t) DIR_RIGHT;
        }

        state = RETREAT;
      }
      break;

    case RETREAT :
      /* Retreat in the direction of the attacker, to try to catch it.
       * When you reach the second to last walkable column,
       * start patrolling or hold your ground
       */
      if (defender_position.j == width - 3) {
        if (abs(current_direction.i) == height_estimate / 2) {
          current_direction = (direction_t) DIR_STAY;
          state = HOLD_GROUND;
        }
        else {
          current_direction = (direction_t){current_direction.i, 0};
          state = PATROL;
        }
      }
      break;

    case HOLD_GROUND : break; /* Stay put */

    case PATROL : 
        /* Keep going up and down as much as you can */
        if (defender_position.i == 1 ||
            defender_position.i >= height_estimate - 2)
        {
          current_direction = reverse_direction(current_direction);
        }
        break;
    }

  previous_position = defender_position;
  return current_direction;
}

/*----------------------------------------------------------------------------*/
/*                             PRIVATE FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

direction_t reverse_direction(direction_t current_direction) {
  return (direction_t){- current_direction.i, - current_direction.j};
}

direction_t rotate_clockwise(direction_t direction, size_t rotations) {
  direction_t d = direction;
  for (size_t i = 0; i < rotations; i++) {
    // f(i, j) = (i + j, -i + j)
    d = (direction_t) {d.i + d.j, -d.i + d.j};
  }
  // Makes sure that the direction is unitary
  if (d.i != 0) d.i = d.i / abs(d.i);
  if (d.j != 0) d.j = d.j / abs(d.j);
  return d;
}

direction_t rotate_counterclockwise(direction_t direction, size_t rotations) {
  direction_t d = direction;
  for (size_t i = 0; i < rotations; i++) {
    // f(i, j) = (i - j, i + j)
    d = (direction_t) {d.i - d.j, d.i + d.j};
  }
  // Makes sure that the direction is unitary
  if (d.i != 0) d.i = d.i / abs(d.i);
  if (d.j != 0) d.j = d.j / abs(d.j);
  return d;
}

bool is_stuck(position_t current_position) {
  if (equal_positions(current_position, previous_position)) {
    rounds_stuck++;
    return true;
  }
  else {
    rounds_stuck = 0;
    rotations_clockwise = 0;
    rotations_counterclockwise = 0;
    return false;
  }
}

direction_t execute_evasion_strategy() {
  if (rounds_stuck % 2 == 1) {
    rotations_clockwise++;
    return rotate_clockwise(current_direction, rotations_clockwise);
  }
  else {
    rotations_counterclockwise++;
    return rotate_counterclockwise(current_direction, rotations_counterclockwise);
  }
}

/*----------------------------------------------------------------------------*/
