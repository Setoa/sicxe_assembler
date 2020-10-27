#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_LEN 256
#define MAX_LEN 256
#define VAL_LEN 10

int LOCCTR[MAX_LEN];

typedef struct symbol{
    char statement[VAL_LEN];
    char address[VAL_LEN];
} symbol;

typedef struct intermediate{
    char loc[VAL_LEN];
    char label[VAL_LEN];
    char operator[VAL_LEN];
    char operand[VAL_LEN];
    char comment[32];
} intermediate;

symbol SYMBOL_TABLE[MAX_LEN];

intermediate INTERMEDIATE_DATA[MAX_LEN];

void writeImmediateFile(FILE* fpw, int hasComment, char temp[][10], int size, int loc)
{
    int size_real=size;
    char locstr[VAL_LEN];
    sprintf(locstr, "%04x", loc);
    if(hasComment) size_real=size-1;
    fprintf(fpw, "%s    ", locstr);
    if(size_real==3)
    {
        fprintf(fpw,"%s %s  %s", temp[0], temp[1], temp[2]);
    }
    else if(size_real==2)
    {
        fprintf(fpw,"   %s  %s", temp[0], temp[1]);
    }
    else if(size_real==1)
    {
        fprintf(fpw,"   %s  ", temp[0]);
    }
    if(hasComment)
    {
        fprintf(fpw, "   %s\n",temp[size_real]);
    }
    else
    {
        fprintf(fpw, "\n");
    }
}




int main(int argc, char* argv[])
{
    FILE* fpr;
    FILE* fpw;
    char buf[BUF_LEN]={ 0 };
    char filename[32];
    int isFirstLine=1;
    int currentLine=0;
    int symIndex=0;
    int interIndex=0;
    int currentLoc=0; //this is decimal, need to convert to hax
    int isNotStart=0;

    if(argc <= 1)
    {
        printf("No input.\n");
        exit(1);
    }
    strcpy(filename, argv[1]);
    fpr=fopen(filename, "r");
    fpw=fopen("intermediate", "w");
    if(fpr==NULL)
    {
        printf("Can't read input file");
        exit(1);
    }

    // pass 1
    while(fgets(buf, BUF_LEN, fpr)!=NULL)
    {
        int i=0;
        if(strlen(buf)==0) break;
        if(buf[0]=='.')
        {
            fprintf(fpw, "  %s\n", buf);
        }
        else
        {
            //parsing
            char* ptr;
            char temp[3][10];
            ptr=strtok(buf, " \t");
            int hasComment=0;

            while(ptr != NULL)
            {
                strcpy(temp[i++],ptr);
                ptr=strtok(NULL, " \t");
                if(ptr[0]=='.')
                {
                    strcat(ptr,strtok(NULL, ""));
                    strcpy(temp[i++],ptr);
                    hasComment=1;
                    break;
                }
            }
            
            if(isFirstLine)
            {
                if(!strcmp(temp[0], "START"))
                {
                    currentLoc=atoi(temp[1]);
                    writeImmediateFile(fpw, hasComment, temp, i, currentLoc);
                    strcpy(SYMBOL_TABLE[symIndex].statement, temp[0]);
                    strcpy(SYMBOL_TABLE[symIndex].address, temp[1]);
                    symIndex++;
                }
                else if(!strcmp(temp[1], "START"))
                {
                    currentLoc=atoi(temp[2]);
                    writeImmediateFile(fpw, hasComment, temp, i, currentLoc);
                    strcpy(SYMBOL_TABLE[symIndex].statement, temp[0]);
                    strcpy(SYMBOL_TABLE[symIndex].address, temp[2]);
                    symIndex++;
                    strcpy(SYMBOL_TABLE[symIndex].statement, temp[1]);
                    strcpy(SYMBOL_TABLE[symIndex].address, temp[2]);
                    symIndex++;
                }
                else
                {
                    currentLoc=0;
                    isNotStart=1;
                }
                isFirstLine=0;
            }

            if(isFirstLine==0 && isNotStart==1)
            {
                
            }
            
            
        }
        currentLine++;
    }

    return 0;
}