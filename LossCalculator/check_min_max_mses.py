import os
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import pandas as pd

# Find model names
model_names = []
dataset_directory = os.path.join("..", "resources", "IBL_rendered_examples")    
for root, dirs, files in os.walk(dataset_directory):
    glb_directories = [d for d in dirs if d.endswith('glb')]
    
    for glb_dir in glb_directories:
        model_names.append(glb_dir)

# Load Metrics .csv file
metrics_directory = os.path.join('..', 'resources')
metrics_dataframe = pd.read_csv(os.path.join(metrics_directory, 'metrics(rendered_images).csv'))

hdr_file_names = [ "blue_photo_studio.hdr", "dancing_hall.hdr", "office.hdr", "pine_attic.hdr", "studio_small_03.hdr", "thatch_chapel.hdr"]

# Sorted by MSE
print('--------------------------------------------')
print (metrics_dataframe.sort_values(['MSE']).head())
print (metrics_dataframe.sort_values(['MSE']).tail())
print()


"""
print('--------------------------------------------')
print (metrics_dataframe.sort_values(['PSNR'], ascending=False).head())
print (metrics_dataframe.sort_values(['PSNR'], ascending=False).tail())
print()

print('--------------------------------------------')
print (metrics_dataframe.sort_values(['SSIM']).head())
print (metrics_dataframe.sort_values(['SSIM']).tail())
"""