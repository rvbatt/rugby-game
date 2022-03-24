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

enum AttackState{START, DISTRACT, GO_TO_CENTER, SPRINT};
static enum AttackState state = START;

static size_t height_estimate; // Either height or (height - 1)

static direction_t current_direction;

/*----------------------------------------------------------------------------*/
/*                              PUBLIC FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

direction_t execute_attacker_strategy(
    position_t attacker_position, Spy defender_spy) {

  switch (state) {
    case START :
      srand((int) time(NULL)); // Randomizes seed
      height_estimate = attacker_position.i * 2;

      // Randomly chooses between going UP or DOWN
      if (rand() <= RAND_MAX / 2)
        current_direction = DIR_UP;
      else
        current_direction = DIR_DOWN;
      
      state = DISTRACT;
      break;

    case DISTRACT :
      /* Keep going until you reach an edge of the field */
      if (attacker_position.i == 1) { // Top of the field
        current_direction = DIR_DOWN_RIGHT;
        state = GO_TO_CENTER;
      }

      if (attacker_position.i >= height_estimate - 2) { // Bottom of the field
        current_direction = DIR_UP_RIGHT;
        state = GO_TO_CENTER;
      }
      break;

    case GO_TO_CENTER :
      /* Keep going until you are close to the center */
      if (attacker_position.i == height_estimate / 2) {
        size_t defender_i_at_spy = get_spy_position(defender_spy).i;

        if (attacker_position.i > defender_i_at_spy) {
          /* The attacker is below the defender in the field,
           * so the attacker will sprint to the bottom corner
           */
          current_direction = DIR_DOWN_RIGHT;
        }

        else if (attacker_position.i < defender_i_at_spy) {
          /* The attacker is above the defender in the field,
           * so the attacker will sprint to top corner
           */
          current_direction = DIR_UP_RIGHT;
        }

        state = SPRINT;
      }
      break;

    case SPRINT :
      /* Go to one of the corners.
       * If you reach a wall, just move straight ahead
       */
      if (attacker_position.i == 1 ||
          attacker_position.i >= height_estimate - 2)
      {
        current_direction = DIR_RIGHT;
      }
      break;
  }

  return current_direction;
}

/*----------------------------------------------------------------------------*/
