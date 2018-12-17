#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define FAILURE         0
#define SUCCESS         1
#define END             2
#define NOT_COMPLETE    0
#define COMPLETE        1
#define OFF             0
#define ON              1
#define MAX_ASSIGNMENTS 10e6

#define ALL_VARIABLES_OK                0
#define VARIABLE_WITH_IMPOSSIBILITY     1

using namespace std;

typedef struct{
    int l1, c1;
    int l2, c2;
}RESTRICT;

typedef struct{
    short int vflags[10];
    int available;
}CHECK_AHEAD;

typedef struct{
    int D;
    int** board;
    int R;
    RESTRICT* restricts;
    CHECK_AHEAD** check_ahead;
}FUTOSHIKI;

FUTOSHIKI futoshiki;

int numAssignments;
short int checkAhead_f, checkAhead_MVR_f;

int alloc_futoshiki_memory(int, int);
void free_futoshiki_memory();
int backtracking_search();
int recursive_backtracking();
int isComplete();
int select_unassigned_variable(int*, int*);
int isConsistent(int, int, int);
void show_futoshiki_board();

void updateCheckAhead();
int executeCheckAhead();

int main(int argc, char* argv[])
{
    int testCase, result, line, col, value, i;
    int N, D, R;
    int x1, y1, x2, y2;
    clock_t startTime, endTime;
    double timeMeasure;

    //Controle das flags de Heurísticas:
    checkAhead_f = OFF;
    checkAhead_MVR_f = OFF;
    switch(argc){
        case 1:
            break;
        case 2:
            if(strcmp(argv[1], "-va") == 0){
                checkAhead_f = ON;
                checkAhead_MVR_f = OFF;
            }
            break;
        case 3:
            if((strcmp(argv[1], "-va") == 0)&&(strcmp(argv[2], "-mvr") == 0)){
                checkAhead_f = ON;
                checkAhead_MVR_f = ON;
            }
            break;
    }

    scanf("%i",&N);
    for(testCase = 1; testCase <= N; testCase++){

        //Leitura da entrada do caso de teste:
        scanf("%i %i",&D,&R);
        result = alloc_futoshiki_memory(D, R);
        if(!result){
            printf("falha: memoria");
            return 1;
        }
        for(line = 0; line < D; line++){
            for(col = 0; col < D; col++){
                scanf("%i", &value);
                futoshiki.board[line][col] = value;
            }
        }
        for(i = 0; i < R; i++){
            scanf("%i %i %i %i",&x1,&y1,&x2,&y2);
            futoshiki.restricts[i].l1 = x1-1;
            futoshiki.restricts[i].c1 = y1-1;
            futoshiki.restricts[i].l2 = x2-1;
            futoshiki.restricts[i].c2 = y2-1;
        }

        //Execução do caso de teste:

        if(checkAhead_f || checkAhead_MVR_f)
            updateCheckAhead();

        numAssignments = 0;
        printf("%i\n", testCase);
        startTime = clock();
        result = backtracking_search();
        endTime = clock();
        timeMeasure = (double)(endTime - startTime) / CLOCKS_PER_SEC;
        if(result == SUCCESS){
            show_futoshiki_board();
            printf("Numero de Atribuicoes: %i\nTempo da busca: %lf\n\n", numAssignments, timeMeasure);
        }
        else if(result == FAILURE)
            printf("Solucao nao encontrada.\n\n");

        //Término do caso de teste:
        free_futoshiki_memory();
    }
    return 0;
}

