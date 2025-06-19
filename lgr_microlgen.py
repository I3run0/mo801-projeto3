from sklearn.datasets import load_diabetes
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LinearRegression

X, y = load_diabetes(return_X_y=True)
X = X[:, [2]]  # Use only one feature
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=20, shuffle=False)

# Usando scikit-learn LinearRegression já importado em outra célula

# Cria e treina o modelo de regressão linear
regressor = LinearRegression().fit(X_train, y_train)

# Faz predições no conjunto de teste
y_pred = regressor.predict(X_test)

# Exibe o score (R²) no conjunto de teste
print("Score no teste (R²):", regressor.score(X_test, y_test))

# Exibe a primeira predição
print("Primeira predição:", y_pred[0])

from micromlgen import port

# Exporta o modelo LinearRegression treinado para código C (TinyML)
with open('diabetes_regressor.h', 'w') as f:
  f.write(port(regressor, classname="diabetes_regressor_predict"))
print("Modelo exportado para diabetes_regressor.h")

# Exemplo de código C para executar o modelo exportado (coloque no seu projeto C/C++)
c_code = """
#include "diabetes_regressor.h"
#include <stdio.h>

int main() {
  // Exemplo de entrada (substitua pelo seu valor)
  float input[1] = {0.03}; // valor da feature
  printf("Predição: %f\\n", predict(input));
  return 0;
}
"""
with open('example_tinyml.c', 'w') as f:
  f.write(c_code)
print("Exemplo de código C salvo em example_tinyml.c")
