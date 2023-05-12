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

    FILE *fp1 = freopen("/home/wangcx/tmp/notes5.txt", "a", stdout);
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
    for(int j = 0; j < batch_time; j++)
    {
        int n1 = -1, n2 = -1; // TODO: 修正评估算法
        for(int i = 0; i < ITER_NUM; i++)
        {
            if(trad_data_avg[j][i] < graphbolt_data_avg[j][i] && n1 == -1)
                n1 = i;
            if(tegra_data_avg[j][i] < graphbolt_data_avg[j][i] && n2 == -1)
                n2 = i;
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
    return 0;
}
