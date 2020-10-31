#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define BUF_LEN 256
#define MAX_LEN 256
#define VAL_LEN 10
#define OPTAB_LEN 59
#define REGTAB_LEN 9

int symIndex = 0;

typedef struct symbol {
    char statement[VAL_LEN];
    char address[VAL_LEN];
} symbol;

typedef struct listing
{
    char loc[VAL_LEN];
    char label[VAL_LEN];
    char opt[VAL_LEN];
    char operand[VAL_LEN];
    char comment[32];
    char objcode[VAL_LEN];
    int isComment;
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

reg REGTAB[] = {
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

opt OPTAB[] = {
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

int notopt(char opt[], char opr[])
{

    if (!strcmp(opt, "WORD"))
    {
        return 3;
    }
    else if (!strcmp(opt, "RESW"))
    {
        return 3 * atoi(opr);
    }
    else if (!strcmp(opt, "RESB"))
    {
        return atoi(opr);
    }
    else if (!strcmp(opt, "BYTE"))
    {
        if (opr[0] == 'C')
        {
            int charcount = 0;
            for (int i = 1; i < strlen(opr); i++)
            {
                if (opr[i] != '\'')
                {
                    charcount++;
                }
            }
            return charcount;
        }
        else if (opr[0] == 'X')
        {
            return 1;
        }
        else
        {
            return 1;
        }
    }
    else if (!strcmp(opt, "BASE"))
    {
        return 0;
    }
    else if (!strcmp(opt, "END"))
    {
        return 0;
    }
    return -1;
}

int findOpt(char opt[])
{
    int len = strlen(opt);
    char* temp = (char*)malloc(sizeof(char) * len);
    int isOpt = 0;
    int hasPlus = 0;
    int format_size;
    if (opt[0] == '+')
    {
        int j = 1;
        for (j; j < len; j++) temp[j - 1] = opt[j];
        hasPlus = 1;
        temp[j - 1] = '\0';
    }
    else strcpy(temp, opt);
    int i = 0;
    for (i; i < OPTAB_LEN; i++)
    {
        if (!strcmp(OPTAB[i].mnemonic, temp))
        {
            isOpt = 1;
            break;
        }
    }
    if (isOpt == 0)
    {
        return -1;
    }
    else
    {
        format_size = OPTAB[i].format;
        if (hasPlus) format_size += 1;
        return format_size;
    }
}

int makeOpcode(char opt[], char opr[], int format)
{
    int len = strlen(opt);
    char* temp = (char*)malloc(sizeof(char) * len);
    int isOpt = 0;
    int hasPlus = 0;
    int coef = 0;
    int opcode = 0;
    if (opt[0] == '+')
    {
        int j = 1;
        for (j; j < len; j++) temp[j - 1] = opt[j];
        hasPlus = 1;
        temp[j - 1] = '\0';
    }
    else strcpy(temp, opt);
    if (format == 1 || format == 2) coef = 0;
    else
    {
        if (opr[0] == '#') coef = 1;
        else if (opr[0] == '@') coef = 2;
        else coef = 3;
    }

    int i = 0;
    for (i; i < OPTAB_LEN; i++)
    {
        if (!strcmp(OPTAB[i].mnemonic, temp))
        {
            break;
        }
    }
    opcode = OPTAB[i].opcode;
    opcode += coef;
    return opcode;
}

int findLabel(char label[])
{
    for (int i = 0; i < symIndex; i++)
    {
        if (!strcmp(label, SYMBOL_TABLE[i].statement))
        {
            return 1;
        }
    }
    return 0;
}

int getRegNum(char ch[])
{
    for (int i = 0; i < REGTAB_LEN; i++)
    {
        if (!strcmp(REGTAB[i].r, ch)) return REGTAB[i].num;
    }
    return -1;
}

void writeImmediateFile(FILE* fpw, int hasComment, char temp[][10], int size, int loc)
{
    int size_real = size;
    char locstr[VAL_LEN];
    int isNoLoc = 0;
    int isEnd = 0;
    sprintf(locstr, "%04X", loc);
    if (hasComment) size_real = size - 1;
    for (int k = 0; k < size; k++)
    {
        if (!strcmp(temp[k], "BASE")) isNoLoc = 1;
        if (!strcmp(temp[k], "END")) isNoLoc = 1;
    }
    if (isNoLoc) fprintf(fpw, "   ");
    else fprintf(fpw, "%s    ", locstr);
    if (size_real == 3)
    {
        fprintf(fpw, "%s %s  %s", temp[0], temp[1], temp[2]);
    }
    else if (size_real == 2)
    {
        fprintf(fpw, "   %s  %s", temp[0], temp[1]);
    }
    else if (size_real == 1)
    {
        fprintf(fpw, "   %s  ", temp[0]);
    }
    if (hasComment)
    {
        fprintf(fpw, "   %s\n", temp[size_real]);
    }
    else
    {
        fprintf(fpw, "\n");
    }
}

void printSYMTAB(FILE* fpw, int symIndex)
{
    fprintf(fpw, "SYMBOL\n");
    for (int i = 0; i < symIndex; i++)
    {
        fprintf(fpw, "%s %s\n", SYMBOL_TABLE[i].statement, SYMBOL_TABLE[i].address);
    }
}

int getSymbolAddr(symbol table[], int table_size, char label[])
{
    int i = 0;
    for (i; i < table_size; i++)
    {
        if (!strcmp(table[i].statement, label)) break;
    }
    if (i == table_size) return -1;
    else return strtol(table[i].address, NULL, 16);
}

void printListingFile(FILE* fpw, listing LISTING[], int len)
{
    for (int i = 0; i < len; i++)
    {
        if (LISTING[i].isComment == 1) fprintf(fpw, "  %s", LISTING[i].comment);
        else fprintf(fpw, "%s %s  %s  %s  %s  %s\n", LISTING[i].loc, LISTING[i].label, LISTING[i].opt, LISTING[i].operand, LISTING[i].comment, LISTING[i].objcode);
    }
}

int main(int argc, char* argv[])
{
    FILE* fpr;
    FILE* fpw;
    char buf[BUF_LEN] = { 0 };
    char filename[32];
    char filename2[32];
	char filename3[32];
    int isFirstLine = 1;
    int tempLoc;
    int interIndex = 0;
    int currentLoc = 0; //this is decimal, need to convert to hax
    int isNotStart = 0;
    if (argc < 4)
    {
        printf("No input.\n");
        exit(1);
    }
    strcpy(filename, argv[1]);
    strcpy(filename2, argv[2]);
	strcpy(filename3, argv[3]);
    fpr = fopen("srccode.asm", "r");
    fpw = fopen("intermediate", "w");
    if (fpr == NULL)
    {
        printf("Can't read input file");
        exit(1);
    }

    // pass 1
    while (fgets(buf, BUF_LEN, fpr) != NULL)
    {
        int i = 0;
        tempLoc = currentLoc;
        if (strlen(buf) == 0)
        {
            continue;
        }
        if (buf[0] == '.')
        {
            fprintf(fpw, "%s", buf);
            continue;
        }
        else
        {
            //parsing
            char* ptr;
            char temp[4][VAL_LEN];
            ptr = strtok(buf, " \t\n");
            int hasComment = 0;

            while (ptr != NULL)
            {
                if (ptr[0] == '.')
                {
                    strcat(ptr, strtok(NULL, "\n"));
                    strcpy(temp[i++], ptr);
                    hasComment = 1;
                    break;
                }
                strcpy(temp[i++], ptr);
                ptr = strtok(NULL, " \t\n");

            }

            if (isFirstLine)
            {
                if (!strcmp(temp[0], "START"))
                {
                    currentLoc = strtol(temp[1], NULL, 16);
                    strcpy(SYMBOL_TABLE[symIndex].statement, temp[0]);
                    sprintf(SYMBOL_TABLE[symIndex].address, "%04X", currentLoc);
                    symIndex++;
                }
                else if (!strcmp(temp[1], "START"))
                {
                    currentLoc = strtol(temp[2], NULL, 16);
                    strcpy(SYMBOL_TABLE[symIndex].statement, temp[0]);
                    sprintf(SYMBOL_TABLE[symIndex].address, "%04X", currentLoc);
                    symIndex++;
                }
                else
                {
                    currentLoc = 0;
                    isNotStart = 1;
                }
                isFirstLine = 0;
                tempLoc = currentLoc;
            }
            if (isFirstLine == 0 && isNotStart == 1)
            {

                int size_real = i;
                if (hasComment) size_real = i - 1;
                int isLabelSymbol = findLabel(temp[0]);

                if (size_real == 3)
                {
                    if (isLabelSymbol)
                    {
                        //error
                        exit(1);
                    }
                    else
                    {
                        strcpy(SYMBOL_TABLE[symIndex].statement, temp[0]);
                        if (strcmp(temp[1], "BASE") != 0 && strcmp(temp[1], "END") != 0) sprintf(SYMBOL_TABLE[symIndex].address, "%04X", currentLoc);
                        symIndex++;
                        int add_size = findOpt(temp[1]);
                        if (add_size == -1)
                        {
                            currentLoc += notopt(temp[1], temp[2]);
                        }
                        else
                        {
                            currentLoc += add_size;
                        }

                    }
                }

                if (size_real == 2)
                {
                    int add_size = findOpt(temp[0]);
                    if (!strcmp(temp[0], "BASE")) add_size = 0;
                    if (!strcmp(temp[0], "END")) add_size = 0;
                    if (add_size == -1) exit(1);
                    currentLoc += add_size;
                }
                if (size_real == 1)
                {
                    int add_size = findOpt(temp[0]);
                    currentLoc += add_size;
                }
            }
            isNotStart = 1;
            writeImmediateFile(fpw, hasComment, temp, i, tempLoc);
        }
    }
    printSYMTAB(fpw, symIndex);
    fclose(fpw);
    fclose(fpr);
    //pass2

    FILE* fpr2;
    FILE* fpw2;
    FILE* fpw3;

    fpr2 = fopen("intermediate", "r");
    fpw2 = fopen(filename2, "w");
    fpw3 = fopen(filename3, "w");
    symbol SYMTAB[MAX_LEN];
    listing LISTING[MAX_LEN];
    int listLen = 0;
    int symLen = 0;
    int isSymbol = 0;
    int hasComment;
    while (fgets(buf, BUF_LEN, fpr2) != NULL)
    {
        int i = 0;

        if (strlen(buf) == 0) continue;
        char* ptr;
        char temp[6][VAL_LEN];
        if (buf[0] == '.')
        {
            strcpy(LISTING[listLen].loc, "");
            strcpy(LISTING[listLen].label, "");
            strcpy(LISTING[listLen].opt, "");
            strcpy(LISTING[listLen].operand, "");
            strcpy(LISTING[listLen].comment, buf);
            strcpy(LISTING[listLen].objcode, "");
            LISTING[listLen].isComment = 1;
            listLen++;
            continue;
        }
        ptr = strtok(buf, " \t\n");
        if (strcmp(ptr, "SYMBOL") == 0)
        {
            isSymbol = 1;
            continue;
        }
        if (isSymbol)
        {
            while (ptr != NULL)
            {
                strcpy(temp[i++], ptr);
                ptr = strtok(NULL, " \t\n");
            }
            strcpy(SYMTAB[symLen].statement, temp[0]);
            strcpy(SYMTAB[symLen].address, temp[1]);
            symLen++;
        }
        else
        {
            hasComment = 0;

            while (ptr != NULL)
            {
                if (ptr[0] == '.')
                {
                    strcat(ptr, strtok(NULL, "\n"));
                    strcpy(temp[i++], ptr);
                    hasComment = 1;
                    break;
                }
                strcpy(temp[i++], ptr);
                ptr = strtok(NULL, " \t\n");
            }
            int size_real = i;
            if (hasComment) size_real -= 1;
            if (size_real == 4)
            {
                strcpy(LISTING[listLen].loc, temp[0]);
                strcpy(LISTING[listLen].label, temp[1]);
                strcpy(LISTING[listLen].opt, temp[2]);
                strcpy(LISTING[listLen].operand, temp[3]);
                if (hasComment) strcpy(LISTING[listLen].comment, temp[4]);
                else strcpy(LISTING[listLen].comment, "");
            }
            else if (size_real == 3)
            {
                if (isdigit(temp[0][0])) //loc present
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
                if (hasComment) strcpy(LISTING[listLen].comment, temp[3]);
                else strcpy(LISTING[listLen].comment, "");
            }
            else if (size_real == 2)
            {
                if (isdigit(temp[0][0])) //loc present
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
                if (hasComment) strcpy(LISTING[listLen].comment, temp[2]);
                else strcpy(LISTING[listLen].comment, "");
            }
            strcpy(LISTING[listLen].objcode, "");
            LISTING[listLen].isComment=0;
            listLen++;
        }
    }
    //LISTING - listLen, SYMTAB - symLen
    //make object code of each line
    int baseaddr = -1;
    int startaddr = 0;
    for (int index = 0; index < listLen; index++)
    {
        if (strcmp(LISTING[index].opt, "START") == 0)
        {
            startaddr = strtol(LISTING[index].operand, NULL, 16);
            continue;
        }
        else
        {
            //opcode making
            int opcode;
            int format = findOpt(LISTING[index].opt);
            int flag;
            if (format == -1)
            {
                // make all of object code
                if (!strcmp(LISTING[index].opt, "RESW"));
                else if (!strcmp(LISTING[index].opt, "RESB"));
                else if (!strcmp(LISTING[index].opt, "END"))
                {

                }
                else if (!strcmp(LISTING[index].opt, "BASE"))
                {
                    baseaddr = getSymbolAddr(SYMTAB, symLen, LISTING[index].operand);
                    continue;
                }
                else if (!strcmp(LISTING[index].opt, "WORD"))
                {
                    sprintf(LISTING[index].objcode, "%06X", atoi(LISTING[index].operand));
                }
                else if (!strcmp(LISTING[index].opt, "BYTE"))
                {
                    if (LISTING[index].operand[0] == 'C')
                    {
                        char temp[VAL_LEN] = "";
                        char temp2[VAL_LEN] = "";
                        int asciitemp;
                        for (int m = 1; m < strlen(LISTING[index].operand); m++)
                        {
                            if (LISTING[index].operand[m] != '\'')
                            {
                                asciitemp = LISTING[index].operand[m];
                                sprintf(temp2, "%02X", asciitemp);
                                strcat(temp, temp2);
                            }
                        }
                        strcpy(LISTING[index].objcode, temp);
                    }
                    else if (LISTING[index].operand[0] == 'X')
                    {
                        char temp3[VAL_LEN] = "";
                        int temp_index = 0;
                        for (int m = 1; m < strlen(LISTING[index].operand); m++)
                        {
                            if (LISTING[index].operand[m] != '\'')
                            {
                                temp3[temp_index++] = LISTING[index].operand[m];
                            }
                            temp3[temp_index] = '\0';
                        }
                        strcpy(LISTING[index].objcode, temp3);
                    }
                    else
                    {
                        sprintf(LISTING[index].objcode, "%02X", atoi(LISTING[index].operand));
                    }
                }
            }
            else
            {
                //operator
                opcode = makeOpcode(LISTING[index].opt, LISTING[index].operand, format);
                if (format == 1)
                {
                    sprintf(LISTING[index].objcode, "%02X", opcode);
                }
                else if (format == 2)
                {
                    char* reg = strtok(LISTING[index].operand, " ,");
                    int q = 0;
                    char temp4[VAL_LEN] = "";
                    char temp5[VAL_LEN] = "";
                    int num;
                    while (reg != NULL)
                    {
                        num = getRegNum(reg);
                        if (num != -1)
                        {
                            sprintf(temp4, "%X", num);
                            strcat(temp5, temp4);
                        }
                        else break;
                        q++;
                        reg = strtok(NULL, " ,");
                    }
                    if (q < 2) strcat(temp5, "0");
                    sprintf(LISTING[index].objcode, "%02X%s", opcode, temp5);
                }
                else if (format == 3)
                {
                    if (strlen(LISTING[index].operand) == 0)
                    {
                        sprintf(LISTING[index].objcode, "%02X0000", opcode);
                        continue;
                    }
                    flag = 2;
                    int plusFlag = 0;
                    char opr12[2][VAL_LEN];
                    char oprtemp[VAL_LEN];
                    char tempoperand[VAL_LEN];
                    char* oprptr;
                    strcpy(tempoperand, LISTING[index].operand);
                    oprptr = strtok(tempoperand, " ,");
                    int oprsize = 0;
                    int disp = 0;
                    while (oprptr != NULL)
                    {
                        strcpy(opr12[oprsize++], oprptr);
                        oprptr = strtok(NULL, " ,");
                    }
                    if (oprsize == 2)
                    {
                        if (!strcmp(opr12[1], "X")) plusFlag = 8;
                    }
                    int firstLoc = 0;
                    int LastLoc = 0;
                    if (opr12[0][0] == '#' || opr12[0][0] == '@')
                    {
                        for (int f = 1; f < strlen(opr12[0]); f++)
                        {
                            oprtemp[f - 1] = opr12[0][f];
                        }
                    }
                    else strcpy(oprtemp, opr12[0]);
                    if (isdigit(oprtemp[0]))
                    {
                        flag = 0;
                        sprintf(LISTING[index].objcode, "%02X%X%03X", opcode, flag, atoi(oprtemp));
                    }
                    else
                    {
                        LastLoc = getSymbolAddr(SYMTAB, symLen, oprtemp);
                        firstLoc = strtol(LISTING[index].loc, NULL, 16);
                        disp = LastLoc - firstLoc - format;
                        if ((disp > 2047 || disp < -2048))
                        {
                            if (baseaddr != -1)
                            {
                                flag = 4;
                                flag += plusFlag;
                                firstLoc = baseaddr;
                                disp = LastLoc - firstLoc;
                                sprintf(LISTING[index].objcode, "%02X%X%03X", opcode, flag, disp);
                            }
                        }
                        else
                        {
                            flag += plusFlag;
                            if (disp < 0) disp = 4095 + (disp + 1);
                            sprintf(LISTING[index].objcode, "%02X%X%03X", opcode, flag, disp);
                        }
                    }

                }
                else if (format == 4)
                {
                    flag = 1;
                    char tempopr[VAL_LEN] = "";
                    int u;
                    if (LISTING[index].operand[0] == '#' || LISTING[index].operand[0] == '@')
                    {
                        for (u = 1; u < strlen(LISTING[index].operand); u++)
                        {
                            tempopr[u - 1] = LISTING[index].operand[u];
                        }
                        tempopr[u - 1] = '\0';
                    }
                    else strcpy(tempopr, LISTING[index].operand);

                    if (isdigit(tempopr[0]))
                    {
                        sprintf(LISTING[index].objcode, "%02X%X%05X", opcode, flag, atoi(tempopr));
                    }
                    else
                    {
                        sprintf(LISTING[index].objcode, "%02X%X%05X", opcode, flag, getSymbolAddr(SYMTAB, symLen, tempopr));
                    }
                }
            }

            //
        }
    }
    printListingFile(fpw2, LISTING, listLen);
    // object file
    fprintf(fpw3, "H%s  ", SYMTAB[0].statement);
    int start = 0;
    int end = 0;
    int t;
    for (t = listLen - 1; t >= 0; t--)
    {
        if (strlen(LISTING[t].loc) != 0) break;
    }
    end = strlen(LISTING[t].objcode) / 2 + strtol(LISTING[t].loc, NULL, 16);
    for (int h = 0; h < listLen; h++)
    {
        if (strlen(LISTING[h].loc) != 0)
        {
            start = strtol(LISTING[h].loc, NULL, 16);
            break;
        }
    }
    fprintf(fpw3, "%06X%06X\n", start, end);
    int indexT = 0;
    //T loop
    while (1)
    {
        if (indexT >= listLen) break;
        //max index 59. real size 60
        char recordT[61] = "";
        int halfRecordT=0;
        fprintf(fpw3, "T");
        while (1)
        {
            if (strlen(LISTING[indexT].loc) != 0 && strlen(LISTING[indexT].objcode) != 0)
            {
                fprintf(fpw3, "%06lX", strtol(LISTING[indexT].loc, NULL, 16));
                break;
            }
            indexT++;
        }
        while (1)
        {
            if (strcmp(LISTING[indexT].opt, "RESW") == 0 || strcmp(LISTING[indexT].opt, "RESB") == 0) break;
            if (strlen(LISTING[indexT].objcode) != 0)
            {
                if (indexT >= listLen) break;
                if ((strlen(recordT) + strlen(LISTING[indexT].objcode)) > 60) break;
                strcat(recordT, LISTING[indexT].objcode);
                indexT++;
            }
            else indexT++;
        }
        halfRecordT=strlen(recordT)/2;
        fprintf(fpw3, "%02X%s\n", halfRecordT, recordT);
    }
    //M loop
    int halfRecordM=0;
    
    for (int cm = 0; cm < listLen; cm++)
    {
        if (findOpt(LISTING[cm].opt)==4) 
        {
            if(LISTING[cm].operand[0] !='#' && LISTING[cm].operand[0] !='@')
            {
                halfRecordM=strlen(LISTING[cm].objcode) / 2 + 1;
                fprintf(fpw3, "M%06lX%02X\n", strtol(LISTING[cm].loc, NULL, 16) + 1, halfRecordM);
            }
        }
    }
    fprintf(fpw3, "E%06X", start);
    fclose(fpr2);
    fclose(fpw2);
    fclose(fpw3);
    return 0;
}