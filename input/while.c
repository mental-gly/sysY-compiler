
int f(int i, int j) {
    while (i < 10) {
        while (i < 100) {
            i = i + 1;
        }
        while (i < 200) {
            i = i + 1;
        }
    }
    return i;
}