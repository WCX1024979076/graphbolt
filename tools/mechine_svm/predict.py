from sklearn.ensemble import RandomForestRegressor
import pandas as pd
import pickle

# 加载模型
with open('rf_model.pkl', 'rb') as f:
  rf_model = pickle.load(f)

# 预测迭代次数
test_data = [[3000, 4, 2, 2002, 3, 4, 5, 8, 2000, 2, 1, 0, 6]]
predicted_price = rf_model.predict(test_data)

print(predicted_price)