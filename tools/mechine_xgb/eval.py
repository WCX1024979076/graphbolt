import xgboost as xgb
import pandas as pd
from sklearn.metrics import mean_squared_error
from sklearn.model_selection import train_test_split
import numpy as np

# TODO 归一化 特征变化较大

csv_path = '/home/wangcx/tmp/result.csv'
# 读取数据
data = pd.read_csv(csv_path)

# 提取特征和目标变量
features = data.drop('graphbolt_iter', axis=1)
target = data['graphbolt_iter']

# 准备数据
X_train, X_test, y_train, y_test = train_test_split(features, target, test_size=0.2)  # 划分训练集和测试集

# 训练模型
xg_reg = xgb.XGBRegressor(objective ='reg:squarederror', 
                          colsample_bytree = 0.3, 
                          learning_rate = 0.1,
                          max_depth = 5, 
                          alpha = 10, 
                          n_estimators = 100)
xg_reg.fit(X_train,y_train)

# 预测结果并计算误差
preds = xg_reg.predict(X_test)
rmse = np.sqrt(mean_squared_error(y_test, preds))
print("RMSE: %f" % (rmse))