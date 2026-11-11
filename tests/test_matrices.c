#include "../headers/matrices.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* ========================================================================== */
/*                               mini framework                               */
/* ========================================================================== */

#define EPS 1e-4f

#define C_RED   "\033[31m"
#define C_GREEN "\033[32m"
#define C_DIM   "\033[2m"
#define C_RESET "\033[0m"

static int g_tests_run = 0;
static int g_tests_failed = 0;
static int g_current_failed = 0;

#define CHECK(cond, ...)                                                    \
    do {                                                                    \
        if (!(cond)) {                                                      \
            g_current_failed = 1;                                           \
            printf("    " C_RED "assert failed" C_RESET " %s:%d: ",         \
                   __FILE__, __LINE__);                                     \
            printf(__VA_ARGS__);                                            \
            printf("\n");                                                   \
        }                                                                   \
    } while (0)

/* each test runs in its own process, so a crash only fails that test */
static void run_test(void (*fn)(void), const char *name)
{
    pid_t pid;
    int   status;

    g_tests_run++;
    fflush(stdout);
    pid = fork();
    if (pid == 0) {
        g_current_failed = 0;
        fn();
        fflush(stdout);
        _exit(g_current_failed ? 1 : 0);
    }
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status)) {
        g_tests_failed++;
        printf(C_RED "[CRASH]" C_RESET " %s (signal %d)\n", name, WTERMSIG(status));
    } else if (WEXITSTATUS(status) != 0) {
        g_tests_failed++;
        printf(C_RED "[FAIL]" C_RESET "  %s\n", name);
    } else
        printf(C_GREEN "[ OK ]" C_RESET "  %s\n", name);
}

#define RUN(test) run_test(test, #test)

/* ========================================================================== */
/*                                  helpers                                   */
/* ========================================================================== */

static t_matrix *mat_new(int rows, int cols, const float *data)
{
    t_matrix *m = malloc(sizeof(t_matrix));
    m->num_of_rows = rows;
    m->num_of_cols = cols;
    m->matrix = calloc((size_t)(rows * cols > 0 ? rows * cols : 1), sizeof(float));
    if (data)
        memcpy(m->matrix, data, (size_t)(rows * cols) * sizeof(float));
    return m;
}

static void mat_free(t_matrix *m)
{
    if (!m)
        return;
    free(m->matrix);
    free(m);
}

/* naive reference implementation */
static t_matrix *mat_ref_mul(const t_matrix *a, const t_matrix *b)
{
    t_matrix *r = mat_new(a->num_of_rows, b->num_of_cols, NULL);
    for (int i = 0; i < a->num_of_rows; i++)
        for (int j = 0; j < b->num_of_cols; j++) {
            double s = 0.0;
            for (int k = 0; k < a->num_of_cols; k++)
                s += (double)a->matrix[i * a->num_of_cols + k]
                   * (double)b->matrix[k * b->num_of_cols + j];
            r->matrix[i * r->num_of_cols + j] = (float)s;
        }
    return r;
}

static t_matrix *mat_identity(int n)
{
    t_matrix *m = mat_new(n, n, NULL);
    for (int i = 0; i < n; i++)
        m->matrix[i * n + i] = 1.0f;
    return m;
}

static t_matrix *mat_random(int rows, int cols)
{
    t_matrix *m = mat_new(rows, cols, NULL);
    for (int i = 0; i < rows * cols; i++)
        m->matrix[i] = (float)(rand() % 2001 - 1000) / 100.0f; /* [-10, 10] */
    return m;
}

static int nearly_equal(float x, float y)
{
    float diff = fabsf(x - y);
    float scale = fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y)));
    return diff <= EPS * scale;
}

static void mat_print(const char *name, const t_matrix *m)
{
    printf(C_DIM "      %s (%dx%d):\n", name, m->num_of_rows, m->num_of_cols);
    for (int i = 0; i < m->num_of_rows; i++) {
        printf("        ");
        for (int j = 0; j < m->num_of_cols; j++)
            printf("%9.3f ", m->matrix[i * m->num_of_cols + j]);
        printf("\n");
    }
    printf(C_RESET);
}