int alloc_futoshiki_memory(int D, int R){
    int i, j, k;

    futoshiki.D = D;
    futoshiki.R = R;
    futoshiki.board = (int**)malloc(D*sizeof(int*));
    if(!futoshiki.board)
        return 0;
    for(i = 0; i < D; i++){
        futoshiki.board[i] = (int*)malloc(D*sizeof(int));
        if(!futoshiki.board[i]){
            for(j = 0; j < i; j++)
                free(futoshiki.board[j]);
            free(futoshiki.board);
            return 0;
        }
    }
    futoshiki.restricts = (RESTRICT*)malloc(R*sizeof(RESTRICT));
    if(!futoshiki.restricts){
        for(i = 0; i < D; i++)
            free(futoshiki.board[i]);
        free(futoshiki.board);
        return 0;
    }
    if(checkAhead_f || checkAhead_MVR_f){
        futoshiki.check_ahead = (CHECK_AHEAD**)malloc(futoshiki.D*sizeof(CHECK_AHEAD*));
        if(!futoshiki.check_ahead){
            for(i = 0; i < D; i++)
                free(futoshiki.board[i]);
            free(futoshiki.board);
            return 0;
        }
        for(i = 0; i < D; i++){
            futoshiki.check_ahead[i] = (CHECK_AHEAD*)malloc(futoshiki.D*sizeof(CHECK_AHEAD));
            if(!futoshiki.check_ahead[i]){
                for(i = 0; i < D; i++)
                    free(futoshiki.board[i]);
                free(futoshiki.board);
                for(j = 0; j < i; j++)
                    free(futoshiki.check_ahead[j]);
                free(futoshiki.check_ahead);
                return 0;
            }
        }
        for(i = 0; i < D; i++){
            for(j = 0; j < D; j++){
                futoshiki.check_ahead[i][j].available = D;
                for(k = 1; k <= D; k++)
                    futoshiki.check_ahead[i][j].vflags[k] = 0;
            }
        }
    }
    return 1;
}

void free_futoshiki_memory(){
    int i;

    for(i = 0; i < futoshiki.D; i++)
        free(futoshiki.board[i]);
    free(futoshiki.board);
    futoshiki.D = 0;
    free(futoshiki.restricts);
    futoshiki.R = 0;
    if(checkAhead_f || checkAhead_MVR_f){
        for(i = 0; i < futoshiki.D; i++)
            free(futoshiki.check_ahead[i]);
        free(futoshiki.check_ahead);
    }
}

int backtracking_search(){
    return recursive_backtracking();
}

int recursive_backtracking(){
    int result, lin, col, value;

    if(isComplete())
        return SUCCESS;
    if(checkAhead_f || checkAhead_MVR_f){
        result = executeCheckAhead();
        if(result == VARIABLE_WITH_IMPOSSIBILITY)
            return FAILURE;
    }
    if(numAssignments > MAX_ASSIGNMENTS){
        printf("Numero de atribuicoes excede limite maximo\n");
        return FAILURE;
    }

    result = select_unassigned_variable(&lin, &col);
    if(result == FAILURE)
        return FAILURE;

    for(value = 1; value <= futoshiki.D; value++){

        if(checkAhead_f || checkAhead_MVR_f){
            if(futoshiki.check_ahead[lin][col].vflags[value] == 1)
                continue;
        }

        if(isConsistent(value, lin, col)){
            futoshiki.board[lin][col] = value; //add {var=value} to assignment
            numAssignments++;

            if(checkAhead_f || checkAhead_MVR_f)
                updateCheckAhead();

            result = recursive_backtracking();
            if(numAssignments > MAX_ASSIGNMENTS)
                return FAILURE;
            if(result != FAILURE)
                return SUCCESS;

            futoshiki.board[lin][col] = 0; //remove {var=value} from assignment

            if(checkAhead_f || checkAhead_MVR_f)
                updateCheckAhead();
        }
    }
    return FAILURE;
}

