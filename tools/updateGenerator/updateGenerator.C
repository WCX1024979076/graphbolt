#include "../../core/common/parallel.h"
#include "../../core/common/parseCommandLine.h"
#include "../common/graphIO.h"
//TODO: 读取图文件 
int parallel_main(int argc, char *argv[]) {
    commandLine P(argc, argv, "[-s] <input SNAP file> <output base ADJ file> <output edgeOperations File> <baserate> <addrate> <batchsize>");
    char *iFile = P.getArgument(5);
    char *obFile = P.getArgument(4);
    char *oeFile = P.getArgument(3);
    double baserate = strtod(P.getArgument(2), NULL);
    double addrate = strtod(P.getArgument(1), NULL);
    int batchsize = atol(P.getArgument(0));

    cout << "Reading graph and creating base SNAP file and edgeOperations File\n";
    edgeArray G = readSNAP(iFile);

    //TODO：从图文件中抽取a%的边作为基础图  
    int Bnums = int(G.nonZeros*baserate);
    int *Bindex = newA(int, Bnums);
    int *addindex = newA(int, G.nonZeros - Bnums);
    bool *rdm = newA(bool, G.nonZeros);
    edge *BE = newA(edge, Bnums);
    edge *AE = newA(edge, G.nonZeros - Bnums);
    //挑选基础图和插入流的下标
    for(int i = 0; i < G.nonZeros; i++) {
        rdm[i] = false;
    }
    int i = 0;
    while(i < Bnums) {
        int random = rand() % G.nonZeros;
        if (rdm[random] == false) {
            rdm[random] = true;
            i++;
        }
    }
    int j = 0;
    int k = 0;
    for(int i = 0; i < G.nonZeros; i++) {
        if(rdm[i]) {
            Bindex[j] = i;
            j++;
        } 
        else {
            addindex[k] = i;
            k++;
        }       
    }
    //构造基础图边数组，写入文件
    parallel_for(int i = 0; i < Bnums; i++) {
        BE[i].u = G.E[Bindex[i]].u;
        BE[i].v = G.E[Bindex[i]].v;
    }
    //构造插入流边数组
    parallel_for(int i = 0; i < G.nonZeros - Bnums; i++) {
        AE[i].u = G.E[addindex[i]].u;
        AE[i].v = G.E[addindex[i]].v;
    }

    //从插入流中抽取batchsize*addrate条插入边
    bool *rdmadd = newA(bool, G.nonZeros - Bnums);
    for(int i = 0; i < G.nonZeros - Bnums; i++) {
        rdmadd[i] = false;
    }
    i = 0;
    int addbatchsize = int(batchsize*addrate);
    cout << "addbatchsize " << addbatchsize << "\n";
    while(i < addbatchsize) {
        int random = rand() % (G.nonZeros - Bnums);
        if (rdmadd[random] == false) {
            rdmadd[random] = true;
            i++;
        }
    }
    cout << "addbatchsize " << i << "\n";
    int *addbatch = newA(int, addbatchsize);
    j = 0;
    for(int i = 0; i < G.nonZeros - Bnums; i++) {
        if(rdmadd[i]) {
            addbatch[j] = i;
            j++;
        }       
    }
    cout << "addbatchsize " << j << "\n";
    edge *AEB = newA(edge, addbatchsize);
    parallel_for(int i = 0; i < addbatchsize; i++) {
        AEB[i].u = AE[addbatch[i]].u;
        AEB[i].v = AE[addbatch[i]].v;
    }

    //从基础中抽取batchsize*（1-deleterate）条删除边
    bool *rdmdelete = newA(bool, Bnums);
    for(int i = 0; i < Bnums; i++) {
        rdmdelete[i] = false;
    }
    i = 0;
    int dltbatchsize = batchsize - addbatchsize;
    cout << "dltbatchsize " << dltbatchsize << "\n";
    while(i < dltbatchsize) {
        int random = rand() % Bnums;
        if(rdmdelete[random] == false) {
            rdmdelete[random] = true;
            i++;
        }
    }
    cout << "dltbatchsize " << i << "\n";
    int *dltbatch = newA(int, dltbatchsize);
    j = 0;
    for (int i = 0; i < Bnums; i++) {
        if(rdmdelete[i]) {
            dltbatch[j] = i;
            j++;
        }       
    }
    cout << "dltbatchsize " << j << "\n";
    edge *DEB = newA(edge, dltbatchsize);
    parallel_for(int i = 0; i < dltbatchsize; i++) {
        DEB[i].u = BE[dltbatch[i]].u;
        DEB[i].v = BE[dltbatch[i]].v;
    }

    cout << "G.nonZeros " << G.nonZeros << "\n";
    cout << "Bnums " << Bnums << "\n";
    
    //输出
    ofstream output_obfile;
    cout << "Printing to file : " << (string)obFile << "\n";
    output_obfile.open(obFile, ios::out);
    output_obfile << fixed;
    output_obfile << setprecision(VAL_PRECISION2);
    for (uintV i = 0; i < Bnums; i++) {
        output_obfile << BE[i].u << "\t" << BE[i].v << "\n";
    }
    ofstream output_oefile;
    cout << "Printing to file : " << (string)oeFile << "\n";
    output_oefile.open(oeFile, ios::out);
    output_oefile << fixed;
    output_oefile << setprecision(VAL_PRECISION2);
    for (uintV i = 0; i < addbatchsize; i++) {
        output_oefile << "a" << "\t" << AEB[i].u << "\t" << AEB[i].v << "\n";
    }
    for (uintV i = 0; i < dltbatchsize; i++) {
        output_oefile << "d" << "\t" << DEB[i].u << "\t" << DEB[i].v << "\n";
    }
    cout << "\n";
    free(Bindex);
    free(addindex);
    free(BE);
    free(AE);
    free(rdmadd);
    free(addbatch);
    free(AEB);
    free(rdmdelete);
    free(dltbatch);
    free(DEB);
}
