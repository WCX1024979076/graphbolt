#include "../../core/common/parallel.h"
#include "../../core/common/parseCommandLine.h"
#include "../common/graphIO.h"
#include "../../core/common/utils.h"
#include <cassert>
#include <ctime>
#define ITER_NUM 50
char name[100];

double *read_data()
{
    double *data = newA(double, ITER_NUM);
    for (int i = 0; i < ITER_NUM; i++)
        scanf("%lf", &data[i]);
    return data;
}

double *get_avg(double *data[3])
{
    double *data_avg = newA(double, ITER_NUM);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < ITER_NUM; j++)
            data_avg[i] += data[i][j];
    for (int i = 0; i < ITER_NUM; i++)
        data_avg[i] /= 3.0;
    return data_avg;
}

int parallel_main(int argc, char *argv[])
{
    FILE *fp = freopen(NOTES_FILE, "r", stdin);
    double batch_rate, degree_avg;
    long long batch_size, batch_time;

    scanf("batch_rate = %lf", &batch_rate);
    scanf("batch_size = %lf", &batch_size);
    scanf("batch_time = %lf", &batch_time);
    scanf("degree_avg = %lf", &degree_avg);
    double *graphbolt_data[3], *tegra_data[3], *trad_data[3];
    double *initial_data[9];
    for (int i = 0; i < 3; i++)
    {
        scanf("%s", name);
        initial_data[i * 3] = read_data();
        graphbolt_data[i] = read_data();

        scanf("%s", name);
        initial_data[i * 3 + 1] = read_data();
        tegra_data[i] = read_data();

        scanf("%s", name);
        initial_data[i * 3 + 2] = read_data();
        trad_data[i] = read_data();
    }
    double *graphbolt_data_avg = get_avg(graphbolt_data);
    double *tegra_data_avg = get_avg(tegra_data);
    double *trad_data_avg = get_avg(trad_data);

    for (int i = 0; i < 9; i++)
        free(initial_data[i]);
    for (int i = 0; i < 3; i++)
    {
        free(graphbolt_data[i]);
        free(tegra_data[i]);
        free(trad_data[i]);
    }
    free(graphbolt_data_avg);
    free(tegra_data_avg);
    free(trad_data_avg);
    fclose(fp);
    return 0;
}