int isComplete(){
    int lin, col, i;
    int domain[10], value;
    int l1, c1, l2, c2;

    //Verificando se todas as posições foram preenchidas:
    for(lin = 0; lin < futoshiki.D; lin++)
        for(col = 0; col < futoshiki.D; col++)
            if(futoshiki.board[lin][col] == 0)
                return NOT_COMPLETE;

    //Verificando se todas as linhas são diferentes:
    for(lin = 0; lin < futoshiki.D; lin++){
        for(i = 1; i < 10; i++)
            domain[i] = 0;
        for(col = 0; col < futoshiki.D; col++){
            value = futoshiki.board[lin][col];
            if(value != 0)
                domain[value]++;
        }
        for(i = 1; i < 10; i++)
            if(domain[i] > 1)
                return NOT_COMPLETE;
    }

    //Verificando se todas as colunas são diferentes:
    for(col = 0; col < futoshiki.D; col++){
        for(i = 1; i < 10; i++)
            domain[i] = 0;
        for(lin = 0; lin < futoshiki.D; lin++){
            value = futoshiki.board[lin][col];
            if(value != 0)
                domain[value]++;
        }
        for(i = 1; i < 10; i++)
            if(domain[i] > 1)
                return NOT_COMPLETE;
    }

    //Verificando as restrições de entrada:
    for(i = 0; i < futoshiki.R; i++){
        l1 = futoshiki.restricts[i].l1;
        c1 = futoshiki.restricts[i].c1;
        l2 = futoshiki.restricts[i].l2;
        c2 = futoshiki.restricts[i].c2;
        if(futoshiki.board[l1][c1] >= futoshiki.board[l2][c2])
            return NOT_COMPLETE;
    }

    return COMPLETE;
}

int select_unassigned_variable(int *lin, int *col){
    int l, c, minAvailable = 0, minLin = 0, minCol = 0;
    short int varFound;

    if(checkAhead_MVR_f){
        varFound = 0;
        for(l = 0; l < futoshiki.D; l++){
            for(c = 0; c < futoshiki.D; c++){
                if((futoshiki.board[l][c] == 0)&&(!varFound)){
                    minLin = l;
                    minCol = c;
                    minAvailable = futoshiki.check_ahead[l][c].available;
                    varFound = 1;
                }
                else{
                    if((futoshiki.board[l][c] == 0)&&
                        (futoshiki.check_ahead[l][c].available < minAvailable)){
                        minLin = l;
                        minCol = c;
                        minAvailable = futoshiki.check_ahead[l][c].available;
                    }
                }
            }
        }
        if(varFound){
            *lin = minLin;
            *col = minCol;
            return SUCCESS;
        }
    }
    else{
        for(l = 0; l < futoshiki.D; l++)
            for(c = 0; c < futoshiki.D; c++)
                if(futoshiki.board[l][c] == 0){
                    *lin = l;
                    *col = c;
                    return SUCCESS;
                }
    }
    return FAILURE;
}

int isConsistent(int value, int lin, int col){
    int l, c, i;
    int domain[10], val;
    int l1, c1, l2, c2;

    //Atribuindo o valor na variável temporariamente
    futoshiki.board[lin][col] = value;

    //Verificando se todas as linhas são diferentes:
    for(i = 1; i < 10; i++)
        domain[i] = 0;
    for(c = 0; c < futoshiki.D; c++){
        val = futoshiki.board[lin][c];
        if(val != 0)
            domain[val]++;
    }
    for(i = 1; i < 10; i++)
        if(domain[i] > 1){
            futoshiki.board[lin][col] = 0; //Restaurando o valor da variável
            return FAILURE;
        }


    //Verificando se todas as colunas são diferentes:
    for(i = 1; i < 10; i++)
        domain[i] = 0;
    for(l = 0; l < futoshiki.D; l++){
        val = futoshiki.board[l][col];
        if(val != 0)
            domain[val]++;
    }
    for(i = 1; i < 10; i++)
        if(domain[i] > 1){
            futoshiki.board[lin][col] = 0; //Restaurando o valor da variável
            return FAILURE;
        }


    //Verificando as restrições de entrada:
    for(i = 0; i < futoshiki.R; i++){
        l1 = futoshiki.restricts[i].l1;
        c1 = futoshiki.restricts[i].c1;
        l2 = futoshiki.restricts[i].l2;
        c2 = futoshiki.restricts[i].c2;
        if((lin == l1)&&(col == c1)){
            if(futoshiki.board[l2][c2] == 0){
                futoshiki.board[lin][col] = 0; //Restaurando o valor da variável
                return SUCCESS;
            }
            else if(value < futoshiki.board[l2][c2]){
                futoshiki.board[lin][col] = 0; //Restaurando o valor da variável
                return SUCCESS;
            }
            futoshiki.board[lin][col] = 0; //Restaurando o valor da variável
            return FAILURE;
        }
        if((lin == l2)&&(col == c2)){
            if(futoshiki.board[l1][c1] == 0){
                futoshiki.board[lin][col] = 0; //Restaurando o valor da variável
                return SUCCESS;
            }
            else if(value > futoshiki.board[l1][c1]){
                futoshiki.board[lin][col] = 0; //Restaurando o valor da variável
                return SUCCESS;
            }
            futoshiki.board[lin][col] = 0; //Restaurando o valor da variável
            return FAILURE;
        }
    }

    futoshiki.board[lin][col] = 0; //Restaurando o valor da variável
    return SUCCESS;
}

