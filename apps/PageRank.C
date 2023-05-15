// Copyright (c) 2020 Mugilan Mariappan, Joanna Che and Keval Vora.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "../core/common/utils.h"
#include "../core/graphBolt/GraphBoltEngine_simple.h"
#include "../core/main.h"
#include <math.h>

// ======================================================================
// PAGERANKINFO
// ======================================================================
template <class vertex>
class PageRankInfo {
public:
  // Should I just use vectors for this?
  graph<vertex> *my_graph; //图结构
  uintV n;                 //点数目
  double epsilon;          //ε 阈值
  double damping;          //权重
  long *out_degrees;       //出度数组

  PageRankInfo() : my_graph(nullptr), n(0), epsilon(0), damping(0), out_degrees(nullptr) {}

  PageRankInfo(graph<vertex> *_my_graph, uintV _n, double _epsilon, double _damping)
      : my_graph(_my_graph), n(_n), epsilon(_epsilon), damping(_damping) {
    if (n > 0) {
      out_degrees = newA(long, n);
      parallel_for(uintV i = 0; i < n; i++) { out_degrees[i] = 0; } //出度数组初始化
    }
  }

  void init(){
    parallel_for(uintV i = 0; i < n; i++) {
      out_degrees[i] = my_graph->V[i].getOutDegree(); //出度数组初始化
    }    
  }

  void copy(const PageRankInfo &object) {
    if (object.n > n) {
      if (n == 0) {
        n = object.n;
        out_degrees = newA(long, n);  //原本无
      } else {
        // realloc
        n = object.n;
        out_degrees = renewA(long, out_degrees, n); //重新分配内存
      }
    }
    long min_n = std::min(object.n, n);
    parallel_for(uintV i = 0; i < min_n; i++) {
      out_degrees[i] = object.out_degrees[i];   //复制出度数组
    }
    epsilon = object.epsilon;
    damping = object.damping;
  }

  ~PageRankInfo() {
    if (n > 0)
      deleteA(out_degrees);
  }

  void processUpdates(edgeArray &edge_additions, edgeArray &edge_deletions) { //新增边集和删减边集
    // Increase out_degrees array size
    if (edge_additions.maxVertex >= n) { //最大的点数大于n
      uintV n_old = n;
      n = edge_additions.maxVertex + 1;
      out_degrees = renewA(long, out_degrees, n); //重新分配
      parallel_for(uintV i = n_old; i < n; i++) { out_degrees[i] = 0; } //新增点出度置为0
    }

    parallel_for(long i = 0; i < edge_additions.size; i++) {
      uintV source = edge_additions.E[i].source;
      uintV destination = edge_additions.E[i].destination;
      writeAdd(&out_degrees[source], (long)1); //出度同步加一
    }
    parallel_for(long i = 0; i < edge_deletions.size; i++) {
      uintV source = edge_deletions.E[i].source;
      uintV destination = edge_deletions.E[i].destination;
      writeAdd(&out_degrees[source], (long)-1); //出度同步减一
    }
  }

  void cleanup() {}

  inline long getOutDegree(const uintV &v) { //查询出度
    return (v < n) ? out_degrees[v] : 0;
  }
};

// ======================================================================
// AGGREGATEVALUE AND VERTEXVALUE INITIALIZATION
// ======================================================================
double initial_aggregation_value = 0;  //初始化聚合值
double initial_vertex_value = 1;       //初始顶点值
double aggregation_value_identity = 0; //聚合值标识 0为和 1为积
double vertex_value_identity = 0;      //点值特性   0为和 1为积
template <class AggregationValueType, class GlobalInfoType> //聚合值类型 全局信息类型
inline void
initializeAggregationValue(const uintV &v, //初始化聚合值
                           AggregationValueType &v_aggregation_value,
                           const GlobalInfoType &global_info) {
  v_aggregation_value = initial_aggregation_value;
}

template <class VertexValueType, class GlobalInfoType> //初始化点权
inline void initializeVertexValue(const uintV &v,
                                  VertexValueType &v_vertex_value,
                                  const GlobalInfoType &global_info) {
  v_vertex_value = initial_vertex_value;
}

template <class AggregationValueType>
inline AggregationValueType &aggregationValueIdentity() { //查询聚合值标识?
  return aggregation_value_identity;
}

template <class VertexValueType> inline VertexValueType &vertexValueIdentity() { //查询点权特性
  return vertex_value_identity;
}
// ======================================================================
// ACTIVATE VERTEX/COMPUTE VERTEX FOR A GIVEN ITERATION 为给定的迭代激活顶点/计算顶点
// ======================================================================
template <class GlobalInfoType>
inline bool forceActivateVertexForIteration(const uintV &v, int iter, //强制激活顶点进行迭代
                                            const GlobalInfoType &global_info) {
  if (iter == 1) {
    return true;
  } else {
    return false;
  }
}

template <class GlobalInfoType>
inline bool forceComputeVertexForIteration(const uintV &v, int iter, //强制计算迭代的顶点 第一次迭代强制更新 第二次无
                                           const GlobalInfoType &global_info) {
  if (iter == 1) {
    return true;
  } else {
    return false;
  }
}
inline bool shouldUseDelta(int iter) { //应该使用 Delta
  if (iter == 1) {
    return false;
  } else {
    return true;
  }
}

