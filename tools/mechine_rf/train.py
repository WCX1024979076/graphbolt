from sklearn.ensemble import RandomForestRegressor
import pandas as pd
import pickle
from sklearn.preprocessing import StandardScaler
import numpy as np

csv_path = '/home/wangcx/tmp/result_end_4.csv'
# 读取数据
data = pd.read_csv(csv_path)

# 提取特征和目标变量
features = data.drop('graphbolt_iter', axis=1)
features = features.drop('tegra_iter', axis=1)
target = data['graphbolt_iter']
target1 = data['tegra_iter']

# scaler = MinMaxScaler()
scaler = StandardScaler()
features = scaler.fit_transform(features)

# 使用 pickle 序列化缩放器对象并保存到文件中
with open('scaler.pkl', 'wb') as f:
    pickle.dump(scaler, f)

# 保存均值和标准差
mean = scaler.mean_
stddev = scaler.scale_

# 将均值和标准差保存到文件中
np.savez("scaling_params.npz", mean=mean, stddev=stddev)

# 创建随机森林模型
rf_model = RandomForestRegressor(n_estimators=100, max_depth=10)
rf_model1 = RandomForestRegressor(n_estimators=100, max_depth=10)

# 拟合模型
rf_model.fit(features, target)
rf_model1.fit(features, target1)

# 保存模型
with open('rf_model.pkl', 'wb') as f:
  pickle.dump(rf_model, f)

# 保存模型
with open('rf_model1.pkl', 'wb') as f:
  pickle.dump(rf_model1, f)