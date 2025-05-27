#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <array>
#include <semaphore.h>
#include "prelude.h"

extern std::array<sem_t*, number_of_workers> ready;

void init_semaphores();

void cleanup_semaphores();

#endif
