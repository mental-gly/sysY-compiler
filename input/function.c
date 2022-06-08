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
        printf("%d %d\n", i, j);
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