
#ifndef _ERREXIT_HH
#define _ERREXIT_HH

/* errExit è una funzione di supporto per stampare
  il messaggio di errore dell'ultima system call
  che ha restituito un errore.
  errExit termina il processo chiamante
*/
void errExit(const char *msg);

#endif
