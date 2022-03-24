// Standard headers
#include <stdio.h>

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

static size_t height_estimate; // Either height or (height - 1)
static size_t width; // Exactly the field width

static direction_t current_direction;

/*----------------------------------------------------------------------------*/
/*                          PRIVATE FUNCTIONS HEADERS                         */
/*----------------------------------------------------------------------------*/
direction_t reverse_direction(direction_t current_direction);

/*----------------------------------------------------------------------------*/
/*                              PUBLIC FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

direction_t execute_defender_strategy(
    position_t defender_position, Spy attacker_spy) {

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
        if (current_direction.i == 0) {
          current_direction = (direction_t) DIR_LEFT;
          state = HOLD_GROUND;
        }
        else {
          current_direction = (direction_t){current_direction.i, 0};
          state = PATROL;
        }
      }
      break;

    case HOLD_GROUND :
      /* Keep going back and forth */
      current_direction = reverse_direction(current_direction);
      break;

    case PATROL : 
        /* Keep going up and down as much as you can */
        if (defender_position.i <= 2 ||
            defender_position.i >= height_estimate - 2)
        {
          current_direction = reverse_direction(current_direction);
        }
        break;
    }

  return current_direction;
}

/*----------------------------------------------------------------------------*/
/*                             PRIVATE FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

direction_t reverse_direction(direction_t current_direction) {
  return (direction_t){- current_direction.i, - current_direction.j};
}

/*----------------------------------------------------------------------------*/
