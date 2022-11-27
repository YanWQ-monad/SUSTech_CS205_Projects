#include <stdio.h>

#include "matrix.h"

int main() {
    matrix *A = createMatrixEx((Size){2, 2});
    matrix *B = createUnallocatedMatrix();
    matrix *C = createUnallocatedMatrix();
    matrix *D = createUnallocatedMatrix();

    // A <- { { .1, .1 }, { 5, .2 } }
    float arr[4] = {.1f, .1f, 5.f, .2f};
    loadMatrixFromArray(A, arr);

    // B <- A * A
    mulMatrix(B, A, A);

    // C <- "matrix.bin" <- B
    saveMatrix(B, "matrix.bin");
    loadMatrix(C, "matrix.bin");

    // D <- C * 2
    mulMatrixScale(D, C, 2.f);

    float maximum = maxInMatrix(D, NULL);
    printf("Maximum value of 2A^2 is: %f\n", maximum);

    puts("Complete matrix of 2A^2:");
    printMatrix(D);

    deleteMatrixEx(A);
    deleteMatrixEx(B);
    deleteMatrixEx(C);
    deleteMatrixEx(D);

    return 0;
}
