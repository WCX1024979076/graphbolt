from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_squared_error, r2_score
import pandas as pd
from sklearn.model_selection import train_test_split

# 读取数据
data = pd.read_csv('house_prices.csv')

# 提取特征和目标变量
features = data.drop('SalePrice', axis=1)
target = data['SalePrice']

# 将数据集分为训练集和测试集
X_train, X_test, y_train, y_test = train_test_split(features, target, test_size=0.2, random_state=42)

# 创建随机森林模型
rf_model = RandomForestRegressor(n_estimators=100, max_depth=10, random_state=42)

# 拟合模型
rf_model.fit(X_train, y_train)

# 预测房屋销售价格
predicted_prices = rf_model.predict(X_test)

# 计算MSE和R²指标
mse = mean_squared_error(y_test, predicted_prices)
r2 = r2_score(y_test, predicted_prices)

print("均方误差(MSE)：", mse)
print("决定系数(R²)：", r2)
