#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <semaphore.h>

#include <array>

#include "prelude.h"

extern std::array<sem_t *, number_of_workers> ready;

void init_semaphores();

void cleanup_semaphores();

#endif
