use std.io;

void quick_sort(int a[], int l, int r) {
    if (l > r) { return;}
    int mid = l + r;
    mid = mid / 2;
    int i = l;
    int j = r;
    int x = a[mid];
    while (i <= j) {
        while (a[i] < x) { i = i + 1; }
        while (a[j] > x) { j = j - 1; }
        if (i <= j) {
            int t = a[i];
            a[i]= a[j];
            a[j] = t;
            i = i + 1;
            j = j - 1;
        }
    }
    ;
    quick_sort(a, l, j);
    quick_sort(a, i, r);
    return;
}

int main() {
    int a[10001];
    int N;
    scanf("%d", &N);
    int i = 0;
    while (i < N) {
        scanf("%d", &a[i]);
        i = i + 1;
    }
    quick_sort(&a[0], 0, N - 1);
    i = 0;
    while (i < N) {
        printf("%d\n", a[i]);
        i = i + 1;
    }
}