//producer.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define ROWS 4
#define COLS 4
#define SHM_KEY 1112

int main() {
    // Shared memory layout:
    // Matrix A: shm[0] to shm[15]     (16 elements)
    // Matrix B: shm[16] to shm[31]    (16 elements)
    // Matrix PRODUCT: shm[32] to shm[47] (16 elements)
    // Flags: shm[48] = producer_ready, shm[49] = consumer_done

    // Producer creates the shared memory
    int shmid = shmget(SHM_KEY, sizeof(int) * 50, IPC_CREAT | 0666);
    if (shmid == -1) { perror("shmget"); exit(1); }

    int *shm = (int *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) { perror("shmat"); exit(1); }

    // Initialize flags
    shm[48] = 0; // producer_ready (0=not ready, 1=ready for consumer)
    shm[49] = 0; // consumer_done (0=not done, 1=done)

    printf("Producer: Shared memory created successfully!\n");

    // Input Matrix A
    printf("Enter 16 elements for Matrix A (4x4) row by row:\n");
    for (int i = 0; i < 16; i++) {
        scanf("%d", &shm[i]);
    }

    // Input Matrix B
    printf("Enter 16 elements for Matrix B (4x4) row by row:\n");
    for (int i = 0; i < 16; i++) {
        scanf("%d", &shm[16 + i]);
    }

    printf("Producer: Matrices input completed. Computing first two rows of product matrix (rows 0-1)...\n");

    // Producer computes first two rows of the product matrix (rows 0-1)
    for (int i = 0; i < 2; i++) { // First two rows
        for (int j = 0; j < COLS; j++) {
            int sum = 0;
            for (int k = 0; k < COLS; k++) {
                // A[i][k] * B[k][j]
                sum += shm[i * COLS + k] * shm[16 + k * COLS + j];
            }
            shm[32 + i * COLS + j] = sum; // Store in PRODUCT matrix
        }
    }

    printf("Producer: First two rows of product matrix completed.\n");

    // Signal consumer that it can start working
    shm[48] = 1;
    printf("Producer: Signal sent to consumer. Waiting for consumer to finish...\n");

    // Producer waits for consumer to set the completion flag
    while (shm[49] == 0) {
        // busy wait for consumer to complete and set flag
    }

    printf("Producer: Consumer completion flag detected!\n");

    // Print Matrix A
    printf("\nMatrix A (4x4):\n");
    for (int i = 0; i < 16; i++) {
        printf("%d ", shm[i]);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }

    // Print Matrix B
    printf("\nMatrix B (4x4):\n");
    for (int i = 0; i < 16; i++) {
        printf("%d ", shm[16 + i]);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }

    // Print the complete result matrix
    printf("\nFinal PRODUCT Matrix (4x4):\n");
    for (int i = 0; i < 16; i++) {
        printf("%d ", shm[32 + i]);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }

    printf("\nProducer: Matrix multiplication completed successfully!\n");

    // Producer detaches and deletes shared memory
    shmdt(shm);
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl - delete");
    } else {
        printf("Producer: Shared memory deleted successfully!\n");
    }

    return 0;
}

---------------------------------------------------------------------------------------------------------------------------

//consumer.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define ROWS 4
#define COLS 4
#define SHM_KEY 1112

int main() {
    printf("Consumer: Starting and waiting for producer...\n");

    // Consumer accesses the shared memory created by producer
    int shmid = shmget(SHM_KEY, sizeof(int) * 50, 0666);
    if (shmid == -1) {
        perror("shmget - Make sure producer is running first!");
        exit(1);
    }

    int *shm = (int *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) { perror("shmat"); exit(1); }

    printf("Consumer: Successfully attached to shared memory!\n");
    printf("Consumer: Waiting for producer to complete first two rows...\n");

    // WAIT for producer to finish its work and signal readiness
    while (shm[48] == 0) {
        // busy wait for producer to complete first two rows and set ready flag
    }

    printf("Consumer: Producer ready signal received! Computing last two rows of product matrix (rows 2-3)...\n");

    // Consumer computes last two rows of the product matrix (rows 2-3)
    for (int i = 2; i < 4; i++) { // Last two rows
        for (int j = 0; j < COLS; j++) {
            int sum = 0;
            for (int k = 0; k < COLS; k++) {
                // A[i][k] * B[k][j]
                sum += shm[i * COLS + k] * shm[16 + k * COLS + j];
            }
            shm[32 + i * COLS + j] = sum; // Store in PRODUCT matrix
        }
    }

    printf("Consumer: Last two rows of product matrix completed!\n");

    // Consumer sets the completion flag
    shm[49] = 1;
    printf("Consumer: Completion flag set! Producer can now print results.\n");

    shmdt(shm);
    printf("Consumer: Detached from shared memory. Exiting...\n");
    return 0;
}

---------------------------------------------------------------------------------------------------------------------------------------

//sender.c

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdlib.h>

struct message{
    long type;
    int data;
};

int main(){
    int running=1;
    int msgid;
    struct message msg;
    msgid = msgget(1099,IPC_CREAT|IPC_EXCL|0666);
    if(msgid==-1){
        perror("Error in creating queue...\n");
        exit(0);
    }
    while(running){
        printf("Enter data: ");
        scanf("%d",&msg.data);
        printf("Enter data's type: ");
        scanf("%ld",&msg.type);
        if(msgsnd(msgid,&msg,sizeof(int),0)==-1){perror("Message not sent!");exit(0);}
        if(msg.data==-1){
            running=0;
        }
    }
    return 0;
}

----------------------------------------------------------------------------------------------------------------------

//receiver.c

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<math.h>
#include<stdlib.h>

struct message{
    long type;
    int data;
};

int main(){
    int running=1;
    int msgid;
    struct message msg;
    long square=1;
    long cube=2;
    msgid = msgget(1099,IPC_CREAT|0666);
    if(msgid==-1){
        perror("Error in creating queue...\n");
        exit(0);
    }
    while(running){
        int check=msgrcv(msgid,&msg,sizeof(int),square,0);
        if(check==-1){perror("Message not received!");exit(0);}
        if(check==sizeof(int)){printf("Data received...\nData : %d\nSquare of the data: %d\n",msg.data,(int)pow(msg.data,2));}
        int check1=msgrcv(msgid,&msg,sizeof(int),cube,0);
        if(check1==-1){perror("Message not received!");exit(0);}
        if(check1==sizeof(int)){printf("Data received...\nData : %d\nCube of the data: %d\n",msg.data,(int)pow(msg.data,3));}
        if(msg.data==-1){
            running=0;
        }
    }
    if(msgctl(msgid,IPC_RMID,0)==-1){perror("Error in removing queue!");}
    return 0;
}
