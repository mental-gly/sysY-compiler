
void gemm(int a[], int An, int Am,
          int b[], int Bn, int Bm,
          int c[]) {

    int i = 0;
    int j = 0;
    int k = 0;
    while (i < An) {
        j = 0;
        while (j < Bm) {
            k = 0;
            while (k < Am) {
                int temp = c[i * Bm + j];
                temp = a[i * Am + k] * b[k * Bm + j]  + temp;
                c[i * Bm + j] = temp;
                k = k + 1;
            }
            j = j + 1;
        }
        i = i + 1;
    }
}

int main() {
    int a[1000];
    int b[1000];
    int c[1000];
    int An, Am, Bn, Bm;
    scanf("%d%d%d%d", &An, &Am);
    int i = 0;
    int j = 0;
    while (i < An) {
        j = 0;
        while (j < Am) {
            scanf("%d", &a[i * Am + j]);
            j = j + 1;
        }
        i = i + 1;
    }
    scanf("%d%d%d%d", &Bn, &Bm);
    i = 0;
    while (i < Bn) {
        j = 0;
        while (j < Bm) {
            scanf("%d", &b[i * Am + j]);
            j = j + 1;
        }
        i = i + 1;
    }
    if (Am != Bn) {
        printf("Incompatiable Dimension\n");
        return 0;
    }
    gemm(a, An, Am, b, Bn, Bm);
    while (i < An) {
        j = 0;
        while (j < Bm) {
            printf("%d", c[i * Bm + j]);
            j = j + 1;
        }
        printf("\n");
        i = i + 1;
    }
    return 0;
}