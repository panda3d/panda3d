/*
 * Application specific interaction between main and buffer
 * process. This is much less generic than the functions in
 * xfermem so I chose to put it in buffer.[hc].
 * 01/28/99 [dk]
 */

void buffer_ignore_lowmem(void);
void buffer_end(void);
void buffer_resync(void);
void buffer_reset(void);
void buffer_start(void);
void buffer_stop(void);
