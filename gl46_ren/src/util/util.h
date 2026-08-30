#pragma once

#define DEBUG

#define OBJECT_ZERO_INIT(self) (*self = (typeof(*self)) { 0 })