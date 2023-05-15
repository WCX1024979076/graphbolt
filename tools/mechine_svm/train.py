# 导入必要的库
import numpy as np
from sklearn.svm import SVR
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_squared_error

csv_path = '/home/wangcx/tmp/result.csv'
# 加载数据
data = np.loadtxt(csv_path, delimiter=',')

# 将特征和目标变量分离开来
X = data[:, :-1]
y = data[:, -1]

# 将数据集划分为训练集和测试集
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# 创建SVM回归模型
svr_model = SVR(kernel='rbf')

# 训练模型
svr_model.fit(X_train, y_train)

# 对测试集进行预测
y_pred = svr_model.predict(X_test)

# 计算均方根误差（RMSE）
rmse = np.sqrt(mean_squared_error(y_test, y_pred))

print("均方根误差：", rmse)
