#pragma once

#define ZERO_INIT_STRUCT(obj) ((*obj) = (typeof(*obj)) { 0 })