/* compare dimensions and every cell; prints both matrices on mismatch */
static void expect_matrix(const t_matrix *got, const t_matrix *exp)
{
    CHECK(got != NULL, "result is NULL");
    if (!got)
        return;
    CHECK(got->num_of_rows == exp->num_of_rows,
          "rows: got %d, expected %d", got->num_of_rows, exp->num_of_rows);
    CHECK(got->num_of_cols == exp->num_of_cols,
          "cols: got %d, expected %d", got->num_of_cols, exp->num_of_cols);
    if (got->num_of_rows != exp->num_of_rows
        || got->num_of_cols != exp->num_of_cols)
        return;
    int bad = 0;
    for (int i = 0; i < exp->num_of_rows * exp->num_of_cols; i++)
        if (!nearly_equal(got->matrix[i], exp->matrix[i])) {
            if (bad < 5)
                CHECK(0, "cell [%d][%d]: got %f, expected %f",
                      i / exp->num_of_cols, i % exp->num_of_cols,
                      got->matrix[i], exp->matrix[i]);
            bad++;
        }
    if (bad) {
        if (bad > 5)
            printf("    ... and %d more wrong cells\n", bad - 5);
        if (exp->num_of_rows <= 8 && exp->num_of_cols <= 8) {
            mat_print("got", got);
            mat_print("expected", exp);
        }
    }
}

/* multiply a*b and compare with hand-written expected data */
static void expect_product(int ar, int ac, const float *ad,
                           int br, int bc, const float *bd,
                           const float *expected)
{
    t_matrix *a = mat_new(ar, ac, ad);
    t_matrix *b = mat_new(br, bc, bd);
    t_matrix *e = mat_new(ar, bc, expected);
    t_matrix *r = multiply_matrices(a, b);
    expect_matrix(r, e);
    mat_free(a);
    mat_free(b);
    mat_free(e);
    mat_free(r);
}

/* multiply a*b and compare with the reference implementation */
static void expect_matches_reference(t_matrix *a, t_matrix *b)
{
    t_matrix *e = mat_ref_mul(a, b);
    t_matrix *r = multiply_matrices(a, b);
    expect_matrix(r, e);
    mat_free(e);
    mat_free(r);
}

/* ========================================================================== */
/*                           invalid input / errors                           */
/* ========================================================================== */

static void test_null_a(void)
{
    t_matrix *b = mat_new(2, 2, NULL);
    CHECK(multiply_matrices(NULL, b) == NULL, "expected NULL for a == NULL");
    mat_free(b);
}

static void test_null_b(void)
{
    t_matrix *a = mat_new(2, 2, NULL);
    CHECK(multiply_matrices(a, NULL) == NULL, "expected NULL for b == NULL");
    mat_free(a);
}

static void test_null_both(void)
{
    CHECK(multiply_matrices(NULL, NULL) == NULL, "expected NULL for NULL, NULL");
}

static void test_dimension_mismatch(void)
{
    t_matrix *a = mat_new(2, 3, NULL);
    t_matrix *b = mat_new(2, 3, NULL);
    CHECK(multiply_matrices(a, b) == NULL, "2x3 * 2x3 should return NULL");
    mat_free(a);
    mat_free(b);
}

static void test_dimension_mismatch_reversed_order(void)
{
    /* 2x3 * 3x4 is valid, but 3x4 * 2x3 is not */
    t_matrix *a = mat_new(3, 4, NULL);
    t_matrix *b = mat_new(2, 3, NULL);
    CHECK(multiply_matrices(a, b) == NULL, "3x4 * 2x3 should return NULL");
    mat_free(a);
    mat_free(b);
}

/* ========================================================================== */
/*                          small hand-computed cases                         */
/* ========================================================================== */

static void test_1x1(void)
{
    expect_product(1, 1, (float[]){3}, 1, 1, (float[]){4}, (float[]){12});
}

static void test_2x2(void)
{
    expect_product(2, 2, (float[]){1, 2,
                                   3, 4},
                   2, 2, (float[]){5, 6,
                                   7, 8},
                   (float[]){19, 22,
                             43, 50});
}

static void test_3x3(void)
{
    expect_product(3, 3, (float[]){1, 2, 3,
                                   4, 5, 6,
                                   7, 8, 9},
                   3, 3, (float[]){9, 8, 7,
                                   6, 5, 4,
                                   3, 2, 1},
                   (float[]){ 30,  24, 18,
                              84,  69, 54,
                             138, 114, 90});
}

