#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>

#define SCORES "scores.txt"
#define USERS "users.txt"

typedef struct {
    char name[50];
    char surname[50];
    char username[20];
    char password[20];
} User;

typedef struct {
    char username[20];
    int score;
} ScorePlayer;

typedef struct {
    int score;
    int E;
    int e;
    int P;
    int p;
} Player;

int timerCountdown(time_t startTime, int duration);
char** createBoard(int N, int M, int *playerX, int *playerY);
void printBoard(char **board, int N, int M, Player *player);
void freeBoard(char **board, int N);
int movePlayer(char **board, Player *player, int *playerX, int *playerY, char direction);
void registerUser(User *users, int *numUsers);
int loginUser(const User *users, int numUsers, char *loggedInUser);
void displayTopScores();
int isValidMove(char **board, int newX, int newY, int N, int M);
int autoPlay(char **board, int N, int M, Player *player, int *playerX, int *playerY);
void saveUsersToFile(const User *users, int numUsers);
void loadUsersFromFile(User *users, int *numUsers);
void saveMapToFile(char **board, int N, int M);
char** loadMapFromFile(int *N, int *M, int *playerX, int *playerY);

int main() {
    int N = 10, M = 10;

    User users[100];
    int numUsers = 0;
    char loggedInUser[20];
    time_t startTime = time(NULL);
    int duration = 300;
    loadUsersFromFile(users, &numUsers);

    int loginResult = -1;
    do {
        printf("\n1. Register\n2. Login\n3. Exit\n");
        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                registerUser(users, &numUsers);
                saveUsersToFile(users, numUsers);
                break;
            case 2:
                loginResult = loginUser(users, numUsers, loggedInUser);
                break;
            case 3:
                saveUsersToFile(users, numUsers);
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }while (loginResult != 0);

    int playerX, playerY;
    Player player = {0, 0, 0, 0, 0};

    int autoplayModeActive = 0;

    printf("\n1. Create a new map\n2. Load a saved map\n");
    int mapChoice;
    scanf("%d", &mapChoice);

    char **board;
    if (mapChoice == 1) {
        printf("Enter the value of N: ");
        scanf("%d", &N);
        printf("Enter the value of M: ");
        scanf("%d", &M);
        board = createBoard(N, M, &playerX, &playerY);
        int x;
        printf("Save the map?(1 for yes  0 for no): ");
        scanf("%d",&x);
        if(x == 1){
            saveMapToFile(board,N,M);
        }
    } else if (mapChoice == 2) {
        board = loadMapFromFile(&N, &M, &playerX, &playerY);
    } else {
        printf("Invalid choice.");
        return 1;
    }

    board[playerX][playerY] = 'X';

    int gameState = 1;
    printBoard(board, N, M, &player);

    printf("Do you want to autoPlay? (1 for Yes, 0 for No): ");
    scanf("%d", &autoplayModeActive);

    while (gameState) {
        if (autoplayModeActive) {
            autoPlay(board, N, M, &player, &playerX, &playerY);
        } else {
            char direction;
            printf("Enter direction via the arrow keys: ");
            char key = _getch();

            switch (key) {
                case 72:
                    direction = 'w';
                    break;
                case 80:
                    direction = 's';
                    break;
                case 75:
                    direction = 'a';
                    break;
                case 77:
                    direction = 'd';
                    break;
                default:
                    printf("Invalid key.");
                    continue;
            }

            gameState = movePlayer(board, &player, &playerX, &playerY, direction);
            if (!timerCountdown(startTime, duration)) {
                gameState = 0;
            }
        }

        if (gameState == 1) {
            printBoard(board, N, M, &player);
        } else {
            FILE *scoreFile = fopen(SCORES, "a");
            fprintf(scoreFile, "%s %d\n", loggedInUser, player.score);
            fclose(scoreFile);

            freeBoard(board, N);

            int showTopScoresChoice;
            printf("Do you want to display top scores? (1 for Yes, 0 for No): ");
            scanf("%d", &showTopScoresChoice);
            if (showTopScoresChoice == 1) {
                displayTopScores();
            }
        }
    }

    return 0;
}

int timerCountdown(time_t startTime, int duration) {
    time_t currentTime = time(NULL);
    int elapsedTime = difftime(currentTime, startTime);
    int remainingTime = duration - elapsedTime;

    if (remainingTime <= 0) {
        printf("Time's up! Game over.\n");
        return 0;
    }

    printf("Time remaining: %d seconds\n", remainingTime);
    return 1;
}

