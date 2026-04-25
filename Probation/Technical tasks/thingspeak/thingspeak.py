import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from sklearn.linear_model import LinearRegression

# Step 1: Load data
url = "https://api.thingspeak.com/channels/3336262/feeds.csv?api_key=0OHHRI9KKOEN31VH"
import ssl
ssl._create_default_https_context = ssl._create_unverified_context
data = pd.read_csv(url)

# Step 2: Clean data
data = data[['field2', 'field3']]
data = data.dropna()

humidity = data['field2'].astype(float)
moisture = data['field3'].astype(float)

# Step 3: Convert to arrays
X = np.array(humidity).reshape(-1, 1)
y = np.array(moisture)

# Step 4: Train model
model = LinearRegression()
model.fit(X, y)
pred = model.predict(X)

# Step 5: Plot
plt.scatter(X, y, label="Data Points")
plt.plot(X, pred, label="Regression Line")

plt.xlabel("Humidity (%)")
plt.ylabel("Soil Moisture")
plt.title("Humidity vs Soil Moisture")

plt.legend()
plt.grid(True)

plt.show()