static void test_2x3_times_3x2(void)
{
    expect_product(2, 3, (float[]){1, 2, 3,
                                   4, 5, 6},
                   3, 2, (float[]){ 7,  8,
                                    9, 10,
                                   11, 12},
                   (float[]){ 58,  64,
                             139, 154});
}

static void test_3x2_times_2x3(void)
{
    expect_product(3, 2, (float[]){1, 2,
                                   3, 4,
                                   5, 6},
                   2, 3, (float[]){7, 8, 9,
                                   10, 11, 12},
                   (float[]){ 27,  30,  33,
                              61,  68,  75,
                              95, 106, 117});
}

static void test_2x4_times_4x1(void)
{
    /* same case as in main() */
    expect_product(2, 4, (float[]){1, 2, 3, 4,
                                   2, 0, 1, 2},
                   4, 1, (float[]){2,
                                   1,
                                   0,
                                   3},
                   (float[]){16,
                             10});
}

static void test_row_times_col_dot_product(void)
{
    /* 1x3 * 3x1 -> 1x1 */
    expect_product(1, 3, (float[]){1, 2, 3},
                   3, 1, (float[]){4, 5, 6},
                   (float[]){32});
}

static void test_col_times_row_outer_product(void)
{
    /* 3x1 * 1x3 -> 3x3 */
    expect_product(3, 1, (float[]){1, 2, 3},
                   1, 3, (float[]){4, 5, 6},
                   (float[]){ 4,  5,  6,
                              8, 10, 12,
                             12, 15, 18});
}

static void test_1x4_times_4x2(void)
{
    expect_product(1, 4, (float[]){1, 0, -1, 2},
                   4, 2, (float[]){1, 2,
                                   3, 4,
                                   5, 6,
                                   7, 8},
                   (float[]){10, 12});
}

static void test_negative_values(void)
{
    expect_product(2, 2, (float[]){-1,  2,
                                    3, -4},
                   2, 2, (float[]){ 5, -6,
                                   -7,  8},
                   (float[]){-19,  22,
                              43, -50});
}

static void test_fractional_values(void)
{
    expect_product(2, 2, (float[]){0.5f, 1.5f,
                                   2.0f, 0.25f},
                   2, 2, (float[]){4.0f, 2.0f,
                                   1.0f, 8.0f},
                   (float[]){3.5f, 13.0f,
                             8.25f, 6.0f});
}

static void test_zero_matrix(void)
{
    expect_product(2, 3, (float[]){1, 2, 3,
                                   4, 5, 6},
                   3, 2, (float[]){0, 0,
                                   0, 0,
                                   0, 0},
                   (float[]){0, 0,
                             0, 0});
}

/* ========================================================================== */
/*                            algebraic properties                            */
/* ========================================================================== */

static void test_identity_left(void)
{
    t_matrix *i = mat_identity(3);
    t_matrix *a = mat_new(3, 4, (float[]){1, 2, 3, 4,
                                          5, 6, 7, 8,
                                          9, 10, 11, 12});
    t_matrix *r = multiply_matrices(i, a);
    expect_matrix(r, a);
    mat_free(i);
    mat_free(a);
    mat_free(r);
}

static void test_identity_right(void)
{
    t_matrix *i = mat_identity(4);
    t_matrix *a = mat_new(3, 4, (float[]){1, 2, 3, 4,
                                          5, 6, 7, 8,
                                          9, 10, 11, 12});
    t_matrix *r = multiply_matrices(a, i);
    expect_matrix(r, a);
    mat_free(i);
    mat_free(a);
    mat_free(r);
}

static void test_associativity(void)
{
    t_matrix *a = mat_random(3, 4);
    t_matrix *b = mat_random(4, 2);
    t_matrix *c = mat_random(2, 5);

    t_matrix *ab = multiply_matrices(a, b);
    t_matrix *bc = multiply_matrices(b, c);
    CHECK(ab && bc, "intermediate product is NULL");
    if (ab && bc) {
        t_matrix *ab_c = multiply_matrices(ab, c);
        t_matrix *a_bc = multiply_matrices(a, bc);
        expect_matrix(ab_c, a_bc);
        mat_free(ab_c);
        mat_free(a_bc);
    }
    mat_free(a);
    mat_free(b);
    mat_free(c);
    mat_free(ab);
    mat_free(bc);
}

