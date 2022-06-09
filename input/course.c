#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getCourse(char a[], int opt){
    int i;
    int retCourse;
    int retCredit;
    int retGPA;
    i = 0;
    retCourse = 0;
    retGPA = 0;
    while(a[i] != '|'){
        if(a[i] >= '0' && a[i] <= '9'){
            retCourse = retCourse * 10 + (a[i] - '0');
        }
        i++;
    }
    retCredit = a[i+1] - 48;
    int len = strlen(a);
    if(a[len - 1] >= 'A' && a[len - 1] <= 'F'){
        retGPA = 4 - a[len - 1 ] + 'A';
    }
    if(opt == 1){
        return retCourse;
    }
    else if (opt == 2){
        return retCredit;
    }
    else if(opt == 3){
        return retGPA;
    }
}

bool PreviousClassesTaken(char a[], int gpa[]){
    int len;
    len = strlen(a);
    int i;
    i = 0;
    if(len == 0){
        return true;
    }
    int curClass;
    curClass = 0;
    bool totflag, flag;
    flag = false;
    totflag = false;
    while(i < len){
        if(a[i] >= '0' && a[i] <= '9'){
            curClass = curClass * 10 + (a[i] - '0');
        }
        if(a[i] == 'c'){
            curClass = 0;//reset
        }
        if(a[i] == ','){
            if(gpa[curClass] == 0 || gpa[curClass] == -1){//if the class is not taken
                flag = true;
            }
        }
        if(a[i] == ';' || i == len - 1){
            if(gpa[curClass] == 0 || gpa[curClass] == -1){//if the class is not taken
                flag = true;
            }
            if(flag == false){
                totflag = true;
                return true; //If there is a plan of classes that satisfies the need then it's usable
            }
            flag = false;
            curClass = 0;
        }
        i++;
    }
    return totflag;
}

int main(){
    int credit[10001];
    memset(credit, 0, sizeof(credit));
    int gpa[10001];
    memset(gpa, 0, sizeof(gpa));
    char pre[101][500];
    int Course[101];
    memset(Course, 0, sizeof(Course));
    int coursecnt;
    coursecnt = 0;
    char recommended[101][500];
    int reccnt;
    int attempted;
    int completed;
    int remaining;
    int sumcredit;
    int sumgpa;
    double avegpa;
    int i;
    int j;
    attempted = 0;
    completed = 0;
    remaining = 0;
    i = 0;
    j = 1;
    sumcredit = 0;
    sumgpa = 0;
    avegpa = 0.0;
    char str[1000];
    int avegpa1, avegpa2, avegpa3;
    reccnt = 0;
    while(gets(str)){
        if(strcmp(str, "") == 0){
            break;
        }
        int i;
        i = 0;
        int cnt;
        cnt = 0;
        int course;
        course = getCourse(&str[0],1);
        Course[coursecnt] = course;
        coursecnt = coursecnt + 1;
        int cur_credit;
        cur_credit = getCourse(&str[0],2);
        sumcredit = sumcredit + cur_credit;
        int cur_gpa;
        cur_gpa = getCourse(&str[0],3);
        credit[course] = cur_credit;
        gpa[course] = cur_gpa;
        char* res;
        res = strchr(str, '|');
        res = res + 1;
        res = strchr(res, '|');
        res = res + 1;
        int len;
        len = strlen(res);
        res[len - 1] = '\0';
        if(res[len - 2] == '|'){
            res[len - 2] = '\0';
        }
        strcpy(pre[course], res);
        strcpy(str, " ");
    }
    while(i < coursecnt)
    {
        
        if(credit[Course[i]] != 0){
            int tmpgpa;
            tmpgpa = gpa[Course[i]];
            if(tmpgpa == -1){
                tmpgpa = 0;
            }
            sumgpa = sumgpa + credit[Course[i]] * tmpgpa;
            //attempted 已经获得成绩的课程的总学分。包括获得 `F` 成绩的课程。
            if(gpa[Course[i]] >0 || gpa[Course[i]] == -1){
                //printf("attempted course: %d\n", i);
                attempted += credit[Course[i]];
            }
            //completed 已经获得的总学分。成绩为 `F` 的课程没有获得学分。
            if(gpa[Course[i]] > 0){
                completed += credit[Course[i]];
            }
            //remaining 培养方案中还有多少学分没有修读，包括成绩为 `F` 的课程
            remaining = sumcredit - completed;
            //满足前置课程条件，可以修读的课程但还没有获得学分的课程。必须按照课程出现在输入中的先后顺序进行输出。
            if(PreviousClassesTaken(pre[Course[i]], gpa) == true && (gpa[Course[i]] == 0 || gpa[Course[i]] == -1)) {
                reccnt++;
                char tmp[100];
                sprintf(tmp, "%c%d", 'c', Course[i]);
                strcpy(recommended[reccnt], tmp);
            }
        }
        i++;
    }
    if(attempted == 0){
        avegpa = 0.0;
        avegpa1 = 0;
        avegpa2 = 0;
    }
    else{
        // avegpa1 = sumgpa / attempted;
        // avegpa = (sumgpa - avegpa1 * attempted) * 10;
        // avegpa2 = avegpa / attempted;
        // avegpa = (avegpa - avegpa2 * attempted) * 10;
        // avegpa3 = avegpa / attempted;
        // if(avegpa3 >= 5)
        //     avegpa2 = avegpa2 + 1;
        // }
        avegpa = cast::<double>::(sumgpa) / cast::<double>::(attempted);
    }
   //printf("GPA: %d.%d\n", avegpa1,avegpa2);
    printf("GPA: %.1f\n", avegpa);
    printf("Hours Attempted: %d\n", attempted);
    printf("Hours Completed: %d\n", completed);
    printf("Credits Remaining: %d\n\n", remaining);
    printf("Possible Courses to Take Next:\n");
    if(reccnt == 0 && remaining == 0){
        printf("None - Congratulations!");
    }
    else{
        while(j <= reccnt){
            printf("%s\n", recommended[j]);
            j++;
        }   
    }
}
