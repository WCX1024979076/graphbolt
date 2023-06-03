#include "../../core/common/parallel.h"
#include "../../core/common/parseCommandLine.h"
#include "../common/graphIO.h"
#include "../../core/common/utils.h"
#include <cassert>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#define ITER_NUM 20
#define BATCH_TIME 1
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
    
    double batch_rate, base_graph_rate, batch_add_rate;
    long long batch_size, batch_time, snap_edge_num, snap_vertex_num, degree_avg;
    scanf("BATCH_SIZE = %lld\n", &batch_size);
    scanf("SNAP_VERTEX_NUM = %lld\n", &snap_vertex_num);
    scanf("SNAP_EDGE_NUM = %lld\n", &snap_edge_num);
    scanf("BASE_GRAPH_RATE = %lf\n", &base_graph_rate);
    scanf("BATCH_ADD_RATE = %lf\n", &batch_add_rate);
    scanf("BATCH_TIME = %lld\n", &batch_time);
    scanf("DEGREE_AVG = %lld\n", &degree_avg);

    FILE *fp1 = freopen("/home/wangcx/tmp/notes_end.txt", "a", stdout), *fp2;
    if(access("/home/wangcx/tmp/result_end_3.csv", F_OK) != -1)
    {
       fp2 = fopen("/home/wangcx/tmp/result_end_3.csv", "a");
    }
    else
    {
       fp2 = fopen("/home/wangcx/tmp/result_end_3.csv", "a");
       fprintf(fp2, "batch_size,snap_vertex_num,snap_edge_num,batch_add_rate,degree_avg,graphbolt_iter\n");
    }
    printf("BATCH_SIZE = %lld\n", batch_size);
    printf("SNAP_VERTEX_NUM = %lld\n", snap_vertex_num);
    printf("SNAP_EDGE_NUM = %lld\n", snap_edge_num);
    printf("BASE_GRAPH_RATE = %f\n", base_graph_rate);
    printf("BATCH_ADD_RATE = %f\n", batch_add_rate);
    printf("BATCH_TIME = %lld\n", batch_time);
    printf("DEGREE_AVG = %lld\n", degree_avg);
    
    fprintf(fp2, "%lld,%lld,%lld,%f,%lld", batch_size, snap_vertex_num, snap_edge_num, batch_add_rate, degree_avg);
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
    //TODO：切换方法？首次大于？多次大于？ 时间最优/空间最优
    double *graphbolt_sum = newA(double, ITER_NUM);
    double *tegra_sum = newA(double, ITER_NUM);
    double *trad_sum = newA(double, ITER_NUM);
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
        if(n1 == -1)
            n1 = ITER_NUM;
        if(n2 == -1)
            n2 = ITER_NUM;
        printf("n1 = %d, n2 = %d\n", n1, n2);
        fprintf(fp2, ",%d\n", min(n1, n2));
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
    fclose(fp2);
    return 0;
}
