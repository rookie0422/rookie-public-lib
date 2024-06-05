#include <stdint.h>
#include "cmsis_os.h"

extern uint32_t os_time;

uint32_t HAL_GetTick(void)
{
  return os_time;
}
