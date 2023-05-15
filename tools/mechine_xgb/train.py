import xgboost as xgb
from sklearn.metrics import mean_squared_error
import pandas as pd

csv_path = '/home/wangcx/tmp/result.csv'
# 导入数据集
data = pd.read_csv(csv_path)
X, y = data.iloc[:,:-1],data.iloc[:,-1]

# 将数据集拆分为训练集和测试集
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=123)

# 定义XGBoost回归模型
xgb_reg = xgb.XGBRegressor(objective='reg:squarederror', colsample_bytree = 0.3, learning_rate = 0.1,
                max_depth = 5, alpha = 10, n_estimators = 100)

# 训练模型
xgb_reg.fit(X_train,y_train)

# 预测测试集结果
y_pred = xgb_reg.predict(X_test)

# 评估模型
mse = mean_squared_error(y_test, y_pred)
rmse = np.sqrt(mse)
print("RMSE:", rmse)
