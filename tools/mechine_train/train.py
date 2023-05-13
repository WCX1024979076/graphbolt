from sklearn.ensemble import RandomForestRegressor
import pandas as pd
import pickle

csv_path = '/home/wangcx/tmp/result.csv'
# 读取数据
data = pd.read_csv(csv_path)

# 提取特征和目标变量
features = data.drop('graphbolt_iter', axis=1)
target = data['graphbolt_iter']

# 创建随机森林模型
rf_model = RandomForestRegressor(n_estimators=100, max_depth=10, random_state=42)

# 拟合模型
rf_model.fit(features, target)

# 保存模型
with open('rf_model.pkl', 'wb') as f:
  pickle.dump(rf_model, f)
