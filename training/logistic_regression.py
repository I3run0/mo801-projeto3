# train_export_iris_lr.py
import numpy as np
from sklearn.datasets import load_iris
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
from micromlgen import port

def main():
    # 1. Load Iris dataset
    iris = load_iris()
    X = iris.data            # 150 samples × 4 features
    y = iris.target

    # Convert to binary: Setosa (0) vs non‑Setosa (1)
    y_bin = (y != 0).astype(int)

    # 2. Split train/test
    X_train, X_test, y_train, y_test = train_test_split(
        X, y_bin, test_size=0.3, random_state=42
    )

    # 3. Train logistic regression
    clf = LogisticRegression(max_iter=500, solver="liblinear")
    clf.fit(X_train, y_train)

    # 4. Evaluate
    y_pred = clf.predict(X_test)
    print(f"Test accuracy: {accuracy_score(y_test, y_pred):.3f}")

    # 5. Export to C
    c_code = port(clf, classname="IrisLR"))
    with open("iris_lr_model.h", "w") as f:
        f.write(c_code)
    print("➡️  Generated iris_lr_model.c with predict_IrisLR()")

if __name__ == "__main__":
    main()

