use std.io;
use std.string;

int getCourse(char a[], int opt){
    int i;
    int retCourse;
    int retCredit;
    int retGPA;
    i = 0;
    retCourse = 0;
    retGPA = 0;
    while(a[i] != '|'){
        if((a[i] >= '0') && (a[i] <= '9')){
            retCourse = retCourse * 10 + cast::<int>::(a[i] - '0');
        }
        i = i + 1;
    }
    retCredit = cast::<int>::(a[i+1]) - 48;
    int len = cast::<int>::(strlen(a));
    if((a[len - 1] >= 'A') && (a[len - 1] <= 'F')){
        retGPA = 4 - cast::<int>::(a[len - 1 ] - 'A');
    }
    if(opt == 1){
        return retCourse;
    }
    if (opt == 2) {
        return retCredit;
    }
    if (opt == 3) {
        return retGPA;
    }
    return 0;
}

int PreviousClassesTaken(char a[], int gpa[]){
    int len = cast::<int>::(strlen(a));
    int i;
    i = 0;
    if(len == 0){
        return 1;
    }
    int curClass;
    curClass = 0;
    int totflag, flag;
    flag = 0;
    totflag = 0;
    while(i < len){
        if((a[i] >= '0') && (a[i] <= '9')){
            curClass = curClass * 10 + cast::<int>::(a[i] - '0');
        }
        if(a[i] == 'c'){
            curClass = 0;//reset
        }
        if(a[i] == ','){
            if((gpa[curClass] == 0) || (gpa[curClass] == -1)){//if the class is not taken
                flag = 1;
            }
        }
        if((a[i] == ';') || (i == len - 1)) {
            if((gpa[curClass] == 0) || (gpa[curClass] == -1)){//if the class is not taken
                flag = 1;
            }
            if(flag == 0){
                totflag = 1;
                return 1; //If there is a plan of classes that satisfies the need then it's usable
            }
            flag = 0;
            curClass = 0;
        }
        i = i + 1;
    }
    return totflag;
}


int main() {
    int credit[10001];
    //memset(&credit[0], 0, 10001 * 4);
    int i = 0;
    while (i < 10001) {
        credit[i] =  0;
        i = i + 1;
    }
    int gpa[10001];
    //memset(&gpa[0], 0, 10001 * 4);
    i = 0;
    while (i < 10001) {
        credit[i] =  0;
        i = i + 1;
    }
    char pre[101][500];
    int Course[101];
    //memset(&Course[0], 0, 101 * 4);
    i = 0;
    while (i < 101) {
        credit[i] =  0;
        i = i + 1;
    }
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
    int j;
    attempted = 0;
    completed = 0;
    remaining = 0;
    i = 0;
    j = 1;
    sumcredit = 0;
    sumgpa = 0;
    avegpa = cast::<double>::(0);
    char str[1000];
    int avegpa1, avegpa2, avegpa3;
    reccnt = 0;
    while (gets(&str[0])) {
        // printf("%s\n", &str[0]);
        if (str[0] != '\0') {
            int i;
            i = 0;
            int cnt;
            cnt = 0;
            int course;
            course = getCourse(&str[0], 1);
            Course[coursecnt] = course;
            coursecnt = coursecnt + 1;
            int cur_credit;
            cur_credit = getCourse(&str[0], 2);
            sumcredit = sumcredit + cur_credit;
            int cur_gpa;
            cur_gpa = getCourse(&str[0], 3);
            credit[course] = cur_credit;
            gpa[course] = cur_gpa;
            char *res;
            res = strchr(&str[0], cast::<int>::('|'));
            res = res + 1;
            res = strchr(&res[0], cast::<int>::('|'));
            res = res + 1;
            int len;
            len = cast::<int>::(strlen(res));
            res[len - 1] = '\0';
            if (res[len - 2] == '|') {
                res[len - 2] = '\0';
            }
            strcpy(&pre[course][0], &res[0]);
            strcpy(&str[0], " ");
        }
    }
    while(i < coursecnt)
    {

        if(credit[Course[i]] != 0){
            int tmpgpa;
            tmpgpa = gpa[Course[i]];
            if(tmpgpa == -1){
                tmpgpa = 0;
            }
            sumgpa = credit[Course[i]] * tmpgpa + sumgpa;
            //attempted 已经获得成绩的课程的总学分。包括获得 `F` 成绩的课程。
            if((gpa[Course[i]] > 0) || (gpa[Course[i]] == -1)){
                //printf("attempted course: %d\n", i);
                attempted = attempted + credit[Course[i]];
            }
            //completed 已经获得的总学分。成绩为 `F` 的课程没有获得学分。
            if(gpa[Course[i]] > 0){
                completed = completed + credit[Course[i]];
            }
            //remaining 培养方案中还有多少学分没有修读，包括成绩为 `F` 的课程
            remaining = sumcredit - completed;
            //满足前置课程条件，可以修读的课程但还没有获得学分的课程。必须按照课程出现在输入中的先后顺序进行输出。
            if((PreviousClassesTaken(&pre[Course[i]][0], &gpa[0]) == 1) && ((gpa[Course[i]] == 0) || (gpa[Course[i]] == -1))) {
                reccnt = reccnt + 1;
                char tmp[100];
                sprintf(&tmp[0], "%c%d", 'c', Course[i]);
                strcpy(&recommended[reccnt][0], &tmp[0]);
            }
        }
        i = i + 1;
    }
    if(attempted == 0){
        avegpa = cast::<double>::(0);
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
    printf("Possible Courses to Take Next\n");
    if((reccnt == 0) && (remaining == 0)){
        printf("  None - Congratulations!\n");
    }
    else{
        while(j <= reccnt){
            printf("  %s\n", &recommended[j][0]);
            j = j + 1;
        }
    }
    return 0;
}