static void test_inputs_not_modified(void)
{
    float ad[] = {1, 2, 3, 4, 5, 6};
    float bd[] = {7, 8, 9, 10, 11, 12};
    t_matrix *a = mat_new(2, 3, ad);
    t_matrix *b = mat_new(3, 2, bd);
    t_matrix *r = multiply_matrices(a, b);

    CHECK(a->num_of_rows == 2 && a->num_of_cols == 3, "a dimensions changed");
    CHECK(b->num_of_rows == 3 && b->num_of_cols == 2, "b dimensions changed");
    CHECK(memcmp(a->matrix, ad, sizeof(ad)) == 0, "a data changed");
    CHECK(memcmp(b->matrix, bd, sizeof(bd)) == 0, "b data changed");
    CHECK(r == NULL || (r != a && r != b), "result aliases an input");
    CHECK(r == NULL || (r->matrix != a->matrix && r->matrix != b->matrix),
          "result buffer aliases an input buffer");
    mat_free(a);
    mat_free(b);
    mat_free(r);
}

static void test_square_self_multiply(void)
{
    t_matrix *a = mat_new(2, 2, (float[]){1, 1,
                                          1, 0});
    t_matrix *e = mat_new(2, 2, (float[]){2, 1,
                                          1, 1});
    t_matrix *r = multiply_matrices(a, a);
    expect_matrix(r, e);
    mat_free(a);
    mat_free(e);
    mat_free(r);
}

/* ========================================================================== */
/*                         randomized vs. reference                           */
/* ========================================================================== */

static void test_random_square_sizes(void)
{
    for (int n = 1; n <= 16; n++) {
        t_matrix *a = mat_random(n, n);
        t_matrix *b = mat_random(n, n);
        expect_matches_reference(a, b);
        mat_free(a);
        mat_free(b);
    }
}

static void test_random_rectangular_sizes(void)
{
    for (int iter = 0; iter < 200; iter++) {
        int m = rand() % 12 + 1;
        int k = rand() % 12 + 1;
        int n = rand() % 12 + 1;
        t_matrix *a = mat_random(m, k);
        t_matrix *b = mat_random(k, n);
        int before = g_current_failed;
        g_current_failed = 0;
        expect_matches_reference(a, b);
        if (g_current_failed)
            printf("    ^ failing shape: %dx%d * %dx%d\n", m, k, k, n);
        g_current_failed |= before;
        mat_free(a);
        mat_free(b);
        if (g_current_failed)
            break;
    }
}

static void test_large_matrix(void)
{
    t_matrix *a = mat_random(64, 100);
    t_matrix *b = mat_random(100, 48);
    expect_matches_reference(a, b);
    mat_free(a);
    mat_free(b);
}

/* ========================================================================== */

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    srand(42);

    printf("\n== invalid input ==\n");
    RUN(test_null_a);
    RUN(test_null_b);
    RUN(test_null_both);
    RUN(test_dimension_mismatch);
    RUN(test_dimension_mismatch_reversed_order);

    printf("\n== hand-computed ==\n");
    RUN(test_1x1);
    RUN(test_2x2);
    RUN(test_3x3);
    RUN(test_2x3_times_3x2);
    RUN(test_3x2_times_2x3);
    RUN(test_2x4_times_4x1);
    RUN(test_row_times_col_dot_product);
    RUN(test_col_times_row_outer_product);
    RUN(test_1x4_times_4x2);
    RUN(test_negative_values);
    RUN(test_fractional_values);
    RUN(test_zero_matrix);

    printf("\n== properties ==\n");
    RUN(test_identity_left);
    RUN(test_identity_right);
    RUN(test_associativity);
    RUN(test_inputs_not_modified);
    RUN(test_square_self_multiply);

    printf("\n== randomized ==\n");
    RUN(test_random_square_sizes);
    RUN(test_random_rectangular_sizes);
    RUN(test_large_matrix);

    printf("\n%d/%d passed\n", g_tests_run - g_tests_failed, g_tests_run);
    return g_tests_failed ? 1 : 0;
}