char** createBoard(int N, int M, int *playerX, int *playerY){
    int i,j;

    char **board = (char **)malloc(N * sizeof(char*));
    if(board == NULL) {
        printf("Memory allocation failed\n");
    }

    for(i = 0; i < N; i++) {
        board[i] = (char *)malloc(M * sizeof(char));
        if(board[i] == NULL) {
            printf("Memory allocation failed\n");
        }
    }

    srand(time(0));

    int countZero = 0;
    for(i = 0; i < N; i++) {
        for(j = 0; j < M; j++) {
            board[i][j] = (rand() % 2) ? '1' : '0';
            if(board[i][j] == '0') countZero++;
        }
    }

    if(countZero < 6) {
        return board;
    }

    int placeG = 0;
    int placeC = 0;

    while(placeG == 0) {
        int x = rand() % N;
        int y = rand() % M;
        if(board[x][y] == '0') {
            board[x][y] = 'G';
            *playerX = x;
            *playerY = y;
            countZero--;
            placeG = 1;
        }
    }

    while(placeC == 0 ) {
        int x = rand() % N;
        int y = rand() % M;
        if(board[x][y] == '0') {
            board[x][y] = 'C';
            countZero--;
            placeC = 1;
        }
    }
    int countK = 0;
    while (countK < N*M/40) {
        int x = rand() % N;
        int y = rand() % M;
        if (board[x][y] == '0' || board[x][y] == '1') {
            board[x][y] = 'K';
            countK++;
        }
    }
    int k,l;
    char symbols[] = {'E', 'e', 'P', 'p'};

    for(k = 0; k < 4; k++) {
        int num = rand() % countZero + 1;
        countZero -= num;
        for(l = 0; l < num; l++) {
            int placeParticle = 0;
            while(placeParticle == 0) {
                int x = rand() % N;
                int y = rand() % M;
                if(board[x][y] == '0' || board[x][y] == '1') {
                    board[x][y] = symbols[k];
                    placeParticle = 1;
                }
            }
        }
    }

    return board;
}

void printBoard(char **board, int N, int M, Player *player){
    int i,j;
    printf("\n");
    for(i = 0; i < N; i++) {
        for(j = 0; j < M; j++)
            printf("%c ", board[i][j]);
        printf("\n");
    }
    printf("Collected particles: p+ %d, p- %d, e+ %d, e- %d\n", player->P, player->p, player->E, player->e);
}

void freeBoard(char **board, int N) {
    int i;
    for(i = 0; i < N; i++) {
        free(board[i]);
    }
    free(board);
}

int movePlayer(char **board, Player *player, int *playerX, int *playerY, char direction) {
    int newX = *playerX;
    int newY = *playerY;

    if (direction == 'w') {
        newX--;
    } else if (direction == 's') {
        newX++;
    } else if (direction == 'a') {
        newY--;
    } else if (direction == 'd') {
        newY++;
    }

    if (board[newX][newY] != '1') {
        if (board[newX][newY] == 'C') {
            player->score += (player->E < player->p) ? player->E : player->p;
            printf("You've exited the maze! Antimatter count is %d.\n", player->score);
            return 0;
        }

        if (board[newX][newY] == 'K'){
            printf("You've entered a black hole.. Antimatter count is %d.\n", player->score);
            return 0;
        }

        if (board[newX][newY] == 'E') player->E++;
        if (board[newX][newY] == 'e') player->e++;
        if (board[newX][newY] == 'P') player->P++;
        if (board[newX][newY] == 'p') player->p++;

        if (player->E > 0 && player->e > 0) { player->E--; player->e--; }
        if (player->P > 0 && player->p > 0) { player->P--; player->p--; }


        board[*playerX][*playerY] = '0';
        *playerX = newX;
        *playerY = newY;
        board[*playerX][*playerY] = 'X';
    }
    return 1;
}
void registerUser(User *users, int *numUsers) {
    if (*numUsers == 100) {
        printf("Maximum user limit reached.\n");
        return;
    }

    User newUser;
    printf("Enter your name: ");
    scanf("%s", newUser.name);
    printf("Enter your surname: ");
    scanf("%s", newUser.surname);
    printf("Enter a username: ");
    scanf("%s", newUser.username);

    for (int i = 0; i < *numUsers; i++) {
        if (strcmp(users[i].username, newUser.username) == 0) {
            printf("Username already exists. Please enter a different one.\n");
            return;
        }
    }

    printf("Enter a password: ");
    scanf("%s", newUser.password);

    users[*numUsers] = newUser;
    (*numUsers)++;
    printf("Registration successful!\n");
}

int loginUser(const User *users, int numUsers, char *loggedInUser) {
    char username[20];
    char password[20];
    printf("Enter your username: ");
    scanf("%s", username);
    printf("Enter your password: ");
    scanf("%s", password);

    for (int i = 0; i < numUsers; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            printf("Login successful!\n");
            strcpy(loggedInUser, username);
            return 0;
        }
    }

    printf("Invalid username or password. Please try again.\n");
    return -1;
}

