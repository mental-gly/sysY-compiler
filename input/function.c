
void quick_sort(int a[], int l, int r) {
    if (l > r) { return;}
    int mid = l + r;
    mid = mid / 2;
    int i = l;
    int j = r;
    int x = a[mid];
    while (i <= j) {
        while (i < x) { i = i + 1; }
        while (j > x) { j = j - 1; }
        if (i <= j) {
            int t = a[i];
            a[i]= a[j];
            a[j] = t;
        }
    }
    quick_sort(a, l, mid);
    quick_sort(a, mid + 1, r);
    return;
}