// ======================================================================
// ADD TO OR REMOVE FROM AGGREGATION VALUES 添加到聚合值或从聚合值中删除
// ======================================================================
template <class AggregationValueType, class GlobalInfoType>
inline void addToAggregation(const AggregationValueType &incoming_value,
                             AggregationValueType &aggregate_value,
                             GlobalInfoType &global_info) {
  aggregate_value += incoming_value;
}

template <class AggregationValueType, class GlobalInfoType>
inline void addToAggregationAtomic(const AggregationValueType &incoming_value, //原子操作增加
                                   AggregationValueType &aggregate_value,
                                   GlobalInfoType &global_info) {
  writeAdd(&aggregate_value, incoming_value);
}

template <class AggregationValueType, class GlobalInfoType>
inline void removeFromAggregation(const AggregationValueType &incoming_value,
                                  AggregationValueType &aggregate_value,
                                  GlobalInfoType &global_info) {
  aggregate_value -= incoming_value;
}

template <class AggregationValueType, class GlobalInfoType>
inline void
removeFromAggregationAtomic(const AggregationValueType &incoming_value,
                            AggregationValueType &aggregate_value,
                            GlobalInfoType &global_info) {
  writeAdd(&aggregate_value, -incoming_value);
}

// ======================================================================
// VERTEX COMPUTE FUNCTION AND DETERMINE END OF COMPUTATION 顶点计算函数并确定计算结束
// ======================================================================
template <class AggregationValueType, class VertexValueType,
          class GlobalInfoType>
inline void computeFunction(const uintV &v,
                            const AggregationValueType &aggregation_value,
                            const VertexValueType &vertex_value_curr,
                            VertexValueType &vertex_value_next,
                            GlobalInfoType &global_info) {
  vertex_value_next =
      (1 - global_info.damping) + (global_info.damping * aggregation_value);
}

template <class VertexValueType, class GlobalInfoType>
inline bool notDelZero(const VertexValueType &value_curr, //判断前后变化是否超过阈值
                      const VertexValueType &value_next,
                      GlobalInfoType &global_info) {
  return (fabs(value_next - value_curr) > global_info.epsilon);
}

// ======================================================================
// EDGE FUNCTIONS
// ======================================================================
template <class AggregationValueType, class VertexValueType,
          class StaticInfoType>
inline void sourceChangeInContribution( //计算贡献
    const uintV &v, AggregationValueType &v_change_in_contribution,
    const VertexValueType &v_value_prev, const VertexValueType &v_value_curr,
    StaticInfoType &global_info) {
  v_change_in_contribution =
      (global_info.getOutDegree(v) != 0)
          ? (v_value_curr - v_value_prev) / global_info.getOutDegree(v)
          : 0;
}

template <class AggregationValueType, class VertexValueType, class EdgeDataType,
          class GlobalInfoType>
inline bool edgeFunction(const uintV &u, const uintV &v, //始终更新
                         const EdgeDataType &edge_weight,
                         const VertexValueType &u_value,
                         AggregationValueType &u_change_in_contribution,
                         GlobalInfoType &global_info) {
  return true;
}

// ======================================================================
// INCREMENTAL COMPUTING / DETERMINING FRONTIER
// =====================================================================
template <class GlobalInfoType>
inline void hasSourceChangedByUpdate(const uintV &v, UpdateType update_type,
                                     bool &activateInCurrentIteration,
                                     bool &forceComputeInCurrentIteration,
                                     GlobalInfoType &global_info,
                                     GlobalInfoType &global_info_old) {
  if (global_info.getOutDegree(v) != global_info_old.getOutDegree(v)) //度不同源发生改变
    activateInCurrentIteration = true;
}

template <class GlobalInfoType>
inline void hasDestinationChangedByUpdate(const uintV &v, //?
                                          UpdateType update_type,
                                          bool &activateInCurrentIteration,
                                          bool &forceComputeInCurrentIteration,
                                          GlobalInfoType &global_info,
                                          GlobalInfoType &global_info_old) {}

// ======================================================================
// HELPER FUNCTIONS
// ======================================================================
template <class AggregationValueType, class VertexValueType,
          class GlobalInfoType>
void printHistory(const uintV &v, AggregationValueType **agg_values, //历史值
                  VertexValueType **vertex_values, GlobalInfoType &info,
                  int history_iterations) {
  for (int iter = 0; iter < history_iterations; iter++) {
    cout << iter << "," << agg_values[iter][v] << "," << vertex_values[iter][v]
         << "\n";
  }
}

template <class GlobalInfoType>
void printAdditionalData(ofstream &output_file, const uintV &v,
                         GlobalInfoType &info) {}

// ======================================================================
// COMPUTE FUNCTION
// ======================================================================
template <class vertex> void compute(graph<vertex> &G, commandLine config) { //计算函数
  uintV n = G.n;
  int max_iters = config.getOptionLongValue("-maxIters", 50);
  int graphbolt_iters = config.getOptionLongValue("-graphboltIters", 50);
  double epsilon = config.getOptionDoubleValue("-epsilon", 0.0000001d);
  max_iters += 1;
  double damping = 0.85;

  PageRankInfo<vertex> global_info(&G, n, epsilon, damping);

  cout << "Initializing engine ....\n";
  GraphBoltEngineSimple<vertex, double, double, PageRankInfo<vertex>> engine(
      G, max_iters, global_info, false, config, graphbolt_iters);
  engine.init();
  cout << "Finished initializing engine\n";

  engine.run();
}
