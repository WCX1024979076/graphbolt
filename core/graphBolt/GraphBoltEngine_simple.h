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

#ifndef GRAPHBOLT_ENGINE_SIMPLE_H
#define GRAPHBOLT_ENGINE_SIMPLE_H

#include "GraphBoltEngine.h"

// ======================================================================
// GRAPHBOLTENGINESIMPLE
// ======================================================================
template <class vertex, class AggregationValueType, class VertexValueType,
          class GlobalInfoType>
class GraphBoltEngineSimple
    : public GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                             GlobalInfoType> {
public:
  GraphBoltEngineSimple(graph<vertex> &_my_graph, int _max_iter,
                        GlobalInfoType &_static_data, bool _use_lock,
                        commandLine _config, int _graphbolt_iter)
      : GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>(_my_graph, _max_iter, _static_data,
                                        _use_lock, _config, _graphbolt_iter) {
    use_source_contribution = true;
  }

  // ======================================================================
  // TEMPORARY STRUCTURES USED BY THE SIMPLE ENGINE 相关临时结构初始化
  // ======================================================================
  void createTemporaryStructures() {
    GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                    GlobalInfoType>::createTemporaryStructures();
  }
  void resizeTemporaryStructures() {
    GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                    GlobalInfoType>::resizeTemporaryStructures();
    initTemporaryStructures(n_old, n);
  }
  void freeTemporaryStructures() {
    GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                    GlobalInfoType>::freeTemporaryStructures();
  }
  void initTemporaryStructures() { initTemporaryStructures(0, n); }
  void initTemporaryStructures(long start_index, long end_index) {
    GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                    GlobalInfoType>::initTemporaryStructures(start_index,
                                                             end_index);
  }
  // ======================================================================
  // TRADITIONAL INCREMENTAL COMPUTATION 传统增量计算模型
  // ======================================================================t
  // TODO : Currently, max_iterations = history_iterations.
  // Need to implement computation without history.
  int traditionalIncrementalComputation(int start_iteration) {
    timer iteration_timer, phase_timer, single_calc_timer; //计时用
    double misc_time, copy_time, phase_time, iteration_time;

    vertexSubset frontier_curr_vs(n, frontier_curr); //当前活跃点集
    bool use_delta = true;
    int iter = start_iteration;

    if (frontier_curr_vs.numNonzeros() == 0) {
      converged_iteration = start_iteration; //收敛迭代
 
    } else {
      for (iter = start_iteration; iter < max_iterations; iter++) {
        // initialize timers
        {
          phase_timer.start();
          single_calc_timer.start();
          misc_time = 0;
          copy_time = 0;
        }

        long edges_to_process = sequence::plusReduceDegree(my_graph.V, frontier_curr_vs.d, (long)my_graph.n);
        // log_to_file("tradtional iter = ", iter);
        cout << "tradtional iter = "<< iter << endl;
        // notes_file << "tradtional calc, iter_num = " << iter << ", front_curr size = " << frontier_curr_vs.numNonzeros() << ", edges_to_process = " << edges_to_process << ", ";

        adaptive_executor.updateApproximateTimeForEdges(edges_to_process);

        // ========== COPY - Prepare curr iteration ==========
        if (iter > 0) {
          // Copy the aggregate and actual value from iter-1 to ier
          parallel_for(uintV v = 0; v < n; v++) {
            vertex_values[iter][v] = vertex_values[iter - 1][v];
#ifdef MECHINE_ITER
            //if(iter <= graphbolt_iterations)
              //aggregation_values[iter][v] = aggregation_values_tmp[v]; 
#else
            aggregation_values[iter][v] = aggregation_values[iter - 1][v];
#endif
            delta[v] = aggregationValueIdentity<AggregationValueType>();
          }
        }
        use_delta = shouldUseDelta(iter); //用于自定义何时应该进行基于增量的增量计算执行。

        // ========== MISC - count active edges for AE ==========
        phase_time = phase_timer.next();
        adaptive_executor.updateCopyTime(iter, phase_time);
        adaptive_executor.updateEdgesProcessed(iter, my_graph,
                                               frontier_curr_vs); //更新迭代需要更新的边的个数
        misc_time = phase_timer.next();
        adaptive_executor.updateMiscTime(iter, misc_time);

        // ========== EDGE COMPUTATION ========== 边计算
        if ((use_source_contribution) && (iter == 1)) {
          // Compute source contribution for first iteration
          parallel_for(uintV u = 0; u < n; u++) {
            if (frontier_curr[u]) { //激活顶点u
              // compute source change in contribution
              sourceChangeInContribution<AggregationValueType, VertexValueType,
                                         GlobalInfoType>(
                  u, source_change_in_contribution[u], //顶点最新值
                  vertexValueIdentity<VertexValueType>(), //顶点初始值
                  vertex_values[iter - 1][u], global_info);
            }
          }
        }

        parallel_for(uintV u = 0; u < n; u++) {
          if (frontier_curr[u]) {
            // check for propagate and retract for the vertices. 计算聚合值
            intE outDegree = my_graph.V[u].getOutDegree();
            granular_for(j, 0, outDegree, (outDegree > 1024), {
              uintV v = my_graph.V[u].getOutNeighbor(j);
              AggregationValueType contrib_change =
                  use_source_contribution 
                      ? source_change_in_contribution[u]
                      : aggregationValueIdentity<AggregationValueType>();
#ifdef EDGEDATA
              EdgeData *edge_data = my_graph.V[u].getOutEdgeData(j);
#else
              EdgeData *edge_data = &emptyEdgeData;
#endif
              bool ret =
                  edgeFunction(u, v, *edge_data, vertex_values[iter - 1][u], //判断是否需要更新
                               contrib_change, global_info);
              if (ret) {
                if (use_lock) {
                  vertex_locks[v].writeLock();
                  addToAggregation(contrib_change, delta[v], global_info); //添加到聚合值
                  vertex_locks[v].unlock();
                } else {
                  addToAggregationAtomic(contrib_change, delta[v], global_info); //原子操作
                }
                if (!frontier_next[v]) //如果没有激活v则对v进行激活?
                  frontier_next[v] = 1;
              }
            });
          }
        }

        phase_time = phase_timer.next();
        adaptive_executor.updateEdgeMapTime(iter, phase_time);

        // ========== VERTEX COMPUTATION ==========
        parallel_for(uintV v = 0; v < n; v++) {
          // Reset frontier for next iteration 
          frontier_curr[v] = 0;
          // Process all vertices affected by EdgeMap
          if (frontier_next[v] ||
              forceComputeVertexForIteration(v, iter, global_info)) {

            frontier_next[v] = 0;
            // Update aggregation value and reset change received[v] (i.e.
            // delta[v])
#ifdef MECHINE_ITER
            addToAggregation(delta[v], aggregation_values_tmp[v],
                             global_info);
#else
            addToAggregation(delta[v], aggregation_values[iter][v],
                             global_info);
#endif
            delta[v] = aggregationValueIdentity<AggregationValueType>();

            // Calculate new_value based on the updated aggregation value
            VertexValueType new_value;
#ifdef MECHINE_ITER
            computeFunction(v, aggregation_values_tmp[v],
                            vertex_values[iter - 1][v], new_value, global_info); //根据聚合值重新计算顶点值
#else
            computeFunction(v, aggregation_values[iter][v],
                            vertex_values[iter - 1][v], new_value, global_info); //根据聚合值重新计算顶点值
#endif
            // Check if change is significant
            if (notDelZero(new_value, vertex_values[iter - 1][v], global_info)) { //阈值判断
              // change is significant. Update vertex_values
              vertex_values[iter][v] = new_value;
              // Set active for next iteration.
              frontier_curr[v] = 1;
            } else {
              // change is not significant. Copy vertex_values[iter-1]
              vertex_values[iter][v] = vertex_values[iter - 1][v];
            }
          }
          frontier_curr[v] =
              frontier_curr[v] ||
              forceActivateVertexForIteration(v, iter + 1, global_info);
          if (frontier_curr[v]) {
            if (use_source_contribution) {
              // update source_contrib for next iteration
              sourceChangeInContribution<AggregationValueType, VertexValueType,
                                         GlobalInfoType>(
                  v, source_change_in_contribution[v],
                  vertex_values[iter - 1][v], vertex_values[iter][v],
                  global_info); //重新计算聚合值
            } else {
              source_change_in_contribution[v] =
                  aggregationValueIdentity<AggregationValueType>();
            }
          }
        }
        phase_time = phase_timer.stop();
        adaptive_executor.updateVertexMapTime(iter, phase_time);

#ifdef MECHINE_ITER
        if (iter < graphbolt_iterations){
          parallel_for(uintV v = 0; v < n; v++) {
            aggregation_values[iter][v] = aggregation_values_tmp[v];
          }
        } 
#endif
        
        vertexSubset temp_vs(n, frontier_curr); //复制一遍重新计算
        frontier_curr_vs = temp_vs;
        misc_time += phase_timer.next();
        iteration_time = iteration_timer.stop();

        log_to_file(single_calc_timer.next(), " ");
        // log_to_file(" timer = ", single_time);
        // log_to_file(" ad_timer = ", adaptive_executor.approximateTimeForCurrIter());
        // log_to_file("\n");
        // notes_file << "calc_timer = " << single_time << endl;
        // notes_time << single_time << " " << adaptive_executor.approximateTimeForCurrIter() << endl;

        if (ae_enabled && iter == 1) {
          adaptive_executor.setApproximateTimeForCurrIter(iteration_time); //为 CurrIter 设置大概时间
        }
        // Convergence check 收敛检查
        converged_iteration = iter;
        if (frontier_curr_vs.isEmpty()) {
          break;
        } 
      }
    }
    log_to_file("\n");
    if (ae_enabled) {
      adaptive_executor.updateEquation(converged_iteration);
    }
    return converged_iteration;
  }

#define COMMA ,

  // ======================================================================
  // Tegra 增量计算模型
  // ======================================================================
  void tegraCompute(int start_iter, edgeArray &edge_additions, edgeArray &edge_deletions) {
    cout << "Tegra calc starter" << endl;
    timer single_calc_timer; //计时用
    double iteration_time = 0.0;

    n_old = n;
    if (edge_additions.maxVertex >= n) {
      processVertexAddition(edge_additions.maxVertex);
    }
    
    // Reset values before incremental computation
    parallel_for(uintV v = 0; v < n; v++) {
#ifdef MECHINE_ITER
      frontier_next[v] = 0;
#else
      frontier_curr[v] = 0;
      frontier_next[v] = 0;
      changed[v] = 0;
#endif
    }
    
#ifdef MECHINE_ITER
    // global_info_old.copy(global_info);
    // global_info.processUpdates(edge_additions, edge_deletions);
#else
    global_info_old.copy(global_info);
    global_info.processUpdates(edge_additions, edge_deletions);
#endif

    parallel_for(long i = 0; i < edge_additions.size; i++) { 
      uintV source = edge_additions.E[i].source;
      uintV destination = edge_additions.E[i].destination;
#ifdef MECHINE_ITER
      frontier_curr[source] = 1;
      changedTegra[source] = 0;
#endif
      intE outDegree = my_graph.V[source].getOutDegree();
      granular_for(i, 0, outDegree, (outDegree > 1024), {
        uintV v = my_graph.V[source].getOutNeighbor(i);
        frontier_curr[v] = 1;
        frontier_init_tegra[v] = 1;
      });
    }

    parallel_for(long i = 0; i < edge_deletions.size; i++) {
      uintV source = edge_deletions.E[i].source;
      uintV destination = edge_deletions.E[i].destination;
#ifdef MECHINE_ITER
      frontier_curr[source] = 1;
      changedTegra[source] = 0;
#endif
      frontier_curr[destination] = 1;
      frontier_init_tegra[destination] = 1;
      intE outDegree = my_graph.V[source].getOutDegree();
      granular_for(i, 0, outDegree, (outDegree > 1024), {
        uintV v = my_graph.V[source].getOutNeighbor(i);
        frontier_curr[v] = 1;
        frontier_init_tegra[v] = 1;
      });
    }

    bool should_switch_now = false;
    if (ae_enabled && shouldSwitch(start_iter, 0)) {
      should_switch_now = true;
    }

    for (int iter = start_iter; iter < max_iterations; iter++) {
      single_calc_timer.start();
      if (iter > converged_iteration || should_switch_now) {
        parallel_for(uintV v = 0; v < n; v++) {
          if (~frontier_curr[v]) {
          // check for propagate and retract for the vertices.
            intE inDegree = my_graph.V[v].getInDegree();
            aggregation_values_tmp[v] = vertexValueIdentity<VertexValueType>();
          
            granular_for(i, 0, inDegree, (inDegree > 1024), {
              uintV u = my_graph.V[v].getInNeighbor(i);
              AggregationValueType contrib_change = vertexValueIdentity<VertexValueType>();
              sourceChangeInContribution<AggregationValueType COMMA VertexValueType COMMA GlobalInfoType>(
                  u, contrib_change, vertexValueIdentity<VertexValueType>(),
                  vertex_values[iter - 2][u], global_info);

// Do repropagate for edge source->destination.
#ifdef EDGEDATA
        EdgeData *edge_data = edge_additions.E[i].edgeData;
#else
        EdgeData *edge_data = &emptyEdgeData;
#endif

              bool ret = edgeFunction(u, v, *edge_data, vertex_values[iter - 2][u],
                               contrib_change, global_info);

              if (ret) {
                if (use_lock) {
                  vertex_locks[v].writeLock();
                  if (ret) {
                    addToAggregation(contrib_change, aggregation_values_tmp[v], global_info);
                  }
                  vertex_locks[v].unlock();
                } else {
                  if (ret) {
                    addToAggregationAtomic(contrib_change, aggregation_values_tmp[v] , global_info);
                  }
                }
              } 
            });
          }
        }
        converged_iteration = performSwitch(iter);
        break;
      }
      // log_to_file("tegra iter = ", iter);
      cout << "tegra iter = " << iter << endl;

      parallel_for(uintV v = 0; v < n; v++) {
        if (frontier_curr[v]) {
          // check for propagate and retract for the vertices.
          intE inDegree = my_graph.V[v].getInDegree();
          aggregation_values_tmp[v] = vertexValueIdentity<VertexValueType>();
          
          granular_for(i, 0, inDegree, (inDegree > 1024), {
            uintV u = my_graph.V[v].getInNeighbor(i);
            AggregationValueType contrib_change = vertexValueIdentity<VertexValueType>();
            sourceChangeInContribution<AggregationValueType COMMA VertexValueType COMMA GlobalInfoType>(
                u, contrib_change, vertexValueIdentity<VertexValueType>(),
                vertex_values[iter - 1][u], global_info);

// Do repropagate for edge source->destination.
#ifdef EDGEDATA
        EdgeData *edge_data = edge_additions.E[i].edgeData;
#else
        EdgeData *edge_data = &emptyEdgeData;
#endif

            bool ret = edgeFunction(u, v, *edge_data, vertex_values[iter - 1][u],
                               contrib_change, global_info);

            if (ret) {
              if (use_lock) {
                vertex_locks[v].writeLock();
                if (ret) {
                  addToAggregation(contrib_change, aggregation_values_tmp[v], global_info);
                }
                vertex_locks[v].unlock();
              } else {
                if (ret) {
                  addToAggregationAtomic(contrib_change, aggregation_values_tmp[v] , global_info);
                }
              }
            } 
          });
        }
      }

      parallel_for(uintV u = 0; u < n; u++) { 
        if(frontier_curr[u]) {
          VertexValueType new_value;
          computeFunction(u, aggregation_values_tmp[u],
              vertex_values[iter - 1][u], new_value, global_info);
          if ((notDelZero(new_value, vertex_values[iter - 1][u], global_info)) && (notDelZero(new_value, vertex_values[iter][u], global_info_old))) {
            vertex_values[iter][u] = new_value;
            frontier_next[u] = 1;
            intE outDegree = my_graph.V[u].getOutDegree();
            granular_for(i, 0, outDegree, (outDegree > 1024), {
              uintV v = my_graph.V[u].getOutNeighbor(i);
              frontier_next[v] = 1;
            });
#ifdef MECHINE_ITER
            changedTegra[u] = 1;
#else
            changed[u] = 1;
#endif
          } else if ((notDelZero(new_value, vertex_values[iter][u], global_info_old))) {
              vertex_values[iter][u] = vertex_values[iter - 1][u];
              frontier_next[u] = 1;
              intE outDegree = my_graph.V[u].getOutDegree();
              granular_for(i, 0, outDegree, (outDegree > 1024), {
                uintV v = my_graph.V[u].getOutNeighbor(i);
                frontier_next[v] = 1;
              });
#ifdef MECHINE_ITER
              changedTegra[u] = 1;
#else
              changed[u] = 1;
#endif
          } else if ((notDelZero(new_value, vertex_values[iter - 1][u], global_info))) {
              vertex_values[iter][u] = new_value;
              frontier_next[u] = 1;
              intE outDegree = my_graph.V[u].getOutDegree();
              granular_for(i, 0, outDegree, (outDegree > 1024), {
                uintV v = my_graph.V[u].getOutNeighbor(i);
                frontier_next[v] = 1;
              });
          } else {
            vertex_values[iter][u] = vertex_values[iter - 1][u];
          }
#ifdef MECHINE_ITER
        } else if(changedTegra[u]) {
#else
        } else if(changed[u]) {
#endif
          vertex_values[iter][u] = vertex_values[iter - 1][u];  
        }
        frontier_next[u] |= frontier_init_tegra[u];
      }

      //cout << " frontier_next " << frontier_next[45929];
      vertexSubset temp_vs(n, frontier_curr);
      parallel_for(uintV u = 0; u < n; u++) {
        frontier_curr[u] = frontier_next[u];
        frontier_next[u] = 0;
      }
      
      iteration_time = single_calc_timer.next();
      if (ae_enabled && shouldSwitch(iter, iteration_time)) {
        should_switch_now = true;
      }
      log_to_file(iteration_time, " ");
      // log_to_file(" timer = ", single_calc_timer.next());
      // log_to_file("\n");

      //cout << "iter " << iter << ", front_size " << temp_vs.numNonzeros() << endl;
      if(temp_vs.isEmpty()) {
        if (iter == converged_iteration) {
          break;
        } else if (iter > converged_iteration) {
          assert(("Missed switching to Traditional incremental computing when "
                  "iter == converged_iter",
                  false));
        } else {
          // Values stable for the changed vertices at this iteration.
          // But, the changed vertices might receive new changes. So,
          // continue loop until iter == converged_iteration vertices may
          // still not have converged. So, keep continuing until
          // converged_iteration is reached.
        }
      }
    }
    cout << "tegra calc end" << endl;
    printOutput();
    log_to_file("\n");
  }

  // ======================================================================
  // DELTACOMPUTE 增量计算模型
  // ======================================================================
  void deltaCompute(edgeArray &edge_additions, edgeArray &edge_deletions) {
    timer iteration_timer, phase_timer, full_timer, pre_compute_timer, single_calc_timer;
    double misc_time, copy_time, phase_time, iteration_time, pre_compute_time;
    iteration_time = 0;
    full_timer.start();

    // TODO : Realloc addition of new vertices
    n_old = n;
    if (edge_additions.maxVertex >= n) {
      processVertexAddition(edge_additions.maxVertex);
    }

    // Reset values before incremental computation
    parallel_for(uintV v = 0; v < n; v++) {
      frontier_curr[v] = 0;
      frontier_next[v] = 0;
      changed[v] = 0;
      frontier_next_delta[v] = 0;
#ifdef MECHINE_ITER
      changedTegra[v] = 0;
      frontier_curr_tegra[v] = 0;
      frontier_init_tegra[v] = 0;
#endif
      vertex_value_old_prev[v] = vertexValueIdentity<VertexValueType>();
      vertex_value_old_curr[v] = vertexValueIdentity<VertexValueType>();
      initializeVertexValue<VertexValueType>(v, vertex_value_old_next[v],
                                             global_info);

      delta[v] = aggregationValueIdentity<AggregationValueType>();
      if (use_source_contribution) {
        source_change_in_contribution[v] =
            aggregationValueIdentity<AggregationValueType>();
      }
    }

    // ==================== UPDATE GLOBALINFO ===============================
    // deltaCompute/initCompute Save a copy of global_info before we lose any
    // relevant information of the old graph For example, In PageRank, we need
    // to save the outDegree for all vertices corresponding to the old graph
    // deltacompute/initcompute保存global_info的副本，例如我们丢失旧图的任何相关信息，
    // 例如，在Pagerank中，我们需要保存所有与旧图相对应的顶点
    global_info_old.copy(global_info);

    // Update global_info based on edge additions or deletions. This is
    // application specific. For example, for pagerank, the the outDegree of
    // vertices with edgeAddition will increase and those with edgeDeletions
    // will decrease
    global_info.processUpdates(edge_additions, edge_deletions); //图更新

    // ========== EDGE COMPUTATION - DIRECT CHANGES - for first iter ==========
    pre_compute_timer.start();
    parallel_for(long i = 0; i < edge_additions.size; i++) { //增加边处理
      uintV source = edge_additions.E[i].source;
      uintV destination = edge_additions.E[i].destination;
#ifdef MECHINE_ITER
      intE outDegree = my_graph.V[source].getOutDegree();
      granular_for(i, 0, outDegree, (outDegree > 1024), {
        uintV v = my_graph.V[source].getOutNeighbor(i);
        frontier_curr_tegra[v] = 1;
        frontier_init_tegra[v] = 1;
      });
#endif
      frontier_next_delta[destination] = 1;
      // Update frontier and changed values
      hasSourceChangedByUpdate(source, edge_addition_enum,
                               frontier_curr[source], changed[source],
                               global_info, global_info_old); //增加的边 出边 激活
      hasSourceChangedByUpdate(destination, edge_addition_enum,
                               frontier_curr[destination], changed[destination],
                               global_info, global_info_old); //增加的边 入边 激活
      if (forceActivateVertexForIteration(source, 1, global_info_old)) { //第一次迭代是否需要强制计算聚合值

        if (frontier_curr[source]) {
          changed[source] = true;
          frontier_next_delta[source] = true;
        }
        if (frontier_curr[destination]) {
          changed[source] = true;
          frontier_next_delta[source] = true;
        }

        AggregationValueType contrib_change; //计算聚合值?
        if (use_source_contribution) {
          sourceChangeInContribution<AggregationValueType, VertexValueType,
                                     GlobalInfoType>(
              source, contrib_change, vertexValueIdentity<VertexValueType>(),
              vertex_values[0][source], global_info_old);
        }

// Do repropagate for edge source->destination.
#ifdef EDGEDATA
        EdgeData *edge_data = edge_additions.E[i].edgeData;
#else
        EdgeData *edge_data = &emptyEdgeData;
#endif 
        bool ret =
            edgeFunction(source, destination, *edge_data,
                         vertex_values[0][source], contrib_change, global_info); //判断聚合值是否归入最终计算
        if (ret) {
          if (use_lock) {
            vertex_locks[destination].writeLock();
            addToAggregation(contrib_change, delta[destination],
                             global_info_old);
            vertex_locks[destination].unlock();
          } else {
            addToAggregationAtomic(contrib_change, delta[destination],
                                   global_info_old);
          }
          if (!changed[destination])
            changed[destination] = true;
        }
      }
    }

    parallel_for(long i = 0; i < edge_deletions.size; i++) { //删除和增加同理
      uintV source = edge_deletions.E[i].source;
      uintV destination = edge_deletions.E[i].destination;
#ifdef MECHINE_ITER
      frontier_curr_tegra[destination] = 1;
      frontier_init_tegra[destination] = 1;
      intE outDegree = my_graph.V[source].getOutDegree();
      granular_for(i, 0, outDegree, (outDegree > 1024), {
        uintV v = my_graph.V[source].getOutNeighbor(i);
        frontier_curr_tegra[v] = 1;
        frontier_init_tegra[v] = 1;
      });
#endif
      frontier_next_delta[destination] = true;
      hasSourceChangedByUpdate(source, edge_deletion_enum,
                               frontier_curr[source], changed[source],
                               global_info, global_info_old);
      hasSourceChangedByUpdate(destination, edge_deletion_enum,
                               frontier_curr[destination], changed[destination],
                               global_info, global_info_old);
      if (forceActivateVertexForIteration(source, 1, global_info_old)) {
        // Update frontier and changed values
        if (frontier_curr[source]) {
          changed[source] = true;
          frontier_next_delta[source] = true;
        }
        if (frontier_curr[destination]) {
          changed[source] = true;
          frontier_next_delta[source] = true;
        }

        AggregationValueType contrib_change;
        if (use_source_contribution) {
          sourceChangeInContribution<AggregationValueType, VertexValueType,
                                     GlobalInfoType>(
              source, contrib_change, vertexValueIdentity<VertexValueType>(),
              vertex_values[0][source], global_info_old);
        }

// Do retract for edge source->destination
#ifdef EDGEDATA
        EdgeData *edge_data = edge_deletions.E[i].edgeData;
#else
        EdgeData *edge_data = &emptyEdgeData;
#endif
        bool ret = edgeFunction(source, destination, *edge_data,
                                vertex_values[0][source], contrib_change,
                                global_info_old);
        if (ret) {
          if (use_lock) {
            vertex_locks[destination].writeLock();
            removeFromAggregation(contrib_change, delta[destination],
                                  global_info_old);
            vertex_locks[destination].unlock();
          } else {
            removeFromAggregationAtomic(contrib_change, delta[destination],
                                        global_info_old);
          }
          if (!changed[destination])
            changed[destination] = true;
        }
      }
    }
    pre_compute_time = pre_compute_timer.stop();

    // =============== INCREMENTAL COMPUTE - REFINEMENT START ================
    vertexSubset frontier_curr_vs(n, frontier_curr);
    bool should_switch_now = false;
    bool use_delta = true;

    if (ae_enabled && shouldSwitch(0, 0)) {
      should_switch_now = true;
    }

#ifdef MECHINE_ITER
    for (int iter = 1; iter < graphbolt_iterations; iter++) {
#else
    for (int iter = 1; iter < max_iterations; iter++) {
      // Perform switch if needed
      if (should_switch_now) { //切换到传统增量计算模型?
        converged_iteration = performSwitch(iter);
        break;
      }
#endif

      // initialize timers
      {
        iteration_timer.start();
        phase_timer.start();
        single_calc_timer.start();
        iteration_time = 0;
        misc_time = 0;
        copy_time = 0;
      }
      use_delta = shouldUseDelta(iter);

      // ================ COPY - PREPARE CURRENT ITERATION ================
      {
        VertexValueType *temp1 = vertex_value_old_prev;
        vertex_value_old_prev = vertex_value_old_curr;
        vertex_value_old_curr = vertex_value_old_next;
        vertex_value_old_next = temp1;

        if (iter <= converged_iteration) {
          parallel_for(uintV v = 0; v < n; v++) {
            vertex_value_old_next[v] = vertex_values[iter][v];
          }
        } else {
          converged_iteration = performSwitch(iter);
          break;
        }
      }
      // log_to_file("delta iter = ", iter);
      cout << "GraphBolt iter = "<< iter << endl;
      // notes_file << "delta calc, iter_num = " << iter << ", front_curr size = " << frontier_curr_vs.numNonzeros() << ", ";
      copy_time += phase_timer.next();
      // ========== EDGE COMPUTATION - aggregation_values ========== 计算新的权值贡献
      if ((use_source_contribution) && (iter == 1)) { //第一次迭代 所有激活得点向邻居发送更新
        // Compute source contribution for first iteration
        parallel_for(uintV u = 0; u < n; u++) {
          if (frontier_curr[u]) {
            // compute source change in contribution
            AggregationValueType contrib_change =
                aggregationValueIdentity<AggregationValueType>();
            sourceChangeInContribution<AggregationValueType, VertexValueType,
                                       GlobalInfoType>(
                u, contrib_change, vertexValueIdentity<VertexValueType>(),
                vertex_values[iter - 1][u], global_info);
            addToAggregation(contrib_change, source_change_in_contribution[u],
                             global_info);
            sourceChangeInContribution<AggregationValueType, VertexValueType,
                                       GlobalInfoType>(
                u, contrib_change, vertexValueIdentity<VertexValueType>(),
                vertex_value_old_curr[u], global_info_old);
            removeFromAggregation(
                contrib_change, source_change_in_contribution[u], global_info);
          }
        }
      }

      parallel_for(uintV u = 0; u < n; u++) { //聚合source_change_in_contribution的值并修改顶点值
        if (frontier_curr[u]) {
          // check for propagate and retract for the vertices.
          intE outDegree = my_graph.V[u].getOutDegree();

          granular_for(i, 0, outDegree, (outDegree > 1024), {
            uintV v = my_graph.V[u].getOutNeighbor(i);
            bool ret = false;
            AggregationValueType contrib_change =
                use_source_contribution
                    ? source_change_in_contribution[u]
                    : aggregationValueIdentity<AggregationValueType>();

#ifdef EDGEDATA
            EdgeData *edge_data = my_graph.V[u].getOutEdgeData(i);
#else
            EdgeData *edge_data = &emptyEdgeData;
#endif
            ret = edgeFunction(u, v, *edge_data, vertex_values[iter - 1][u],
                               contrib_change, global_info);

            if (ret) {
              if (use_lock) {
                vertex_locks[v].writeLock();
                if (ret) {
                  addToAggregation(contrib_change, delta[v], global_info);
                }
                vertex_locks[v].unlock();

              } else {
                if (ret) {
                  addToAggregationAtomic(contrib_change, delta[v], global_info);
                }
              }
              if (!changed[v])
                changed[v] = 1;
              if (!frontier_next_delta[v])
                frontier_next_delta[v] = 1;
            }
          });
        }
      }
      phase_time = phase_timer.next();

      // ========== VERTEX COMPUTATION  ========== 将 delta[v] 进行聚合并更改值
      bool use_delta_next_iteration = shouldUseDelta(iter + 1);
      parallel_for(uintV v = 0; v < n; v++) {
        // changed vertices need to be processed
        frontier_curr[v] = 0;
        if ((v >= n_old) && (changed[v] == false)) {
          changed[v] = forceComputeVertexForIteration(v, iter, global_info);
          frontier_next_delta[v] = forceComputeVertexForIteration(v, iter, global_info);
        }

        if (frontier_next_delta[v]) {
          frontier_curr[v] = 0;
          frontier_next_delta[v] = 0;

          // delta has the current cumulative change for the vertex.
          // Update the aggregation value in history
          addToAggregation(delta[v], aggregation_values[iter][v], global_info);

          VertexValueType new_value;
          computeFunction(v, aggregation_values[iter][v],
                          vertex_values[iter - 1][v], new_value, global_info);

          if (forceActivateVertexForIteration(v, iter + 1, global_info)) {
            frontier_curr[v] = 1;
          }
          AggregationValueType contrib_change =
              aggregationValueIdentity<AggregationValueType>();
          source_change_in_contribution[v] =
              aggregationValueIdentity<AggregationValueType>();

          if (notDelZero(new_value, vertex_values[iter - 1][v], global_info)) {
            // change is significant. Update vertex_values
            vertex_values[iter][v] = new_value;
            frontier_curr[v] = 1;
            if (use_delta_next_iteration) {
              sourceChangeInContribution<AggregationValueType, VertexValueType,
                                         GlobalInfoType>(
                  v, contrib_change, vertex_values[iter - 1][v],
                  vertex_values[iter][v], global_info);
            } else {
              sourceChangeInContribution<AggregationValueType, VertexValueType,
                                         GlobalInfoType>(
                  v, contrib_change, vertexValueIdentity<VertexValueType>(),
                  vertex_values[iter][v], global_info);
            }
            addToAggregation(contrib_change, source_change_in_contribution[v],
                             global_info);


          } else {
            // change is not significant. Copy vertex_values[iter-1]
            vertex_values[iter][v] = vertex_values[iter - 1][v];
          }

          if (notDelZero(vertex_value_old_next[v], vertex_value_old_curr[v],
                        global_info_old)) {
            // change is significant. Update v_change
            frontier_curr[v] = 1;
            if (use_delta_next_iteration) {
              sourceChangeInContribution<AggregationValueType, VertexValueType,
                                         GlobalInfoType>(
                  v, contrib_change, vertex_value_old_curr[v],
                  vertex_value_old_next[v], global_info_old);
            } else {

              sourceChangeInContribution<AggregationValueType, VertexValueType,
                                         GlobalInfoType>(
                  v, contrib_change, vertexValueIdentity<VertexValueType>(),
                  vertex_value_old_next[v], global_info_old);
            }
            removeFromAggregation(contrib_change,
                                  source_change_in_contribution[v],
                                  global_info_old);
          }

#ifdef MECHINE_ITER //TODO changedTegra frontier_curr_tegra 边算边更新
          if ((notDelZero(vertex_values[iter][v], vertex_value_old_next[v], global_info_old))) {
              changedTegra[v] = 1;  
          }
#endif
        } else if(changed[v]) {
          vertex_values[iter][v] = vertex_values[iter - 1][v];
          aggregation_values[iter][v] = aggregation_values[iter - 1][v];
        }
#ifdef MECHINE_ITER
        if (iter == graphbolt_iterations - 1) {
          aggregation_values_tmp[v] = aggregation_values[iter][v];
          if(changed[v]) {
            VertexValueType new_value;
            computeFunction(v, aggregation_values[iter][v],
                vertex_values[iter - 1][v], new_value, global_info);
            intE outDegree = my_graph.V[v].getOutDegree();
            if ((notDelZero(new_value, vertex_values[iter - 1][v], global_info)) || (notDelZero(new_value, vertex_value_old_next[v], global_info_old))) {
              frontier_curr_tegra[v] = 1;
              granular_for(i, 0, outDegree, (outDegree > 1024), {
                uintV u = my_graph.V[v].getOutNeighbor(i);
                frontier_curr_tegra[u] = 1;
              });
            }
          }
          frontier_curr_tegra[v] |= frontier_init_tegra[v];
        }
#endif
      }
      //cout << " changedTegra " << changedTegra[45929] << endl;
      phase_time = phase_timer.next();

      // ========== EDGE COMPUTATION - DIRECT CHANGES - for next iter ========== 计算下次迭代
      bool has_direct_changes = false;
      parallel_for(long i = 0; i < edge_additions.size; i++) {
        uintV source = edge_additions.E[i].source;
        uintV destination = edge_additions.E[i].destination;
        AggregationValueType contrib_change;
        frontier_next_delta[source] = 1;
        frontier_next_delta[destination] = 1;
        if (notDelZero(vertex_value_old_curr[source],
                      vertex_value_old_next[source], global_info_old) ||
            (forceActivateVertexForIteration(source, iter + 1,
                                             global_info_old))) {
          if (use_delta_next_iteration) {
            sourceChangeInContribution<AggregationValueType, VertexValueType,
                                       GlobalInfoType>(
                source, contrib_change, vertex_value_old_curr[source],
                vertex_value_old_next[source], global_info_old);
          } else {
            sourceChangeInContribution<AggregationValueType, VertexValueType,
                                       GlobalInfoType>(
                source, contrib_change, vertexValueIdentity<VertexValueType>(),
                vertex_value_old_next[source], global_info_old);
          }
// Do repropagate for edge source->destination.
#ifdef EDGEDATA
          EdgeData *edge_data = edge_additions.E[i].edgeData;
#else
          EdgeData *edge_data = &emptyEdgeData;
#endif
          bool ret = edgeFunction(source, destination, *edge_data,
                                  vertex_values[0][source], contrib_change,
                                  global_info);

          if (ret) {
            if (use_lock) {
              vertex_locks[destination].writeLock();
              addToAggregation(contrib_change, delta[destination],
                               global_info_old);
              vertex_locks[destination].unlock();
            } else {
              addToAggregationAtomic(contrib_change, delta[destination],
                                     global_info_old);
            }
            if (!changed[destination])
              changed[destination] = 1;
            if (!has_direct_changes)
              has_direct_changes = true;
          }
        }
      }

      parallel_for(long i = 0; i < edge_deletions.size; i++) {
        uintV source = edge_deletions.E[i].source;
        uintV destination = edge_deletions.E[i].destination;
        AggregationValueType contrib_change;
        frontier_next_delta[source] = 1;
        frontier_next_delta[destination] = 1;
        if (notDelZero(vertex_value_old_curr[source],
                      vertex_value_old_next[source], global_info_old) ||
            (forceActivateVertexForIteration(source, iter + 1,
                                             global_info_old))) {
          // Do repropagate for edge source->destination.
          if (use_delta_next_iteration) {
            sourceChangeInContribution<AggregationValueType, VertexValueType,
                                       GlobalInfoType>(
                source, contrib_change, vertex_value_old_curr[source],
                vertex_value_old_next[source], global_info_old);
          } else {
            sourceChangeInContribution<AggregationValueType, VertexValueType,
                                       GlobalInfoType>(
                source, contrib_change, vertexValueIdentity<VertexValueType>(),
                vertex_value_old_next[source], global_info_old);
          }
#ifdef EDGEDATA
          EdgeData *edge_data = edge_deletions.E[i].edgeData;
#else
          EdgeData *edge_data = &emptyEdgeData;
#endif
          bool ret = edgeFunction(source, destination, *edge_data,
                                  vertex_values[0][source], contrib_change,
                                  global_info);

          if (ret) {
            if (use_lock) {
              vertex_locks[destination].writeLock();
              removeFromAggregation(contrib_change, delta[destination],
                                    global_info_old);
              vertex_locks[destination].unlock();

            } else {
              removeFromAggregationAtomic(contrib_change, delta[destination],
                                          global_info_old);
            }
            if (!changed[destination])
              changed[destination] = 1;
            if (!has_direct_changes)
              has_direct_changes = true;
          }
        }
      }
      phase_time = phase_timer.next();

      vertexSubset temp_vs(n, frontier_curr);
      frontier_curr_vs = temp_vs;

      misc_time += phase_timer.next();
      iteration_time = iteration_timer.next();
      // notes_file << "calc_timer = " << single_calc_timer.next() << endl;
      // Convergence check
      if (!has_direct_changes && frontier_curr_vs.isEmpty()) {
        // There are no more active vertices
        if (iter == converged_iteration) {
          break;
        } else if (iter > converged_iteration) {
          assert(("Missed switching to Traditional incremental computing when "
                  "iter == converged_iter",
                  false));
        } else {
          // Values stable for the changed vertices at this iteration.
          // But, the changed vertices might receive new changes. So,
          // continue loop until iter == converged_iteration vertices may
          // still not have converged. So, keep continuing until
          // converged_iteration is reached.
        }
      }
      if (iter == 1) {
        iteration_time += pre_compute_time;
      }

      log_to_file(single_calc_timer.next(), " ");
      // log_to_file(" timer = ", iteration_time);
      // log_to_file(" ad_timer = ", adaptive_executor.approximateTimeForCurrIter());
      // log_to_file("\n");
      // notes_time << iteration_time << " " << adaptive_executor.approximateTimeForCurrIter() << endl;
 
      if (ae_enabled && shouldSwitch(iter, iteration_time)) {
        should_switch_now = true;
      }
      misc_time += phase_timer.stop();
      iteration_time += iteration_timer.stop();
    }
#ifdef MECHINE_ITER
    // if (should_switch_now) { //切换到传统增量计算模型?
    //     converged_iteration = performSwitch(graphbolt_iterations);
    // } else {
      parallel_for(uintV v = 0; v < n; v++) {
        frontier_curr[v] = frontier_curr_tegra[v];
      }
      performSwitchInc(graphbolt_iterations, edge_additions, edge_deletions);
    // }
#endif

    cout << "Finished batch : " << full_timer.stop() << "\n";
    cout << "Number of iterations : " << converged_iteration << "\n";
    // testPrint();
    log_to_file("\n");
    printOutput();
  }

  // Refactor this in a better way
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::my_graph;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::config;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::max_iterations;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::history_iterations;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::graphbolt_iterations;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::converged_iteration;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::use_lock;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::vertex_locks;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::aggregation_values;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::aggregation_values_tmp;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::vertex_values;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::n;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::global_info;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::delta;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::use_source_contribution;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::source_change_in_contribution;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::n_old;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::global_info_old;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::vertex_value_old_next;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::vertex_value_old_curr;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::vertex_value_old_prev;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::all;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::frontier_curr;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::frontier_next;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::changed;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::ingestor;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::current_batch;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::adaptive_executor;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::ae_enabled;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::testPrint;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::printOutput;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::shouldSwitch;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::performSwitch;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::processVertexAddition;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::frontier_init_tegra;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::frontier_next_delta;
#ifdef MECHINE_ITER
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::changedTegra;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::performSwitchInc;
  using GraphBoltEngine<vertex, AggregationValueType, VertexValueType,
                        GlobalInfoType>::frontier_curr_tegra;
#endif
};
#endif