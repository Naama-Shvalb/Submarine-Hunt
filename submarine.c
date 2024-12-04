#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

// minimum length of submarines
#define MIN_SUBMARINE_LENGTH 2  
// maximum length of submarines  
#define MAX_SUBMARINE_LENGTH 4 
// number of threads- We saw on the example output there are 4 so we choose 5 to be the maximun option 
#define MAX_THREADS 5            

//the log file
#define LOGFILE "game_log.txt" 
// the file length
#define FILESIZE 40000  


char **board;             
// board size (NxN)
int N;                    
// % of board to cover with submarines
int COVERAGE_PERCENTAGE;  
// for discovering the submarine func
int parts = 0;            
//mutex to sync
pthread_mutex_t board_mutex;

// memory-mapped file
int log_fd;
//pointer to the file
char *log_memory;

// function to print the board to console and the log file
void print_board() {
    //lock the mutex to prevent
    pthread_mutex_lock(&board_mutex);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%c ", board[i][j]);
            strncat(log_memory, &board[i][j], 1);
            strncat(log_memory, " ", 2);
        }
        //adding new line for the console
        printf("\n");
        //adding new line on the log file
        strncat(log_memory, "\n", 2);
    }
    //adding new line on the log file
    strncat(log_memory, "\n", 2);
    //unlock mutex
    pthread_mutex_unlock(&board_mutex);
}

// randomly place submarines on the board
void place_submarines() {
    //the total number of cells on board
    int total_cells = N * N; 
    //the amount of the cells to cover
    int cover_cells = total_cells * COVERAGE_PERCENTAGE / 100;
    

    srand(time(NULL));
    while (parts < cover_cells) {
        //randomly generate submarine length and position
        int length = rand() % (MAX_SUBMARINE_LENGTH - MIN_SUBMARINE_LENGTH + 1) + MIN_SUBMARINE_LENGTH;
        // 0 for horizontal, 1 for vertical
        int direction = rand() % 2;  
        int row = rand() % N;
        int col = rand() % N;

        //flag to check if the submarine can be placed 
        int can_place = 1;

        //check if submarine fits to the board boundries
        for (int i = 0; i < length; i++) {
            int r = row + (direction == 1 ? i : 0);
            int c = col + (direction == 0 ? i : 0);

            if (r >= N || c >= N || board[r][c] != '.') {
                can_place = 0;
                break;
            }
        }
        //in case the requested position is valid- mark the cell as part of submarine- X
        if (can_place) {
            for (int i = 0; i < length; i++) {
                int r = row + (direction == 1 ? i : 0);
                int c = col + (direction == 0 ? i : 0);
                board[r][c] = 'X';
            }
            //update remaining submarine parts count
            parts += length;
        }
    }
}

// Thread function to search the board
void *search(void *arg) {
    // get thread id
    int thread_id = *(int *)arg;
    //free allocated memory for thread id
    free(arg);

    while (1) {
        // lock mutex to access shared board
        pthread_mutex_lock(&board_mutex);

        if (parts == 0) {
            pthread_mutex_unlock(&board_mutex);
            break;
        }

        //randomly choose a cell to search
        int row = rand() % N;
        int col = rand() % N;

        char log_entry[256];
        //if the current cell contains a part of submarine
        if (board[row][col] == 'X') {
            board[row][col] = 'V';
            parts--;
            snprintf(log_entry, sizeof(log_entry), "Thread %d: Found a part of a submarine at [%d,%d]\n", thread_id, row, col);
        } 
        //if the current cell has already been discovered
        else if (board[row][col] == 'V') {
            snprintf(log_entry, sizeof(log_entry), "Thread %d: Skipped at [%d,%d]\n", thread_id, row, col);
        } 
        //if the cuurent cell doesn't conatain a submarine part
        else {
            snprintf(log_entry, sizeof(log_entry), "Thread %d: Missed at [%d,%d]\n", thread_id, row, col);
        }

        //write action to the log file
        strncat(log_memory, log_entry, FILESIZE - strlen(log_memory) - 1);
        //print action to the console
        printf("%s", log_entry);

        //unlock mutex
        pthread_mutex_unlock(&board_mutex);
        //sleep for a short duration to simulate search delay
        usleep(100000);
    }

    return NULL;
}

int main() {
    //getting the board size and the coverage precentage from the user, trust him to enter valid values
    printf("Enter board size: ");
    scanf("%d", &N);
    printf("Enter coverage percentage number between 10-15: ");
    scanf("%d", &COVERAGE_PERCENTAGE);

    //allocate the board with dots
    board = malloc(N * sizeof(char *));
    for (int i = 0; i < N; i++) {
        board[i] = malloc(N * sizeof(char));
        memset(board[i], '.', N * sizeof(char));
    }

    // Initialize mutex
    pthread_mutex_init(&board_mutex, NULL);

    // Initialize the memory-mapped file
    log_fd = open(LOGFILE, O_RDWR | O_CREAT | O_TRUNC, 0666);
    ftruncate(log_fd, FILESIZE);
    log_memory = mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, log_fd, 0);
    //clear the file
    memset(log_memory, 0, FILESIZE);

    // Place submarines on the board
    place_submarines();

    // Print initial board state
    printf("Initial Board:\n");
    strncat(log_memory, "Initial Board:\n", FILESIZE - strlen(log_memory) - 1);
    print_board();

    // Create and start threads
    pthread_t threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&threads[i], NULL, search, id);
    }

    // Wait for all threads to finish
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print final board state after game ends
    printf("\nFinal Board:\n");
    strncat(log_memory, "\nFinal Board:\n", FILESIZE - strlen(log_memory) - 1);
    print_board();

    // Clean up resources
    pthread_mutex_destroy(&board_mutex);
    for (int i = 0; i < N; i++) {
        free(board[i]);
    }
    free(board);

    //manually synchronize the memory-mapped region- tried some ways until we managed to write all to the console and to the file.
    strncat(log_memory, "Game over!\n", FILESIZE - strlen(log_memory) - 1);
    msync(log_memory, FILESIZE, MS_SYNC); 

    // Unmap log file from memory
    munmap(log_memory, FILESIZE);
    // Close log file descriptor
    close(log_fd);

    printf("Game over!\n");
    return 0;
}
