#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_LEN 256
#define MAX_LEN 256
#define VAL_LEN 10

int symIndex=0;

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

typedef struct operator
{
    char mnemonic[VAL_LEN];
    int format;
    int opcode;
} operator;

operator OPTAB[]={
    {"ADD",3,0x18},
    {"ADDF",3,0x58},
    {"ADDR",2,0x90},
    {"AND",3,0x40},
    {"CLEAR",2,0xB4},
    {"COMP",3,0x28},
    {"COMPF",3,0x88},
    {"COMPR",2,0xA0},
    {"DIV",3,0x24},
    {"DIVF",3,0x64},
    {"DIVR",2,0x9C},
    {"FIX",1,0xC4},
    {"FLOAT",1,0xC0},
    {"HIO",1,0xF4},
    {"J",3,0x3C},
    {"JEQ",3,0x30},
    {"JGT",3,0x34},
    {"JLT",3,0x38},
    {"JSUB",3,0x48},
    {"LDA",3,0x00},
    {"LDB",3,0x68},
    {"LDCH",3,0x50},
    {"LDF",3,0x70},
    {"LDL",3,0x08},
    {"LDS",3,0x6C},
    {"LDT",3,0x74},
    {"LDX",3,0x04},
    {"LPS",3,0xD0},
    {"MUL",3,0x20},
    {"MULF",3,0x60},
    {"MULR",2,0x98},
    {"NORM",1,0xC8},
    {"OR",3,0x44},
    {"RD",3,0xD8},
    {"RMO",2,0xAC},
    {"RSUB",3,0x4C},
    {"SHIFTL",2,0xA4},
    {"SHIFTR",2,0xA8},
    {"SIO",1,0xF0},
    {"SSK",3,0xEC},
    {"STA",3,0x0C},
    {"STB",3,0x78},
    {"STCH",3,0x54},
    {"STF",3,0x80},
    {"STI",3,0xD4},
    {"STL",3,0x14},
    {"STS",3,0x7C},
    {"STSW",3,0xE8},
    {"STT",3,0x84},
    {"STX",3,0x10},
    {"SUB",3,0x1C},
    {"SUBF",3,0x5C},
    {"SUBR",2,0x94},
    {"SVC",2,0xB0},
    {"TD",3,0xE0},
    {"TIO",1,0xF8},
    {"TIX",3,0x2C},
    {"TIXR",2,0xB8},
    {"WD",3,0xDC}
};


symbol SYMBOL_TABLE[MAX_LEN];

intermediate INTERMEDIATE_DATA[MAX_LEN];

int findLabel(char label[])
{
    for(int i=0; i<symIndex; i++)
    {
        if(!strcmp(label, SYMBOL_TABLE[i].statement))
        {
            return 1;
        }
    }
    return 0;
}

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
        if(strlen(buf)==0) 
        {
            currentLine++;
            continue;
        }
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
                isNotStart=1;
                int size_real=i;
                if(hasComment) size_real=i-1;
                int isLabelSymbol=findLabel(temp[0]);
                
                if(size_real==3)
                {
                    if(isLabelSymbol)
                    {
                        //error
                    }
                    else
                    {
                        strcpy(SYMBOL_TABLE[symIndex].statement, temp[0]);
                        sprintf(SYMBOL_TABLE[symIndex].address, "%04x", currentLoc);
                        symIndex++;
                    }
                }

                if(size_real==2)
                {

                }
                if(size_real==1)
                {

                }
            }
            
            
        }
        currentLine++;
    }

    return 0;
}