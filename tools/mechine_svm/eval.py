# 导入必要的库
import pandas as pd
import numpy as np
from sklearn.svm import SVR
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_squared_error, r2_score
from sklearn.preprocessing import MinMaxScaler

csv_path = '/home/wangcx/tmp/result_end_2.csv'
# 读取数据
data = pd.read_csv(csv_path)

# 提取特征和目标变量
features = data.drop('graphbolt_iter', axis=1)
target = data['graphbolt_iter']

scaler = MinMaxScaler()
features = scaler.fit_transform(features)

# 将数据集划分为训练集和测试集
# X_train, X_test, y_train, y_test = train_test_split(features, target, test_size=0.2, random_state=143)
X_train, X_test, y_train, y_test = train_test_split(features, target, test_size=0.2)

# 创建SVM回归模型
svr_model = SVR(kernel='rbf', gamma=0.0001)
# svr_model = SVR(kernel='sigmoid', coef0=1)
# svr_model = SVR(kernel='poly', degree=3, coef0=1)


# 训练模型
svr_model.fit(X_train, y_train)

# 对测试集进行预测
y_pred = svr_model.predict(X_test)

# 计算均方根误差（RMSE）和确定系数（R2）
rmse = np.sqrt(mean_squared_error(y_test, y_pred))
r2 = r2_score(y_test, y_pred)

print("均方根误差：", rmse)
print("确定系数：", r2)
