import os
import numpy as np
import pandas as pd
import sqlite3

# Load Metrics .csv file
metrics_directory = os.path.join('..', 'resources')
metrics_dataframe = pd.read_csv(os.path.join(metrics_directory, 'metrics(rendered_images).csv'))

hdr_file_names = [ "blue_photo_studio.hdr", "dancing_hall.hdr", "office.hdr", "pine_attic.hdr", "studio_small_03.hdr", "thatch_chapel.hdr"]

# Sorted by MSE
print('--------------------------------------------')
print (metrics_dataframe.sort_values(['MeanSquaredError']).head())
print (metrics_dataframe.sort_values(['MeanSquaredError']).tail())
print()

# Connect database file
con = sqlite3.connect('./MSEs.db')

metrics_dataframe.to_sql('MSE', con=con, if_exists='replace', index=False)

cur = con.cursor()
cur.execute('select Model, round(avg(MeanSquaredError), 2) as average_mse from MSE group by Model')

print('--------Model--------')
for row in cur:
    print(row)
print()

cur.execute('select Environment, round(avg(MeanSquaredError), 2) as average_mse from MSE group by Environment')

print('--------Environment--------')
for row in cur:
    print(row)
print()

con.close()