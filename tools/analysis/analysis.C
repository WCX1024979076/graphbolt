#include "../../core/common/parallel.h"
#include "../../core/common/parseCommandLine.h"
#include "../common/graphIO.h"
#include "../../core/common/utils.h"
#include <cassert>
#include <ctime>
#define ITER_NUM 50
#define BATCH_TIME 10
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
            data_avg[j] += data[i][j];
    for (int i = 0; i < ITER_NUM; i++)
        data_avg[i] /= 3.0;
    return data_avg;
}

void read_name()
{
    while(1)
    {
        scanf("%s", name);
        if(strlen(name) > 3)
            break;
    }
}

int parallel_main(int argc, char *argv[])
{
    FILE *fp = freopen(NOTES_FILE, "r", stdin);
    
    double batch_rate, degree_avg[5], base_graph_rate, batch_add_rate;
    long long batch_size, batch_time, snap_edge_num, snap_vertex_num;
    scanf("BATCH_SIZE = %lld\n", &batch_size);
    scanf("SNAP_VERTEX_NUM = %lld\n", &snap_vertex_num);
    scanf("SNAP_EDGE_NUM = %lld\n", &snap_edge_num);
    scanf("BASE_GRAPH_RATE = %lf\n", &base_graph_rate);
    scanf("BATCH_ADD_RATE = %lf\n", &batch_add_rate);
    scanf("batch_rate = %lf\n", &batch_rate);
    scanf("batch_size = %lld\n", &batch_size);
    scanf("batch_time = %lld\n", &batch_time);

    FILE *fp1 = freopen("/home/wangcx/tmp/notes6.txt", "a", stdout);
    printf("BATCH_SIZE = %lld\n", batch_size);
    printf("SNAP_VERTEX_NUM = %lld\n", snap_vertex_num);
    printf("SNAP_EDGE_NUM = %lld\n", snap_edge_num);
    printf("BASE_GRAPH_RATE = %f\n", base_graph_rate);
    printf("BATCH_ADD_RATE = %f\n", batch_add_rate);
    printf("batch_rate = %f\n", batch_rate);
    printf("batch_size = %lld\n", batch_size);
    printf("batch_time = %lld\n", batch_time);
    for(int j = 0; j < batch_time; j++) {
        scanf("degree_avg = %lf\n", &degree_avg[j]);
    }
    double *graphbolt_data[BATCH_TIME][3], *tegra_data[BATCH_TIME][3], *trad_data[BATCH_TIME][3]; 
    double *graphbolt_data_avg[BATCH_TIME], *tegra_data_avg[BATCH_TIME], *trad_data_avg[BATCH_TIME]; 
    double *initial_data[9];
    for (int i = 0; i < 3; i++)
    {
        read_name();
        initial_data[i * 3] = read_data();
        for(int j = 0; j < batch_time; j++)
        {
            read_name();
            graphbolt_data[j][i] = read_data();
        }

        read_name();
        initial_data[i * 3 + 1] = read_data();
        for(int j = 0; j < batch_time; j++)
        {
            read_name();
            tegra_data[j][i] = read_data();
        }

        read_name();
        initial_data[i * 3 + 2] = read_data();
        for(int j = 0; j < batch_time; j++)
        {
            read_name();
            trad_data[j][i] = read_data();
        }
    }

    for(int j = 0; j < batch_time; j++)
    {
        graphbolt_data_avg[j] = get_avg(graphbolt_data[j]);
        tegra_data_avg[j] = get_avg(tegra_data[j]);
        trad_data_avg[j] = get_avg(trad_data[j]);
    }
    double *graphbolt_sum = newA(double, ITER_NUM);
    double *tegra_sum = newA(double, ITER_NUM);
    double *trad_sum = newA(double, ITER_NUM);
    for(int j = 0; j < batch_time; j++)
    {
        for(int i = 0; i < ITER_NUM; i++)
        {
            if(i == 0)
            {
                graphbolt_sum[i] = graphbolt_data_avg[j][i];
                tegra_sum[i] = tegra_data_avg[j][i];
                trad_sum[i] = trad_data_avg[j][i];
            }
            else
            {
                graphbolt_sum[i] = graphbolt_sum[i - 1] + graphbolt_data_avg[j][i];
                tegra_sum[i] = tegra_sum[i - 1] + tegra_data_avg[j][i];
                trad_sum[i] = trad_sum[i - 1] + trad_data_avg[j][i];
            }
        }
        int n1 = -1, n2 = -1; // TODO: 修正评估算法
        double min_time = 99999999999;
        for(int n_1 = 0; n_1 < ITER_NUM; n_1++)
        {
            for(int n_2 = n_1 + 1; n_2 < ITER_NUM; n_2++)
            {
                double sum = 0;
                if(n_1 != 0)
                {
                    sum += graphbolt_sum[n_1 - 1];
                    sum += tegra_sum[n_2 - 1] - tegra_sum[n_1 - 1];
                    sum += trad_sum[ITER_NUM - 1] - trad_sum[n_2 - 1]; 
                }
                else
                {
                    sum += tegra_sum[n_2 - 1];
                    sum += trad_sum[ITER_NUM - 1] - trad_sum[n_2 - 1];
                }
                if(sum < min_time)
                {
                    n1 = n_1;
                    n2 = n_2;
                    min_time = sum;
                }
            }
        }
        printf("degree_avg = %f\n", degree_avg[j]);
        printf("n1 = %d, n2 = %d\n", n1, n2);
    }
    
    for(int j = 0; j < batch_time; j++)
    {
        for(int i = 0; i < ITER_NUM; i++)
            printf("%f ", graphbolt_data_avg[j][i]);
        printf("\n");
        for(int i = 0; i < ITER_NUM; i++)
            printf("%f ", tegra_data_avg[j][i]);
        printf("\n");
        for(int i = 0; i < ITER_NUM; i++)
            printf("%f ", trad_data_avg[j][i]);
        printf("\n");
    }
    for (int i = 0; i < 9; i++)
        free(initial_data[i]);
    for(int j = 0; j < batch_time; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            free(graphbolt_data[j][i]);
            free(tegra_data[j][i]);
            free(trad_data[j][i]);
        }
        free(graphbolt_data_avg[j]);
        free(tegra_data_avg[j]);
        free(trad_data_avg[j]);
    }
    free(graphbolt_sum);
    free(tegra_sum);
    free(trad_sum);
    return 0;
}
