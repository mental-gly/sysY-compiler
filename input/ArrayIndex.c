
void f(int a[], int N, int i, int bound) {
    while (i < N) {
        if (a[i] < bound) {
            a[i+1] = i + 1;
        }
        else {
            a[i] = bound;
        }
        i = i  + 1;
    }
}