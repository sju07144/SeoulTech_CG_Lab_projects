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
metrics_dataframe = pd.read_csv(os.path.join(metrics_directory, 'metrics.csv'))

hdr_file_names = [ "blue_photo_studio.hdr", "dancing_hall.hdr", "office.hdr", "pine_attic.hdr", "studio_small_03.hdr", "thatch_chapel.hdr"]

# Sorted by MSE
print('--------------------------------------------')
print (metrics_dataframe.sort_values(['MSE']).head())
print (metrics_dataframe.sort_values(['MSE']).tail())
print()

mse_dfs_by_hdr = {}
for hdr_index in range(0, 6):
    mse_dfs_by_hdr[hdr_file_names[hdr_index]] = metrics_dataframe[metrics_dataframe['HDR name'] == hdr_file_names[hdr_index]]  


for hdr_name in hdr_file_names:
    fig, axes = plt.subplots(2, 3, figsize=(15, 10), subplot_kw=dict(projection='3d'))
    fig.subplots_adjust(hspace=0.5, wspace=0.3)

    indexX, indexY = 0, 0
    
    for model_name in model_names:
        current_dataframe = mse_dfs_by_hdr[hdr_name]
        mse_by_model = current_dataframe[current_dataframe['Model name'] == model_name]

        thetas = mse_by_model['Theta']
        phis = mse_by_model['Phi']
        mses = mse_by_model['MSE']
        axes[indexX][indexY].view_init(elev=30., azim=120) 
        axes[indexX][indexY].scatter(thetas, phis, mses)
        axes[indexX][indexY].set_xlabel('Theta')
        axes[indexX][indexY].set_ylabel('Phi')
        axes[indexX][indexY].set_zlabel('MSE')
        axes[indexX][indexY].set_title(model_name)
        
        indexY += 1
        if indexY == 3:
            indexX += 1
            indexY = 0
            if indexX == 2:
                indexX = 0
    
    plt.suptitle(hdr_name)
    plt.savefig(os.path.join(metrics_directory, 'statistics', hdr_name.replace('.hdr', '.png')))
    
print('Complete(by)')
    
fig = plt.figure()
ax = fig.gca(projection='3d')

thetas = metrics_dataframe['Theta']
phis = metrics_dataframe['Phi']
mses = metrics_dataframe['MSE']

ax.view_init(elev=0., azim=0) 
ax.scatter(thetas, phis, mses)
ax.set_xlabel('Theta')
ax.set_ylabel('Phi')
ax.set_zlabel('MSE')
ax.set_title("Dataset")

plt.savefig(os.path.join(metrics_directory, 'statistics', 'dataset(phi).png'))
    
print('Complete(all)')
        




"""
print('--------------------------------------------')
print (metrics_dataframe.sort_values(['PSNR'], ascending=False).head())
print (metrics_dataframe.sort_values(['PSNR'], ascending=False).tail())
print()

print('--------------------------------------------')
print (metrics_dataframe.sort_values(['SSIM']).head())
print (metrics_dataframe.sort_values(['SSIM']).tail())
"""