void displayTopScores() {
    FILE *scoreFile = fopen(SCORES, "r");
    if (scoreFile == NULL) {
        printf("No scores available.\n");
        return;
    }

    ScorePlayer scores[100];
    int numScores = 0;

    while (fscanf(scoreFile, "%s %d", scores[numScores].username, &scores[numScores].score) == 2) {
        numScores++;
        if (numScores == 100) {
            break;
        }
    }

    fclose(scoreFile);

    for (int i = 0; i < numScores - 1; i++) {
        for (int j = 0; j < numScores - i - 1; j++) {
            if (scores[j].score < scores[j + 1].score) {
                ScorePlayer temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }

    printf("Top 5 Scores:\n");
    for (int i = 0; i < numScores && i < 5; i++) {
        printf("%d. %s - %d\n", i + 1, scores[i].username, scores[i].score);
    }
}

int isValidMove(char **board, int newX, int newY, int N, int M) {
    return newX >= 0 && newX < N && newY >= 0 && newY < M && board[newX][newY] != '1' && board[newX][newY] != 'K';
}

int autoPlay(char **board, int N, int M, Player *player, int *playerX, int *playerY) {
    int newX, newY;

    do {
        int randomMove = rand() % 4;
        newX = *playerX;
        newY = *playerY;

        if (randomMove == 0) {
            newX--;
        } else if (randomMove == 1) {
            newX++;
        } else if (randomMove == 2) {
            newY--;
        } else if (randomMove == 3) {
            newY++;
        }
    } while (!isValidMove(board, newX, newY, N, M));

    char direction;
    if (newX < *playerX) {
        direction = 'w';
    } else if (newX > *playerX) {
        direction = 's';
    } else if (newY < *playerY) {
        direction = 'a';
    } else {
        direction = 'd';
    }

    movePlayer(board, player, playerX, playerY, direction);
    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = 500000000;
    nanosleep(&delay, NULL);

    if (board[*playerX][*playerY] == 'C') {
        printf("You've exited the maze! Antimatter count is %d.\n", player->score);
        return 0;
    }

    return 1;
}




void saveUsersToFile(const User *users, int numUsers) {
    FILE *usersFile = fopen(USERS, "w");
    if (usersFile != NULL) {
        for (int i = 0; i < numUsers; i++) {
            fprintf(usersFile, "%s %s %s %s\n", users[i].name, users[i].surname,
                    users[i].username, users[i].password);
        }
        fclose(usersFile);
    } else {
        printf("Error saving user data to file.\n");
    }
}

void loadUsersFromFile(User *users, int *numUsers) {
    FILE *usersFile = fopen(USERS, "r");
    if (usersFile != NULL) {
        while (fscanf(usersFile, "%s %s %s %s", users[*numUsers].name, users[*numUsers].surname,
                      users[*numUsers].username, users[*numUsers].password) == 4) {
            (*numUsers)++;
        }
        fclose(usersFile);
    } else {
        printf("Error loading user data from file.\n");
    }
}

void saveMapToFile(char **board, int N, int M) {
    char mapFileName[100];
    printf("Enter the map file name to save: ");
    scanf("%s", mapFileName);

    FILE *mapFile = fopen(mapFileName, "w");
    if (mapFile != NULL) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                fprintf(mapFile, "%c ", board[i][j]);
            }
            fprintf(mapFile, "\n");
        }
        fclose(mapFile);
        printf("Map saved successfully.\n");
    } else {
        printf("Error saving map to file.\n");
    }
}

char** loadMapFromFile(int *N, int *M, int *playerX, int *playerY) {
    char mapFileName[100];
    printf("Enter the map file name to load: ");
    scanf("%s", mapFileName);

    FILE *mapFile = fopen(mapFileName, "r");
    if (mapFile != NULL) {
        fscanf(mapFile, "%d %d", N, M);

        char **board = (char **)malloc((*N) * sizeof(char*));
        if (board == NULL) {
            printf("Memory allocation failed\n");
            exit(1);
        }

        int startFound = 0;

        for (int i = 0; i < *N; i++) {
            board[i] = (char *)malloc((*M) * sizeof(char));
            if (board[i] == NULL) {
                printf("Memory allocation failed\n");
                exit(1);
            }

            for (int j = 0; j < *M; j++) {
                fscanf(mapFile, " %c", &board[i][j]);
                if (board[i][j] == 'G') {
                    *playerX = i;
                    *playerY = j;
                    startFound = 1;
                }
            }
        }

        if (!startFound) {
            printf("No starting point found in the map.\n");
            fclose(mapFile);
            freeBoard(board, *N);
            return NULL;
        }

        fclose(mapFile);
        printf("Map loaded successfully.\n");
        return board;
    } else {
        printf("Can not load map from file.\n");
        return NULL;
    }
}