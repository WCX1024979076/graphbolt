from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_squared_error, r2_score
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
from sklearn.preprocessing import StandardScaler

csv_path = '/home/wangcx/tmp/result_end_2.csv'
# 读取数据
data = pd.read_csv(csv_path)

# 提取特征和目标变量
features = data.drop('graphbolt_iter', axis=1)
target = data['graphbolt_iter']

# scaler = MinMaxScaler()
scaler = StandardScaler()
features = scaler.fit_transform(features)

# 将数据集分为训练集和测试集
X_train, X_test, y_train, y_test = train_test_split(features, target, test_size=0.2, random_state=157)

# 创建随机森林模型
rf_model = RandomForestRegressor(n_estimators=100, max_depth=10)

# 拟合模型
rf_model.fit(X_train, y_train)

# 预测迭代次数
predicted_graphbolt_iter = rf_model.predict(X_test)

# 计算MSE和R²指标
mse = mean_squared_error(y_test, predicted_graphbolt_iter)
r2 = r2_score(y_test, predicted_graphbolt_iter)

print("均方误差(MSE)：", mse)
print("决定系数(R²)：", r2)

for i in y_test:
  print("%02d" % i ,end = " ")

print()
for i in predicted_graphbolt_iter :
  if i <= 0 :
    print("%02d" % i, end = " ")
  else :
    print("%02d" % int(i) , end = " ")
