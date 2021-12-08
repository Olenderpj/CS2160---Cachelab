#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_64_64(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int Bsize;
    int rowBlock, colBlock;
    int r, c;
    int temp = 0, d = 0;

    if (N == 32) {
        Bsize = 8;
        for (colBlock = 0; colBlock < N; colBlock += 8) {
            for (rowBlock = 0; rowBlock < N; rowBlock += 8) {
                for (r = rowBlock; r < rowBlock + 8; r++) {
                    for (c = colBlock; c < colBlock + 8; c++) {
                        if (r != c) {
                            B[c][r] = A[r][c];
                        } else {
                            temp = A[r][c];
                            d = r;
                        }
                    }
                    if (rowBlock == colBlock) {
                        B[d][d] = temp;
                    }
                }
            }
        }
    } else if (N == 64) {
        transpose_64_64(M, N, A, B);
    } else {
        Bsize = 16;

        for (colBlock = 0; colBlock < M; colBlock += Bsize) {
            for (rowBlock = 0; rowBlock < N; rowBlock += Bsize) {
                for (r = rowBlock; (r < N) && (r < rowBlock + Bsize); r++) {
                    for (c = colBlock; (c < M) && (c < colBlock + Bsize); c++) {
                        if (r != c) {
                            B[c][r] = A[r][c];
                        } else {
                            temp = A[r][c];
                            d = r;
                        }
                    }
                    if (rowBlock == colBlock) {
                        B[d][d] = temp;
                    }
                }
            }
        }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */
char transpose_64_64_desc[] = "Transpose the 64 x 64 matrix";

void transpose_64_64(int M, int N, int A[N][M], int B[M][N]) {
    int colRun, rowRun, k, a0, a1, a2, a3, a4, a5, a6, a7;

    for (colRun = 0; colRun < 64; colRun += 8) {
        for (rowRun = 0; rowRun < 64; rowRun += 8) {
            for (k = 0; k < 4; k++) {
                a0 = A[colRun + k][rowRun + 0];
                a1 = A[colRun + k][rowRun + 1];
                a2 = A[colRun + k][rowRun + 2];
                a3 = A[colRun + k][rowRun + 3];
                a4 = A[colRun + k][rowRun + 4];
                a5 = A[colRun + k][rowRun + 5];
                a6 = A[colRun + k][rowRun + 6];
                a7 = A[colRun + k][rowRun + 7];

                B[rowRun + 0][colRun + k + 0] = a0;
                B[rowRun + 0][colRun + k + 4] = a5;
                B[rowRun + 1][colRun + k + 0] = a1;
                B[rowRun + 1][colRun + k + 4] = a6;
                B[rowRun + 2][colRun + k + 0] = a2;
                B[rowRun + 2][colRun + k + 4] = a7;
                B[rowRun + 3][colRun + k + 0] = a3;
                B[rowRun + 3][colRun + k + 4] = a4;
            }

            a0 = A[colRun + 4][rowRun + 4];
            a1 = A[colRun + 5][rowRun + 4];
            a2 = A[colRun + 6][rowRun + 4];
            a3 = A[colRun + 7][rowRun + 4];
            a4 = A[colRun + 4][rowRun + 3];
            a5 = A[colRun + 5][rowRun + 3];
            a6 = A[colRun + 6][rowRun + 3];
            a7 = A[colRun + 7][rowRun + 3];

            B[rowRun + 4][colRun + 0] = B[rowRun + 3][colRun + 4];
            B[rowRun + 4][colRun + 4] = a0;
            B[rowRun + 3][colRun + 4] = a4;
            B[rowRun + 4][colRun + 1] = B[rowRun + 3][colRun + 5];
            B[rowRun + 4][colRun + 5] = a1;
            B[rowRun + 3][colRun + 5] = a5;
            B[rowRun + 4][colRun + 2] = B[rowRun + 3][colRun + 6];
            B[rowRun + 4][colRun + 6] = a2;
            B[rowRun + 3][colRun + 6] = a6;
            B[rowRun + 4][colRun + 3] = B[rowRun + 3][colRun + 7];
            B[rowRun + 4][colRun + 7] = a3;
            B[rowRun + 3][colRun + 7] = a7;

            for (k = 0; k < 3; k++) {

                a0 = A[colRun + 4][rowRun + 5 + k];
                a1 = A[colRun + 5][rowRun + 5 + k];
                a2 = A[colRun + 6][rowRun + 5 + k];
                a3 = A[colRun + 7][rowRun + 5 + k];
                a4 = A[colRun + 4][rowRun + k];
                a5 = A[colRun + 5][rowRun + k];
                a6 = A[colRun + 6][rowRun + k];
                a7 = A[colRun + 7][rowRun + k];

                B[rowRun + 5 + k][colRun + 0] = B[rowRun + k][colRun + 4];
                B[rowRun + 5 + k][colRun + 4] = a0;
                B[rowRun + k][colRun + 4] = a4;
                B[rowRun + 5 + k][colRun + 1] = B[rowRun + k][colRun + 5];
                B[rowRun + 5 + k][colRun + 5] = a1;
                B[rowRun + k][colRun + 5] = a5;
                B[rowRun + 5 + k][colRun + 2] = B[rowRun + k][colRun + 6];
                B[rowRun + 5 + k][colRun + 6] = a2;
                B[rowRun + k][colRun + 6] = a6;
                B[rowRun + 5 + k][colRun + 3] = B[rowRun + k][colRun + 7];
                B[rowRun + 5 + k][colRun + 7] = a3;
                B[rowRun + k][colRun + 7] = a7;
            }
        }
    }
}

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}