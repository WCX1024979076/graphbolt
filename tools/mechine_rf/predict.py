from sklearn.ensemble import RandomForestRegressor
import pandas as pd
import pickle
import sys
import numpy as np

# 获取命令行参数
batch_size = int(sys.argv[1])
snap_vertex_num = int(sys.argv[2])
snap_edge_num = int(sys.argv[3])
batch_add_rate = float(sys.argv[4])
degree_avg = int(sys.argv[5])

# 加载缩放参数
params = np.load('./tools/mechine_rf/scaling_params.npz')
mean_loaded = params['mean']
stddev_loaded = params['stddev']

# 加载缩放器对象
with open('./tools/mechine_rf/scaler.pkl', 'rb') as f:
    scaler_loaded = pickle.load(f)

scaler_loaded.mean_ = mean_loaded
scaler_loaded.scale_ = stddev_loaded

# 加载模型
with open('./tools/mechine_rf/rf_model.pkl', 'rb') as f:
  rf_model = pickle.load(f)

# 预测迭代次数
test_data = [[batch_size, snap_vertex_num, snap_edge_num, batch_add_rate, degree_avg]]
test_data = scaler_loaded.transform(test_data)

predicted_graphbolt_iter = rf_model.predict(test_data)

print(int(predicted_graphbolt_iter))