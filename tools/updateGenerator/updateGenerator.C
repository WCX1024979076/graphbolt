#include "../../core/common/parallel.h"
#include "../../core/common/parseCommandLine.h"
#include "../common/graphIO.h"

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
    commandLine P(argc, argv, "[-s] <input SNAP file> <output base ADJ file> <output edgeOperations File> <baserate> <batchsize> <batchtime>");
    char *iFile = P.getArgument(5);
    char *obFile = P.getArgument(4);
    char *oeFile = P.getArgument(3);
    double baserate = strtod(P.getArgument(2), NULL);
    int batchsize = atol(P.getArgument(1));
    int batchtime = atol(P.getArgument(0));

    cout << "Reading graph and creating base SNAP file and edgeOperations File\n";
    edgeArray G = readSNAP(iFile);

    //TODO：从图文件中抽取a%的边作为基础图  
    int Bnums = int(G.nonZeros*baserate);
    bool *rdm = newA(bool, G.nonZeros);
    bool *ingraph = newA(bool, G.nonZeros);
    edge *BE = newA(edge, G.nonZeros);
    edge *AE = newA(edge, G.nonZeros);
    edge *DE = newA(edge, G.nonZeros);

    GetRand(rdm, G.nonZeros, Bnums);

    int basic_num = 0;
    int add_num = 0;
    int del_num = 0;
    for(int i = 0; i < G.nonZeros; i++) {
        if(rdm[i]) {
            BE[basic_num++] =  G.E[i];
            ingraph[i] = true;
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

    while(batchtime --)
    {
        int add_num = 0;
        int del_num = 0;
        GetRand(rdm, G.nonZeros, batchsize);
        for(int i = 0; i < G.nonZeros; i++) {
            if(rdm[i]) {
                if(ingraph[i])
                    DE[del_num++] = G.E[i];
                else
                    AE[add_num++] = G.E[i];
                ingraph[i] = ~ingraph[i];
            }
        }
        cout << "addbatchsize " << add_num << "\t";
        cout << "dltbatchsize " << del_num << "\n";
        for (uintV i = 0; i < add_num; i++)
            output_oefile << "a" << "\t" << AE[i].u << "\t" << AE[i].v << "\n";
        for (uintV i = 0; i < del_num; i++)
            output_oefile << "d" << "\t" << DE[i].u << "\t" << DE[i].v << "\n";
    }

    output_obfile.close();
    output_oefile.close();
    
    free(rdm);
    free(ingraph);
    free(BE);
    free(AE);
    free(DE);

    return 0;
}
