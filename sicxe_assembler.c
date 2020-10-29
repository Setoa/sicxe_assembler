#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define BUF_LEN 256
#define MAX_LEN 256
#define VAL_LEN 10
#define OPTAB_LEN 59
#define REGTAB_LEN 9

int symIndex=0;

int LOCCTR[MAX_LEN];

typedef struct symbol{
    char statement[VAL_LEN];
    char address[VAL_LEN];
} symbol;

typedef struct interm{

    char loc[VAL_LEN];
    char label[VAL_LEN];
    char opt[VAL_LEN];
    char operand[VAL_LEN];
    char comment[32];
} interm;

typedef struct listing
{
    char loc[VAL_LEN];
    char label[VAL_LEN];
    char opt[VAL_LEN];
    char operand[VAL_LEN];
    char comment[32];
    char objcode[VAL_LEN];
} listing;


typedef struct opt
{
    char mnemonic[VAL_LEN];
    int format;
    int opcode;
} opt;

typedef struct reg
{
    char r[3];
    int num;
} reg;

reg REGTAB[]={
    {"A", 0},
    {"X", 1},
    {"L", 2},
    {"B", 3},
    {"S", 4},
    {"T", 5},
    {"F", 6},
    {"PC", 8},
    {"SW", 9}
};

opt OPTAB[]={
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

//intermediate INTERMEDIATE_DATA[MAX_LEN];

int notopt(char opt[], char opr[])
{

    if(!strcmp(opt,"WORD"))
    {
        return 3;
    }
    else if(!strcmp(opt,"RESW"))
    {
        return 3*atoi(opr);
    }
    else if(!strcmp(opt,"RESB"))
    {
        return atoi(opr);
    }
    else if(!strcmp(opt,"BYTE"))
    {
        if(opr[0]=='C')
        {
            int charcount=0;
            for(int i=1; i<strlen(opr); i++)
            {
                if(opr[i]!='\'')
                {
                    charcount++;
                }
            }
            return charcount;
        }
        else if(opr[0]=='X')
        {
            return 1;
        }
        else
        {
            return 1;
        }
    }
    else if(!strcmp(opt,"BASE"))
    {
        return 0;
    }
    else if(!strcmp(opt,"END"))
    {
        return 0;
    }
    return -1;
}

int findOpt(char opt[])
{
    int len=strlen(opt);
    char* temp=(char*)malloc(sizeof(char)*len);
    int isOpt=0;
    int hasPlus=0;
    int format_size;
    if(opt[0]=='+')
    {
        int j=1;
        for(j; j<len; j++) temp[j-1]=opt[j];
        hasPlus=1;
        temp[j-1]='\0';
    }
    else strcpy(temp, opt);
    int i=0;
    for(i; i<OPTAB_LEN; i++)
    {
        if(!strcmp(OPTAB[i].mnemonic,temp))
        {
            isOpt=1;
            break;
        }
    }
    if(isOpt==0)
    {
        return -1;
    } 
    else
    {
        format_size=OPTAB[i].format;
        if(hasPlus) format_size+=1;
        return format_size;
    }
}

int isOprAllReg(char opr[])
{
    int isReg=0;
    char* ptr=strtok(opr," ,");
    while(ptr!=NULL)
    {
        isReg=0;
        for(int k=0; k<REGTAB_LEN; k++)
        {
            if(!strcmp(ptr,REGTAB[k].r))
            {
                isReg=1;
                break;
            }
        }
        if(isReg==0) return 0;
        else strtok(NULL, " ,");
    }
    return 1;
}

int makeOpcode(char opt[], char opr[])
{
    int len=strlen(opt);
    char* temp=(char*)malloc(sizeof(char)*len);
    int isOpt=0;
    int hasPlus=0;
    int coef=0;
    int opcode=0;
    if(opt[0]=='+')
    {
        int j=1;
        for(j; j<len; j++) temp[j-1]=opt[j];
        hasPlus=1;
        temp[j-1]='\0';
    }
    else strcpy(temp, opt);
    int isReg=0;
    if(opr[0]=='#') coef=1;
    else if(opr[0]=='@') coef=2;
    else
    {
        isReg=isOprAllReg(opr);
        if(isReg) coef=0;
        else coef=3;
    }
    
    int i=0;
    for(i; i<OPTAB_LEN; i++)
    {
        if(!strcmp(OPTAB[i].mnemonic,temp))
        {
            isOpt=1;
            break;
        }
    }
    if(isOpt)
    {
        opcode=strtol(OPTAB[i].opcode, NULL, 16);
        opcode+=coef;
        return opcode;
    } 
    else return -1;
}

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
    int isNoLoc=0;
    int isEnd=0;
    sprintf(locstr, "%04x", loc);
    if(hasComment) size_real=size-1;
    for(int k=0; k<size; k++)
    {
        if(!strcmp(temp[k],"BASE")) isNoLoc=1;
        if(!strcmp(temp[k],"END")) isNoLoc=1;
    }
    if(isNoLoc) fprintf(fpw, "   ");
    else fprintf(fpw, "%s    ", locstr);
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

void printSYMTAB(FILE* fpw, int symIndex)
{
    fprintf(fpw, "SYMBOL\n");
    for(int i=0; i<symIndex; i++)
    {
        fprintf(fpw,"%s %s\n", SYMBOL_TABLE[i].statement, SYMBOL_TABLE[i].address);
    }
}

int main(int argc, char* argv[])
{
    FILE* fpr;
    FILE* fpw;
    char buf[BUF_LEN]={ 0 };
    char filename[32];
    char filename2[32];
    int isFirstLine=1;
    int tempLoc;
    int interIndex=0;
    int currentLoc=0; //this is decimal, need to convert to hax
    int isNotStart=0;
    if(argc < 3)
    {
        printf("No input.\n");
        exit(1);
    }

    strcpy(filename, argv[1]);
    strcpy(filename2, argv[2]);
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
        tempLoc=currentLoc;
        if(strlen(buf)==0) 
        {
            continue;
        }
        if(buf[0]=='.')
        {
            fprintf(fpw, "% %s", buf);
            continue;
        }
        else
        {
            //parsing
            char* ptr;
            char temp[4][VAL_LEN];
            ptr=strtok(buf, " \t\n");
            int hasComment=0;

            while(ptr != NULL)
            {
                if(ptr[0]=='.')
                {
                    strcat(ptr,strtok(NULL, "\n"));
                    strcpy(temp[i++],ptr);
                    hasComment=1;
                    break;
                }
                strcpy(temp[i++],ptr);
                ptr=strtok(NULL, " \t\n");
                
            }
            
            if(isFirstLine)
            {
                if(!strcmp(temp[0], "START"))
                {
                    currentLoc=atoi(temp[1]);
                    strcpy(SYMBOL_TABLE[symIndex].statement, temp[0]);
                    strcpy(SYMBOL_TABLE[symIndex].address, temp[1]);
                    symIndex++;
                }
                else if(!strcmp(temp[1], "START"))
                {
                    currentLoc=atoi(temp[2]);
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
                tempLoc=currentLoc;
            }
            if(isFirstLine==0 && isNotStart==1)
            {

                int size_real=i;
                if(hasComment) size_real=i-1;
                int isLabelSymbol=findLabel(temp[0]);
                
                if(size_real==3)
                {
                    if(isLabelSymbol)
                    {
                        //error
                        exit(1);
                    }
                    else
                    {
                        strcpy(SYMBOL_TABLE[symIndex].statement, temp[0]);
                        if(strcmp(temp[1],"BASE")!=0 && strcmp(temp[1],"END")!=0) sprintf(SYMBOL_TABLE[symIndex].address, "%04x", currentLoc);
                        symIndex++;
                        int add_size=findOpt(temp[1]);
                        if(add_size==-1)
                        {
                            currentLoc+=notopt(temp[1], temp[2]);
                        }
                        else
                        {
                            currentLoc+=add_size;
                        }
                        
                    }
                }

                if(size_real==2)
                {
                    int add_size=findOpt(temp[0]);
                    if(!strcmp(temp[0],"BASE")) add_size=0;
                    if(!strcmp(temp[0],"END")) add_size=0;
                    if(add_size==-1) exit(1);
                    currentLoc+=add_size;
                }
                if(size_real==1)
                {
                    int add_size=findOpt(temp[0]);
                    currentLoc+=add_size;
                }
            }
            isNotStart=1;
            writeImmediateFile(fpw, hasComment, temp, i, tempLoc);
        }
    }
    printSYMTAB(fpw, symIndex);
    fclose(fpw);
    fclose(fpr);
    //pass2
    
    FILE* fpr2;
    FILE* fpw2;
    fpr2=fopen("intermediate", "r");
    fpw2=fopen(filename2, "w");
    symbol SYMTAB[MAX_LEN];
    listing LISTING[MAX_LEN];
    int listLen=0;
    int symLen=0;
    while(fgets(buf, BUF_LEN, fpr2)!=NULL)
    {
        int i=0;
        int isSymbol=0;
        if(strlen(buf)==0) continue;
        char* ptr;
        char temp[6][VAL_LEN];
        ptr=strtok(buf, " \t\n");
        if(!strcmp(ptr, "SYMBOL"))
        {
            isSymbol=1;
            continue;
        }
        if(isSymbol)
        {
            while(ptr!=NULL)
            {
                strcpy(temp[i++],ptr);
                strtok(NULL, " \t\n");
            }
            strcpy(SYMTAB[symLen].statement,temp[0]);
            strcpy(SYMTAB[symLen].address,temp[1]);
            symLen++;
        }
        else
        {
            int hasComment=0;
            while(ptr!=NULL)
            {
                if(ptr[0]=='.')
                {
                    strcat(ptr, strtok(NULL, "\n"));
                    strcpy(temp[i++], ptr);
                    hasComment=1;
                    break;
                }
                strcpy(temp[i++], ptr);
                ptr=strtok(NULL, " \t\n");
            }
            int size_real=i-1;
            if(size_real==4)
            {
                strcpy(LISTING[listLen].loc, temp[0]);
                strcpy(LISTING[listLen].label, temp[1]);
                strcpy(LISTING[listLen].opt, temp[2]);
                strcpy(LISTING[listLen].operand, temp[3]);
                if(hasComment) strcpy(LISTING[listLen].comment, temp[4]);
            }
            else if(size_real==3)
            {
                if(isdigit(temp[0])) //loc present
                {
                    strcpy(LISTING[listLen].loc, temp[0]);
                    strcpy(LISTING[listLen].label, "");
                    strcpy(LISTING[listLen].opt, temp[1]);
                    strcpy(LISTING[listLen].operand, temp[2]);
                }
                else // loc miss
                {
                    strcpy(LISTING[listLen].loc, "");
                    strcpy(LISTING[listLen].label, temp[0]);
                    strcpy(LISTING[listLen].opt, temp[1]);
                    strcpy(LISTING[listLen].operand, temp[2]);
                }
                if(hasComment) strcpy(LISTING[listLen].comment, temp[3]);
            }
            else if(size_real==2)
            {
                if(isdigit(temp[0])) //loc present
                {
                    strcpy(LISTING[listLen].loc, temp[0]);
                    strcpy(LISTING[listLen].label, "");
                    strcpy(LISTING[listLen].opt, temp[1]);
                    strcpy(LISTING[listLen].operand, "");
                }
                else // loc miss
                {
                    strcpy(LISTING[listLen].loc, "");
                    strcpy(LISTING[listLen].label, "");
                    strcpy(LISTING[listLen].opt, temp[0]);
                    strcpy(LISTING[listLen].operand, temp[1]);
                }
                if(hasComment) strcpy(LISTING[listLen].comment, temp[2]);
            }
            strcpy(LISTING[listLen].objcode, "");
            listLen++;
        }
    }
    //LISTING - listLen, SYMTAB - symLen
    //make object code of each line
    
    for(int index=0; index<listLen; index++)
    {
        if(strcmp(LISTING[index].opt, "START")==0) continue;
        else
        {
            //opcode making
            int opcode;
            opcode=makeOpcode(LISTING[index].opt, LISTING[index].operand);
            if(opcode == -1)
            {

            }
            else
            {
                
            }
            
            //
        }
    }
    return 0;
}