import pickle

# 训练和拟合随机森林模型
model = RandomForestRegressor(n_estimators=100, max_depth=10)
model.fit(X_train, y_train)

# 保存模型
with open('model.pkl', 'wb') as f:
    pickle.dump(model, f)