void show_futoshiki_board(){
    int lin, col;

    for(lin = 0; lin < futoshiki.D; lin++){
        for(col = 0; col < futoshiki.D; col++){
            if(col <= futoshiki.D-2)
                printf("%i ", futoshiki.board[lin][col]);
            else
                printf("%i\n", futoshiki.board[lin][col]);
        }
    }
}

void updateCheckAhead(){
    int lin, col, i, j, available, l1, c1, l2, c2;
    short int aux[10];

    for(lin = 0; lin < futoshiki.D; lin++){
        for(col = 0; col < futoshiki.D; col++){
            if(futoshiki.board[lin][col] == 0){
                for(i = 1; i <= futoshiki.D; i++)
                    futoshiki.check_ahead[lin][col].vflags[i] = 0;
                futoshiki.check_ahead[lin][col].available = 0;
            }
        }
    }

    for(lin = 0; lin < futoshiki.D; lin++){
        for(col = 0; col < futoshiki.D; col++){
            if(futoshiki.board[lin][col] == 0){
                for(i = 1; i <= futoshiki.D; i++)
                    aux[i] = 0;
                for(j = 0; j < futoshiki.D; j++)
                    if(futoshiki.board[lin][j] > 0)
                        aux[futoshiki.board[lin][j]] = 1;
                for(i = 0; i < futoshiki.D; i++)
                    if(futoshiki.board[i][col] > 0)
                        aux[futoshiki.board[i][col]] = 1;
                for(i = 1, available = 0; i <= futoshiki.D; i++)
                    if(aux[i] == 0)
                        available++;
                for(i = 1; i <= futoshiki.D; i++)
                    futoshiki.check_ahead[lin][col].vflags[i] = aux[i];
                futoshiki.check_ahead[lin][col].available = available;
            }
        }
    }

    for(i = 0; i < futoshiki.R; i++){
        l1 = futoshiki.restricts[i].l1;
        c1 = futoshiki.restricts[i].c1;
        l2 = futoshiki.restricts[i].l2;
        c2 = futoshiki.restricts[i].c2;

        if((futoshiki.board[l1][c1] == 0)&&(futoshiki.board[l2][c2] > 0)){
            for(j = 1; j <= futoshiki.D; j++)
                if((futoshiki.check_ahead[l1][c1].vflags[j] == 0)&&(j > futoshiki.board[l2][c2])){
                    futoshiki.check_ahead[l1][c1].vflags[j] = 1;
                    futoshiki.check_ahead[l1][c1].available--;
                }
        }
        else{
            if((futoshiki.board[l1][c1] > 0)&&(futoshiki.board[l2][c2] == 0)){
                for(j = 1; j <= futoshiki.D; j++)
                    if((futoshiki.check_ahead[l2][c2].vflags[j] == 0)&&(j < futoshiki.board[l1][c1])){
                        futoshiki.check_ahead[l2][c2].vflags[j] = 1;
                        futoshiki.check_ahead[l2][c2].available--;
                    }
            }
        }
    }
}

int executeCheckAhead(){
    int lin, col;

    for(lin = 0; lin < futoshiki.D; lin++)
        for(col = 0; col < futoshiki.D; col++)
            if((futoshiki.check_ahead[lin][col].available == 0)&&(futoshiki.board[lin][col] == 0))
                return VARIABLE_WITH_IMPOSSIBILITY;
    return ALL_VARIABLES_OK;
}
