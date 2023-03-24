#include "../../core/common/parallel.h"
#include "../../core/common/parseCommandLine.h"
#include "../common/graphIO.h"
#include "../../core/common/utils.h"
#include <cassert>
#include <ctime>

void CalcPangeRank(bool *ingraph, edgeArray G, int current_batch, string output_file_path);

void GetRand(bool* rdm, long nonZeros, int Bnums)
{
    for(int i = 0; i < nonZeros; i++) {
        rdm[i] = false;
    }
    int i = 0;
    while(i < Bnums) {
        int random = rand() % nonZeros;
        if (rdm[random] == false) {
            rdm[random] = true;
            i++;
        }
    }
}
int parallel_main(int argc, char *argv[]) {
    unsigned int seed = time(nullptr);
    srand(seed);
    
    commandLine P(argc, argv, "[-s] <input SNAP file> <output base ADJ file> <output edgeOperations File> <baserate> <addrate> <batchsize> <batchtime> <output_pagerank>");
    char *iFile = P.getArgument(7);
    char *obFile = P.getArgument(6);
    char *oeFile = P.getArgument(5);
    double baserate = strtod(P.getArgument(4), NULL);
    double addrate = strtod(P.getArgument(3), NULL);
    int batchsize = atol(P.getArgument(2));
    int batchtime = atol(P.getArgument(1));
    char *output_file_path = P.getArgument(0);
    
    cout << "Reading graph and creating base SNAP file and edgeOperations File\n";
    edgeArray G = readSNAP(iFile);

    //TODO：从图文件中抽取a%的边作为基础图  
    int Bnums = int(G.nonZeros * baserate);
    int Anums = int(batchsize * addrate);
    bool *rdm = newA(bool, G.nonZeros);
    bool *rdm1 = newA(bool, G.nonZeros);
    bool *ingraph = newA(bool, G.nonZeros);
    edge *BE = newA(edge, G.nonZeros);
    edge *AE = newA(edge, G.nonZeros);
    edge *DE = newA(edge, G.nonZeros);
    int ingraph_num = 0;

    GetRand(rdm, G.nonZeros, Bnums);

    int basic_num = 0;
    int add_num = 0;
    int del_num = 0;
    for(int i = 0; i < G.nonZeros; i++) {
        if(rdm[i]) {
            BE[basic_num++] =  G.E[i];
            ingraph[i] = true;
            ingraph_num++;
        }
    }

    cout << "G.nonZeros " << G.nonZeros << "\n";
    cout << "Bnums " << Bnums << "\n";

    ofstream output_obfile;
    cout << "Printing to file : " << (string)obFile << "\n";
    output_obfile.open(obFile, ios::out);
    output_obfile << fixed;
    output_obfile << setprecision(VAL_PRECISION2);
    for (uintV i = 0; i < basic_num; i++)
        output_obfile << BE[i].u << "\t" << BE[i].v << "\n";
    
    ofstream output_oefile;
    cout << "Printing to file : " << (string)oeFile << "\n";
    output_oefile.open(oeFile, ios::out);
    output_oefile << fixed;
    output_oefile << setprecision(VAL_PRECISION2);

    CalcPangeRank(ingraph, G, 0, output_file_path);

    for(int current_batch = 1; current_batch <= batchtime; current_batch++)
    {
        int add_num = 0, add_index = 0;
        int del_num = 0, del_index = 0;
        assert(G.nonZeros - ingraph_num >= Anums);
        assert(ingraph_num >= batchsize - Anums);
        GetRand(rdm, G.nonZeros - ingraph_num, Anums);
        GetRand(rdm1, ingraph_num, batchsize - Anums);
        for(int i = 0; i < G.nonZeros; i++) {
            if(ingraph[i]) {
                if(rdm1[del_index]) {
                    ingraph[i] = false;
                    DE[del_num++] = G.E[i];
                    ingraph_num--;
                }
                del_index++;
            } else {
                if(rdm[add_index]) {
                    ingraph[i] = true;
                    AE[add_num++] = G.E[i];
                    ingraph_num++;
                }
                add_index++;
            }
        }
        cout << "current_batch " << current_batch << "\t";
        cout << "addbatchsize " << add_num << "\t";
        cout << "dltbatchsize " << del_num << "\n";
        for (uintV i = 0; i < add_num; i++)
            output_oefile << "a" << "\t" << AE[i].u << "\t" << AE[i].v << "\n";
        for (uintV i = 0; i < del_num; i++)
            output_oefile << "d" << "\t" << DE[i].u << "\t" << DE[i].v << "\n";
        
        CalcPangeRank(ingraph, G, current_batch, output_file_path);
    }

    output_obfile.close();
    output_oefile.close();
    
    free(rdm);
    free(rdm1);
    free(ingraph);
    free(BE);
    free(AE);
    free(DE);

    return 0;
}


void CalcPangeRank(bool *ingraph, edgeArray G, int current_batch, string output_file_path) {
    uintV ecnt = 0;
    uintV ncnt = 0;
    edge *BE = newA(edge, G.nonZeros);
    for(int i = 0; i < G.nonZeros; i++) {
        if(ingraph[i]) {
            BE[ecnt++] = G.E[i];
            ncnt = max(ncnt, G.E[i].u + 1);
            ncnt = max(ncnt, G.E[i].v + 1);
        }
    }

	double eps=0.1;
    int* d_in   = newA(int, ncnt);
    int* d_out  = newA(int, ncnt);
    double* ra  = newA(double, ncnt);
    double* rb  = newA(double, ncnt);
    for(int i = 0; i < ncnt; i++) {
		ra[i] = 1;
        rb[i] = 0;
        d_in[i] = 0;
        d_out[i] = 0;
    }
    for(int i = 0; i < ecnt; i++) {
        d_out[BE[i].u] ++;
        d_in[BE[i].v] ++;
    }
	while(eps > 0.0000001)//set ε=10^(-7), control the number of iterations
	{
        eps = 0;
		for(int i = 0;i < ecnt; i++) {
			rb[BE[i].v] += ra[BE[i].u] / d_out[BE[i].u]; //first step to initialize the rank value
        }
		for(int i = 0;i < ncnt; i++) {
			rb[i] = rb[i] * 0.85 + 0.15; //add the random jumping coefficient β, and set β=0.8
			eps += ra[i] > rb[i] ? (ra[i] - rb[i]) : (rb[i] - ra[i]);//compute the Difference between the old rank value and new rank value, and update the ε
			ra[i] = rb[i];
			rb[i] = 0;
		}
	}
    string curr_output_file_path = output_file_path + to_string(current_batch);
    std::cout << "Printing to file : " << curr_output_file_path << "\n";
    ofstream output_file;
    output_file.open(curr_output_file_path, ios::out);
    output_file << fixed;
    output_file << setprecision(VAL_PRECISION2);
    for (int i = 0; i < ncnt; i++) {
        output_file << i << " " << d_in[i] << " " << d_out[i] << " ";
        output_file << ra[i] << "\n";
    }
    output_file.close();

    free(d_in);
    free(d_out);
    free(ra);
    free(rb);
    return ;
}