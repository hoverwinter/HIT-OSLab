/*
 * Compile: "gcc testlab2.c"
 * Run: "./a.out"
 */

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#define __LIBRARY__
#include <unistd.h>

_syscall2(int, whoami,char*,name,unsigned int,size);
_syscall1(int, iam, const char*, name);

#define MAX_NAME_LEN        23
#define NAMEBUF_SIZE        (MAX_NAME_LEN + 1)
/* truncate a long name to SHORT_NAME_LEN for display */
#define SHORT_NAME_LEN      (MAX_NAME_LEN + 2)

/*           name               score */
#define TEST_CASE { \
    {"x",                           10,  1, NAMEBUF_SIZE,  1},\
    {"sunner",                      10,  6, NAMEBUF_SIZE,  6},\
    {"Twenty-three characters",      5, 23, NAMEBUF_SIZE, 23},\
    {"123456789009876543211234",     5, -1, 0,            -1},\
    {"abcdefghijklmnopqrstuvwxyz",   5, -1, 0,            -1},\
    {"Linus Torvalds",               5, 14, NAMEBUF_SIZE, 14},\
    {"",                             5,  0, NAMEBUF_SIZE,  0},\
    {"whoami(0xbalabala, 10)",       5, 22,           10, -1},\
    {NULL, 0, 0, 0, 0}  /* End of cases */ \
}
/*改动一：增加size,和rval2*/

int test(const char* name, int max_score, int expected_rval1, int size, int expected_rval2);
void print_message(const char* msgfmt, const char* name);

struct test_case 
{
    char *name;
    int score;
    int rval1;  /* return value of iam() */
     /*改动2：增加size,和rval2定义*/
    int size;   /*Patch for whoami,2009.11.2*/
    int rval2;  /* return value of whoami() */
};

int main(void)
{
    struct test_case cases[] = TEST_CASE;

    int total_score=0, i=0;

    while (cases[i].score != 0)
    {
        int score;

        printf("Test case %d:", i+1);

         /*改动3：增加size,和rval2的参数阿*/
        score = test( cases[i].name, 
                      cases[i].score, 
                      cases[i].rval1,
                      cases[i].size,
                      cases[i].rval2 );

        total_score += score;
        i++;
    }

    printf("Final result: %d%%\n", total_score);
    return 0;

}
 /*改动4：增加size,和rval2的声明*/
int test(const char* name, int max_score, int expected_rval1, int size, int expected_rval2)
{
    int rval;
    int len;
    char * gotname;
    int score=-1;

    assert(name != NULL);

    print_message("name = \"%s\", length = %d...", name);

    /*Test iam()*/
    len = strlen(name);
    rval = iam(name);
    /* printf("Return value = %d\n", rval);*/
 
/*改动5：增加的expected_rval1*/
    if (rval == expected_rval1)
    {
        if (rval == -1 && errno == EINVAL) /*The system call can detect bad name*/
        {
            /* print_message("Long name, %s(%d), detected.\n", name);*/
            printf("PASS\n");
            score = max_score;
        }
        else if (rval == -1 && errno != EINVAL)
        {
            printf("\nERROR iam(): Bad errno %d. It should be %d(EINVAL).\n", errno, EINVAL);
            score = 0;
        }
        /* iam() is good. Test whoami() next. */
    }
    else
    {
        printf("\nERROR iam(): Return value is %d. It should be %d.\n", rval, expected_rval1);
        score = 0;
    }

    if (score != -1) 
        return score;

    /*Test whoami()*/
    gotname = (char*)malloc(len+1);
    if (gotname == NULL)
        exit(-1);

    memset(gotname, 0, len+1);

    /* printf("Get: buffer length = %d.\n", len+1); */

    rval = whoami(gotname, size);
    /* printf("Return value = %d\n", rval); */

/*改动6：增加的expected_rval2*/
/*改动＋＋：比较多 ，但还是顺序的改改*/

    if(rval == expected_rval2)
    {   
        if(rval == -1)
        {
            printf("PASS\n");
            score = max_score;
        }       
        else 
        {
            if (strcmp(gotname, name) == 0)
            {
                /* print_message("Great! We got %s(%d) finally!\n", gotname); */
                printf("PASS\n");
                score = max_score;
            }
            else
            {
                print_message("\nERROR whoami(): we got %s(%d). ", gotname);
                print_message("It should be %s(%d).\n", name);
                score = 0;
            }
        }
    }
    else if (rval == -1)
    {
        printf("\nERROR whoami(): Return value is -1 and errno is %d. Why?\n", errno);
        score = 0;
    }
    else 
    {
        printf("\nERROR whoami(): Return value should be %d, not %d.\n", expected_rval2, rval);
        score = 0;
    }

    free(gotname);
    assert(score != -1);

    return score;
}

void print_message(const char* msgfmt, const char* name)
{
    char short_name[SHORT_NAME_LEN + 4] = {0};
    int len;
    
    len = strlen(name);

    if (len == 0)
    {
        strcpy(short_name, "NULL");
    }
    else if (len <= SHORT_NAME_LEN)
    {
        strcpy(short_name, name);
    }
    else
    {
        memset(short_name, '.', SHORT_NAME_LEN+3);
        memcpy(short_name, name, SHORT_NAME_LEN);
    }
    
    printf(msgfmt, short_name, len);
}