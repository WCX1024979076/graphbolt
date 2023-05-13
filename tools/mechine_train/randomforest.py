from sklearn.ensemble import RandomForestRegressor
import pandas as pd

# 读取数据
data = pd.read_csv('house_prices.csv')

# 提取特征和目标变量
features = data.drop('SalePrice', axis=1)
target = data['SalePrice']

# 创建随机森林模型
rf_model = RandomForestRegressor(n_estimators=100, max_depth=10, random_state=42)

# 拟合模型
rf_model.fit(features, target)

# 预测房屋销售价格
test_data = [[3000, 4, 2, 2002, 3, 4, 5, 8, 2000, 2, 1, 0, 6]]
predicted_price = rf_model.predict(test_data)

print(predicted_price)
