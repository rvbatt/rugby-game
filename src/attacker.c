// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Internal headers
#include "direction.h"
#include "position.h"
#include "spy.h"

// Main header
#include "attacker.h"

// Macros
#define UNUSED(x) (void)(x) // Auxiliary to avoid error of unused parameter

/*----------------------------------------------------------------------------*/
/*                         PRIVATE VARIABLES                                  */
/*----------------------------------------------------------------------------*/

enum Attack_state{START, DISTRACT, GO_TO_CENTER, SPRINT};
static enum Attack_state state = START;

static position_t previous_position;
static direction_t current_direction;

static size_t height_estimate; // Either height or (height - 1)

static size_t rounds_stuck = 0;
static size_t rotations_clockwise = 0;
static size_t rotations_counterclockwise = 0;

/*----------------------------------------------------------------------------*/
/*                          PRIVATE FUNCTIONS HEADERS                         */
/*----------------------------------------------------------------------------*/

static direction_t rotate_clockwise(direction_t direction, size_t rotations);
static direction_t rotate_counterclockwise(direction_t direction, size_t rotations);

static direction_t obstacle_evasion_direction();
static direction_t execute_detour_strategy();
static void reset_stuck_data();

/*----------------------------------------------------------------------------*/
/*                              PUBLIC FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

direction_t execute_attacker_strategy(
    position_t attacker_position, Spy defender_spy) {

  /* Check if attacker is stuck */
  if (equal_positions(attacker_position, previous_position)) {
    rounds_stuck++;
    return obstacle_evasion_direction();
  }
  else if (rounds_stuck >= 3) {
    return execute_detour_strategy();
  }
  else {
    reset_stuck_data();
  }

  switch (state) {
    case START :
      height_estimate = attacker_position.i * 2;

      // Randomly chooses between going UP or DOWN
      srand((int) time(NULL));
      if (rand() <= RAND_MAX / 2)
        current_direction = (direction_t) DIR_UP;
      else
        current_direction = (direction_t) DIR_DOWN;
      
      state = DISTRACT;
      break;

    case DISTRACT :
      /* Keep going UP or DOWN until you reach an edge of the field,
       * then start moving to the center in a diagonal line
       */
      if (attacker_position.i == 1) { // Top of the field
        current_direction = (direction_t) DIR_DOWN_RIGHT;
        state = GO_TO_CENTER;
      }

      else if (attacker_position.i >= height_estimate - 2) { // Bottom of the field
        current_direction = (direction_t) DIR_UP_RIGHT;
        state = GO_TO_CENTER;
      }
      break;

    case GO_TO_CENTER :
      /* Keep going until you are close to the center, then Spy and
       * start sprinting to the opposite side of the defender
       */
      if (attacker_position.i == height_estimate / 2) {
        size_t defender_i_at_spy = get_spy_position(defender_spy).i;

        if (attacker_position.i > defender_i_at_spy) {
          current_direction = (direction_t) DIR_DOWN_RIGHT;
        }
        else { // Defender is below or on the same height
          current_direction = (direction_t) DIR_UP_RIGHT;
        }

        state = SPRINT;
      }
      break;

    case SPRINT :
      /* Go to one of the far right corners.
       * If you reach a wall, just move straight ahead
       */
      if (attacker_position.i == 1 ||
          attacker_position.i >= height_estimate - 2)
      {
        current_direction = (direction_t) DIR_RIGHT;
      }
      break;

    default : // Invalid state. Restart strategy
      state = START;
  }

  previous_position = attacker_position;
  return current_direction;
}

/*----------------------------------------------------------------------------*/
/*                             PRIVATE FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

direction_t rotate_clockwise(direction_t direction, size_t rotations) {
  direction_t d = direction;
  for (size_t i = 0; i < rotations; i++) {
    // f(i, j) = (i + j, -i + j)
    d = (direction_t) {d.i + d.j, -d.i + d.j};
  }
  // Fix norm increase in linear transformation
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
  // Fix norm increase in linear transformation
  if (d.i != 0) d.i = d.i / abs(d.i);
  if (d.j != 0) d.j = d.j / abs(d.j);
  return d;
}

direction_t obstacle_evasion_direction() {
  if (rounds_stuck % 2 == 1)
    return rotate_clockwise(current_direction, ++rotations_clockwise);
  else
    return rotate_counterclockwise(current_direction,
                                        ++rotations_counterclockwise);
}

direction_t execute_detour_strategy() {
  rounds_stuck -= 2;
  if (rounds_stuck % 2 == 1)
    return rotate_clockwise(current_direction, --rotations_clockwise);
  else
    return rotate_counterclockwise(current_direction,
                                        --rotations_counterclockwise);
}

void reset_stuck_data() {
  rounds_stuck = 0;
  rotations_clockwise = 0;
  rotations_counterclockwise = 0;
}

/*----------------------------------------------------------------------------*/
