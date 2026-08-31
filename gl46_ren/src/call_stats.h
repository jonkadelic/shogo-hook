#pragma once

void call_stats__mark_called(char const* func_name);
void call_stats__log_all(void);
void call_stats__clear(void);
