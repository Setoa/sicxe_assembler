#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_LEN 256












int main(int argc, char* argv[])
{
    FILE* fp;
    char buf[BUF_LEN];
    char filename[32];

    if(argc <= 1)
    {
        printf("No input.\n");
        exit(1);
    }
    strcpy(filename, argv[1]);
    fp=fopen(filename, "r");
    if(fp==NULL)
    {
        printf("Can't read input file");
        exit(1);
    }

    // pass 1
    while(fgets(buf, BUF_LEN, fp)!=NULL)
    {
        int i=0;
        int isFirstLine=1;
        if(strlen(buf)==0) break;
        if(buf[0]=='.')
        {
            //To write comment(this buf) on the file, input this to 2dimension array
        }
        else
        {
            char* ptr;
            char* temp[3];
            ptr=strtok(buf, " \t");

            while(ptr != NULL)
            {
                strcpy(temp[i++], ptr);
                ptr=strtok(NULL, " \t");
            }
            
        }
        
    }

    return 0;
}