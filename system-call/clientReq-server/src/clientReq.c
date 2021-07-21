#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>

#include "../inc/constant.h"
#include "../inc/errExit.h"
#include "../inc/request_response.h"

char *baseClientFifoPath = "/tmp/client_fifo";
char *serverFifoPath = "/tmp/server_fifo";

int main (int argc, char *argv[]) {

    char clientFifoPath [100];
    
    struct Request request;

    printf("Benvenuto a ClientReq!\n");
    printf("Offro i seguenti servizi: Stampa, Salva e Invia!\n");
    printf("Inserire il codice identificativo: ");
    scanf("%s", request.user_code);
    
    printf("Inserire il servizio richiesto: ");
    scanf("%s", request.service);

    request.pid = getpid();              						

    //Creazione FIFO Client
    sprintf(clientFifoPath, "%s_%d", baseClientFifoPath, request.pid);	//clientFifoPath mette baseClient --> associato al pid
    									//piu richieste del cient ognugna pid diverso
    if(mkfifo(clientFifoPath, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
        errExit("Creation fifo client failed");
    
    //Connessione alla FIFO Server
    int serverFifoFD = open(serverFifoPath, O_WRONLY);
    if(serverFifoFD == -1) {
        errExit("Connecting to server FIFO has failed");
    }
    //Invio dei dati al Server attraverso FIFO SERVER
    if(write(serverFifoFD, &request, sizeof(struct Request)) 		
        != sizeof(struct Request)) {			
        errExit("Write Request in FIFO Server has failed");
    }

    //Aspetto e leggo la risposta in FIFO CLIENT
    printf("Messaggio inviato, attesa di risposta... \n");
    int clientFifoPathFD = open(clientFifoPath, O_RDONLY);
    if(clientFifoPathFD == -1) {
        errExit("Read client FIFO has failed");
    }

    struct Response response;
    int bufferRead = -1;		//-1 perchè non restistuisce nulla, non inizializzati 	magari la sizof potrebbe essere 0				
    bufferRead = read(clientFifoPathFD, &response, sizeof(struct Response));
    if(bufferRead != sizeof(struct Response)) {
        errExit("Response Fifo Client read failed");
    }
    //RESPONSE
    if(response.key == -1) {
        printf("Servizio richiesto al server non disponibile\n");
    } else if(response.key == 0) {
        printf("Chiave non generata. La memoria e` piena.\n");
    } else {
        printf("Chiave rilasciata del server: %li \n", response.key);
    }
    //Chiusura e unlink del FD e del file
    if(close(serverFifoFD) || close(clientFifoPathFD) == -1)
        errExit("Error closing FIFOs");
    
    if(unlink(clientFifoPath) != 0){
        errExit("Unlink client fifo failed");
    }
    
    return 0;
}
