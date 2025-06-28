import numpy as np
from sklearn.datasets import load_digits
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
from micromlgen import port

def main():
    # 1. Load digits dataset (1797 samples, 64 features) :contentReference[oaicite:1]{index=1}
    X, y = load_digits(return_X_y=True)

    # 2. Split data into train/test sets
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.25, random_state=42
    )

    # 3. Train a multiclass logistic regression model
    clf = LogisticRegression(
        multi_class="multinomial",
        solver="lbfgs",
        max_iter=500
    )
    clf.fit(X_train, y_train)

    # 4. Evaluate model accuracy
    accuracy = accuracy_score(y_test, clf.predict(X_test))
    print(f"Test accuracy: {accuracy:.3f}")

    # 5. Export model to C/C++ code
    code = port(clf, classmap={i: str(i) for i in range(10)})
    with open("digits_lr_model.cpp", "w") as f:
        f.write(code)
    print("✅ Generated digits_lr_model.cpp with predict() function")

if __name__ == "__main__":
    main()

