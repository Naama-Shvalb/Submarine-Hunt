# Submarine Game

This C program simulates a submarine search game where submarines are randomly placed on a board, and multiple threads attempt to find them. The program uses memory-mapped files to log game activity and thread actions.

## Features

- **Random Submarine Placement**: Submarines of random lengths are placed either horizontally or vertically on the game board.
- **Multithreading**: Multiple threads search the board concurrently for the submarines.
- **Memory-Mapped Logging**: A memory-mapped file is used to log the game progress and thread actions.

## Compilation and Running the Program

### Steps to Compile and Run
we run the program using Docker so you need to run this two orders first:
* **Build the Docker Image**
    In the terminal (inside the project directory), 
    type the following command to build the Docker image:

    ```bash
     docker build -t memory_mapped_io_demo .
    ```
* **Run the Docker Container** After building the image, run the following command to start the container and mount the current project directory:
  ```bash
    docker run -it --rm -v "YOUR_FOLDER_PATH/memory_mapped_io_demo:/app" memory_mapped_io_demo
   ```
Then to run the program and get an output to the next steps:
1. **Clone the Repository or Copy the Source File**
   - Download or clone the repository containing the source code file `submarine.c`.

2. **Open a Terminal and Navigate to the Directory**
   - Use `cd` to navigate to the directory containing `submarine.c`.

3. **Compile the Program**
   - Run the following command to compile the program using GCC:
     ```bash
     gcc -o submarine submarine.c 
     ```

4. **Run the Program**
   - After successful compilation, run the program with the following command:
     ```bash
     ./submarine
     ```
   - The program will prompt for the board size and coverage percentage. Enter valid inputs, and the game will start.

### Example Output
```text
Enter board size (N): 10
Enter coverage percentage: 13
Initial Board:
X X . . . . . . . . 
. . . . X . . . . . 
. . . . X . . . . . 
. . . X X . . . . . 
. . . X X . . . . . 
. . . X X . . . . . 
. . . X . X X . . . 
. . . . . . . . . . 
. . . . . . . . . . 
. . . . . . . . . . 

Thread 3: Missed at [4,0]
Thread 5: Missed at [3,9]
Thread 1: Missed at [6,9]
Thread 2: Skipped at [6,3]
Thread 4: Missed at [4,1]
Thread 5: Missed at [8,4]
Thread 3: Missed at [1,1]
Thread 1: Missed at [0,9]
Thread 5: Missed at [7,3]
Thread 2: Skipped at [1,4]
Thread 4: Found a part of a submarine at [5,4]
.....

Final Board:
V V . . . . . . . .
. . . . V . . . . .
. . . . V . . . . .
. . . V V . . . . .
. . . V V . . . . .
. . . V V . . . . .
. . . V . V V . . .
. . . . . . . . . .
. . . . . . . . . .
. . . . . . . . . .
Game over!
```

## Log File

The game logs the thread actions (submarine hits, misses, or skips) in a file called `game_log.txt`. The log file is memory-mapped, and the log entries are added dynamically as threads search the board.

### Example of part of `game_log.txt`:
```text
Thread 1: Missed at [0,4]
Thread 2: Missed at [6,9]
Thread 3: Missed at [1,6]
Thread 4: Missed at [9,3]
Thread 5: Missed at [0,4]
Thread 1: Missed at [8,9]
Thread 4: Missed at [5,5]
Thread 2: Missed at [1,7]
Thread 3: Missed at [5,0]
Thread 5: Missed at [5,6]
Thread 1: Missed at [1,7]
Thread 4: Missed at [7,1]
Thread 2: Found a part of a submarine at [5,3]
Thread 3: Missed at [0,7]
Thread 5: Missed at [1,2]
Thread 1: Found a part of a submarine at [0,1]
Thread 4: Missed at [8,6]
Thread 2: Missed at [1,9]
Thread 3: Missed at [5,2]
Thread 5: Missed at [5,5]
Thread 1: Found a part of a submarine at [6,3]
Thread 2: Missed at [4,1]
Thread 4: Missed at [1,7]
Thread 3: Missed at [1,6]
Thread 5: Missed at [8,6]
Thread 1: Missed at [4,1]
Thread 2: Missed at [3,1]
Thread 3: Missed at [2,0]
Thread 4: Missed at [6,4]
Thread 5: Missed at [0,7]
Thread 2: Missed at [6,0]
Thread 1: Missed at [9,5]
Thread 3: Missed at [8,2]
Thread 4: Found a part of a submarine at [4,3]
```
Thread 1: Found a part of a submarine at [3, 5] Thread 2: Skipped at [4, 4]
Thread 3: Missed at [2, 7] ...

## Memory-Mapped File Explanation

The program uses `mmap` to create a memory-mapped file for logging. This allows different threads to log their actions in a shared memory space without the need for traditional file locking mechanisms.
 Here's a brief explanation of how `mmap` is implemented:

- **Memory Mapping**: The program uses `mmap` to map the `game_log.txt` file into the process's address space.
 This allows threads to write directly to the memory-mapped region as if it were part of the program's memory.
- **Shared Logging**: The `log_memory` pointer holds the address of the memory-mapped file, and threads use it to append their log entries.
- **Thread Safety**: The `board_mutex` ensures that only one thread can modify the game board at a time to avoid race conditions.

### Code Snippet for `mmap` Usage:
```c
log_fd = open(LOGFILE, O_RDWR | O_CREAT | O_TRUNC, 0666);
ftruncate(log_fd, FILESIZE);
log_memory = mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, log_fd, 0);
memset(log_memory, 0, FILESIZE);

